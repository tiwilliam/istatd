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
 
#ifndef _CONF_H
#define _CONF_H

#include <vector>
#include <iostream>

#define COMMENT_CHAR '#'
#define ARRAY_CHAR_BEG '('
#define ARRAY_CHAR_END ')'

class Property
{
    public:
        std::string var;
        std::string val;
        std::vector<std::string> array;
        
        unsigned int get_array_size();
        std::string get_array(unsigned int _index);
};

class Config
{
    public:
        Config(const std::string & _filename) : filename(_filename) {}
        
        void parse();
        void validate();
        bool isset(const std::string & _var);
        std::string get(const std::string & _var, const std::string & _default = "");
        Property get_property(const std::string & _var);
        
    private:
        void remove_junk(std::string & _line);
        
        std::string filename;
        std::vector<Property> properties;
};

#endif