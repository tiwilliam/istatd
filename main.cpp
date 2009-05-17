/*
 *  Copyright 2008 William Tisäter. All rights reserved.
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
 
#include <signal.h>
#include <iostream>

#include "main.h"
#include "conf.h"
#include "stats.h"
#include "system.h"
#include "config.h"
#include "daemon.h"
#include "socket.h"
#include "utility.h"
#include "argument.h"
#include "socketset.h"
#include "clientset.h"
#include "switchboard.h"

using namespace std;

SignalResponder * pn_signalresponder = NULL;

int main(int argc, char ** argv)
{
    string data;
    unsigned int i;
    stringstream buf;
    
    Stats stats;
    ClientSet clients;
    SocketSet sockets;
    Switchboard switchboard;
    ArgumentSet arguments(argc, argv);

    if (arguments.isset("version") || arguments.isset("v"))
    {
        cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
        return 0;
    }
    
    if (arguments.isset("help") || arguments.isset("h"))
    {
        cout << "usage: istat [-a HOST] [-p PORT]" << endl;
        cout << endl;
        cout << "    -d          run in background" << endl;
        cout << "    -c          custom config file location" << endl;
        cout << "    -a          listen on this address" << endl;
        cout << "    -p          listen on this port" << endl;
        cout << "    -u          change running user" << endl;
        cout << "    -h          print this help text" << endl;
        cout << "    -v          print version number" << endl;
        cout << endl;
        cout << "    --pid       custom pid file location" << endl;
        cout << "    --code      custom lock code" << endl;
        cout << endl;
        
        return 0;
    }
    
    // Load and parse configuration
    Config config(arguments.get("c", CONFIG_FILE_PATH));
    
    config.parse();
    config.validate();
    
    // Load configuration properties from command line and config file
    bool prop_back        = arguments.isset("d");
    string prop_host      = arguments.get("a", config.get("network_addr", "0.0.0.0"));
    string prop_port      = arguments.get("p", config.get("network_port", "5109"));
    string prop_user      = arguments.get("u", config.get("server_user", "istat"));
    string prop_pid       = arguments.get("pid", config.get("server_pid", "/var/run/istat/istat.pid"));
    string prop_code      = arguments.get("code", config.get("server_code", "00000"));
    string prop_sock      = arguments.get("socket", config.get("server_socket", "/tmp/istat.sock"));
    string prop_cache     = config.get("cache_dir", "/var/cache/istat");
   
	if(-1 == sys_init())
	{
		return 1;
	}
    Daemon unixdaemon(prop_pid, prop_sock);
    Socket listener(prop_host, to_int(prop_port));
    SignalResponder signalresponder(&sockets, &listener, &unixdaemon, &clients);
    
    ::pn_signalresponder = &signalresponder;

    // Get old sessions from disk cache
    clients.read_cache(prop_cache);
    
    // Clear cache of saved sessions
    if (arguments.isset("clear-sessions"))
    {
        clients.clear_cache();
        return 0;
    }

    // Create socket, pid file and put in background if desired
    unixdaemon.create(prop_back, prop_user);

    signal(SIGHUP,  handler);
    signal(SIGUSR1, handler);
    signal(SIGINT,  handler);
    signal(SIGTERM, handler);
    signal(SIGPIPE, handler);

    if (!listener.listen()) return 1;
    
    sockets += listener;
    
    // Add disks for monitoring
    if (config.get("monitor_disk") != "")
    {
        // No array found. Add value given.
        stats.add_disk(config.get("monitor_disk").c_str());
    }
    else
    {
        // Array found. Add all values in the array.
        for (i = 0; i < config.get_property("monitor_disk").get_array_size(); i++)
        {
            stats.add_disk(config.get_property("monitor_disk").get_array(i).c_str());
        }
    }
    
    // Add network interfaces for monitoring
    if (config.get("monitor_net") != "")
    {
        // No array found. Add value given.
        stats.add_net(config.get("monitor_net").c_str());
    }
    else
    {
        // Array found. Add all values in the array.
        for (i = 0; i < config.get_property("monitor_net").get_array_size(); i++)
        {
            stats.add_net(config.get_property("monitor_net").get_array(i).c_str());
        }
    }
    
    while (1)
    {
        stats.update_system_stats();
        
        if (sockets.get_status(1))
        {
            if (sockets == listener)
            {
                Socket new_socket(listener.accept());
                
                sockets += new_socket;
            }
            else
            {
                Socket active_socket(sockets.get_ready());
        
                if (active_socket.receive(data, 1024))
                {
                    switchboard.parse(&sockets, &clients, &active_socket, &stats, &arguments, data, prop_code);
                }
                else
                {
                    sockets -= active_socket;
                }
            }
        }
    }
    
    ::pn_signalresponder = NULL;
    
    return 0;
}

void handler(int _signal)
{
    if (pn_signalresponder)
    {
        switch (_signal)
        {
            case SIGINT:
                pn_signalresponder->on_sigint();
                return;
            
            case SIGTERM:
                pn_signalresponder->on_sigterm();
                return;
        }
    }
}
