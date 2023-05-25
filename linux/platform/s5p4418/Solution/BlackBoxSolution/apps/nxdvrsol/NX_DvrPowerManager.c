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
#include <sys/time.h>

#include <nx_adc.h>

#include "NX_DvrPowerManager.h"

// Vref * ADC / 4096 * 1000 = DC * R1 / (R1 + R2) * 1000

#define VREF						1800	// mV
#define ADC_MAX						4096	// digit
#define R1							10		// Kohm
#define R2							100		// Kohm

#define POWER_WARNNING				11000	// mV
#define POWER_CRITICAL				10000	// mV

#define POWER_WARNING_CHECKTIME 	10000	// 10sec
#define POWER_CRITICAL_CHECKTIME	2000	// 2sec

#define TEMP_WARNNING				1580	// mV ( 80 )
#define TEMP_CRITICAL				1690	// mV ( 100 )

#define TEMP_WARNING_CHECKTIME		10000	// 10sec
#define TEMP_CRITICAL_CHECKTIME		2000	// 2sec

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private Function
//
static uint64_t DvrGetTime( void )		// return value ms
{
	uint64_t ret;
	struct timeval	tv;
	gettimeofday( &tv, NULL );
	ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
	return ret;
}

void *DvrPowerManagerThread( void *arg )
{
	POWER_MANAGER_HANDLE hManager = (POWER_MANAGER_HANDLE)arg;
	CMD_MESSAGE cmd;

	NX_ADC_HANDLE hADC0 = NX_AdcInit( NX_ADC0 );	// Temperature
	NX_ADC_HANDLE hADC1 = NX_AdcInit( NX_ADC1 );	// voltage detect

	int32_t adc0Value, voltage;
	int32_t adc1Value, temperature;

	uint64_t curTime = DvrGetTime();
	uint64_t TempWarningTime		= curTime + TEMP_WARNING_CHECKTIME;
	uint64_t TempCriticalTime		= curTime + TEMP_CRITICAL_CHECKTIME;
	uint64_t VoltageWarningTime		= curTime + POWER_WARNING_CHECKTIME;
	uint64_t VoltageCriticalTime	= curTime + POWER_CRITICAL_CHECKTIME;

	int32_t bPowerNotify = false;
	int32_t bTempNotify = false;
	
	while( hManager->bThreadRun )
	{
		// a. Read ADC value. (temperature, power)
		adc0Value = NX_AdcRead( hADC0 );
		usleep(250000);		// ADC stable time
		adc1Value = NX_AdcRead( hADC1 );
		usleep(250000);		// ADC stable time

		// b. calculate temperature, voltage
		temperature	= (int32_t)( (double)VREF / (double)ADC_MAX * (double)adc0Value );
		voltage 	= (int32_t)( (double)VREF * ((double)adc1Value / (double)ADC_MAX) * ((double)(R1 + R2) / (double)R1) );

		// c. set value
		pthread_mutex_lock( &hManager->hLock );
		hManager->temperature = temperature;
		hManager->power = voltage;
		pthread_mutex_unlock( &hManager->hLock );

		// d. power exception case. ( detect temperature and power value )
		curTime = DvrGetTime();

		// d-1. check temperature
		if( temperature > TEMP_WARNNING && temperature < TEMP_CRITICAL ) {
			if( curTime > TempWarningTime ) {
				if( !bTempNotify ) {
					bTempNotify = true;
					memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
					cmd.cmdType = CMD_TYPE_HIGH_TEMP;
					if( hManager->hCmd ) DvrCmdQueuePush( hManager->hCmd, &cmd );					
				}
			}
			else {
				printf("%s(): Detect Temperature Warning. (adc0 = %d, temperature = %d mV)\n", __func__, adc0Value, temperature);
			}
		}
		else if( temperature > TEMP_CRITICAL ) {
			if( curTime > TempCriticalTime ) {
					if( !bTempNotify ) {
					bTempNotify = true;
					memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
					cmd.cmdType = CMD_TYPE_HIGH_TEMP;
					if( hManager->hCmd ) DvrCmdQueuePush( hManager->hCmd, &cmd );					
				}
			}
			else {
				printf("%s(): Detect Temperature Critical. (adc0 = %d, temperature = %d mV)\n", __func__, adc0Value, temperature);
			}
		}
		else {
			bTempNotify = false;
			TempWarningTime 	= curTime + POWER_WARNING_CHECKTIME;
			TempCriticalTime	= curTime + POWER_CRITICAL_CHECKTIME;
		}
		
		// d-2. check power
		if( voltage > POWER_CRITICAL && voltage < POWER_WARNNING ) {
			if( curTime > VoltageWarningTime ) {
				if( !bPowerNotify ) {
					bPowerNotify = true;
					memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
					cmd.cmdType = CMD_TYPE_LOW_VOLTAGE;
					if( hManager->hCmd ) DvrCmdQueuePush( hManager->hCmd, &cmd );
				}
			}
			else {
				printf("%s(): Detect Voltage Warning. (adc1 = %d, voltage = %d mV)\n", __func__, adc1Value, voltage);
			}
		}
		else if( voltage < POWER_CRITICAL ) {
			if( curTime > VoltageCriticalTime ) {
				if( !bPowerNotify ) {
					bPowerNotify = true;
					memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
					cmd.cmdType = CMD_TYPE_LOW_VOLTAGE;
					if( hManager->hCmd ) DvrCmdQueuePush( hManager->hCmd, &cmd );
				}
			}
			else {
				printf("%s(): Detect Voltage Critical. (adc1 = %d, voltage = %d mV)\n", __func__, adc1Value, voltage);
			}
		}
		else {
			bPowerNotify = false;
			VoltageWarningTime	= curTime + TEMP_WARNING_CHECKTIME;
			VoltageCriticalTime	= curTime + POWER_CRITICAL_CHECKTIME;
		}
	}

	if( hADC0 ) NX_AdcDeinit( hADC0 );
	if( hADC1 )	NX_AdcDeinit( hADC1 );

	return (void*)0xDEADDEAD;
}

void *DvrPowerIntManagerThread( void *arg )
{
	POWER_MANAGER_HANDLE hManager = (POWER_MANAGER_HANDLE)arg;
	CMD_MESSAGE cmd;

	int32_t bIntrruptNotify = false;

	while( hManager->bThreadRun )
	{
		if( !NX_GpioGetInterrupt( hManager->hInterrupt ) ) {
			if( !bIntrruptNotify ) {
				bIntrruptNotify = true;
				memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
				cmd.cmdType = CMD_TYPE_MICOM_INT;
				if( hManager->hCmd ) DvrCmdQueuePush( hManager->hCmd, &cmd );
			}
		}
		else {
			bIntrruptNotify = false;
		}
	}

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
	hManager->hInterrupt	= NX_GpioInit( GPIOB26 );

	NX_GpioDirection( hManager->hInterrupt, GPIO_DIRECTION_IN );
	NX_GPioSetEdge( hManager->hInterrupt, GPIO_EDGE_FALLING );

	pthread_mutex_init( &hManager->hLock, NULL );

	return hManager;
}

void DvrPowerManagerDeinit( POWER_MANAGER_HANDLE hManager )
{
	assert( hManager );
	pthread_mutex_destroy( &hManager->hLock );

	NX_GpioDeinit( hManager->hInterrupt );
	
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

	if( 0 > pthread_create( &hManager->hThreadInterrupt, NULL, &DvrPowerIntManagerThread, (void*)hManager) ) {
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
	NX_GpioPostInterrupt( hManager->hInterrupt );
	pthread_join( hManager->hThread, NULL );
	pthread_join( hManager->hThreadInterrupt, NULL );
	
	return 0;
}

int32_t DvrPowerManagerRegCmd( POWER_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	hManager->hCmd = hCmd;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrPowerGetData( POWER_MANAGER_HANDLE hManager, int32_t *pPower, int32_t *pTemperature )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	*pPower			= hManager->power;
	*pTemperature	= hManager->temperature;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}