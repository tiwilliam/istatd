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
#include <errno.h>
#include <string.h>
#include <iostream>

#include "socketset.h"

using namespace std;

SocketSet::SocketSet()
{
    highest = 0;
    
    FD_ZERO(&socketset);
}

void SocketSet::operator += (Socket & _socket)
{
    if (_socket.get_id() > highest)
        highest = _socket.get_id();
    
    connections.push_back(_socket);
    
    FD_SET(_socket.get_id(), &socketset);
    
    cout << "(" << _socket.get_address() << ":" << _socket.get_port() << ") Socket added to socket set." << endl;
}

void SocketSet::operator -= (Socket & _socket)
{
    cout << "(" << _socket.get_address() << ":" << _socket.get_port() << ") Disconnected." << endl;
    
    for (std::vector<Socket>::iterator eraser = connections.begin(); eraser != connections.end(); ++eraser)
    {
        if ((*eraser).get_id() == _socket.get_id())
        {
            cout << "(" << _socket.get_address() << ":" << _socket.get_port() << ") Socket deleted from socket set." << endl;
            
            connections.erase(eraser);
            
            break;
        }
    }
    
    if (_socket.get_id() == highest)
    {
        highest = 0;
        
        for (std::vector<Socket>::iterator higher = connections.begin(); higher != connections.end(); ++higher)
        {
            if (highest < (*higher).get_id())
            {
                highest = (*higher).get_id();
            }
        }
    }
    
    FD_CLR(_socket.get_id(), &socketset);
    
    ::close(_socket.get_id());
}

bool SocketSet::operator == (Socket & _socket)
{
    return FD_ISSET(_socket.get_id(), &readyset);
}

Socket & SocketSet::get_ready()
{
    for (vector<Socket>::iterator ready = connections.begin(); ready != connections.end(); ++ready)
    {
            if (FD_ISSET((*ready).get_id(), &readyset))
            {
                    return *ready;
            }
    }
    
    return connections.front();
}

Socket & SocketSet::get_socket(int _socket)
{
    for (vector<Socket>::iterator ready = connections.begin(); ready != connections.end(); ++ready)
    {
            if ((*ready).get_id() == _socket)
            {
                    return *ready;
            }
    }
    
    return connections.front();
}

int SocketSet::get_status(int _timeout)
{
    int result;
    timeval timeout;
    
    timeout.tv_sec = _timeout;
    timeout.tv_usec = 0;
    
    readyset = socketset;
    
    if (_timeout > 0)
        result = select(highest + 1, &readyset, NULL, NULL, &timeout);
    else
        result = select(highest + 1, &readyset, NULL, NULL, NULL);
    
    if (result == -1)
    {
        cout << "Could not select socket: " << strerror(errno) << endl;
    }
    
    return result;
}

void SocketSet::send(const string & _data)
{
    for (vector<Socket>::iterator socket = connections.begin(); socket != connections.end(); ++socket)
    {
        if (!(*socket).get_listener()) (*socket).send(_data);
    }
}

void SocketSet::close()
{
    for (vector<Socket>::iterator socket = connections.begin(); socket != connections.end(); ++socket)
    {
        ::close((*socket).get_id());
    }
}
