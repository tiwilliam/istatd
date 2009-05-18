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

#include <vector>
#include <iostream>

#include "stats.h"
#include "system.h"

using namespace std;

void Stats::update_system_stats()
{
    sys_info stats;
    net_data net_data;
    vector<net_info>::iterator nit;
    vector<disk_info>::iterator dit;
    
    get_mem_info(&stats.mem);
    get_cpu_load(&stats.cpu);
    get_load_avg(&stats.avg);
    
    for (nit = nets.begin(); nit < nets.end(); nit++)
    {
        if ((*nit).active == 0) continue;
        
        if (get_net_info((*nit).device, &net_data) == -1)
        {
            cout << "Could not get network data for '" << (*nit).device << "'. Device not found." << endl;
            (*nit).active = 0;
        }
        else
        {
            net_data.upt = get_uptime();
            net_data.uxt = get_unixtime();

            // TODO: clean_history((*nit).history, net_data, 300);
            if (!(*nit).history.size()) (*nit).history.insert((*nit).history.begin(), net_data);
            if (net_data.upt != (*nit).history.front().upt) (*nit).history.insert((*nit).history.begin(), net_data);
            if ((*nit).history.size() > 300) (*nit).history.pop_back();
        }
    }
    
    for (dit = disks.begin(); dit < disks.end(); dit++)
    {
        if ((*dit).active == 0) continue;
        
        if (get_disk_info((*dit).device, &(*dit)) == -1)
        {
            cout << "Could not get disk data for '" << (*dit).device << "'. Device not found in /etc/mtab." << endl;
            (*dit).active = 0;
        }
    }
    
    stats.upt = get_uptime();
    stats.uxt = get_unixtime();
    
    // TODO: clean_history(history, stats, 300);
    if (!history.size()) history.insert(history.begin(), stats);
    if (stats.upt != history.front().upt) history.insert(history.begin(), stats);
    if (history.size() > 300) history.pop_back();
}

sys_info Stats::get_stats()
{
    return history.at(0);
}

unsigned int Stats::get_size()
{
    return history.size();
}

void Stats::add_net(const char * _iface)
{
    net_info temp;
    unsigned int id = 1;

    if (nets.size())
    {
        id = nets.back().id + 1;
        cout << "Warning! Only support for one network interface on client side. Will monitor the first given." << endl;
    }
    
    temp.id = id;
    temp.device = _iface;
    temp.active = true;
    
    nets.push_back(temp);
}

void Stats::add_disk(const char * _disk)
{
    disk_info temp;
    
    temp.device = _disk;
    temp.active = true;
    
    disks.push_back(temp);
}

vector<sys_info> Stats::get_history(int _pos)
{
    vector<sys_info> temp;
    
    for (vector<sys_info>::iterator cur = history.end() - 1; cur >= history.begin(); --cur)
    {
        if (cur->upt >= _pos || _pos == -1)
        {
            temp.push_back((*cur));
        }
    }
    
    return temp;
}

vector<net_info> Stats::get_net_history(int _pos)
{
    net_info temp;
    vector<net_info> templist;
    
    for (vector<net_info>::iterator cur = nets.end() - 1; cur >= nets.begin(); --cur)
    {
        if ((*cur).active == true)
        {
            temp.id = (*cur).id;
            temp.device = (*cur).device;
            temp.active = (*cur).active;

            for (vector<net_data>::iterator curhis = (*cur).history.end() - 1; curhis >= (*cur).history.begin(); --curhis)
            {
                if (curhis->upt > _pos || _pos == -1)
                {
                    temp.history.push_back((*curhis));
                }
            }
            
            templist.push_back(temp);
            
            // Currently only support for one interface
            break;
        }
    }
    
    return templist;
}

vector<disk_info> Stats::get_disk_history()
{
    return disks;
}
