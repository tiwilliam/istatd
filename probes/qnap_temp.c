/*
 *  Copyright 2010 Guntram Blohm. All rights reserved.
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
 *  DISCLAIMED. IN NO EVENT SHALL GUNTRAM BLOHM BE LIABLE FOR ANY
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

#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#ifdef	HAVE_QNAPTEMP
char *systempfile="/proc/tsinfo/systemp";
static FILE *systempfp;

unsigned int get_qnaptemp(unsigned int _id, struct sensor_data *_data)
{
	int systemp;

	_data->id = _id;
	_data->chip = 0;
	_data->sensor = 0;
	_data->label = strdup("SysTemp");
	_data->kind = SENSOR_TEMP;
	if (_id != 0 || ((systempfp=fopen(systempfile, "r")) == NULL))
		systemp=-1;
	else  {
		fseek(systempfp, 0l, 0);
		if (fscanf(systempfp, "%d", &systemp)!=1)
			systemp=-1;
		fclose(systempfp);
	}
	_data->data=systemp;
	return 0;
}

unsigned int have_qnaptemp(void)
{
	int  temp;

	if ((systempfp=fopen(systempfile, "r"))==NULL) {
		fprintf(stderr, "%s: Error %d\n", systempfile, errno);
		return 0;
	}
	if (fscanf(systempfp, "%d", &temp)!=1) {
		fprintf(stderr, "can't read an integer from %s\n", systempfile);
		fclose(systempfp);
		return 0;
	}
	fclose(systempfp);
	return 1;
}
#endif
