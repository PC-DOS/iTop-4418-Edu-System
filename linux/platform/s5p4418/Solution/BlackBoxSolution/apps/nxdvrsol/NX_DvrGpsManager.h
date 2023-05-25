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

#ifndef __NX_DVRGPSMANAGER_H__
#define __NX_DVRGPSMANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <nx_nmea_parser.h>

#include "NX_DvrCmdQueue.h"

typedef struct _GPS_MANAGER_INFO {
	int32_t				hDevice;
	int32_t				hParser;
	struct nmea_gprmc	gpsData;
	pthread_t			hThread;
	int32_t				bThreadRun;
	pthread_mutex_t		hLock;
	CMD_QUEUE_HANDLE	hCmd;
} GPS_MANAGER_INFO;

typedef GPS_MANAGER_INFO		*GPS_MANAGER_HANDLE;

GPS_MANAGER_HANDLE	DvrGpsManagerInit	( void );
void				DvrGpsManagerDeinit	( GPS_MANAGER_HANDLE hManager );

int32_t				DvrGpsManagerStart	( GPS_MANAGER_HANDLE hManager );
int32_t				DvrGpsManagerStop	( GPS_MANAGER_HANDLE hManager );

int32_t 			DvrGpsManagerRegCmd	( GPS_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd );

int32_t				DvrGpsGetData		( GPS_MANAGER_HANDLE hManager, struct nmea_gprmc *pGprmc );

#endif	// __NX_DVRGPSMANAGER_H__