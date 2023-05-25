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

#ifndef __NX_DVRCMDQUEUE_H__
#define __NX_DVRCMDQUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "NX_Semaphore.h"
#include "NX_Queue.h"

enum {
	CMD_TYPE_SD_INSERT		= 0x0000,
	CMD_TYPE_SD_REMOVE		= 0x0001,

	CMD_TYPE_EVENT			= 0x1000,
	CMD_TYPE_CAPTURE		= 0x1001,
	CMD_TYPE_NORMAL			= 0x1002,
	CMD_TYPE_MOTION			= 0x1003,
	CMD_TYPE_CHG_MODE		= 0x1004,

	CMD_TYPE_DSP_CROP		= 0x2000,
	CMD_TYPE_DSP_POS		= 0x2001,
	CMD_TYPE_CHG_LCD		= 0x2002,
	CMD_TYPE_CHG_HDMI		= 0x2003,

	CMD_TYPE_LOW_VOLTAGE	= 0x3000,
	CMD_TYPE_HIGH_TEMP		= 0x3001,
	CMD_TYPE_MICOM_INT		= 0x3002,
};

typedef struct _CMD_QUEUE_INFO {
	pthread_t			hThread;
	int32_t				bThreadRun;
	pthread_mutex_t		hLock;
	NX_QUEUE_HANDLE		hCmd;
	NX_SEM_HANDLE		hSem;
} CMD_QUEUE_INFO;

typedef struct _CMD_MESSAGE {
	uint32_t			cmdType;
	uint32_t			cmdData;
	//uint32_t			dataLength;
} CMD_MESSAGE;

typedef CMD_QUEUE_INFO			*CMD_QUEUE_HANDLE;

CMD_QUEUE_HANDLE	DvrCmdQueueInit		( void );
void				DvrCmdQueueDeinit	( CMD_QUEUE_HANDLE hCmdQueue );

int32_t				DvrCmdQueuePush		( CMD_QUEUE_HANDLE hCmdQueue, CMD_MESSAGE *pMessage );
int32_t				DvrCmdQueuePop		( CMD_QUEUE_HANDLE hCmdQueue, CMD_MESSAGE *pMessage );

#endif	// __NX_DVRCMDQUEUE_H__
