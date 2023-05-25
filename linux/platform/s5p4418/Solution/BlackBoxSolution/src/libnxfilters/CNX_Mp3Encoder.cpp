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

#include <string.h>
#include <linux/sched.h> 	// SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH
#include <asm/unistd.h> 	// __NR_gettid
#include <unistd.h> 		// __NR_gettid

#include "CNX_Mp3Encoder.h"

#define	NX_DTAG	"[CNX_Mp3Encoder] "
#include "NX_DbgMsg.h"

#define	NEW_SCHED_POLICY	SCHED_RR
#define	NEW_SCHED_PRIORITY	25
#define MAX_AUD_QCOUNT		64

#define	DUMP_MP3			0

#define DUMP_PCM			0

#if( DUMP_MP3 )
#define		MP3_DUMP_FILENAME	"./dump.mp3"
static FILE *outFp = NULL;
#endif

#if( DUMP_PCM )
#define		PCM_DUMP_FILENAME	"./dump.pcm"
static FILE *outPcmFp = NULL;
#endif

//------------------------------------------------------------------------------
CNX_Mp3Encoder::CNX_Mp3Encoder()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0 )
	, m_PacketID( 0 )
	, m_hMp3Enc( NULL )
	, m_Channels( 2 )
	, m_Frequency( 48000 )
	, m_Bitrate( 128 )
	, m_iNumOfBuffer( 0 )
{
	m_pSemIn	= new CNX_Semaphore( MAX_AUD_QCOUNT, 0 );
	m_pSemOut	= new CNX_Semaphore( MAX_AUD_QCOUNT, 0 );
	m_pInStatistics		= new CNX_Statistics();
	m_pOutStatistics 	= new CNX_Statistics();

	NX_ASSERT( m_pSemIn );
	NX_ASSERT( m_pSemOut );
}

//------------------------------------------------------------------------------
CNX_Mp3Encoder::~CNX_Mp3Encoder()
{
	if( true == m_bInit )
		Deinit();

	delete m_pSemIn;
	delete m_pSemOut;
	delete m_pInStatistics;
	delete m_pOutStatistics;
}

//------------------------------------------------------------------------------
void	CNX_Mp3Encoder::Init( NX_AUDENC_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );

	if( false == m_bInit )
	{
		m_Channels	= pConfig->channels;
		m_Frequency	= pConfig->frequency;
		m_Bitrate	= pConfig->bitrate / 1000;

		NxDbgMsg( NX_DBG_INFO, (TEXT("[Audio|Encoder] Channel=%d, Frequency=%d, Bitrate=%d\n"), m_Channels, m_Frequency, m_Bitrate)  );

		m_hMp3Enc = open_mp3enc();

		if( NULL == m_hMp3Enc ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): MP3 encoder failed.\n"), __func__) );
			return;
		}

		//	Apply Configs
		if( 0 > init_mp3enc( m_hMp3Enc, m_Channels, m_Frequency, m_Bitrate, (m_Channels==2) ? MODE_JOINT_STEREO : MODE_MONO ) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): MP3 encoder init failed.\n"), __func__) );
			return ;
		}

		AllocateBuffer( NUM_ALLOC_BUFFER );
		m_bInit = true;

#if( DUMP_MP3 )
		outFp = fopen(MP3_DUMP_FILENAME, "wb+");
#endif

#if( DUMP_PCM )
		outPcmFp = fopen(PCM_DUMP_FILENAME, "wb+");
#endif

	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_Mp3Encoder::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit )
	{
		if( m_bRun )	Stop();
		if( NULL != m_hMp3Enc )
		{
			close_mp3enc( m_hMp3Enc );
			m_hMp3Enc = NULL;
		}

		m_bInit = false;
	}

#if( DUMP_MP3 )
	if( outFp ) {
		fclose(outFp);
		outFp = NULL;
	}		
#endif

#if( DUMP_PCM )
	if( outPcmFp ) {
		fclose(outPcmFp);
		outPcmFp = NULL;
	}
#endif
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp3Encoder::Receive( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != pSample );

#if( DUMP_MP3 )
#else
	if( !m_pOutFilter )
		return true;
#endif

	m_pInStatistics->CalculateFps();
	m_pInStatistics->CalculateBufNumber( m_SampleInQueue.GetSampleCount() );

	pSample->Lock();
	m_SampleInQueue.PushSample(pSample);
	m_pSemIn->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp3Encoder::ReleaseSample( CNX_Sample *pSample )
{
	m_SampleOutQueue.PushSample( pSample );
	m_pSemOut->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp3Encoder::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );	
	if( m_bRun == false ) {
		m_bThreadExit 	= false;
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
int32_t	CNX_Mp3Encoder::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemIn->Post();
		m_pSemOut->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void	CNX_Mp3Encoder::AllocateBuffer( int32_t numOfBuffer )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NX_ASSERT(numOfBuffer <= MAX_BUFFER);
	
	m_SampleOutQueue.Reset();
	m_SampleOutQueue.SetQueueDepth(numOfBuffer);

	m_pSemOut->Init();
	for( int32_t i = 0; i < numOfBuffer; i++) {
		m_OutSample[i].SetOwner(this);
		m_OutSample[i].SetBuffer( (uint8_t*)&m_OutBuf[i], 9216);

		m_SampleOutQueue.PushSample( &m_OutSample[i] );
		m_pSemOut->Post();
	}
	m_iNumOfBuffer = numOfBuffer;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()-- (m_iNumOfBuffer=%d)\n"), __func__, m_iNumOfBuffer) );
}

//------------------------------------------------------------------------------
void	CNX_Mp3Encoder::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	for( int32_t i = 0; i < m_iNumOfBuffer; i++ )
	{
		NX_ASSERT( NULL != m_OutBuf[i] );
	}
	m_pSemIn->Post();		//	Send Dummy
	m_pSemOut->Post();		//	Send Dummy
	m_iNumOfBuffer = 0;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s--\n"), __func__) );
}

//------------------------------------------------------------------------------
//	GetSample은 input에서 들어온 버퍼를 queue에 넣어 두었을 경우 queue로부터
//	sample을 가져오는 루틴이다.
//	Locking과 unlocking은 sample push 전 (Receive) 과 pop 후에 하여야 한다.
int32_t	CNX_Mp3Encoder::GetSample( CNX_Sample **ppSample )
{
	m_pSemIn->Pend();
	if( true == m_SampleInQueue.IsReady() ){
		m_SampleInQueue.PopSample( ppSample );
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp3Encoder::GetDeliverySample( CNX_Sample **ppSample )
{
	m_pSemOut->Pend();
	if( true == m_SampleOutQueue.IsReady() ) {
		m_SampleOutQueue.PopSample( ppSample );
		(*ppSample)->Lock();
		return true;
	}
	NxDbgMsg( NX_DBG_WARN, (TEXT("Sample is not ready\n")) );
	return false;
}

//------------------------------------------------------------------------------
void	CNX_Mp3Encoder::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	int32_t	ret;
	CNX_MediaSample		*pSample = NULL;
	CNX_MuxerSample		*pOutSample = NULL;

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

	while( !m_bThreadExit )
	{
		//	Get sample from in-queue.
		if( false == GetSample((CNX_Sample **)&pSample) )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetSample() Failed\n")) );
			continue;
		}
		if( NULL == pSample )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("Sample is NULL\n")) );
			continue;
		}
		if( false == GetDeliverySample((CNX_Sample **)&pOutSample) )	// output queue
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetDeliverySample() Failed\n")) );
			pSample->Unlock();
			continue;
		}
		if( NULL == pOutSample )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("Encode buffer is NULL\n")) );
			pSample->Unlock();
			continue;
		}

		m_pOutStatistics->CalculateBpsStart();
		ret = EncodeAudio( pSample, pOutSample );
		m_pOutStatistics->CalculateBpsEnd( pOutSample->GetActualDataLength() );

		m_pOutStatistics->CalculateFps();
		m_pOutStatistics->CalculateBufNumber( m_iNumOfBuffer - m_SampleInQueue.GetSampleCount() );

		if( ret > 0 ) {
			Deliver( pOutSample );
		}
		else if( ret < 0 ) {
			// if( m_pNotify ){
			// 	m_pNotify->EventNotify( 0xF003, NULL, 0 );
			// }
		}
		else {
			NxDbgMsg( NX_DBG_DEBUG, (TEXT("Have no MP3 encode result.\n")) );
		}

		if( pSample )
			pSample->Unlock();

		if( pOutSample )
			pOutSample->Unlock();
	}

	while( m_SampleInQueue.IsReady() )
	{
		m_SampleInQueue.PopSample((CNX_Sample**)&pSample);
		if( pSample )
			pSample->Unlock();
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_Mp3Encoder::ThreadMain(void*arg)
{
	CNX_Mp3Encoder *pClass = (CNX_Mp3Encoder *)arg;
	NX_ASSERT( NULL != pClass );

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
#define	SAMPLE_PER_BYTE 2
#define MP3_FRAME_SIZE	1152

int32_t	CNX_Mp3Encoder::EncodeAudio( CNX_MediaSample *pInSample, CNX_MuxerSample *pOutSample )
{
	//	Convert Audio Data
	uint8_t *pSrc, *pDst;
	int32_t srcSize, dstSize;
	int32_t outSize;
	int32_t frameLength;

	pInSample->GetBuffer( &pSrc, &srcSize );
	pOutSample->GetBuffer( &pDst, &dstSize );
	srcSize = pInSample->GetActualDataLength();

	frameLength = srcSize / m_Channels / SAMPLE_PER_BYTE;
	if( frameLength < MP3_FRAME_SIZE )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Frame Size Mismatch. ( %d )\n"), __func__, frameLength));
		return -1;
	}


#if( DUMP_PCM )
	if( outPcmFp ) {
		fwrite( pSrc, 1, srcSize, outPcmFp );
	}		
#endif
	outSize = encode_mp3enc( m_hMp3Enc, (int16_t*)((void*)pSrc), srcSize, pDst, dstSize );
	
	if( outSize > 0 )
	{
		pOutSample->SetActualDataLength( outSize );
		pOutSample->SetDataType( m_PacketID );
		pOutSample->SetTimeStamp( pInSample->GetTimeStamp() );
	}

#if( DUMP_MP3 )
	if( outFp ) {
		fwrite( pDst, 1, outSize, outFp );
	}		
#endif

	return outSize;
}

//----------------------------------------------------------------------------
// External Interfaces
//----------------------------------------------------------------------------
int32_t	CNX_Mp3Encoder::SetPacketID( uint32_t PacketID )
{
	m_PacketID = PacketID;
	NxDbgMsg( NX_DBG_DEBUG, (TEXT("%s(): Packet ID = %d\n"), __func__, m_PacketID) );
	return true;
}

//------------------------------------------------------------------------------
int32_t  CNX_Mp3Encoder::GetStatistics( NX_FILTER_STATISTICS *pStatistics )
{
	return true;
}