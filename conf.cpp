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
#include <stdio.h>
#include <errno.h>
#include <fstream>
#include <sys/un.h>
#include <stdlib.h>
#include <iostream>

#include "conf.h"
#include "utility.h"

using namespace std;

unsigned int Property::get_array_size()
{
    return array.size();
}

std::string Property::get_array(unsigned int _index)
{
    if (_index < array.size())
        return array.at(_index);
    
    return "";
}

void Config::parse()
{
    Property property;
    unsigned int num = 1;
    vector<string> array;
    string::size_type pos;
    string var, val, line;
    
    ifstream config(filename.c_str());
    
    if (config.good())
    {
        while (getline(config, line))
        {
            // Remove whitespace and comments
            remove_junk(line);
            
            // Okey we got something to analyze...
            if (line.length())
            {
                pos = line.find_first_of(" \t");
                var = line.substr(0, pos);
                val = trim(line.substr(pos));
                
                property.var = var;
                property.val = val;
                
                // Parse arrays
                if (val[0] == ARRAY_CHAR_BEG && val[val.length() - 1] == ARRAY_CHAR_END)
                {
                    val = trim(val.substr(1, val.length() - 2));
                    array = explode(val, " \t");
                    
                    property.val = "";
                    property.array = array;
                }
            }
            
            properties.push_back(property);
            
            // Count lines for syntax error
            num++;
        }
        
        config.close();
    }
    else
    {
        cout << "Could not read configuration from " << filename << ": " << strerror(errno) << endl;
        config.close();
        exit(1);
    }
}

void Config::validate()
{
    if (properties.size())
    {
        /*
        if (this->get("server_addr") != "")   cout << "Validating server_addr: " << this->get("server_addr") << endl;
        if (this->get("server_port") != "")   cout << "Validating server_port: " << this->get("server_port") << endl;
        if (this->get("server_code") != "")   cout << "Validating server_code: " << this->get("server_code") << endl;
        if (this->get("server_pid") != "")    cout << "Validating server_pid: " << this->get("server_pid") << endl;
        if (this->get("server_socket") != "") cout << "Validating server_socket: " << this->get("server_socket") << endl;
        */
    }
}

void Config::remove_junk(string & _line)
{
    string::size_type pos;
    
    // Remove whitespace
    _line = trim(_line);
    
    // Remove comments
    if ((pos = _line.find_first_of(COMMENT_CHAR)) != string::npos)
    {
        _line = _line.substr(0, pos);
    }
}

bool Config::isset(const string & _var)
{
    for (vector<Property>::iterator i = properties.begin(); i != properties.end(); i++)
    {
        if (i->var == _var)
            if (i->val != "")
                return 1;
    }
    
    return 0;
}

string Config::get(const string & _var, const std::string & _default)
{
    for (vector<Property>::iterator i = properties.begin(); i != properties.end(); i++)
    {
        if (i->var == _var)
            return i->val;
    }
    
    return _default;
}

Property Config::get_property(const string & _var)
{
    Property null;
    
    null.val = "";
    null.var = "";
    
    for (vector<Property>::iterator i = properties.begin(); i != properties.end(); i++)
    {
        if (i->var == _var)
            return (*i);
    }
    
    return null;
}
