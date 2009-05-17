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
#include <iostream>

#include "isr.h"
#include "stats.h"
#include "utility.h"

using namespace std;

string isr_create_header()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
}

string isr_create_session(int _rid, int _ds, int _ts, int _fs)
{
    stringstream temp;
    temp << "<isr ds=\"" << _ds << "\" ts=\"" << _ts << "\" fs=\"" << _fs << "\" rid=\"" << _rid << "\">";
    return temp.str();
}

string isr_accept_code()
{
    stringstream temp;
    temp << isr_create_header() << "<isr ready=\"1\"></isr>";
    return temp.str();
}

string isr_reject_code()
{
    stringstream temp;
    temp << isr_create_header() << "<isr athrej=\"1\"></isr>";
    return temp.str();
}

string isr_conntest()
{
    stringstream temp;
    temp << isr_create_header() << "<isr></isr>";
    return temp.str();
}

string isr_accept_connection(int _ath, int _ss, int _c, int _n)
{
    stringstream temp;
    temp << isr_create_header() << "<isr pl=\"2\" ath=\"" << _ath << "\" ss=\"" << _ss << "\" c=\"" << _c << "\" n=\"" << _n << "\"></isr>";
    return temp.str();
}

string isr_cpu_data(vector<sys_info> * _history, int _init)
{
    int uptime;
    sys_info prev;
    stringstream temp;
    int diff_u, diff_s, diff_n, diff_i;
    float load_t, load_u, load_s, load_n;
    
	if(0 == _history->size()) return temp.str();
    temp << "<CPU>";
    
    for (vector<sys_info>::iterator cur = _history->begin() + 1; cur != _history->end(); ++cur)
    {
        uptime = cur->upt;
        
        if (_init == -1)
        {
            cur = _history->end() - 1;
            uptime = -1;
        }
        
        cur--; prev = (*cur); cur++;
        
        diff_u = cur->cpu.u - prev.cpu.u;
        diff_s = cur->cpu.s - prev.cpu.s;
        diff_n = cur->cpu.n - prev.cpu.n;
        diff_i = cur->cpu.i - prev.cpu.i;
        
        load_t = diff_u + diff_s + diff_n + diff_i;
        
        load_u = ((float) diff_u / load_t) * 100;
        load_s = ((float) diff_s / load_t) * 100;
        load_n = ((float) diff_n / load_t) * 100;
        
        temp << "<c id=\"" << uptime << "\" u=\"" << load_u << "\" s=\"" << load_s << "\" n=\"" << load_n << "\"></c>";
    }
    
    temp << "</CPU>";
    
    return temp.str();
}

string isr_network_data(vector<net_info> * _history, int _init)
{
    stringstream temp;

	if(0 == _history->size()) return temp.str();    
    for (vector<net_info>::iterator cur = _history->begin(); cur != _history->end(); ++cur)
    {
        if ((*cur).active == false) continue;
        
        temp << "<NET if=\"" << cur->id << "\">";
        
        for (vector<net_data>::iterator curhis = cur->history.begin(); curhis != cur->history.end(); ++curhis)
        {
            if (_init == -1)
            {
                curhis = cur->history.end() - 1;
                curhis->upt = -1;
            }
           
            temp << "<n id=\"" << curhis->upt << "\" d=\"" << curhis->r << "\" u=\"" << curhis->s << "\" t=\"" << curhis->uxt << "\"></n>";
        }
        
        temp << "</NET>";
    }
    
    return temp.str();
}

string isr_disk_data(vector<disk_info> * _disks, int _init)
{
    stringstream temp;
    
    if(0 == _disks->size()) return temp.str();
    
    temp << "<DISKS>";
 
    for (vector<disk_info>::iterator cur = _disks->begin(); cur != _disks->end(); ++cur)
    {
        if ((*cur).active == false) continue;
        
        temp << "<d n=\"" << (*cur).device << "\" uuid=\"" << (*cur).device << "\" f=\"" << (*cur).f / 1000 << "\" p=\"" << (*cur).p << "\"></d>";
    }
    
    temp << "</DISKS>";
    
    return temp.str();
}

string isr_uptime_data(vector<sys_info> * _history)
{
    stringstream temp;
    
	if(0 == _history->size()) return temp.str();
    temp << "<UPT u=\"" << _history->back().upt << "\"></UPT>";
    
    return temp.str();
}

string isr_loadavg_data(vector<sys_info> * _history)
{
    stringstream temp;
    
	if(0 == _history->size()) return temp.str();	
    temp << "<LOAD one=\"" << _history->back().avg.one << "\" fv=\"" << _history->back().avg.two << "\" ff=\"" << _history->back().avg.three << "\"></LOAD>";
    
    return temp.str();
}

string isr_memory_data(vector<sys_info> * _history)
{
    stringstream temp;
    
	if(0 == _history->size()) return temp.str();	
    temp << "<MEM w=\"" << _history->back().mem.c / 1000 << "\" a=\"" << _history->back().mem.a / 1000 << "\" i=\"" << _history->back().mem.i / 1000 << "\" f=\"" << _history->back().mem.f / 1000 << "\" t=\"" << _history->back().mem.t / 1000 << "\" su=\"0\" st=\"" << _history->back().mem.swt / 1000 << "\" pi=\"" << _history->back().mem.swi << "\" po=\"" << _history->back().mem.swo << "\"></MEM>";
    
    return temp.str();
}

