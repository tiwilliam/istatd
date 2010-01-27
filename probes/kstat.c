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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#include <sys/types.h>

#ifdef HAVE_SYS_PROCESSOR_H
# include <sys/processor.h>
#endif

#ifdef HAVE_SYS_SYSINFO_H
# include <sys/sysinfo.h>
#endif

#ifdef HAVE_KSTAT_H
# include <kstat.h>
#endif

#include "system.h"

#ifdef HAVE_LIBKSTAT

kstat_ctl_t *ksh;

int kstat_init(void)
{
	if(NULL == (ksh = kstat_open()))
		{
			fprintf(stderr, "kstat_open(): %s\n", strerror(errno));
			return -1;
		}
	return 0;
}

static unsigned long long ksgetull(kstat_named_t *kn)
{
	switch(kn->data_type)
	{
# ifdef KSTAT_DATA_INT32
		case KSTAT_DATA_INT32:		return kn->value.i32;
		case KSTAT_DATA_UINT32:		return kn->value.ui32;
		case KSTAT_DATA_INT64:		return kn->value.i64;
		case KSTAT_DATA_UINT64:		return kn->value.ui64;
# else
		case KSTAT_DATA_LONG:		return kn->value.l;
		case KSTAT_DATA_ULONG:		return kn->value.ul;
		case KSTAT_DATA_LONGLONG:	return kn->value.ll;
		case KSTAT_DATA_ULONGLONG:	return kn->value.ull;
# endif
		default:
			return (unsigned long long) -1;
	}
}   

# ifdef USE_UPTIME_KSTAT
int get_uptime()
{
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
}
# endif /*USE_UPTIME_KSTAT*/

# ifdef USE_CPU_KSTAT
int get_cpu_data(struct cpu_data * _cpu)
{
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
	return 0;
}
# endif /*USE_CPU_KSTAT*/

# ifdef USE_MEM_KSTAT
int get_mem_data(struct mem_data * _mem)
{
	kstat_t *ksp;
	kstat_named_t *kn;
	unsigned long long lv, ps;
	int cc, ncpu;
	cpu_stat_t cs;
	
	memset(_mem, 0, sizeof(struct mem_data));
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
	return get_swp_data(_mem);
}
# endif /*USE_MEM_KSTAT*/

# ifdef USE_NET_KSTAT
int get_net_data(const char * _dev, struct net_data * _data)
{
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
	return 0;
}
# endif /*NET_USE_KSTAT*/

#endif /*HAVE_LIBKSTAT*/
