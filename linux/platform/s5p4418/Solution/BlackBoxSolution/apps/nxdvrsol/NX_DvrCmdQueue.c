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

#include "NX_DvrCmdQueue.h"

CMD_QUEUE_HANDLE DvrCmdQueueInit( void )
{
	CMD_QUEUE_HANDLE hCmdQueue = (CMD_QUEUE_HANDLE)malloc( sizeof(CMD_QUEUE_INFO) );
	memset( hCmdQueue, 0x00, sizeof(CMD_QUEUE_HANDLE) );

	hCmdQueue->hThread		= 0;
	hCmdQueue->bThreadRun	= 0;
	pthread_mutex_init( &hCmdQueue->hLock, NULL );

	hCmdQueue->hCmd	= NX_QueueInit( NX_MAX_QUEUE_ELEMENT );
	// hCmdQueue->hSem	= NX_SemaporeInit( NX_MAX_QUEUE_ELEMENT, 0 );

	return hCmdQueue;
}

void DvrCmdQueueDeinit( CMD_QUEUE_HANDLE hCmdQueue )
{
	CMD_MESSAGE *pMsg;
	assert( hCmdQueue );
	
	pthread_mutex_lock( &hCmdQueue->hLock );
	while( NX_QueueGetCount(hCmdQueue->hCmd) )
	{	
		pMsg = NULL;
		NX_QueuePop( hCmdQueue->hCmd, (void**)&pMsg ) ;
		if( pMsg ) free( pMsg );
	}
	pthread_mutex_unlock( &hCmdQueue->hLock );

	// NX_SemaporePost( hCmdQueue->hSem );
	// NX_SemaporeDeinit( hCmdQueue->hSem );
	NX_QueueDeinit( hCmdQueue->hCmd );

	pthread_mutex_destroy( &hCmdQueue->hLock );
	if( hCmdQueue ) free( hCmdQueue );
}

int32_t DvrCmdQueuePush( CMD_QUEUE_HANDLE hCmdQueue, CMD_MESSAGE *pMessage )
{
	CMD_MESSAGE *pMsg = NULL;
	assert( hCmdQueue );

	pthread_mutex_lock( &hCmdQueue->hLock );
	pMsg = (CMD_MESSAGE*)malloc( sizeof( CMD_MESSAGE ) );
	memcpy( pMsg, pMessage, sizeof(CMD_MESSAGE) );
	NX_QueuePush( hCmdQueue->hCmd, (void*)pMsg );
	pthread_mutex_unlock( &hCmdQueue->hLock );

	// NX_SemaporePost( hCmdQueue->hSem );

	return 0;
}

int32_t DvrCmdQueuePop( CMD_QUEUE_HANDLE hCmdQueue, CMD_MESSAGE *pMessage)
{
	CMD_MESSAGE *pMsg = NULL;
	assert( hCmdQueue );

	pthread_mutex_lock( &hCmdQueue->hLock );
	
	if( !NX_QueueGetCount( hCmdQueue->hCmd ) ) {
		pthread_mutex_unlock( &hCmdQueue->hLock );
		// NX_SemaporePend( hCmdQueue->hSem );
		usleep(100000);
		return -1;
	}

	if( 0 > NX_QueuePop( hCmdQueue->hCmd, (void**)&pMsg ) ) {
		pthread_mutex_unlock( &hCmdQueue->hLock );
		return -1;		
	}
	else {
		if( pMsg )	{
			memcpy( pMessage, pMsg, sizeof(CMD_MESSAGE) );
			free( pMsg );
		}
	}

	pthread_mutex_unlock( &hCmdQueue->hLock );

	return 0;
}
