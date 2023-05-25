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

#ifndef __NX_DVRGSENSORMANAGER_H__
#define __NX_DVRGSENSORMANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "NX_DvrGsensor.h"
#include "NX_DvrCmdQueue.h"

enum {
	DVR_MODE_NORMAL,
	DVR_MODE_EVENT,
	DVR_MODE_MOTION,
};

typedef struct _GSENSOR_MANAGER_INFO {
	pthread_t			hThread;
	int32_t				bThreadRun;
	pthread_mutex_t		hLock;
	
	int32_t				nDvrMode;
	int32_t				bEnableMotion;

	GSENSOR_HANDLE		hGsensor;
	GSENSOR_VALUE		value;
	GSENSOR_VALUE		threshold;
	CMD_QUEUE_HANDLE	hCmd;
} GSENSOR_MANAGER_INFO;

typedef GSENSOR_MANAGER_INFO		*GSENSOR_MANAGER_HANDLE;

GSENSOR_MANAGER_HANDLE	DvrGsensorManagerInit			( void );
void					DvrGsensorManagerDeinit			( GSENSOR_MANAGER_HANDLE hManager );

int32_t					DvrGsensorManagerStart			( GSENSOR_MANAGER_HANDLE hManager );
int32_t					DvrGsensorManagerStop			( GSENSOR_MANAGER_HANDLE hManager );

int32_t					DvrGsensorManagerRegCmd			( GSENSOR_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd );
int32_t 				DvrGsensorManagerSetStatus		( GSENSOR_MANAGER_HANDLE hManager, int32_t nDvrMode );
int32_t 				DvrGsensorManagerMotionEnable	( GSENSOR_MANAGER_HANDLE hManager, int32_t bEnable );
int32_t					DvrGSensorManagerSetThreshold	( GSENSOR_MANAGER_HANDLE hManager, GSENSOR_VALUE threshold );

int32_t					DvrGsensorGetData				( GSENSOR_MANAGER_HANDLE hManager, GSENSOR_VALUE *pValue ); 

#endif	// __NX_DVRGSENSORMANAGER_H__