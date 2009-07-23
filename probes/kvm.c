
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

	struct nlist nl[] = {
		{ "_cnt" },
		{ NULL }
	};

	_mem->t = _mem->f = _mem->a = _mem->i = _mem->c = _mem->swi = _mem->swo = _mem->swt = 0;

	if ((kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open()")) != NULL)
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
	}

	kvm_close(kd);

	return 0;
}
# endif /*USE_MEM_KVM*/

#endif /*HAVE_LIBKVM*/
