//------------------------------------------------------------------------------
//
//	Copyright (C) 2014 Nexell Co. All Rights Reserved
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
#include <stdlib.h>
#include <string.h>

#define NX_DTAG	"[CNX_Mp4Notify]"
#include <NX_DbgMsg.h>

#include "CNX_Mp4Notify.h"

//------------------------------------------------------------------------------
CNX_Mp4Notify::CNX_Mp4Notify()
	: NotifyCallback( NULL )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_pSem( NULL )
	, m_iHeadIndex( 0 )
	, m_iTailIndex( 0 )
	, m_nMsgCount( 0 )
	, m_nQueueDepth( MAX_EVENT_QUEUE_DEPTH )
{
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_Mp4Notify::~CNX_Mp4Notify()
{
	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
bool CNX_Mp4Notify::Run( void )
{
	if( true != m_bRun ) {
		m_bThreadExit = false;
		m_pSem = new CNX_Semaphore( MAX_EVENT_QUEUE_DEPTH, 0 );

		if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) )
		{
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Create Thread\n"), __func__) );
			return false;
		}
		m_bRun = true;
	}
	return true;
}

//------------------------------------------------------------------------------
bool CNX_Mp4Notify::Stop( void )
{
	if( m_pSem ){
		delete m_pSem;
		m_pSem = NULL;
	}

	if( true == m_bRun ) {
		m_bThreadExit = true;
		pthread_join( m_hThread, NULL );
		m_bRun = false;
	}
	return true;
}

//------------------------------------------------------------------------------
void CNX_Mp4Notify::Push( DVR_EVENT_MESSAGE *pSample )
{
	if( m_nMsgCount >= m_nQueueDepth)
		return;

	pthread_mutex_lock( &m_hLock );
	m_pEventMsg[m_iTailIndex] = pSample;
	m_iTailIndex = (m_iTailIndex+1) % MAX_EVENT_QUEUE_DEPTH;
	m_nMsgCount++;
	pthread_mutex_unlock( &m_hLock );
}

//------------------------------------------------------------------------------
void CNX_Mp4Notify::Pop( DVR_EVENT_MESSAGE **ppSample )
{
	if( m_nMsgCount <= 0 )
		return;

	pthread_mutex_lock( &m_hLock );
	*ppSample = m_pEventMsg[m_iHeadIndex];
	m_iHeadIndex = (m_iHeadIndex+1) % MAX_EVENT_QUEUE_DEPTH;
	m_nMsgCount--;
	pthread_mutex_unlock( &m_hLock );
}

//------------------------------------------------------------------------------
void CNX_Mp4Notify::ThreadLoop( void )
{
	DVR_EVENT_MESSAGE *pMsg = NULL;
	
	while( !m_bThreadExit )
	{
		if( m_pSem->Pend() < 0 )
			break;

		Pop( &pMsg );

		if( pMsg )
		{
			if( NotifyCallback ) {
				NotifyCallback( pMsg->eventType, pMsg->eventData, pMsg->dataLength );
			}
			if( pMsg->eventData ) {
				free( pMsg->eventData );
			}
			free( pMsg );
		}
		
		pMsg = NULL;
	}		
}

//------------------------------------------------------------------------------
void* CNX_Mp4Notify::ThreadMain( void *arg )
{
	CNX_Mp4Notify *pClass = (CNX_Mp4Notify *)arg;
	pClass->ThreadLoop();
	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
uint32_t CNX_Mp4Notify::RegisterNotifyCallback( uint32_t (*cbNotifyCallback)(uint32_t, uint8_t*, uint32_t) )
{
	NotifyCallback = cbNotifyCallback;
	return 0;
}

//------------------------------------------------------------------------------
void CNX_Mp4Notify::EventNotify( uint32_t eventCode, void *pEventData, uint32_t dataLength )
{
	DVR_EVENT_MESSAGE *pMsg = (DVR_EVENT_MESSAGE *)malloc( sizeof(DVR_EVENT_MESSAGE) );

	if( m_bThreadExit || !m_pSem )
	{
		NxErrMsg( (TEXT("Unable to handle event message. (eventCode=0x%08x)\n"), eventCode) );
		free( pMsg );
		return;
	}

	pMsg->eventType = eventCode;
	if( pEventData && (dataLength > 0) && (dataLength < 2048) )
	{
		pMsg->eventData = (uint8_t*)malloc(dataLength);
		pMsg->dataLength = dataLength;
		memcpy(pMsg->eventData, pEventData, dataLength );
	}
	else{
		pMsg->eventData = NULL;
		pMsg->dataLength = 0;
	}
	Push( pMsg );
	m_pSem->Post();
}
