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

#include <vector>
#include <sstream>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "utility.h"
#include "clientset.h"
#include "socketset.h"

using namespace std;

void ClientSet::operator += (Client & _client)
{
	clients.push_back(_client);
}

Client *ClientSet::get_client(int _socket)
{
	vector<Client>::iterator search;
	
	for (search = clients.begin(); search != clients.end(); ++search)
	{
		if (search->socket == _socket)
		{
			return &(*search);
		}
	}
	
	return &(*search);
}

void ClientSet::authenticate(int _socket)
{
	for (std::vector<Client>::iterator search = clients.begin(); search != clients.end(); ++search)
	{
		if (search->socket == _socket)
		{
			search->ready = 1;
		}
	}
}

int ClientSet::is_authenticated(const string & _duuid)
{
	for (std::vector<Client>::iterator search = clients.begin(); search != clients.end(); ++search)
	{
		if (search->duuid == _duuid && search->ready)
		{
			return 1;
		}
	}
	
	return 0;
}

void ClientSet::init_session(const string & _duuid, int _socket, const string & _name)
{
	bool found_entry = false;
	
	for (std::vector<Client>::iterator eraser = clients.begin(); eraser != clients.end(); ++eraser)
	{
		if (eraser->duuid == _duuid)
		{
			eraser->name = _name;
			eraser->socket = _socket;
			
			found_entry = true;
			
			break;
		}
	}
	
	if (!found_entry)
	{
		Client new_client;
		
		new_client.ready = 0;
		new_client.name = _name;
		new_client.duuid = _duuid;
		new_client.socket = _socket;
		new_client.sid_disk = 0;
		new_client.sid_temp = 0;
		new_client.sid_fans = 0;
		
		*this += new_client;
	}
}

int ClientSet::length(void)
{
	return clients.size();
}

void ClientSet::clear_cache(void)
{
	if (this->cache_dir.length())
	{
		stringstream path;
		string cache_file = "clients.dat";
		path << this->cache_dir << "/" << cache_file;

		if (check_file_exist(path.str()))
		{
			if (unlink(path.str().c_str()) == 0)
			{
				cout << "Successfully cleared all sessions." << endl;
			}
			else
			{
				cout << "Could not clear sessions in '" << path.str() << "': " << strerror(errno) << endl;
			}
		}
	}
}

void ClientSet::save_cache(void)
{
	if (this->cache_dir.length())
	{
		stringstream path;
		string cache_file = "clients.dat";
		path << this->cache_dir << "/" << cache_file;
		
		ofstream out(path.str().c_str());
		chmod(path.str().c_str(), 0600);
		
		if (!out)
		{
			cout << "Could not create file '" << path.str() << "': " << strerror(errno) << endl;
			return;
		}
		for (std::vector<Client>::iterator client = clients.begin(); client != clients.end(); ++client)
		{
			out << client->name << ":" << client->duuid << ":" << client->ready << ":" << client->sid_disk << ":" << client->sid_temp << ":" << client->sid_fans << endl;
		}
		
		out.close();
	}
}

void ClientSet::read_cache(const std::string & _cache_dir)
{
	string line;
	Client client;
	stringstream path;
	vector<string> array;
	bool client_cached = false;
	string cache_file = "clients.dat";
	
	this->cache_dir = _cache_dir;
	path << this->cache_dir << "/" << cache_file;
	
	ifstream cache(path.str().c_str());
	
	if (cache.good())
	{
		while (getline(cache, line))
		{
			if (line.length())
			{
				array = explode(line, ":");
				
				if (array.size() < 5) continue;
				
				client.socket = 0;
				client.name = array[0];
				client.duuid = array[1];
				client.ready = to_int(array[2]);
				client.sid_disk = to_int(array[3]);
				client.sid_temp = to_int(array[4]);
				client.sid_fans = to_int(array[5]);
				
				if (clients.size())
				{
					for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
					{
						if (it->duuid == client.duuid)
							client_cached = true;
					}
				}
				
				if (!client_cached)
					*this += client;
			}
		}
	}
	else
	{
		// Ignore no such file errors, we will create the file upon save
		if (errno != ENOENT)
			cout << "Could not read cache file '" << path.str() << "': " << strerror(errno) << endl;
	}

	cache.close();
}
