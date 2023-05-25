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

#ifndef __NX_DVRFILEMANAGER_H__
#define __NX_DVRFILEMANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "NX_DvrFileQueue.h"
#include "NX_DvrCmdQueue.h"

typedef struct _FILE_MANAGER_INFO {
	char				topDir[256];
	int64_t				curSize;
	int64_t				maxSize;
	QHANDLE				hQueue;
	pthread_t			hThread;
	int32_t				bThreadRun;
	pthread_mutex_t		hLock;
	CMD_QUEUE_HANDLE	hCmd;
} FILE_MANAGER_INFO;

typedef FILE_MANAGER_INFO	*FILE_MANAGER_HANDLE;

FILE_MANAGER_HANDLE		DvrFileManagerInit	( const char *topDir, int32_t ratio, const char *pFileExtension );
void					DvrFileManagerDeinit( FILE_MANAGER_HANDLE hManager );

int32_t					DvrFileManagerStart	( FILE_MANAGER_HANDLE hManager );
int32_t					DvrFileManagerStop	( FILE_MANAGER_HANDLE hManager );

int32_t					DvrFileManagerPush	( FILE_MANAGER_HANDLE hManager, char *pData );
int32_t					DvrFileManagerRegCmd( FILE_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd );

#endif	// __NX_DVRFILEMANAGER_H__