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
#include <pthread.h>
#include <unistd.h>
#include <nx_gpio.h>

static pthread_t	hLedEventThread = 0;
static int32_t 		bLedEventThreadRun = false;

void *DvrLedEventThread( void *arg )
{
	int32_t value = false;

	NX_GPIO_HANDLE	hGPIOB28, hGPIOB29;

	hGPIOB28 = NX_GpioInit( GPIOB28 );
	hGPIOB29 = NX_GpioInit( GPIOB29 );

	NX_GpioDirection( hGPIOB28, GPIO_DIRECTION_OUT );
	NX_GpioDirection( hGPIOB29, GPIO_DIRECTION_OUT );

	while( bLedEventThreadRun )
	{
		value = !value;
		NX_GpioSetValue( hGPIOB28, value );
		NX_GpioSetValue( hGPIOB29, value );

		usleep(500000);
	}
	
	NX_GpioSetValue( hGPIOB28, false );
	NX_GpioSetValue( hGPIOB29, false );

	NX_GpioDeinit( hGPIOB28 );
	NX_GpioDeinit( hGPIOB29 );

	return (void*)0xDEADDEAD;
}

int32_t DvrLedEventStart( void )
{
	if( bLedEventThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	bLedEventThreadRun = true;
	if( 0 > pthread_create( &hLedEventThread, NULL, &DvrLedEventThread, NULL) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

int32_t DvrLedEventStop( void )
{
	if( !bLedEventThreadRun) {
		printf("%s(): Fail, Already stopping.\n", __func__);
		return -1;
	}

	bLedEventThreadRun = false;
	pthread_join( hLedEventThread, NULL );
	
	return 0;
}

static pthread_t	hLedMotionThread = 0;
static int32_t 		bLedMotionThreadRun = false;

void *DvrLedMotionThread( void *arg )
{
	int32_t value = false;

	NX_GPIO_HANDLE	hGPIOB27 = NX_GpioInit( GPIOB27 );
	NX_GpioDirection( hGPIOB27, GPIO_DIRECTION_OUT );

	while( bLedMotionThreadRun )
	{
		value = !value;
		NX_GpioSetValue( hGPIOB27, value );
		usleep(500000);
	}

	NX_GpioSetValue( hGPIOB27, false );
	NX_GpioDeinit( hGPIOB27 );

	return (void*)0xDEADDEAD;
}

int32_t DvrLedMotionStart( void )
{
	if( bLedMotionThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	bLedMotionThreadRun = true;
	if( 0 > pthread_create( &hLedMotionThread, NULL, &DvrLedMotionThread, NULL) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

int32_t DvrLedMotionStop( void )
{
	if( !bLedMotionThreadRun) {
		printf("%s(): Fail, Already stopping.\n", __func__);
		return -1;
	}

	bLedMotionThreadRun = false;
	pthread_join( hLedMotionThread, NULL );
	
	return 0;
}