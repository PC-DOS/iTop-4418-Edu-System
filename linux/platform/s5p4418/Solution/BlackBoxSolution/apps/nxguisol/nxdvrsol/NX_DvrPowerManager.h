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

typedef struct _POWER_MANAGER_INFO {
	pthread_t		hThread;
	int32_t			bThreadRun;
	pthread_mutex_t	hLock;
	int32_t			value;		// mV
} POWER_MANAGER_INFO;

typedef POWER_MANAGER_INFO		*POWER_MANAGER_HANDLE;

POWER_MANAGER_HANDLE	DvrPowerManagerInit		( void );
void					DvrPowerManagerDeinit	( POWER_MANAGER_HANDLE hManager );

int32_t					DvrPowerManagerStart	( POWER_MANAGER_HANDLE hManager );
int32_t					DvrPowerManagerStop		( POWER_MANAGER_HANDLE hManager );
int32_t					DvrPowerGetData			( POWER_MANAGER_HANDLE hManager, int32_t *pValue ); 

#endif	// __NX_DVRPOWERMANAGER_H__
