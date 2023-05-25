//------------------------------------------------------------------------------
//
//	Copyright (C) 2010 Nexell co., Ltd All Rights Reserved
//
//	Module     : Queue Module
//	File       : 
//	Description:
//	Author     : RayPark
//	History    :
//------------------------------------------------------------------------------
#include <assert.h>
#include <pthread.h>

#include <NX_OMXQueue.h>
#include <NX_OMXMem.h>
#include <NX_OMXDebugMsg.h>
#include <NX_MediaTypes.h>

#define DEBUG_QUEUE		0

#ifdef DbgMsg
#undef DbgMsg
#endif

#if DEBUG_QUEUE
#define	DbgMsg(fmt,...)		NX_DbgTrace(fmt, ##__VA_ARGS__)
#else
#define DbgMsg(fmt,...)		 
#endif

//
//	Description : Initialize queue structure
//	Return      : 0 = no error, -1 = error
//
int32 NX_InitQueue( NX_QUEUE *pQueue, uint32 maxNumElement )
{
	//	Initialize Queue
	NxMemset( pQueue, 0, sizeof(NX_QUEUE) );
	if( maxNumElement > NX_MAX_QUEUE_ELEMENT ){
		return -1;
	}
	if( 0 != pthread_mutex_init( &pQueue->hMutex, NULL ) ){
		return -1;
	}
	pQueue->maxElement = maxNumElement;
	pQueue->bEnabled = TRUE;
	return 0;
}

int32 NX_PushQueue( NX_QUEUE *pQueue, void *pElement )
{
	assert( NULL != pQueue );
	DbgMsg( "%s(pQueue = 0x%08x) In\n", __FUNCTION__, (int32)pQueue );
	pthread_mutex_lock( &pQueue->hMutex );
	//	Check Buffer Full
	if( pQueue->curElements >= pQueue->maxElement || !pQueue->bEnabled ){
		pthread_mutex_unlock( &pQueue->hMutex );
		DbgMsg( "%s() curElements=%d, Enable=%d : Out NOK.\n", __FUNCTION__, pQueue->curElements, pQueue->bEnabled );
		return -1;
	}else{
		pQueue->pElements[pQueue->tail] = pElement;
		pQueue->tail = (pQueue->tail+1)%pQueue->maxElement;
		pQueue->curElements ++;
	}
	pthread_mutex_unlock( &pQueue->hMutex );
	DbgMsg( "%s() Out OK(%d).\n", __FUNCTION__, pQueue->curElements );
	return 0;
}

int32 NX_PopQueue( NX_QUEUE *pQueue, void **pElement )
{
	assert( NULL != pQueue );
	DbgMsg( "%s(pQueue = 0x%08x) In\n", __FUNCTION__, (int32)pQueue );
	pthread_mutex_lock( &pQueue->hMutex );
	//	Check Buffer Full
	if( pQueue->curElements == 0 || !pQueue->bEnabled ){
		pthread_mutex_unlock( &pQueue->hMutex );
		DbgMsg( "%s() curElements=%d, Enable=%d : Out NOK.\n", __FUNCTION__, pQueue->curElements, pQueue->bEnabled );
		return -1;
	}else{
		*pElement = pQueue->pElements[pQueue->head];
		pQueue->head = (pQueue->head + 1)%pQueue->maxElement;
		pQueue->curElements --;
	}
	pthread_mutex_unlock( &pQueue->hMutex );
	DbgMsg( "%s() Out OK(%d).\n", __FUNCTION__, pQueue->curElements );
	return 0;
}

int32 NX_GetNextQueuInfo( NX_QUEUE *pQueue, void **pElement )
{
	assert( NULL != pQueue );
	DbgMsg( "%s(pQueue = 0x%08x) In\n", __FUNCTION__, (int32)pQueue );
	pthread_mutex_lock( &pQueue->hMutex );
	//	Check Buffer Full
	if( pQueue->curElements == 0 || !pQueue->bEnabled ){
		pthread_mutex_unlock( &pQueue->hMutex );
		DbgMsg( "%s() curElements=%d, Enable=%d : Out NOK.\n", __FUNCTION__, pQueue->curElements, pQueue->bEnabled );
		return -1;
	}else{
		*pElement = pQueue->pElements[pQueue->head];
	}
	pthread_mutex_unlock( &pQueue->hMutex );
	DbgMsg( "%s() Out OK(%d).\n", __FUNCTION__, pQueue->curElements );
	return 0;
}

uint32 NX_GetQueueCnt( NX_QUEUE *pQueue )
{
	assert( NULL != pQueue );
	return pQueue->curElements;
}

void NX_DeinitQueue( NX_QUEUE *pQueue )
{
	assert( NULL != pQueue );
	pthread_mutex_lock( &pQueue->hMutex );
	pQueue->bEnabled = FALSE;
	pthread_mutex_unlock( &pQueue->hMutex );
	pthread_mutex_destroy( &pQueue->hMutex );
	NxMemset( pQueue, 0, sizeof(NX_QUEUE) );
}
