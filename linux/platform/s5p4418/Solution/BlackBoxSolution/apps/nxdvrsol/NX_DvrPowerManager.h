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

#ifndef __NX_DVRPOWERMANAGER_H__
#define __NX_DVRPOWERMANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <nx_gpio.h>

#include "NX_DvrCmdQueue.h"

typedef struct _POWER_MANAGER_INFO {
	pthread_t			hThread;
	int32_t				bThreadRun;

	NX_GPIO_HANDLE		hInterrupt;
	pthread_t			hThreadInterrupt;
	
	pthread_mutex_t		hLock;
	
	int32_t				power;			// mV
	int32_t				temperature;	// mV
	CMD_QUEUE_HANDLE	hCmd;
} POWER_MANAGER_INFO;

typedef POWER_MANAGER_INFO		*POWER_MANAGER_HANDLE;

POWER_MANAGER_HANDLE	DvrPowerManagerInit		( void );
void					DvrPowerManagerDeinit	( POWER_MANAGER_HANDLE hManager );

int32_t					DvrPowerManagerStart	( POWER_MANAGER_HANDLE hManager );
int32_t					DvrPowerManagerStop		( POWER_MANAGER_HANDLE hManager );

int32_t					DvrPowerManagerRegCmd	( POWER_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd );
int32_t 				DvrPowerGetData			( POWER_MANAGER_HANDLE hManager, int32_t *pPower, int32_t *pTemperature );

#endif	// __NX_DVRPOWERMANAGER_H__
