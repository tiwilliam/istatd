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

#ifndef _UTILITY_H
#define _UTILITY_H

#include <vector>
#include <sstream>
#include <iostream>

int get_current_user();
int get_file_owner(const std::string & file);
int pid_dead(int pid);
int check_dir_exist(const std::string & dir);
int check_file_exist(const std::string & file);

std::string trim(const std::string & source, const char * delims = " \t\r\n");
std::vector<std::string> split(const std::string & str, const std::string str_delim);
std::vector<std::string> explode(std::string source, const std::string & delims = " ");

template<class T> int to_int(const T & val)
{
    int n;
    std::stringstream buffer(val);
    buffer >> n;
    return n;
}

template<class T> std::string to_ascii(const T & val)
{
    std::ostringstream buffer;
    for (std::string::const_iterator i = val.begin(); i != val.end(); i++) buffer << (int) * i;
    return buffer.str();
}

template<class T> std::string to_string(const T & val)
{
    std::ostringstream buffer;
    buffer << val;
    return buffer.str();
}

#endif
