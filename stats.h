/*
 *  Copyright 2010 William Tisäter. All rights reserved.
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

#ifndef _STATS_H
#define _STATS_H

#define NET_HISTORY		300
#define DISK_HISTORY		300
#define STAT_HISTORY		300

#include <vector>
#include <limits.h>

#include "system.h"

class net_info
{
	public:
		bool active;
		unsigned int id;
		
		char device[PATH_MAX];
		unsigned int last_update;
		
		std::vector<net_data> history;
};

class disk_info
{
	public:
		bool active;
		unsigned int id;
		
		char uuid[37];
		char label[33];
		char name[PATH_MAX];
		char device[PATH_MAX];
		unsigned int last_update;
		
		std::vector<disk_data> history;
};

class sensor_info
{
	public:
		bool active;
		unsigned int id;
		unsigned int last_update;
		
		struct sensor_data data;
};

class Stats
{
	public:
		void add_net(const char *_iface);
		void add_disk(const char *_disk);
		void add_sensor(struct sensor_data *_sensor);
		
		void update_system_stats();
		
		sys_info get_stats();
		unsigned int get_size();
		std::vector<disk_info> get_disk_history();
		std::vector<sys_info> get_history(int _pos);
		std::vector<net_info> get_net_history(int _pos);
		std::vector<sensor_info> get_fan_sensors();
		std::vector<sensor_info> get_temp_sensors();
		
	private:
		std::vector<net_info> nets;
		std::vector<disk_info> disks;
		std::vector<sensor_info> sensors;
		std::vector<sys_info> history;
};

#endif
