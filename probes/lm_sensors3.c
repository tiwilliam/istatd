/*
 *  Copyright 2010 William Tisäter. All rights reserved.
 *  Copyright 2010 Guntram Blohm.
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

#ifdef HAVE_SENSORS_SENSORS_H
# include <sensors/sensors.h>
#endif

#ifdef HAVE_LIBSENSORS
#if SENSORS_API_VERSION < 0x0400 /* libsensor 3 and earlier */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "system.h"

unsigned int get_sensor_data(unsigned int _id, struct sensor_data *_data)
{
	int a, b, c, num;
	const sensors_chip_name * chip;
	const sensors_feature_data * features;

	a = num = 0;
	_data->kind = -1;

	while ((chip = sensors_get_detected_chips(&a)))
	{
		b = c = 0;

		while ((features = sensors_get_all_features(*chip, &b, &c)))
		{
			if (!memcmp(features->name, "fan", 3) && features->name[4]=='\0')
			{
				if (_id == num)
				{
					_data->id = _id;
					_data->chip = chip->addr;
					_data->sensor = features->number;
					_data->kind = SENSOR_FAN;
					sensors_get_label(*chip, _data->sensor, &_data->label);
					sensors_get_feature(*chip, _data->sensor, &_data->data);
				}

				num++;
			}

			if (!memcmp(features->name, "temp", 3) && features->name[5]=='\0')
			{
				if (_id == num)
				{
					_data->id = _id;
					_data->chip = chip->addr;
					_data->sensor = features->number;
					_data->kind = SENSOR_TEMP;
					sensors_get_label(*chip, _data->sensor, &_data->label);
					sensors_get_feature(*chip, _data->sensor, &_data->data);
				}

				num++;
			}
		}
	}

	return 0;
}

unsigned int get_sensor_num(void)
{
	int a, b, c, num;
	const sensors_chip_name *chip;
	const sensors_feature_data *features;
	FILE *fp;

	if ((fp = fopen("/etc/sensors.conf", "r")) == NULL)
		return 0;
	if (sensors_init(fp) != 0) {
		fclose(fp);
		return 0;
	}

	a = num = 0;

	while ((chip = sensors_get_detected_chips(&a)))
	{
		b = c = 0;

		while ((features = sensors_get_all_features(*chip, &b, &c)))
		{
			if (!memcmp(features->name, "fan", 3) && features->name[4]=='\0')
				num++;

			if (!memcmp(features->name, "temp", 3) && features->name[5]=='\0')
				num++;
		}
	}

	return num;
}

#endif
#endif
