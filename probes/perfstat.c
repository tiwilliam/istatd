/*
 *  Copyright 2009 William Tisäter. All rights reserved.
 *  Copyright 2009 Guntram Blohm
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

#ifdef HAVE_LIBPERFSTAT_H
# include <libperfstat.h>
# include <sys/proc.h>
#endif

#include "system.h"

#ifdef HAVE_LIBPERFSTAT

# ifdef USE_UPTIME_PERFSTAT
int get_uptime()
{
	// TODO - perfstat_cpu_total(), calc (user+sys+idle+wait)*ncpus*HZ ??
	return 0;

}
# endif /*USE_UPTIME_PERFSTAT*/

# ifdef USE_CPU_PERFSTAT
int get_cpu_data(struct cpu_data * _cpu)
{
	perfstat_cpu_total_t cpustats;
	perfstat_cpu_total(NULL, &cpustats, sizeof cpustats, 1);
	_cpu->u = cpustats.user;
	_cpu->s = cpustats.sys;
	_cpu->i = cpustats.idle;
	_cpu->n = 0;		/* AIX doesn't keep nice anywhere. */
	return 0;
}
# endif /*USE_CPU_PERFSTAT*/

# ifdef USE_LOAD_PERFSTAT
int get_avg_data(struct cpu_data * _cpu)
{
	perfstat_cpu_total_t cpustats;
	perfstat_cpu_total(NULL, &cpustats, sizeof cpustats, 1);
	_cpu->one = cpustats.loadavg[0]/(float)(1<<SBITS);
	_cpu->two = cpustats.loadavg[1]/(float)(1<<SBITS);
	_cpu->three = cpustats.loadavg[2]/(float)(1<<SBITS);
	return 0;
}
# endif /*USE_LOAD_PERFSTAT*/

# ifdef USE_MEM_PERFSTAT
int get_mem_data(struct mem_data * _mem)
{
	perfstat_memory_total_t meminfo;
	int ps;

	ps=getpagesize();
	perfstat_memory_total(NULL, &meminfo, sizeof meminfo, 1);
	_mem->t   = ps * meminfo.real_total	/ 1024;
	_mem->c   = ps * meminfo.real_pinned	/ 1024;
	_mem->f   = ps * meminfo.real_free	/ 1024;
	_mem->swi = ps * meminfo.pgspins		/ 1024;
	_mem->swo = ps * meminfo.pgspouts		/ 1024;
	_mem->swt = ps * meminfo.pgsp_total	/ 1024;
	_mem->swu = ps * (meminfo.pgsp_total-
			  meminfo.pgsp_free)	/ 1024;

	_mem->a = _mem->t - _mem->c - _mem->f;

	return 0;
}
# endif /*USE_MEM_PERFSTAT*/

# ifdef USE_NET_PERFSTAT
int get_net_data(const char * _dev, struct net_data * _data)
{
	perfstat_netinterface_total_t ninfo;
	perfstat_netinterface_total(NULL, &ninfo, sizeof ninfo, 1);

	_data->s = ninfo.obytes;
	_data->r = ninfo.ibytes;
	return 0;
}
# endif /*NET_USE_PERFSTAT*/

#endif /*HAVE_LIBPERFSTAT*/
