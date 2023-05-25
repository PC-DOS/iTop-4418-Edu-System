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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <nx_adc.h>

#include "NX_DvrPowerManager.h"

// Vref * ADC / 4096 * 1000 = DC * R1 / (R1 + R2) * 1000

#define VREF			1800	// mV
#define ADC_MAX			4096	// digit
#define R1				10		// Kohm
#define R2				100		// Kohm

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private Function
//
void *DvrPowerManagerThread( void *arg )
{
	POWER_MANAGER_HANDLE hManager = (POWER_MANAGER_HANDLE)arg;
	int32_t adcValue;

	NX_ADC_HANDLE hAdc = NX_AdcInit( NX_ADC1 );

	while( hManager->bThreadRun )
	{
		pthread_mutex_lock( &hManager->hLock );
		
		// Power Read Action
		// 
		adcValue = NX_AdcRead( hAdc );
		hManager->value = (int32_t)((double)VREF * ((double)adcValue / (double)ADC_MAX) * ((double)(R1 + R2) / (double)R1));

		pthread_mutex_unlock( &hManager->hLock );
		usleep(500000);
	}

	NX_AdcDeinit( hAdc );

	return (void*)0xDEADDEAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//
POWER_MANAGER_HANDLE DvrPowerManagerInit( void )
{
	POWER_MANAGER_HANDLE hManager = (POWER_MANAGER_HANDLE)malloc( sizeof(POWER_MANAGER_INFO) );
	memset( hManager, 0x00, sizeof(POWER_MANAGER_INFO) );

	hManager->hThread		= 0;
	hManager->bThreadRun	= 0;
	pthread_mutex_init( &hManager->hLock, NULL );

	return hManager;
}

void DvrPowerManagerDeinit( POWER_MANAGER_HANDLE hManager )
{
	assert( hManager );
	pthread_mutex_destroy( &hManager->hLock );

	if( hManager ) free( hManager );
}

int32_t DvrPowerManagerStart( POWER_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( hManager->bThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	hManager->bThreadRun = true;
	if( 0 > pthread_create( &hManager->hThread, NULL, &DvrPowerManagerThread, (void*)hManager) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

int32_t DvrPowerManagerStop( POWER_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( !hManager->bThreadRun) {
		printf("%s(): Fail, Already stopping.\n", __func__);
		return -1;
	}

	hManager->bThreadRun = false;
	pthread_join( hManager->hThread, NULL );
	
	return 0;
}

int32_t DvrPowerGetData( POWER_MANAGER_HANDLE hManager, int32_t *pValue )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	*pValue = hManager->value;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}