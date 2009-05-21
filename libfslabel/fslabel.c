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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "p_fslabel.h"

static fslabel_probe_t probes[] = {
	fslabel__ext_probe,
	NULL
};

int
fslabel_identify(const char *path, fslabel_t *fslabel)
{
	int fd, cr, r;
	
	do
	{
		fd = open(path, O_RDONLY|O_BINARY);
	}
	while(-1 == fd && EINTR == errno);
	if(-1 == fd)
	{
		return -1;
	}
	r = fslabel_identify_fd(fd, fslabel);
	do
	{
		cr = close(fd);
	}
	while(-1 == cr && EINTR == errno);
	return r;
}

int
fslabel_identify_fd(int fd, fslabel_t *fslabel)
{
	int r;
	size_t c;
	uint8_t buf[4096];
	fslabel_src_t src;
	
	src.fd = fd;
	src.buf = buf;
	src.buflen = sizeof(buf);
	if(-1 == fslabel__src_read(&src))
	{
		return -1;
	}
	for(c = 0; NULL != probes[c]; c++)
	{
		src.bufp = buf;
		switch((r = probes[c](&src, fslabel)))
		{
			case 0:
				/* No match */
				break;
			case 1:
				/* Matched */
				return 1;
			case -1:
				/* Failed */
				return -1;
		}
	}
	/* Nothing matched */
	return 0;
}

char *
fslabel__strlcpy(char *dest, const char *src, size_t destlen)
{
	strncpy(dest, src, destlen);
	dest[destlen - 1] = 0;
	return dest;
}

int
fslabel__src_read(fslabel_src_t *src)
{
	ssize_t r;
	
	do
	{
		r = read(src->fd, src->buf, src->buflen);
	}
	while(-1 == r && EINTR == errno);
	if(r == -1)
	{
		return -1;
	}
	src->buflen = r;
	return 0;
}

int
fslabel__src_seek(fslabel_src_t *src, size_t nbytes)
{
	if(nbytes > src->buflen - (src->bufp - src->buf))
	{
		errno = E2BIG;
		return -1;
	}
	src->bufp += nbytes;
	return 0;
}

int
fslabel__src_readbuf(fslabel_src_t *src, void *dest, size_t nbytes)
{
	if(nbytes > src->buflen - (src->bufp - src->buf))
	{
		errno = E2BIG;
		return -1;
	}
	memcpy(dest, src->bufp, nbytes);
	src->bufp += nbytes;
	return 0;
}
