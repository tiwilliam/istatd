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

#include <stdio.h>
#include <string.h>

#ifdef HAVE_ERRNO_H
# include "errno.h"
#endif

#include "fslabel.h"

int
main(int argc, char **argv)
{
	int c, s, r;
	fslabel_t label;
	
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s DEVICE [DEVICE...]\n", argv[0]);
		return 1;
	}
	for(c = 1, s = 0; c < argc; c++)
	{
		if(1 == (r = fslabel_identify(argv[c], &label)))
		{
			printf("%s: %s {%s} (%s)\n", argv[c], label.label, label.uuid, label.fstype);
			s++;
		}
		else if(0 == r)
		{
			fprintf(stderr, "%s: cannot identify filesystem\n", argv[c]);
		}
		else
		{
			fprintf(stderr, "%s: %s\n", argv[c], strerror(errno));
		}
	}
	return (s ? 0 : 1);
}
