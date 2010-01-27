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

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#ifdef HAVE_KVM_H
# include <kvm.h>
#endif

#ifdef HAVE_SYS_VMMETER_H
# include <sys/vmmeter.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#include "system.h"

#ifdef HAVE_LIBKVM

# ifdef USE_MEM_KVM
int get_mem_data(struct mem_data * _mem)
{
	kvm_t *kd;
	size_t len;
	double kbpp;
	struct vmmeter sum;
	struct kvm_swap swap[1];
	static int first_time = 1;

	struct nlist nl[] = {
		{ "_cnt" },
		{ NULL }
	};

	_mem->t = _mem->f = _mem->a = _mem->i = _mem->c = _mem->swi = _mem->swo = _mem->swt = 0;

	if ((kd = kvm_open(NULL, NULL, NULL, O_RDONLY, 
			first_time ? "kvm_open()" : NULL)) != NULL)
	{
		/* get virtual memory data */
		if (kvm_nlist(kd, nl) == -1)
		{
			fprintf(stderr, "kvm_nlist(): %s\n", strerror(errno));
			kvm_close(kd);
			return -1;
		}

		len = sizeof(sum);

		if (kvm_read(kd, nl[0].n_value, &sum, len) == -1)
		{
			fprintf(stderr, "kvm_read(): %s\n", strerror(errno));
			kvm_close(kd);
			return -1;
		}

		/* kilo bytes per page */
		kbpp = sum.v_page_size / 1024;

		_mem->t = sum.v_page_count * kbpp;
		_mem->f = sum.v_free_count * kbpp;
		_mem->a = sum.v_active_count * kbpp;
		_mem->i = sum.v_inactive_count * kbpp;
		_mem->c = sum.v_cache_count * kbpp;

		_mem->swi = sum.v_swappgsin;
		_mem->swo = sum.v_swappgsout;

		if (kvm_getswapinfo(kd, swap, 1, 0) == -1)
		{
			fprintf(stderr, "kvm_getswapinfo(): %s\n", strerror(errno));
			kvm_close(kd);
			return -1;
		}

		_mem->swt = swap[0].ksw_total * kbpp;

		kvm_close(kd);
	}

	first_time = 0;		/* don't warn on failures on subsequent calls */

	return 0;
}
# endif /*USE_MEM_KVM*/

#endif /*HAVE_LIBKVM*/
