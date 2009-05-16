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

#include "token.h"

Tokens::Tokens(std::string _data, const char * _token)
{
    unsigned int pos;
    
    tokens.clear();
    
    _data = stripBreak(_data);
    
    pos = _data.find_first_of(_token);
    
    if (pos != std::string::npos)
    {
        while (pos != std::string::npos)
        {
            tokens.push_back(_data.substr(0, pos));
            
            _data = _data.substr(pos + 1);
            pos = _data.find_first_of(_token);
        }
        
        tokens.push_back(_data.substr(0, pos));
    }
    else
    {
        tokens.push_back(_data);
    }
}

std::string Tokens::getText(int _token)
{
    int i = 0;
    std::string temp, text = "";
    
    for (std::list<std::string>::iterator list = tokens.begin(); list != tokens.end(); ++list)
    {
        if (_token <= i++)
        {
            temp = *list;
            
            text.append(temp);
            text.append(" ");
        }
    }
    
    return text;
}

std::string Tokens::getToken(int _token)
{
    int i = 0;
    
    for (std::list<std::string>::iterator list = tokens.begin(); list != tokens.end(); ++list)
    {
        if (_token == i++)
            return *list;
    }
    
    return "";
}

std::string Tokens::stripBreak(std::string _data)
{
    return _data.substr(0, _data.find_last_not_of("\n\r\t ") + 1);
}