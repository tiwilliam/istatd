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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include "socket.h"

using namespace std;

int Socket::listen()
{
	int yes = 1;
	hostent * host = NULL;
	
	if ((socket = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "Could not create socket: " << strerror(errno) << endl;
		return 0;
	}
	
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &yes, sizeof(yes));
	
	sockaddr_in myAddress;
	
	memset(&myAddress, 0, sizeof(sockaddr_in));
	
	if (!(host = gethostbyname(address.c_str())))
		myAddress.sin_addr.s_addr = inet_addr(address.c_str());
	else
		myAddress.sin_addr.s_addr = ((in_addr *) host->h_addr)->s_addr;
	
	myAddress.sin_family = AF_INET;
	myAddress.sin_port = htons(port);
	
	if (::bind(socket, (sockaddr *) &myAddress, sizeof(myAddress)) == -1)
	{
		cout << "Could not bind socket: " << strerror(errno) << endl;
		return 0;
	}
	
	// cout << "(" << address << ":" << port << ") Socket binded." << endl;
	
	if (::listen(socket, 5) == -1)
	{
		cout << "Could not listen on socket: " << strerror(errno) << endl;
		return 0;
	}

	// cout << "(" << address << ":" << port << ") Socket initialized." << endl;
	return 1;
}

int Socket::nonblock()
{ 
	int opts;
	
	opts = (fcntl(socket, F_GETFL) | O_NONBLOCK);
	
	if (fcntl(socket, F_SETFL, opts) < 0)
		cout << "Could not set non-blocking: " << strerror(errno) << endl;
	
	return 0;
}

Socket Socket::accept()
{
	int theirSocket;
	sockaddr_in theirAddress;
	socklen_t size = sizeof(sockaddr_in);		
	
	if ((theirSocket = ::accept(socket, (sockaddr *) &theirAddress, &size)) == -1)
	{
		cout << "Could not accept connection: " << strerror(errno) << endl;
	}
	
	// cout << "(" << inet_ntoa(theirAddress.sin_addr) << ":" << ntohs(theirAddress.sin_port) << ") New connection accepted." << endl;
	
	return Socket(theirSocket, inet_ntoa(theirAddress.sin_addr), ntohs(theirAddress.sin_port));
}

int Socket::send(const string & _data)
{
	int result = 0;
	
	if ((result = ::send(socket, _data.c_str(), _data.size(), 0)) == -1)
	{
		cout << "Could not send data: " << strerror(errno) << endl;
		
		return 0;
	}
	
	// cout << "(" << address << ":" << port << ") Data sent: " << _data << endl;
	
	return result;
}

int Socket::receive(string & _data, int _max)
{
	int result = 0;
	char *buffer = static_cast<char*>(alloca(sizeof(char)*(_max + 1)));
	
	memset(buffer, 0, _max + 1);
	
	if ((result = recv(socket, buffer, _max, 0)) == -1)
	{
		cout << "Could not recv data: " << strerror(errno) << endl;
		
		return 0;
	}
	
	_data = buffer;
	
	/*
	if (_data.length() > 0)
		cout << "(" << address << ":" << port << ") Data recv: " << _data << endl;
	*/
	
	return result;
}

void Socket::close()
{
	::close(this->socket);
}
