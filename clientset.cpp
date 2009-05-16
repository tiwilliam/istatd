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

#include <vector>
#include <sstream>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <sys/stat.h>

#include "utility.h"
#include "clientset.h"
#include "socketset.h"

using namespace std;

void ClientSet::operator += (Client & _client)
{
    for (std::vector<Client>::iterator search = clients.begin(); search != clients.end(); ++search)
    {
        if (search->duuid == _client.duuid)
        {
            search->name = _client.name;
            
            return;
        }
    }
    
    clients.push_back(_client);
    
    cout << "(" << _client.name << ":" << _client.duuid << ") Client added to client set." << endl;
}

Client ClientSet::get_client(int _socket)
{
    vector<Client>::iterator search;
    
    for (search = clients.begin(); search != clients.end(); ++search)
    {
        if (search->socket == _socket)
        {
            return (*search);
        }
    }
    
    return (*search);
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

void ClientSet::update_sessions(const std::string & _duuid, int _socket, SocketSet * _sockets)
{
    for (std::vector<Client>::iterator eraser = clients.begin(); eraser != clients.end(); ++eraser)
    {
        if (eraser->duuid == _duuid)
        {
            eraser->socket = _socket;
            break;
        }
    }
}

int ClientSet::length()
{
    return clients.size();
}

void ClientSet::save_cache()
{
    stringstream path;
    string cache_file = "clients.dat";
    string cache_directory = "/var/cache/istat";
    
    if (!check_dir_exist(cache_directory))
    {
        if (mkdir(cache_directory.c_str(), 0755) != 0)
        {
            cout << "Could not create cache directory '" << cache_directory << "': " << strerror(errno) << endl;
            return;
        }
    }
    
    path << cache_directory << "/" << cache_file;
    
    ofstream out(path.str().c_str());
    
    if (!out)
    {
        cout << "Could not create cache file '" << path.str() << "': " << strerror(errno) << endl;
        return;
    }
    
    for (std::vector<Client>::iterator client = clients.begin(); client != clients.end(); ++client)
    {
        out << client->name << ":" << client->duuid << ":" << client->ready << endl;
    }
    
    out.close();
}

void ClientSet::read_cache()
{
    string line;
    Client client;
    stringstream path;
    vector<string> array;
    string cache_file = "clients.dat";
    string cache_directory = "/var/cache/istat";
    
    path << cache_directory << "/" << cache_file;
    
    ifstream cache(path.str().c_str());
    
    if (cache.good())
    {
        while (getline(cache, line))
        {
            if (line.length())
            {
                array = explode(line, ":");
            
                client.socket = 0;
                client.name = array.at(0);
                client.duuid = array.at(1);
                client.ready = to_int(array.at(2));
            
                cout << "Loading clients from file cache." << endl;
            
                if (clients.size())
                {
                    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
                    {
                        if (it->duuid != client.duuid) *this += client;
                    }
                }
                else
                {
                    *this += client;
                }
            }
        }
    }
    else
    {
        cout << "Could not read cache file '" << path.str() << "': " << strerror(errno) << endl;
    }

    cache.close();
}
