//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		: 
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <nx_adc.h>

#define MAX_SAMPLE_COUNT		4

#define DEBUG_READ_ADC0 "cat /sys/devices/platform/nxp-adc/iio:device0/in_voltage0_raw"

int main(void)
{
	NX_ADC_HANDLE	hAdc;
	
	int32_t i;
	int32_t value[MAX_SAMPLE_COUNT];
	int64_t sum = 0;

	int32_t value_min, value_max;
	int32_t temp_min, temp_max;

	hAdc = NX_AdcInit(0);

	for(i = 0; i < MAX_SAMPLE_COUNT; i++)
	{
		//system(DEBUG_READ_ADC0);
		value[i] = NX_AdcRead( hAdc );
		usleep(100000);		// 100mSec
	}

	temp_min = value[0];
	temp_max = value[0];

	value_min = 0;
	value_max = 0;

	// simple search error value( drop min/max value)
	for(i = 1; i < MAX_SAMPLE_COUNT; i++) {
		// search max_value
		if( temp_max < value[i] ) {
			temp_max = value[i];
			value_max = i;
		}

		// search min_value
		if( temp_min > value[i] ) {
			temp_min = value[i];
			value_min = i;
		}
	}

	// average
	for(i = 0; i < MAX_SAMPLE_COUNT; i++) {
		if(i == value_max || i == value_min) 
			continue;
		
		sum += value[i];
	}
	sum /= (MAX_SAMPLE_COUNT - 2);

	//NX_AdcDeinit( hAdc );
	
	// Message 
	printf("Read Value\n");
	for(i = 0; i < MAX_SAMPLE_COUNT; i++) {
		printf("value[%d] = %d\n", i, value[i]);
	}

	printf("\nError Value\n");
	printf("[MIN] cnt = %d, value = %d\n", value_min, value[value_min]);
	printf("[MAX] cnt = %d, value = %d\n", value_max, value[value_max]);

	printf("\nCorretion Value = %lld\n", sum);

	return 0;
}
