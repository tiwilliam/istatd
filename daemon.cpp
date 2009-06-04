/*
 *  Copyright 2009 William Tisäter. All rights reserved.
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
#include <sys/un.h>
#include <string.h>
#include <sys/socket.h>

#include "main.h"
#include "daemon.h"
#include "utility.h"

using namespace std;

void Daemon::create(bool _back, const string & _user)
{
	int pid, uid;

	// Obtain new process group
	setsid();
	
	// Switch user
	if ((uid = get_id_from_name(_user)) >= 0)
	{
		if (setuid(uid) != 0)
		{
			cout << "Could not switch user: " << strerror(errno) << endl;
		}
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

	// Write pid file
	ofstream out(pidfile.c_str());
	
	if (!out)
	{
		cout << "Could not create pid file " << pidfile << ": " << strerror(errno) << endl;
		exit(1);
	}
	
	out << getpid();
	out.close();

	// Create UNIX socket
	int unix_socket;
	socklen_t length;
	struct sockaddr_un local;
	
	if ((unix_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		cout << "Could not create UNIX socket: " << strerror(errno) << endl;
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
	unlink(pidfile.c_str());
	unlink(sockfile.c_str());
	
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
