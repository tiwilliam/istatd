/* Copyright (c) 2009 Mo McRoberts.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the author(s) of this software may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * AUTHORS OF THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef P_FSLABEL_H_
# define P_FSLABEL_H_                  1

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <fcntl.h>
# include <unistd.h>

# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif

# ifdef HAVE_STDINT_H
#  include <stdint.h>
# endif

# ifdef HAVE_ERRNO_H
#  include <errno.h>
# endif

# ifdef HAVE_ARPA_INET_H
#  include <netinet/in.h>
#  include <arpa/inet.h>
# endif

# include "fslabel.h"

# ifndef O_BINARY
#  define O_BINARY                     0
# endif

typedef struct fslabel_src_struct fslabel_src_t;

struct fslabel_src_struct
{
	int fd;
	uint8_t *buf;
	uint8_t *bufp;
	size_t buflen;
};

typedef int (*fslabel_probe_t)(fslabel_src_t *src, fslabel_t *dest);

char *fslabel__strlcpy(char *dest, const char *src, size_t maxlen);

int fslabel__src_read(fslabel_src_t *src);
int fslabel__src_seek(fslabel_src_t *src, size_t nbytes);
int fslabel__src_readbuf(fslabel_src_t *src, void *dest, size_t nbytes);

int fslabel__ext_probe(fslabel_src_t *src, fslabel_t *dest);


#endif /*!P_FSLABEL_H_ */

