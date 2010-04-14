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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#elif defined(HAVE_SYS_TIME_H)
# include <sys/time.h>
#else
# include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#ifdef HAVE_NET_IF_H
# include <net/if.h>
#endif

#ifdef HAVE_NET_IF_MIB_H
# include <net/if_mib.h>
#endif

#include "system.h"

#ifdef HAVE_SYSCTL

# ifdef USE_CPU_SYSCTL
int get_cpu_data(struct cpu_data * _cpu)
{
	size_t len;
        long cp_time[5];

	_cpu->u = _cpu->n = _cpu->s = _cpu->i = 0;

        len = sizeof(cp_time);

        if (sysctlbyname("kern.cp_time", cp_time, &len, NULL, 0) < 0)
	{
                perror("kern.cp_time()");
		return -1;
        }

        _cpu->u = (unsigned int) cp_time[0];
        _cpu->n = (unsigned int) cp_time[1];
        _cpu->s = (unsigned int) cp_time[2];
        _cpu->i = (unsigned int) cp_time[4];

	return 0;
}
# endif /*USE_CPU_SYSCTL*/

# ifdef USE_NET_SYSCTL
int get_ifcount(void)
{
	size_t len;
	int name[5], count;

	name[0] = CTL_NET;
	name[1] = PF_LINK;
	name[2] = NETLINK_GENERIC;
	name[3] = IFMIB_SYSTEM;
	name[4] = IFMIB_IFCOUNT;

	len = sizeof(int);

	sysctl(name, 5, &count, &len, NULL, 0);

	return count;
}

int get_net_data(const char * _dev, struct net_data * _data)
{
        int i;
	int name[6];
	size_t len;
	struct ifmibdata ifmd;
	int ifcount = get_ifcount();
	int found_device = 0;

	len = sizeof(ifmd);

	name[0] = CTL_NET;
	name[1] = PF_LINK;
	name[2] = NETLINK_GENERIC;
	name[3] = IFMIB_IFDATA;
	name[5] = IFDATA_GENERAL;

	for (i = 1; i <= ifcount; i++)
	{
		name[4] = i;

		sysctl(name, 6, &ifmd, &len, (void *) 0, 0);

		if (!strcmp(ifmd.ifmd_name, _dev))
		{
			_data->s = ifmd.ifmd_data.ifi_obytes;
			_data->r = ifmd.ifmd_data.ifi_ibytes;

			found_device = 1;

			break;
		}
	}

	if (!found_device || ifcount == 0) return -1;

	return 0;
}
# endif /*NET_USE_SYSCTL*/

#endif /*HAVE_SYSCTL*/

#ifdef USE_UPTIME_GETTIME
int get_uptime()
{
	struct timespec tp;

	tp.tv_sec = 0;
        tp.tv_nsec = 0;

        clock_gettime(CLOCK_UPTIME, &tp);

        return tp.tv_sec;
}
#endif /*USE_UPTIME_GETTIME*/

#ifdef USE_UPTIME_SYSCTL
int
get_uptime(void)
{
	struct timeval tm;
	time_t now;
	int mib[2];
	size_t size;
	
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(tm);
	now = time(NULL);
	if(-1 != sysctl(mib, 2, &tm, &size, NULL, 0) && 0 != tm.tv_sec)
		{
			return now - tm.tv_sec;
		}
	return 0;
}
#endif /* USE_UPTIME_SYSCTL */
