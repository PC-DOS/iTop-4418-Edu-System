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

#include <linux/sched.h> 	// SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH
#include <asm/unistd.h> 	// __NR_gettid
#include <unistd.h>
#include <string.h>

#include "CNX_UserDataFilter.h"
#define	NX_DTAG		"[CNX_UserDataFilter] "
#include "NX_DbgMsg.h"

#define	NEW_SCHED_POLICY	SCHED_RR
#define	NEW_SCHED_PRIORITY	25

//------------------------------------------------------------------------------
CNX_UserDataFilter::CNX_UserDataFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0 )
	, m_PacketID( 0 )
	, m_UserBufferSize( MAX_USERDATA_SIZE )
	, m_UserInterval( 0 )
	, UserDataCallbackFunc( NULL )
{
	m_pSemOut				= new CNX_Semaphore(MAX_BUFFER, 0);
	m_pRefClock				= CNX_RefClock::GetSingletonPtr();

	pthread_mutex_init( &m_hStatisticsLock, NULL );
	memset( &m_FilterStatistics, 0x00, sizeof( NX_FILTER_STATISTICS ) );
}

//------------------------------------------------------------------------------
CNX_UserDataFilter::~CNX_UserDataFilter()
{
	if (true == m_bInit)
		Deinit();
	
	delete m_pSemOut;

	pthread_mutex_destroy( &m_hStatisticsLock );
}

//------------------------------------------------------------------------------
void	CNX_UserDataFilter::Init(NX_USERDATA_CONFIG *pConfig)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NX_ASSERT(false == m_bInit);

	if( pConfig->bufSize > MAX_USERDATA_SIZE )
		return;

	if (false == m_bInit) {
		m_UserBufferSize	= pConfig->bufSize;
		m_UserInterval		= pConfig->interval;

		if( NULL == m_UserBuffer )
		{
			NX_ASSERT( m_UserBuffer );
			return;
		}

		AllocateBuffer(2);

		m_bInit = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_UserDataFilter::Deinit(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NX_ASSERT(true == m_bInit);

	if (true == m_bInit) {
		if (m_bRun)		Stop();
		
		FreeBuffer();
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_UserDataFilter::AllocateBuffer( int32_t numOfBuffer )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( numOfBuffer <= MAX_BUFFER );

	int32_t i;

	m_SampleOutQueue.Reset();
	m_SampleOutQueue.SetQueueDepth( numOfBuffer );

	m_pSemOut->Init();

	for( i = 0; i < numOfBuffer; i++ )
	{
		m_UserSample[i].SetOwner( this );
		m_UserSample[i].SetBuffer( m_UserBuffer[i], m_UserBufferSize );
		m_SampleOutQueue.PushSample( &m_UserSample[i] );
		m_pSemOut->Post();
	}

	m_iNumOfBuffer = i;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()-- (m_iNumOfBuffer=%d)\n"), __func__, m_iNumOfBuffer) );
}

//------------------------------------------------------------------------------
void CNX_UserDataFilter::FreeBuffer(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_SampleOutQueue.Reset();
	m_iNumOfBuffer = 0;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_UserDataFilter::Run(void)
{
	if (m_bRun == false) {
		m_bThreadExit 	= false;
		NX_ASSERT( !m_hThread );

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
int32_t	CNX_UserDataFilter::Stop(void)
{
	if (true == m_bRun) {
		m_bThreadExit = true;
		m_pSemOut->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_UserDataFilter::Receive(CNX_Sample *pSample)
{
	NX_ASSERT(false);
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_UserDataFilter::GetSample(CNX_Sample **ppSample)
{
	NX_ASSERT(false);
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_UserDataFilter::ReleaseSample(CNX_Sample *pSample)
{
	m_SampleOutQueue.PushSample( pSample );
	m_pSemOut->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_UserDataFilter::GetDeliverySample(CNX_Sample **ppSample)
{
	m_pSemOut->Pend();
	if( true == m_SampleOutQueue.IsReady() ) {
		m_SampleOutQueue.PopSample( ppSample );
		(*ppSample)->Lock();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void	CNX_UserDataFilter::RegUserDataCallback( int32_t(*cbFunc)( uint8_t *, uint32_t ) )
{
	NX_ASSERT( cbFunc );
	if( cbFunc )
	{
		UserDataCallbackFunc = cbFunc;
	}
}

//------------------------------------------------------------------------------
void	CNX_UserDataFilter::GetUserDataFromCallback( CNX_MuxerSample *pSample )
{
	if( UserDataCallbackFunc )
	{
		uint8_t *pBuf;
		uint32_t bufSize;

		if( true == pSample->GetBuffer( &pBuf, (int32_t*)&bufSize ) )
		{
			uint32_t outSize = UserDataCallbackFunc( pBuf, m_UserBufferSize );
			pSample->SetTimeStamp( m_pRefClock ? m_pRefClock->GetCorrectTickCount() : NX_GetTickCount() );
			pSample->SetActualDataLength( outSize );
			pSample->SetDataType( m_PacketID );
		}
	}
}

//------------------------------------------------------------------------------
void	CNX_UserDataFilter::ThreadLoop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_MuxerSample	*pSample = NULL;
	uint64_t prevTime = 0, curTime = 0;

#if (0)
	{
		pid_t pid = getpid();
		pid_t tid = (pid_t)syscall(__NR_gettid);
		struct sched_param param;
		memset( &param, 0, sizeof(param) );
		NxDbgMsg( NX_DBG_DEBUG, (TEXT("Thread info ( pid:%4d, tid:%4d )\n"), pid, tid) );
		param.sched_priority = NEW_SCHED_PRIORITY;
		if( 0 != sched_setscheduler( tid, NEW_SCHED_POLICY, &param ) ){
			NxDbgMsg( NX_DBG_ERR, (TEXT("Failed sched_setscheduler!!!(pid=%d, tid=%d)\n"), pid, tid) );
		}
	}
#endif

	if( m_pRefClock ) {
		curTime		= m_pRefClock->GetCorrectTickCount();
	} else {
		curTime		= NX_GetTickCount();
	}

	while( !m_bThreadExit )
	{
		if( false == GetDeliverySample( (CNX_Sample **)&pSample ) )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetDeliverySample() Failed\n")) );
			continue;
		}
		if( NULL == pSample )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("Sample is NULL\n")) );
			continue;
		}
		
		if( m_pRefClock )
			curTime = m_pRefClock->GetCorrectTickCount();
		else
			curTime = NX_GetTickCount();

		if( !prevTime ) {
			// First data delivery
			GetUserDataFromCallback( pSample );
			Deliver( pSample );
		}
		else {
			if( prevTime + m_UserInterval <= curTime ) {
				prevTime = curTime;
				GetUserDataFromCallback( pSample );
				Deliver( pSample );
			} 
		}
		pSample->Unlock();
		usleep(10000);
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_UserDataFilter::ThreadMain( void *arg )
{
	CNX_UserDataFilter *pClass = (CNX_UserDataFilter *)arg;
	NX_ASSERT( NULL != pClass );

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//----------------------------------------------------------------------------
//	External Interfaces
//----------------------------------------------------------------------------
int32_t	CNX_UserDataFilter::SetPacketID( uint32_t PacketID )
{
	m_PacketID = PacketID;
	NxDbgMsg( NX_DBG_DEBUG, (TEXT("%s(): Packet ID = %d\n"), __func__, m_PacketID) );
	return true;
}

//------------------------------------------------------------------------------
int32_t  CNX_UserDataFilter::GetStatistics( NX_FILTER_STATISTICS *pStatistics )
{
	pthread_mutex_lock( &m_hStatisticsLock );
	memset( pStatistics, 0x00, sizeof(NX_FILTER_STATISTICS) );
	
	m_FilterStatistics.outBuf[0].limit	= m_iNumOfBuffer;
	m_FilterStatistics.outBuf[0].cur	= m_iNumOfBuffer - m_SampleOutQueue.GetSampleCount();
	if( m_FilterStatistics.outBuf[0].cur > m_FilterStatistics.outBuf[0].max )
		m_FilterStatistics.outBuf[0].max = m_FilterStatistics.outBuf[0].cur;

	memcpy( pStatistics, &m_FilterStatistics, sizeof(NX_FILTER_STATISTICS) );
	pthread_mutex_unlock( &m_hStatisticsLock );
	return true;
}