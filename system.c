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

#define _GNU_SOURCE                    1

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
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#elif defined(HAVE_MNTENT_H)
# include <mntent.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#elif defined(HAVE_SYS_STATFS_H)
# include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_LOADAVG_H
# include <sys/loadavg.h>
#endif
#ifdef HAVE_SYS_PROCESSOR_H
# include <sys/processor.h>
#endif
#ifdef HAVE_SYS_SYSINFO_H
# include <sys/sysinfo.h>
#endif
#ifdef HAVE_KSTAT_H
# include <kstat.h>
#endif
#ifdef HAVE_SYS_SWAP_H
# include <sys/swap.h>
#endif

#include "system.h"

#ifndef _PATH_MOUNTED
# define _PATH_MOUNTED                 "/etc/mtab"
#endif

#ifdef HAVE_LIBKSTAT
kstat_ctl_t *ksh;
#endif

int sys_init(void)
{
#ifdef HAVE_LIBKSTAT
    if(NULL == (ksh = kstat_open()))
        {
            fprintf(stderr, "kstat_open(): %s\n", strerror(errno));
            return -1;
        }
#endif
    return 0;
}

#ifdef HAVE_LIBKSTAT
static unsigned long long ksgetull(kstat_named_t *kn)
{
    switch(kn->data_type)
    {
#ifdef KSTAT_DATA_INT32
        case KSTAT_DATA_INT32:         return kn->value.i32;
        case KSTAT_DATA_UINT32:        return kn->value.ui32;
        case KSTAT_DATA_INT64:         return kn->value.i64;
        case KSTAT_DATA_UINT64:        return kn->value.ui64;
#else
        case KSTAT_DATA_LONG:          return kn->value.l;
        case KSTAT_DATA_ULONG:         return kn->value.ul;
        case KSTAT_DATA_LONGLONG:      return kn->value.ll;
        case KSTAT_DATA_ULONGLONG:     return kn->value.ull;
#endif
        default:
            return (unsigned long long) -1;
    }
}   
#endif

int get_uptime()
{
#ifdef HAVE_LIBKSTAT
    kstat_t *ksp;
    kstat_named_t *kn;
    static time_t boottime;
    
    if(0 == boottime)
    {
        kstat_chain_update(ksh);
        if(NULL == (ksp = kstat_lookup(ksh, "unix", -1, "system_misc"))) return -1;
        if(-1 == kstat_read(ksh, ksp, NULL)) return -1;
        if(NULL == (kn = (kstat_named_t *) kstat_data_lookup(ksp, "boot_time"))) return -1;
        boottime = (time_t) ksgetull(kn);
    }
    return time(NULL) - boottime;
#else
    int uptime;
    static FILE * fp = NULL;

    if (!(fp = fopen("/proc/uptime", "r"))) return -1;

    if(1 != fscanf(fp, "%d", &uptime))
	{
		return -1;
	}

    fclose(fp);

    return uptime;
#endif
}

int get_unixtime()
{
    time_t cur;    
    
    time(&cur);
    
    return cur;
}

int get_cpu_load(struct cpu_load * _cpu)
{
#ifdef HAVE_LIBKSTAT
    kstat_t *ksp;
    static int ncpu;
    int c;
    cpu_stat_t cs;
    
    ncpu = sysconf(_SC_NPROCESSORS_CONF);
    kstat_chain_update(ksh);
    _cpu->u = _cpu->n = _cpu->i = _cpu->s = 0;
    for(c = 0; c < ncpu; c++)
    {
        if(p_online(c, P_STATUS) != P_ONLINE)
        {
            continue;
        }
        if(NULL == (ksp = kstat_lookup(ksh, "cpu_stat", c, NULL))) return -1;
        if(-1 == kstat_read(ksh, ksp, &cs)) return -1;
        _cpu->u += cs.cpu_sysinfo.cpu[CPU_USER];
        _cpu->s += cs.cpu_sysinfo.cpu[CPU_KERNEL];
        _cpu->i += cs.cpu_sysinfo.cpu[CPU_IDLE];
    }
#else
    char buf[320];
    static FILE * fp = NULL;
    
    if (!(fp = fopen("/proc/stat", "r"))) return -1;
    if (!fgets(buf, sizeof(buf), fp)) return -1;
    
    sscanf(buf, "cpu %llu %llu %llu %llu", &_cpu->u, &_cpu->n, &_cpu->s, &_cpu->i);
    fclose(fp);
#endif    
    return 0;
}

int get_mem_info(struct mem_info * _mem)
{
#if defined(HAVE_SYS_SWAP_H) && defined(HAVE_SWAPCTL)
    struct swaptable *st;
    struct swapent *ent;
    char *pbuf, *obuf;
    int bpp, num, c;
#endif
#ifdef HAVE_LIBKSTAT
    kstat_t *ksp;
    kstat_named_t *kn;
    unsigned long long lv, ps;
    int cc, ncpu;
    cpu_stat_t cs;
    
    memset(_mem, 0, sizeof(struct mem_info));
    if(NULL == (ksp = kstat_lookup(ksh, "unix", -1, "system_pages"))) return -1;
    if(-1 == kstat_read(ksh, ksp, NULL)) return -1;
    ps = getpagesize();
    if(NULL != (kn = (kstat_named_t *) kstat_data_lookup(ksp, "pagestotal")))
    {
        lv = ksgetull(kn);
        _mem->t = (lv * ps) / 1024;
    }
    if(NULL != (kn = (kstat_named_t *) kstat_data_lookup(ksp, "pageslocked")))
    {
        lv = ksgetull(kn);
        _mem->c = (lv * ps) / 1024;
    }
    if(NULL != (kn = (kstat_named_t *) kstat_data_lookup(ksp, "pagesfree")))
    {
        lv = ksgetull(kn);
        _mem->f = (lv * ps) / 1024;
    }
    _mem->a = _mem->t - _mem->c - _mem->f;
    ncpu = sysconf(_SC_NPROCESSORS_CONF);
    for(cc = 0; cc < ncpu; cc++)
    {
        if(p_online(cc, P_STATUS) != P_ONLINE)
        {
            continue;
        }
        if(NULL != (ksp = kstat_lookup(ksh, "cpu_stat", cc, NULL)))
        {
            if(-1 != kstat_read(ksh, ksp, &cs))
            {
                _mem->swi += cs.cpu_vminfo.pgin;
                _mem->swo += cs.cpu_vminfo.pgout;
            }
        }
    }
#else
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
        sscanf(buf, "Cached: %llu kB", &_mem->c);
        sscanf(buf, "SwapTotal: %llu kB", &_mem->swt);
    }
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
#endif
# if defined(HAVE_SYS_SWAP_H) && defined(HAVE_SWAPCTL)
    if (0 == _mem->swt && -1 != (num = swapctl(SC_GETNSWP, NULL)))
    {
        bpp = getpagesize() >> DEV_BSHIFT;
        st = (struct swaptable *) malloc(num = sizeof(swapent_t) + sizeof(int));
        pbuf = obuf = (char *) malloc(num * MAXPATHLEN);
        ent = st->swt_ent;
        /* Provide buffers for the swap device names */
        for (c = 0; c < num; c++, ent++)
        {
            ent->ste_path = pbuf;
            pbuf += MAXPATHLEN;
        }
        /* Retrieve the devices */
        if(-1 != (num = swapctl(SC_LIST, st)))
        {
            ent = st->swt_ent;
            for (c = 0; c < num; c++, ent++)
            {
                _mem->swt += ent->ste_pages * bpp * DEV_BSIZE / 1024;
            }
        }
        free(obuf);
        free(st);
    }
# endif
    return 0;
}

int get_net_info(const char * _dev, struct net_data * _data)
{
#ifdef HAVE_LIBKSTAT
    kstat_t *ksp;
    kstat_named_t *kn;
    char name[32];

    strncpy(name, _dev, sizeof(name) - 1);
    name[sizeof(name) - 1] = 0;
    if(NULL == (ksp = kstat_lookup(ksh, "link", -1, name))) return -1;
    if(-1 == kstat_read(ksh, ksp, NULL)) return -1;
    kn = (kstat_named_t *) ksp->ks_data;
    if(NULL == (kn = (kstat_named_t *) kstat_data_lookup(ksp, "obytes64")))
    {
        return -1;
    }
    _data->s = ksgetull(kn);
    if(NULL == (kn = (kstat_named_t *) kstat_data_lookup(ksp, "rbytes64")))
    {
        return -1;
    }
    _data->r = ksgetull(kn);
#else   
    char dev[8];
    static FILE * fp = NULL;
    int found_device = 0;
    
    if (!(fp = fopen("/proc/net/dev", "r"))) return -1;
    
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
#endif
    return 0;
}

int get_load_avg(struct load_avg * _load)
{
#if defined(HAVE_GETLOADAVG)
    double loadavg[3];
    
    if(-1 == getloadavg(loadavg, 3)) return -1;
    _load->one = (float) loadavg[0];
    _load->two = (float) loadavg[1];
    _load->three = (float) loadavg[2];
#else
    static FILE * fp = NULL;
    
    if (!(fp = fopen("/proc/loadavg", "r"))) return -1;
    
    fscanf(fp, "%f %f %f", &_load->one, &_load->two, &_load->three);
    
    fclose(fp);
#endif
    return 0;
}

int get_disk_info(const char * _dev, struct disk_info * _disk)
{
    FILE * table;
    unsigned long long bsize;
#ifdef HAVE_STATVFS
    struct statvfs space;
#else
    struct statfs space;
#endif
    int get_size = 0;
#ifdef USE_STRUCT_MNTENT
    struct mntent * entry;
#elif defined(USE_STRUCT_MNTTAB)
    struct mnttab *entry, ebuf;
#endif
    
#ifdef HAVE_SETMNTENT
    if (!(table = setmntent(_PATH_MOUNTED, "r"))) return -1;
#else
    if (!(table = fopen(_PATH_MOUNTED, "r"))) return -1;
    resetmnttab(table);
#endif
    
#ifdef USE_STRUCT_MNTENT
    while ((entry = getmntent(table)) != 0)
#elif defined(USE_STRUCT_MNTTAB)
    entry = &ebuf;
    while (!getmntent(table, entry))
# define mnt_fsname mnt_special
# define mnt_dir mnt_mountp 
#endif
    {
        if (strcmp(entry->mnt_fsname, _dev) == 0 || strcmp(entry->mnt_dir, _dev) == 0)
        {
            get_size = 1;
            
            _disk->name = entry->mnt_dir;
            _disk->device = _dev;
            
            break;
        }
    }
    
#ifdef HAVE_SETMNTENT
    endmntent(table);
#else
    fclose(table);
#endif
    if (get_size)
    {
#ifdef HAVE_STATVFS
        if (statvfs(_disk->name, &space) == 0)
#else
        if (statfs(_disk->name, &space) == 0)
#endif
        {
#ifdef HAVE_STATVFS_FRSIZE
            bsize = space.f_frsize;
#else
            bsize = space.f_bsize;
#endif
            _disk->t = ((unsigned long long) space.f_blocks * bsize) / 1024;
            _disk->u = ((unsigned long long) (space.f_blocks - space.f_bavail) * bsize) / 1024;
            _disk->f = _disk->t - _disk->u;
            _disk->p = ((float) _disk->u / _disk->t) * 100;
            
            return 0;
        }
    }
    
    return -1;
}
