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

#include <NX_DvrControl.h>

#include "NX_DvrGsensorManager.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private Function
//
#define MOTION_THRESHOLD_X			100
#define MOTION_THRESHOLD_Y			100
#define MOTION_THRESHOLD_Z			100

#define CHECK_NO_SENSOR_TIME		120 * (1000)		// 60sec

static uint64_t DvrGetTime( void )		// return value ms
{
	uint64_t ret;
	struct timeval	tv;
	gettimeofday( &tv, NULL );
	ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
	return ret;
}

void *DvrGsensorManagerThread( void *arg )
{
	GSENSOR_MANAGER_HANDLE hManager = (GSENSOR_MANAGER_HANDLE)arg;
	
	CMD_MESSAGE cmd;
	GSENSOR_VALUE value;

	int32_t prev_x, prev_y, prev_z;	
	int32_t diff_x, diff_y, diff_z;

	uint64_t curTime, targetTime;

	// Dummy read.
	DvrGsensorGetData( hManager, &value );
	usleep(100000);

	DvrGsensorGetData( hManager, &value );
	diff_x = 0;
	diff_y = 0;
	diff_z = 0;

	prev_x = value.x;
	prev_y = value.y;
	prev_z = value.z;

	usleep(100000);

	curTime		= DvrGetTime();
	targetTime	= curTime + CHECK_NO_SENSOR_TIME;

	while( hManager->bThreadRun )
	{
		DvrGsensorGetData( hManager, &value );
		
		diff_x = value.x - prev_x;
		diff_y = value.y - prev_y;
		diff_z = value.z - prev_z;

		if( diff_x < 0 ) diff_x = -diff_x;
		if( diff_y < 0 ) diff_y = -diff_y;
		if( diff_z < 0 ) diff_z = -diff_z;

		prev_x = value.x;
		prev_y = value.y;
		prev_z = value.z;

		pthread_mutex_lock( &hManager->hLock );
		
		curTime = DvrGetTime();

		switch( hManager->nDvrMode )
		{
			case DVR_MODE_NORMAL :
				// Motion Detection Action
				if( hManager->bEnableMotion )
				{
					if( diff_x > MOTION_THRESHOLD_X || diff_y > MOTION_THRESHOLD_Y || diff_z > MOTION_THRESHOLD_Z )
					{
						targetTime	= curTime + CHECK_NO_SENSOR_TIME;
					}
					else
					{
						if( curTime > targetTime )
						{
							printf("%s(): Enter the motion detection mode.\n", __func__);
							hManager->nDvrMode = DVR_MODE_MOTION;

							cmd.cmdType = CMD_TYPE_CHG_MODE;
							cmd.cmdData = DVR_ENCODE_MOTION;
							DvrCmdQueuePush( hManager->hCmd, &cmd );
						}
					}
				}

				// Event Action
				if( diff_x > hManager->threshold.x || diff_y > hManager->threshold.y || diff_z > hManager->threshold.z )
				{
					hManager->nDvrMode = DVR_MODE_EVENT;
					printf("%s(): diff( %d, %d, %d ), threshold( %d, %d, %d )\n", 
						__func__,
						diff_x, diff_y, diff_z,
						hManager->threshold.x, hManager->threshold.y, hManager->threshold.z
					);
					cmd.cmdType = CMD_TYPE_EVENT;
					DvrCmdQueuePush( hManager->hCmd, &cmd );
				}
				break;
			case DVR_MODE_EVENT :
				break;
			case DVR_MODE_MOTION :
				if( hManager->bEnableMotion )
				{
					if( diff_x > MOTION_THRESHOLD_X || diff_y > MOTION_THRESHOLD_Y || diff_z > MOTION_THRESHOLD_Z )
					{
						printf("%s(): Enter the normal mode.\n", __func__);
						targetTime	= curTime + CHECK_NO_SENSOR_TIME;
						hManager->nDvrMode = DVR_MODE_NORMAL;
						
						cmd.cmdType = CMD_TYPE_CHG_MODE;
						cmd.cmdData = DVR_ENCODE_NORMAL;
						DvrCmdQueuePush( hManager->hCmd, &cmd );
					}
				}
				break;
		}
		pthread_mutex_unlock( &hManager->hLock );

		usleep(100000);
	}

	return (void*)0xDEADDEAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//
#define GSENSOR_DEF_THRESHOLD_X			500
#define GSENSOR_DEF_THRESHOLD_Y			500
#define GSENSOR_DEF_THRESHOLD_Z			500

GSENSOR_MANAGER_HANDLE DvrGsensorManagerInit( void )
{
	GSENSOR_HANDLE hGsensor = DvrGsensorOpen( "/dev/accel" );
	GSENSOR_MANAGER_HANDLE hManager = NULL;
	
	if( !hGsensor ) return NULL;
	
	hManager = (GSENSOR_MANAGER_HANDLE)malloc( sizeof(GSENSOR_MANAGER_INFO) );
	memset( hManager, 0x00, sizeof(GSENSOR_MANAGER_HANDLE) );

	hManager->hThread		= 0;
	hManager->bThreadRun	= 0;
	hManager->hGsensor		= hGsensor;
	
	hManager->value.x		= 0;
	hManager->value.y		= 0;
	hManager->value.z		= 0;

	hManager->nDvrMode		= DVR_MODE_NORMAL;
	hManager->bEnableMotion	= true;

	hManager->threshold.x 	= GSENSOR_DEF_THRESHOLD_X;
	hManager->threshold.y 	= GSENSOR_DEF_THRESHOLD_Y;
	hManager->threshold.z 	= GSENSOR_DEF_THRESHOLD_Z;
	pthread_mutex_init( &hManager->hLock, NULL );

	return hManager;
}

void DvrGsensorManagerDeinit( GSENSOR_MANAGER_HANDLE hManager )
{
	assert( hManager );
	DvrGsensorClose( hManager->hGsensor );
	pthread_mutex_destroy( &hManager->hLock );

	if( hManager ) free( hManager );
}

int32_t DvrGsensorManagerStart( GSENSOR_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( hManager->bThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	hManager->bThreadRun = true;
	if( 0 > pthread_create( &hManager->hThread, NULL, &DvrGsensorManagerThread, (void*)hManager) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

int32_t DvrGsensorManagerStop( GSENSOR_MANAGER_HANDLE hManager )
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

int32_t DvrGsensorManagerRegCmd( GSENSOR_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	hManager->hCmd = hCmd;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrGsensorManagerSetStatus( GSENSOR_MANAGER_HANDLE hManager, int32_t nDvrMode )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	hManager->nDvrMode = nDvrMode;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrGsensorManagerMotionEnable( GSENSOR_MANAGER_HANDLE hManager, int32_t bEnable )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	
	hManager->bEnableMotion = bEnable;

	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrGsensorManagerSetThreshold( GSENSOR_MANAGER_HANDLE hManager, GSENSOR_VALUE threshold )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	
	hManager->threshold.x = threshold.x;
	hManager->threshold.y = threshold.y;
	hManager->threshold.z = threshold.z;

	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrGsensorGetData( GSENSOR_MANAGER_HANDLE hManager, GSENSOR_VALUE *pValue )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );

#if(1)
	DvrGsensorValue( hManager->hGsensor, &hManager->value );

	pValue->x = hManager->value.x;
	pValue->y = hManager->value.y;
	pValue->z = hManager->value.z;

#else
	#include <time.h>

	srand((unsigned int)time(NULL));
	pValue->x = rand() % 30 + 1;
	pValue->y = rand() % 30 + 1;
	pValue->z = rand() % 30 + 1;
#endif
	pthread_mutex_unlock( &hManager->hLock );
	return 0;
}

