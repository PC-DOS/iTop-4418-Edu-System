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

#include <CNX_BufferingFilter.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/sched.h> 	// SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH
#include <asm/unistd.h> 	// __NR_gettid

#define	NX_DTAG		"[CNX_BufferFilter] "
#include <NX_DbgMsg.h>

#if( 0 )
#define NxDbgColorMsg(A, B) 	do {										\
									if( gNxFilterDebugLevel>=A ) {			\
										printf("\033[1;37;41m%s", NX_DTAG);	\
										DEBUG_PRINT B;						\
										printf("\033[0m\r\n");				\
									}										\
								} while(0)

#define NxDbgPushPopMsg(A, B)	do{									\
									if( gNxFilterDebugLevel>=A ) {	\
										DEBUG_PRINT(NX_DTAG);		\
										DEBUG_PRINT B;				\
									}								\
								}while(0)
#else
#define NxDbgColorMsg(A, B) 	do {} while(0)
#define NxDbgPushPopMsg(A, B)	do {} while(0)
#endif

#define	NEW_SCHED_POLICY	SCHED_RR
#define	NEW_SCHED_PRIORITY	25

#define MAX_BUFFER_QCOUNT	64
#define MAX_MODE_COUNT		1

//------------------------------------------------------------------------------
CNX_BufferingFilter::CNX_BufferingFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0x00 )
	, m_EventBufferedTime( 10 )
	, m_EventCount( 0 )
	, m_EventHead( 0 )
	, m_EventTail( 0 )
	, m_PrvTimeStamp( 0 )
	, m_CurTimeStamp( 0 )
	, m_bPopBufferdData( false )
	, m_Flags( FLAGS_WRITING_NORMAL_START )
	, m_Status( STATUS_NORMAL_WRITING )
	, m_bStartFile( false )
	, m_bStopFile( false )
	, m_bChangeFile( false )
{
	m_pSemIn		= new CNX_Semaphore( MAX_BUFFER_QCOUNT, 0 );
	m_pInStatistics	= new CNX_Statistics();
	
	NX_ASSERT( m_pSemIn );

	pthread_mutex_init( &m_hBufferedLock, NULL );
}

//------------------------------------------------------------------------------
CNX_BufferingFilter::~CNX_BufferingFilter()
{
	if( true == m_bInit )
		Deinit();

	pthread_mutex_destroy( &m_hBufferedLock );

	delete m_pSemIn;
	delete m_pInStatistics;
}

//------------------------------------------------------------------------------
void CNX_BufferingFilter::Init( NX_BUFFERING_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );
	NX_ASSERT( pConfig->bufferedTime / 1000 <= MAX_BUFFERD_TIME);

	if( false == m_bInit ) {
		m_EventBufferedTime = pConfig->bufferedTime;
		m_bInit = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_BufferingFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit ) {
		if( m_bRun )
			Stop();
		
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::Receive( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != pSample );
	
	pSample->Lock();

	int32_t nMaxSampleCount = m_SampleInQueue.GetMaxSampleCount();
	int32_t nCurSampleCount = m_SampleInQueue.GetSampleCount();
	
	while( nCurSampleCount >= nMaxSampleCount )
	{
		// NxDbgMsg( NX_DBG_WARN, (TEXT("Buffer is full. Wait..\n")) );
		nCurSampleCount = m_SampleInQueue.GetSampleCount();
		usleep(1000);
	}

	m_SampleInQueue.PushSample( pSample );
	m_pSemIn->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::ReleaseSample( CNX_Sample *pSample )
{
	CNX_MuxerSample *pDummySample = (CNX_MuxerSample*)pSample;
	if( pDummySample ) delete pDummySample;

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bThreadExit = false;
		NX_ASSERT( !m_hThread );

		if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Create Thread\n"), __func__) );
			return false;
		}

		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemIn->Post();		// Send Dummy
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::AllocateMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::FreeMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::GetSample( CNX_Sample **ppSample )
{
	m_pSemIn->Pend();
		
	if( true == m_SampleInQueue.IsReady() ){
		m_SampleInQueue.PopSample( ppSample );
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
void CNX_BufferingFilter::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	CNX_MuxerSample	*pSample = NULL;
	
	m_CurTimeStamp =
	m_PrvTimeStamp = 0;

#if(0)
	{
		pid_t pid = getpid();
		pid_t tid = (pid_t)syscall(__NR_gettid);
		struct sched_param param;
		memset( &param, 0, sizeof(param) );
		NxDbgMsg( NX_DBG_DEBUG, (TEXT("Thread info ( pid:%4d, tid:%4d )\n"), pid, tid) );
		param.sched_priority = NEW_SCHED_PRIORITY;
		if( 0 != sched_setscheduler( tid, NEW_SCHED_POLICY, &param ) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("Failed sched_setscheduler!!!(pid=%d, tid=%d)\n"), pid, tid) );
		}
	}
#endif

	while( !m_bThreadExit )
	{
		if( false == GetSample((CNX_Sample **)&pSample) ) {
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetSample() Failed\n")) );
			continue;
		}
		PushStream( pSample );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void* CNX_BufferingFilter::ThreadMain(void*arg)
{
	CNX_BufferingFilter *pClass = (CNX_BufferingFilter *)arg;

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::DeliverFlagSample( int32_t flags )
{
	CNX_MuxerSample *pDummySample = new CNX_MuxerSample();
	
	if( pDummySample == NULL ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Sample allocate failed.\n")) );
		return -1;
	}

	pDummySample->SetFlags( flags );
	pDummySample->SetOwner( this );
	pDummySample->Lock();
	Deliver( pDummySample );
	pDummySample->Unlock();

	return 0;
}

//------------------------------------------------------------------------------
// 입력된 sample을 스트림 단위로 push한다.
int32_t	CNX_BufferingFilter::PushStream( CNX_MuxerSample *pSample )
{
	CNX_AutoLock lock( &m_hBufferedLock );
	CNX_MuxerSample *pOutSample = NULL;

	if( !m_PrvTimeStamp ) 
		m_PrvTimeStamp = pSample->GetTimeStamp();
	m_CurTimeStamp = pSample->GetTimeStamp();

	// timeStamp is over 700mSec (for 1sec unit - GOP = frame * 1/2 ), key frame, master data( video#0 )
	if( (m_CurTimeStamp >= (m_PrvTimeStamp + 700)) && (0 == pSample->GetDataType()) && (pSample->GetSyncPoint()) ) {
		// 1. Start / Stop / Event Flags Transfer
		if( m_bStopFile ) {
			if( m_Status == STATUS_NORMAL_WRITING ) {
				NxDbgColorMsg( NX_DBG_VBS, (TEXT("Normal writing stop flags.")) );
				DeliverFlagSample( FLAGS_WRITING_NORMAL_STOP );
				m_Status = STATUS_NORMAL_WRITING;
			}
			else if( m_Status == STATUS_EVENT_WRITING ) {
				NxDbgColorMsg( NX_DBG_VBS, (TEXT("Event writing stop flags.")) );
				DeliverFlagSample( FLAGS_WRITING_EVENT_STOP );
				m_Status = STATUS_NORMAL_WRITING;
			}
			m_bStopFile = false;
		}

		if( m_bStartFile ) {
			NxDbgColorMsg( NX_DBG_VBS, (TEXT("Normal writing start flags.")) );
			DeliverFlagSample( FLAGS_WRITING_NORMAL_START );
			m_bStartFile = false;
		}

		if( m_bChangeFile ) {
			if( m_BufferingMode == BUFFERING_MODE_BOTH ) {
				if( m_Status == STATUS_NORMAL_WRITING ) {
					NxDbgColorMsg( NX_DBG_VBS, (TEXT("Normal writing stop flags.")) );
					DeliverFlagSample( FLAGS_WRITING_NORMAL_STOP );
					NxDbgColorMsg( NX_DBG_VBS, (TEXT("Normal writing start flags.")) );
					DeliverFlagSample( FLAGS_WRITING_NORMAL_START );
				}
				else if( m_Status == STATUS_EVENT_WRITING ) {
					m_Status = STATUS_NORMAL_WRITING;
					NxDbgColorMsg( NX_DBG_VBS, (TEXT("Event writing stop flags.")) );
					DeliverFlagSample( FLAGS_WRITING_EVENT_STOP );
					NxDbgColorMsg( NX_DBG_VBS, (TEXT("Normal writing start flags.")) );
					DeliverFlagSample( FLAGS_WRITING_NORMAL_START );
				}
			}
			else if( m_BufferingMode == BUFFERING_MODE_EVENT_ONLY ) {
				m_Status = STATUS_NORMAL_WRITING;
				NxDbgColorMsg( NX_DBG_VBS, (TEXT("Event writing stop flags.")) );
				DeliverFlagSample( FLAGS_WRITING_EVENT_STOP );
			}
			m_bChangeFile = false;
		}

		// 2. Event occur case.
		if( m_bPopBufferdData ) {
			// a. Check mode, and Normal writing stop.
			if( m_BufferingMode == BUFFERING_MODE_BOTH ) {
				NxDbgColorMsg( NX_DBG_VBS, (TEXT("Normal writing stop flags.")) );
				DeliverFlagSample( FLAGS_WRITING_NORMAL_STOP );
			}

			// b2. Deliver dummy sample. (event writing start)
			NxDbgColorMsg( NX_DBG_VBS, (TEXT("Event writing start flags.")) );
			DeliverFlagSample( FLAGS_WRITING_EVENT_START );

			// c. Deliver buffered stream.
			PopStreamAll();
			
			// d. change status & clear flags.
			m_Status = STATUS_EVENT_WRITING;
			m_bPopBufferdData = false;
		}

		// 3. Check event buffer.
		// if buffer is full, last buffered sample is droped. ( 1sec unit )
		if( m_EventCount >= m_EventBufferedTime ) {
			PopStream();
		}

		// 4. Buffered Queue push. (1sec unit)
		while( m_StreamQueue.IsReady() )
		{
			m_StreamQueue.PopSample( (CNX_Sample**)&pOutSample);
			m_EventQueue[m_EventHead].PushSample( (CNX_Sample*)pOutSample );

			if( m_BufferingMode == BUFFERING_MODE_BOTH || (m_BufferingMode == BUFFERING_MODE_EVENT_ONLY && m_Status == STATUS_EVENT_WRITING) ) {
				pOutSample->SetFlags( FLAGS_WRITING_NONE );
				Deliver( pOutSample );
			}
		}
		m_EventHead = (m_EventHead + 1) % (m_EventBufferedTime + 1);
		m_EventCount++;

		m_PrvTimeStamp = m_CurTimeStamp;

		NxDbgPushPopMsg( NX_DBG_DEBUG, (TEXT("%s(): Head( %d ), Tail( %d ), Count( %d )\n"), __func__, m_EventHead, m_EventTail, m_EventCount) );
	}

	// Gathering sample ( for 1sec unit )
	m_StreamQueue.PushSample( pSample );
	
	return true;
}

//------------------------------------------------------------------------------
// 저장된 스트림을 pop하여 버리고, Sample을 Unlock한다.
int32_t	CNX_BufferingFilter::PopStream( void )
{
	CNX_MuxerSample *pOutSample;
	
	while( m_EventQueue[m_EventTail].IsReady() )
	{
		m_EventQueue[m_EventTail].PopSample( (CNX_Sample**)&pOutSample );
		pOutSample->Unlock();
	}
	m_EventTail = (m_EventTail + 1) % (m_EventBufferedTime + 1);
	m_EventCount--;

	NxDbgPushPopMsg( NX_DBG_DEBUG, (TEXT("%s(): Head( %d ), Tail( %d ), Count( %d )\n"), __func__, m_EventHead, m_EventTail, m_EventCount) );

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::PopStreamAll( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_DEBUG, (TEXT("%s(): Buffering Data : %d sec\n"), __func__, m_EventCount) );
	
	while( m_EventCount > 0)
	{
		while( m_EventQueue[m_EventTail].IsReady() )
		{
			CNX_MuxerSample *pOutSample = NULL;
			m_EventQueue[m_EventTail].PopSample( (CNX_Sample**)&pOutSample );
			
			pOutSample->SetFlags( FLAGS_WRITING_NONE );
			Deliver( (CNX_Sample*)pOutSample );
			pOutSample->Unlock();
		}
		m_EventTail = (m_EventTail + 1) % (m_EventBufferedTime + 1);
		m_EventCount--;

		NxDbgPushPopMsg( NX_DBG_DEBUG, (TEXT("%s(): Head( %d ), Tail( %d ), Count( %d )\n"), __func__, m_EventHead, m_EventTail, m_EventCount) );
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::ChangeBufferingMode( int32_t mode )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hBufferedLock );
	m_BufferingMode = mode;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_BufferingFilter::PopBufferdData( int32_t bPopBufferdData )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hBufferedLock );
	m_bPopBufferdData = bPopBufferdData;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::StartFile( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hBufferedLock );
	m_bStartFile = true;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::StopFile( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hBufferedLock );
	m_bStopFile = true;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::ChangeFile( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hBufferedLock );
	m_bChangeFile = true;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
uint64_t CNX_BufferingFilter::GetTimeStamp( void )
{
	CNX_AutoLock lock( &m_hBufferedLock );
	return m_CurTimeStamp;
}

//------------------------------------------------------------------------------
int32_t CNX_BufferingFilter::GetStatistics( NX_FILTER_STATISTICS *pStatistics )
{
	NX_ASSERT( NULL != pStatistics );
	return true;
}

