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
#include <string.h>
#include <iostream>

#include "stats.h"
#include "system.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

using namespace std;

void Stats::update_system_stats()
{
	time_t uxt;
	
	char tmp[PATH_MAX];
	unsigned int last_update_diff;
	
	sys_info stats;
	net_data net_data;
	disk_data disk_data;
	vector<net_info>::iterator nit;
	vector<disk_info>::iterator dit;
	vector<sensor_info>::iterator sit;
	
	uxt = get_unixtime();
	
	get_mem_data(&stats.mem);
	get_cpu_data(&stats.cpu);
	get_avg_data(&stats.cpu);
	
	for (nit = nets.begin(); nit < nets.end(); nit++)
	{
		last_update_diff = uxt - (*nit).last_update;
		
		// Skip this device if last try is less than one minute away.
		if ((*nit).active == false && last_update_diff < 10) continue;
		
		// Save last probe time. Good for fail check interval.
		(*nit).last_update = uxt;

		strncpy(tmp, (*nit).device, sizeof(tmp) - 1);
		tmp[sizeof(tmp) - 1] = 0;
		
		if (get_net_data(tmp, &net_data) == -1)
		{
			// Only print warning if device is active
			if ((*nit).active)
			{
				cout << "Could not get network data for '" << (*nit).device << "'. Device not found." << endl;
				(*nit).active = false;
			}
		}
		else
		{
			net_data.uxt = uxt;
			net_data.upt = get_uptime();
			
			(*nit).active = true;
			
			if (!(*nit).history.size())
				(*nit).history.insert((*nit).history.begin(), net_data);
			
			if (net_data.uxt != (*nit).history.front().uxt)
				(*nit).history.insert((*nit).history.begin(), net_data);
			
			if ((*nit).history.size() > NET_HISTORY) (*nit).history.pop_back();
		}
	}
	
	for (dit = disks.begin(); dit < disks.end(); dit++)
	{
		if ((*dit).active == false) continue;
		
		// Update the disk information every minute
		last_update_diff = uxt - (*dit).last_update;
		
		if (last_update_diff > 60)
		{
			get_disk_info((*dit).device, (*dit).uuid, (*dit).label, (*dit).name);
			
			(*dit).last_update = uxt;
		}
		
		// Get block size data from mount path
		if (get_disk_data((*dit).name, &disk_data) == -1)
		{
			cout << "Could not get disk data for '" << (*dit).device << "'. Device not found." << endl;
			(*dit).active = false;
		}
		else
		{
			disk_data.uxt = uxt;
			
			// Insert new data and pop out old
			if (!(*dit).history.size())
				(*dit).history.insert((*dit).history.begin(), disk_data);
			
			if (disk_data.uxt != (*dit).history.front().uxt)
				(*dit).history.insert((*dit).history.begin(), disk_data);
			
			if ((*dit).history.size() > DISK_HISTORY) (*dit).history.pop_back();
		}
	}
	
#ifdef HAVE_LIBSENSORS
	for (sit = sensors.begin(); sit < sensors.end(); sit++)
	{
		if (sit->active == false) continue;
		
		last_update_diff = uxt - sit->last_update;
		
		if (last_update_diff >= 10)
		{
			if (get_sensor_data(sit->data.id, &sit->data) == -1)
			{
				cout << "Could not get sensor data for '" << sit->data.label << "'. Device not found." << endl;
				sit->active = false;
			}
			
			sit->last_update = uxt;
		}
	}
#endif
	
	stats.uxt = uxt;
	stats.upt = get_uptime();

	if (!history.size())
		history.insert(history.begin(), stats);
	
	if (stats.uxt != history.front().uxt)
		history.insert(history.begin(), stats);
	
	if (history.size() > STAT_HISTORY) history.pop_back();
}

sys_info Stats::get_stats()
{
	return history[0];
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
	strncpy(temp.device, _iface, sizeof(temp.device) - 1);
	temp.device[sizeof(temp.device) - 1] = 0;
	temp.active = true;
	
	nets.push_back(temp);
}

void Stats::add_disk(const char * _disk)
{
	disk_info temp;
	
	strncpy(temp.device, _disk, sizeof(temp.device) - 1);
	temp.device[sizeof(temp.device) - 1] = 0;
	temp.active = true;
	
	disks.push_back(temp);
}

void Stats::add_sensor(struct sensor_data *_sensor)
{
	sensor_info temp;
	
	temp.data = (*_sensor);
	temp.active = true;
	
	sensors.push_back(temp);
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
			temp = *cur;
			temp.history.clear();
			
			for (vector<net_data>::iterator curhis = (*cur).history.end() - 1; curhis >= (*cur).history.begin(); --curhis)
			{
				if (curhis->upt > _pos || _pos == -1)
				{
					temp.history.push_back(*curhis);
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

vector<sensor_info> Stats::get_fan_sensors()
{
	return sensors;
}

vector<sensor_info> Stats::get_temp_sensors()
{
	return sensors;
}
