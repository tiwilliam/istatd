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

#ifndef _UTILITY_H
#define _UTILITY_H

#include <vector>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

uid_t get_current_uid();
gid_t get_current_gid();
uid_t get_uid_from_str(const std::string & _user);
gid_t get_gid_from_str(const std::string & _group);
int get_file_owner(const std::string & _file);
int pid_dead(int _pid);
int check_dir_exist(const std::string & _dir);
int check_file_exist(const std::string & _file);
int create_directory(const std::string &_dir, mode_t _mask);

std::string trim(const std::string & _source, const char * _delims = " \t\r\n");
std::vector<std::string> split(const std::string &_str, const std::string _delim);
std::vector<std::string> explode(std::string _str, const std::string &_delim = " ");

template<class T> int to_int(const T &_val)
{
	int n;
	std::stringstream buffer;
	buffer << _val;
	buffer >> n;
	return n;
}

template<class T> std::string to_ascii(const T &_val)
{
	std::ostringstream buffer;
	for (std::string::const_iterator i = _val.begin(); i != _val.end(); i++) buffer << static_cast<int>(*i);
	return buffer.str();
}

template<class T> std::string to_string(const T &_val)
{
	std::ostringstream buffer;
	buffer << _val;
	return buffer.str();
}

#endif
