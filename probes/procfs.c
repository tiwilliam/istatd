/*
 *  Copyright 2009 William Tisäter. All rights reserved.
 *  Copyright 2009 Mo McRoberts.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define _GNU_SOURCE					1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_PATHS_H
# include <paths.h>
#endif
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#include <sys/types.h>

#include "system.h"

#ifdef USE_UPTIME_PROCFS
int get_uptime(void)
{
	int uptime;
	static FILE * fp = NULL;

	if (!(fp = fopen("/proc/uptime", "r"))) return -1;

	if(1 != fscanf(fp, "%d", &uptime))
	{
		return -1;
	}

	fclose(fp);

	return uptime;
}
#endif

int get_unixtime(void)
{
	time_t cur;
	
	time(&cur);
	
	return cur;
}

#ifdef USE_CPU_PROCFS
int get_cpu_data(struct cpu_data * _cpu)
{
	char buf[320];
	static FILE * fp = NULL;
	
	if (!(fp = fopen("/proc/stat", "r"))) return -1;
	if (!fgets(buf, sizeof(buf), fp)) return -1;
	
	sscanf(buf, "cpu %llu %llu %llu %llu", &_cpu->u, &_cpu->n, &_cpu->s, &_cpu->i);
	fclose(fp);
	return 0;
}
#endif

#ifdef USE_MEM_PROCFS
int get_mem_data(struct mem_data * _mem)
{
	char buf[320];
	static FILE * fp = NULL;
	unsigned long long memtotal = 0;

	if (!(fp = fopen("/proc/meminfo", "r"))) return -1;

	while (fgets(buf, sizeof(buf), fp))
	{
		sscanf(buf, "MemTotal: %llu kB", &memtotal);
		sscanf(buf, "MemFree: %llu kB", &_mem->f);
		sscanf(buf, "Active: %llu kB", &_mem->a);
		sscanf(buf, "Inactive: %llu kB", &_mem->i);
		sscanf(buf, "SwapTotal: %llu kB", &_mem->swt);
	}

	_mem->c = memtotal - _mem->f - _mem->a - _mem->i;

	if (0 == _mem->a && 0 == _mem->i && 0 == _mem->c)
	{
		_mem->a = memtotal - _mem->f;
	}
	fclose(fp);
	
	if (!(fp = fopen("/proc/vmstat", "r"))) return -1;
	
	while (fgets(buf, sizeof(buf), fp))
	{
		sscanf(buf, "pswpin %llu", &_mem->swi);
		sscanf(buf, "pswpout %llu", &_mem->swo);
	}
	
	fclose(fp);
	
	_mem->t = _mem->f + _mem->a + _mem->i + _mem->c + 1;
	return 0;
}
#endif

#ifdef USE_NET_PROCFS
int get_net_data(const char * _dev, struct net_data * _data)
{
	char dev[8];
	static FILE * fp = NULL;
	int found_device = 0;
	
	if (!(fp = fopen("/proc/net/dev", "r")))
	{
		perror("fopen: /proc/net/dev");
		return -1;
	}
	
	fscanf(fp, "%*[^\n] %*[^\n] ");
	
	while (!feof(fp))
	{
		fscanf(fp, "%7[^:]:%llu %*u %*u %*u %*u %*u %*u %*u %llu %*[^\n] ", dev, &_data->r, &_data->s);
		
		if (strncmp(dev, _dev, 7) == 0)
		{
			found_device = 1;
			break;
		}
	}
	
	fclose(fp);
	
	if (!found_device) return -1;
	return 0;
}
#endif

#ifdef USE_LOAD_PROCFS
int get_avg_data(struct cpu_data *_cpu)
{
	static FILE * fp = NULL;

	if (!(fp = fopen("/proc/loadavg", "r"))) return -1;                

	fscanf(fp, "%f %f %f", &_cpu->one, &_cpu->two, &_cpu->three);
	fclose(fp);
	
	return 0;
}
#endif
