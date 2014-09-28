/*
 *  Copyright 2010 William Tisäter. All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *    1.  Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *    2.  Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *    3.  The name of the copyright holder may not be used to endorse or promote
 *        products derived from this software without specific prior written
 *        permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL WILLIAM TISÄTER BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <errno.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "main.h"
#include "daemon.h"
#include "utility.h"

#include "avahi.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

using namespace std;

void Daemon::create(bool _back, const string &_user, const string &_group)
{
	uid_t uid;
	gid_t gid;
	ofstream out;
	string piddir;
	unsigned int pid;
	string::size_type pos;

	// Obtain new process group
	setsid();
	
	// Translate user from configuration to uid
	if ((uid = get_uid_from_str(_user)) == (uid_t) -1)
	{
		cout << "Warning! Cannot get uid for username " << _user << ", will run as current user." << endl;
		uid = get_current_uid();
	}
	
	// Translate group from configuration to gid
	if ((gid = get_gid_from_str(_group)) == (gid_t) -1)
	{
		cout << "Warning! Cannot get gid for group " << _group << ", will run as current group." << endl;
		gid = get_current_gid();
	}

#ifdef USE_MEM_KVM
	gid = get_gid_from_str("kmem");
#endif
	
	// Create cache directory if it does not exist
	if (check_dir_exist(cachedir) == 0)
	{
		create_directory(cachedir, 0755);
		chown(cachedir.c_str(), uid, 0);
	}
	
	// Create pid directory if it does not exist
	pos = pidfile.find_last_of("/");
	if (pos != string::npos)
	{
		piddir = pidfile.substr(0, pidfile.find_last_of("/"));
	
		if (check_dir_exist(piddir) == 0 && pidfile.length())
		{
			create_directory(piddir, 0755);
			chown(piddir.c_str(), uid, 0);
		}
	}
	
	// Only change owner if we want to change the pidfile
	if (pidfile.length())
	{
		out.open(pidfile.c_str());
		
		if (!out)
		{
			cout << "Could not create pid file " << pidfile << ": " << strerror(errno) << endl;
			exit(1);
		}
			
		chmod(pidfile.c_str(), 0644);
		chown(pidfile.c_str(), uid, gid);
	}
	
	// Drop root privileges now, if we were run that way
	if (geteuid() == 0)
	{
		if (uid || gid)
		{
			if (setgid(gid) != 0)
				cout << "Could not switch to group " << _group << ": " <<
						strerror(errno) << endl;
			if (setuid(uid) != 0)
				cout << "Could not switch to user " << _user << ": " <<
						strerror(errno) << endl;
		}
		else
		{
			cout << "WARNING: istatd set to run as root in istat.conf!  "
					"Not recommended." << endl;
		}
	}
	else {
		cout << "Ignoring server_{user,group} settings, wasn't run as root." << endl;
	}
	
	if (_back)
	{
		if ((pid = fork()) > 0)
		{
			// cout << "Entering standalone mode with pid " << pid << endl;
			exit(0);
		}
		
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		
		sleep(1);
	}
	
	// Only write pid if we have the file specified in conf or as argument
	if (pidfile.length())
	{
		out << getpid() << '\n'; 
		out.close();
	}
	
	// Create UNIX socket
	int unix_socket;
	socklen_t length;
	struct sockaddr_un local;
	
	if ((unix_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		cout << "Could not create UNIX socket file " << sockfile << ": " << strerror(errno) << endl;
		exit(1);
	}
	
	local.sun_family = AF_UNIX;
	
	strcpy(local.sun_path, sockfile.c_str());
	unlink(local.sun_path);
	
	length = offsetof(struct sockaddr_un, sun_path) + strlen(sockfile.c_str());
	
	if (bind(unix_socket, (struct sockaddr *) &local, length) == -1)
	{
		cout << "Could not bind UNIX socket: " << strerror(errno) << endl;
		exit(1);
	}
	
	if (listen(unix_socket, 5) == -1)
	{
		cout << "Could not listen to UNIX socket: " << strerror(errno) << endl;
		exit(1);
	}
	
}

void Daemon::destroy()
{
        int ret;
        ofstream out;

	ret = unlink(pidfile.c_str());

        if (ret == -1)
        {
            // Empty pid file if we can't remove it
            out.open(pidfile.c_str());

            if (out.good())
            {
                out << "";
                out.close();
            }
        }

	unlink(sockfile.c_str());
		
	avahi_stop();

	exit(0);
}

void SignalResponder::destroy()
{
	// cout << endl << "Shutting down and saving clients." << endl;
	
	this->sockets->close();
	this->listener->close();
	this->clients->save_cache();
	this->unixdaemon->destroy();
}

void SignalResponder::on_sigint()
{
	this->destroy();
}

void SignalResponder::on_sigterm()
{
	this->destroy();
}

void SignalResponder::on_sighup()
{
	cout << "Placeholder for reloading config." << endl;
}
