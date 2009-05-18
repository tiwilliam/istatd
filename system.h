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

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <time.h>
#include <stdio.h>

struct cpu_load
{
    unsigned long u, n, s, i;
};

struct mem_info
{
    unsigned long t, f, a, i, c, swt, swi, swo;
};

struct load_avg
{
    float one, two, three;
};

struct disk_info
{
    float p;
    bool active;
    const char * name, * device;
    unsigned long t, u, f;
};

struct net_data
{
    int upt;
    time_t uxt;
    unsigned long s, r;
};

struct sys_info
{
    int upt;
    time_t uxt;
    struct mem_info mem;
    struct cpu_load cpu;
    struct load_avg avg;
};

int sys_init(void);
int get_uptime();
int get_unixtime();
int get_cpu_load(struct cpu_load * _cpu);
int get_mem_info(struct mem_info * _mem);
int get_load_avg(struct load_avg * _load);
int get_net_info(const char * _dev, struct net_data * _data);
int get_disk_info(const char * _dev, struct disk_info * _disk);

#endif
