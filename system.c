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

#include <stdio.h>
#include <string.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/statfs.h>

#include "system.h"

int get_uptime()
{
    int uptime;
    static FILE * fp = NULL;

    if (!(fp = fopen("/proc/uptime", "r"))) return -1;

    fscanf(fp, "%d", &uptime);

    fclose(fp);

    return uptime;
}

int get_unixtime()
{
    time_t cur;    
    
    time(&cur);
    
    return cur;
}

int get_cpu_load(struct cpu_load * _cpu)
{
    char buf[320];
    static FILE * fp = NULL;
    
    if (!(fp = fopen("/proc/stat", "r"))) return -1;
    if (!fgets(buf, sizeof(buf), fp)) return -1;
    
    sscanf(buf, "cpu %Lu %Lu %Lu %Lu", &_cpu->u, &_cpu->n, &_cpu->s, &_cpu->i);
    fclose(fp);
    
    return 0;
}

int get_mem_info(struct mem_info * _mem)
{
    char buf[320];
    static FILE * fp = NULL;

    if (!(fp = fopen("/proc/meminfo", "r"))) return -1;

    while (fgets(buf, sizeof(buf), fp))
    {
        sscanf(buf, "MemFree: %Lu kB", &_mem->f);
        sscanf(buf, "Active: %Lu kB", &_mem->a);
        sscanf(buf, "Inactive: %Lu kB", &_mem->i);
        sscanf(buf, "Cached: %Lu kB", &_mem->c);
        sscanf(buf, "SwapTotal: %Lu kB", &_mem->swt);
    }
    
    fclose(fp);
    
    if (!(fp = fopen("/proc/vmstat", "r"))) return -1;
    
    while (fgets(buf, sizeof(buf), fp))
    {
        sscanf(buf, "pswpin %Lu", &_mem->swi);
        sscanf(buf, "pswpout %Lu", &_mem->swo);
    }
    
    fclose(fp);
    
    _mem->t = _mem->f + _mem->a + _mem->i + _mem->c + 1;

    return 0;
}

int get_net_info(const char * _dev, struct net_data * _data)
{
    char dev[8];
    static FILE * fp = NULL;
    bool found_device = false;
    
    if (!(fp = fopen("/proc/net/dev", "r"))) return -1;
    
    fscanf(fp, "%*[^\n] %*[^\n] ");
    
    while (!feof(fp))
    {
        fscanf(fp, "%7[^:]:%Lu %*u %*u %*u %*u %*u %*u %*u %Lu %*[^\n] ", dev, &_data->r, &_data->s);
        
        if (strncmp(dev, _dev, 7) == 0)
        {
            found_device = true;
            break;
        }
    }
    
    fclose(fp);
    
    if (!found_device) return -1;

    return 0;
}

int get_load_avg(struct load_avg * _load)
{
    static FILE * fp = NULL;
    
    if (!(fp = fopen("/proc/loadavg", "r"))) return -1;
    
    fscanf(fp, "%f %f %f", &_load->one, &_load->two, &_load->three);
    
    fclose(fp);
    
    return 0;
}

int get_disk_info(const char * _dev, struct disk_info * _disk)
{
    FILE * table;
    struct statfs space;
    bool get_size = false;
    struct mntent * entry;
    
    if (!(table = setmntent("/etc/mtab", "r"))) return -1;
    
    while ((entry = getmntent(table)) != 0)
    {
        if (strcmp(entry->mnt_fsname, _dev) == 0)
        {
            get_size = true;
            
            _disk->name = entry->mnt_dir;
            _disk->device = _dev;
            
            break;
        }
    }
    
    endmntent(table);
    
    if (get_size)
    {
        if (statfs(_disk->name, &space) == 0)
        {
            _disk->t = (space.f_blocks * (unsigned long long) space.f_bsize + 1024 / 2) / 1024;
            _disk->u = ((space.f_blocks - space.f_bfree) * (unsigned long long) space.f_bsize + 1024 / 2) / 1024;
            _disk->f = _disk->t - _disk->u;
            _disk->p = ((float) _disk->u / _disk->t) * 100;
        }
        
        return 0;
    }
    
    return -1;
}
