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

#ifndef __DVR_FILEQUEUE_H__
#define __DVR_FILEQUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_FILE_NAME		1024

typedef struct _QNODE {
	char			fileName[MAX_FILE_NAME];
	uint64_t		fileSize;
	struct _QNODE	*next;
} QNODE;

typedef struct _QHANDLE_INFO {
	QNODE			*front;
	QNODE			*rear;
	uint32_t		nodeNum;
	pthread_mutex_t	hMutex;
} QHANDLE_INFO;

typedef QHANDLE_INFO		*QHANDLE;

QHANDLE DvrFileQueueInit	( void );
int32_t	DvrFileQueueDeinit	( QHANDLE hQueue );

int32_t	DvrFileQueuePush	( QHANDLE hQueue, char *pFileName, uint64_t fileSize );
int32_t	DvrFileQueuePop		( QHANDLE hQueue, char *pFileName, uint64_t *pFileSize);

int32_t DvrFileQueueInsert	( QHANDLE hQueue, char *pFileName, uint64_t fileSize, int32_t bSort );	// bSort : -1 (Asending) / 1 (Desending)

int32_t DvrFileQueueList	( QHANDLE hQueue );
int32_t DvrFileQueueNum		( QHANDLE hQueue );
int32_t	DvrFileQueueIsEmpty	( QHANDLE hQueue );

#endif // __DVR_FILEQUEUE_H__