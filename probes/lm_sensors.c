/*
 *  Copyright 2010 William Tisäter. All rights reserved.
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

#include <stdlib.h>

#ifdef HAVE_SENSORS_SENSORS_H
# include <sensors/sensors.h>
#endif

#include "system.h"

#ifdef HAVE_LIBSENSORS
unsigned int get_sensor_data(unsigned int _id, struct sensor_data *_data)
{
	int a, b, c, num;
	const sensors_chip_name * chip;
	const sensors_feature * features;
	const sensors_subfeature * subfeatures;
	
	a = num = 0;
	
	while ((chip = sensors_get_detected_chips(NULL, &a)))
	{
		b = 0;
		while ((features = sensors_get_features(chip, &b)))
		{
			c = 0;
			while ((subfeatures = sensors_get_all_subfeatures(chip, features, &c)))
			{
				if (subfeatures->type == SENSORS_SUBFEATURE_FAN_INPUT)
				{
					if (_id == num)
					{
						_data->id = _id;
						_data->chip = chip->addr;
						_data->sensor = features->number;
						_data->label = sensors_get_label(chip, features);
						_data->kind = SENSOR_FAN;
  						sensors_get_value(chip, subfeatures->number, &_data->data);
					}
					num++;
				}
					
				if (subfeatures->type == SENSORS_SUBFEATURE_TEMP_INPUT)
				{
					if (_id == num)
					{
						_data->id = _id;
						_data->chip = chip->addr;
						_data->sensor = features->number;
						_data->label = sensors_get_label(chip, features);
						_data->kind = SENSOR_TEMP;
  						sensors_get_value(chip, subfeatures->number, &_data->data);
					}
					num++;
				}
			}
		}
	}
	
	return 0;
}

unsigned int get_sensor_num(void)
{
	int a, b, c, num;
	const sensors_chip_name * chip;
	const sensors_feature * features;
	const sensors_subfeature * subfeatures;
	
	sensors_init(NULL);
	
	a = num = 0;
	while ((chip = sensors_get_detected_chips(NULL, &a)))
	{
		b = 0;
		while ((features = sensors_get_features(chip, &b)))
		{
			c = 0;
			while ((subfeatures = sensors_get_all_subfeatures(chip, features, &c)))
			{
				if (subfeatures->type == SENSORS_SUBFEATURE_FAN_INPUT)
				{
					// chip = chip->addr
					// sensor = features->number
					num++; break;
				}
					
				if (subfeatures->type != SENSORS_SUBFEATURE_TEMP_INPUT)
				{
					num++; break;
				}
			}
		}
	}
	
	return num;
}
#endif
