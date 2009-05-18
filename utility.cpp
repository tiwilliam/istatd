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

#include <pwd.h>
#include <vector>
#include <sstream>
#include <errno.h>
#include <fstream>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "utility.h"

using namespace std;

int get_current_user()
{
    return geteuid();
}

int get_id_from_name(const string & name)
{
    struct passwd * ent;
    
    if(!(ent = getpwnam(name.c_str())))
    {
        cout << "Could not get uid for username " << name << ", will run as current user instead." << endl;
        if (get_current_user() == 0) cout << "You are now running the daemon as root, this is not recommended." << endl;
        return -1;
    }
    
    return(ent->pw_uid);
}

int get_file_owner(const string & file)
{
    struct stat stats;
    
    stat(file.c_str(), &stats);
    
    return stats.st_uid;
}

int pid_dead(int pid)
{
    int ret = 0;
    
    if (waitpid(pid, NULL, WNOHANG) != 0)
        ret = 1;
    
    // Return 1 if process is dead        
    return ret;
}

int check_dir_exist(const string & dir)
{
    int result;
    struct stat stats;
    
    if (stat(dir.c_str(), &stats) == 0 && S_ISDIR(stats.st_mode) == 1)
        result = 1;
    else
        result = 0;
    
    return result;
}

int check_file_exist(const string & file)
{
    int result;
    struct stat stats;
    
    if (stat(file.c_str(), &stats) == 0 && S_ISREG(stats.st_mode) == 1)
        result = 1;
    else
        result = 0;
    
    return result;
}

string trim(const string & source, const char * delims)
{
    string result(source);
    string::size_type index = result.find_last_not_of(delims);
    
    if (index != string::npos)
        result.erase(++index);
    
    index = result.find_first_not_of(delims);
    
    if (index != string::npos)
        result.erase(0, index);
    else
        result.erase();
    
    return result;
}

vector<string> split(const string & str, const string str_delim)
{
    vector<string> v;
    string str_elem("");
    std::string::size_type ui_cur_pos, ui_last_pos = 0;
    
    // Check for empty string
    if (str.empty()) return v;
    
    ui_cur_pos = str.find_first_of(str_delim.c_str(), ui_last_pos);
    
    while(ui_cur_pos != str.npos)
    {
        str_elem = str.substr(ui_last_pos, ui_cur_pos-ui_last_pos);
        v.push_back(str_elem);
        
        ui_last_pos = ui_cur_pos + 1;
        ui_cur_pos = str.find_first_of(str_delim.c_str(), ui_last_pos);
    }
    
    // Handle last substring - if any
    if(str.length() != ui_last_pos)
    {
        str_elem = str.substr(ui_last_pos,str.length()-ui_last_pos);
        v.push_back(str_elem);
    }
    
    return v;
}

vector<string> explode(string source, const string & delims)
{
    vector<string> ret;
    string splitted_part;
    unsigned int i, no_match = 0;
    string::size_type pos, split_pos;
    
    // Loop array string until we can't find more delimiters
    while (no_match < delims.length())
    {
        no_match = 0;
        split_pos = string::npos;
        
        // Find first occuring splitter
        for (i = 0; i < delims.length(); i++)
        {
            pos = source.find(delims[i], 0);
            
            if (pos == string::npos) no_match++;
            if (pos < split_pos) split_pos = pos;
        }
        
        // Be nice to things wrapped with quotes
        if (source[0] == '"' && source.substr(1).find_first_of("\"") != string::npos)
        {
            split_pos = source.substr(1).find_first_of("\"") + 2;
        }
        
        // One value from the array
        splitted_part = source.substr(0, split_pos);
        
        // Save the value if it's not empty
        if (splitted_part != "")
        {
            ret.push_back(splitted_part);
        }
        
        // Remove value from string
        source.erase(0, split_pos + 1);
    }
    
    return ret;
}

int create_file(const string & dir, const string & file, mode_t mask)
{
    stringstream temp;
    temp << dir << "/" << file;
    
    umask(mask);

    ofstream out(temp.str().c_str());

    if (!out)
    {
        cout << "Could not create file '" << temp.str() << "': " << strerror(errno) << endl;
        return -1;
    }

    out.close();
    
    return 0;
}
