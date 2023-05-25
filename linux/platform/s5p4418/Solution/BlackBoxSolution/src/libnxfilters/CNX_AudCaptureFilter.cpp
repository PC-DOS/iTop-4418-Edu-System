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

#include <CNX_AudCaptureFilter.h>

#include <linux/sched.h> 	// SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH
#include <asm/unistd.h> 	// __NR_gettid

#define	NX_DTAG	"[CNX_AudCaptureFilter] "
#include <NX_DbgMsg.h>

#define	NEW_SCHED_POLICY	SCHED_RR
#define	NEW_SCHED_PRIORITY	25
#define MAX_AUD_QCOUNT		128

#define BROKEN_PIPE_TEST	0
#define TIMESTAMP_CORRECT	1

#define	DUMP_PCM			0

#if( DUMP_PCM )
#define		PCM_DUMP_FILENAME	"/mnt/mmc/dump.pcm"
static FILE *outFp = NULL;
#endif

#if( 1 )
#define NxDbgColorMsg(A, B) do {										\
								if( gNxFilterDebugLevel>=A ) {			\
									printf("\033[1;37;41m%s", NX_DTAG);	\
									DEBUG_PRINT B;						\
									printf("\033[0m\r\n");				\
								}										\
							} while(0)
#else
#define NxDbgColorMsg(A, B) do {} while(0)
#endif

//------------------------------------------------------------------------------
CNX_AudCaptureFilter::CNX_AudCaptureFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0 )
	, m_Channels( 2 )
	, m_Frequency( 48000 )
	, m_Samples( 1152 )
	, m_PrevAudioSampleTime( 0 )
	, m_TotalReadSampleSize( 0 )
	, m_pSampleBuffer( NULL )
	, m_iNumOfBuffer( 0 )
	, m_hAudCapture( NULL )
{
	m_pRefClock			= CNX_RefClock::GetSingletonPtr();
	m_pSemOut			= new CNX_Semaphore(MAX_AUD_QCOUNT, 0);
	m_pOutStatistics	= new CNX_Statistics();

	NX_ASSERT( m_pSemOut );
}

//------------------------------------------------------------------------------
CNX_AudCaptureFilter::~CNX_AudCaptureFilter()
{
	if( true == m_bInit )
		Deinit();

	delete m_pSemOut;
	delete m_pOutStatistics;
}

//------------------------------------------------------------------------------
void	CNX_AudCaptureFilter::Init( NX_AUDCAPTURE_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );

	if( false == m_bInit ) {
		m_Channels	= pConfig->channels;
		m_Frequency	= pConfig->frequency;
		m_Samples	= pConfig->samples;
		NxDbgMsg( NX_DBG_INFO, (TEXT("Channel = %d, Frequency = %d\n"), m_Channels, m_Frequency) );

		AllocateBuffer( (m_Frequency + m_Samples) / m_Samples );
		
		// Sample Time Correct Parameter
		m_ClockCorrectThreshold = 2 * 1000 * m_Samples / m_Frequency;	// Time of Sample(mSec) = 1000 * m_Samples / m_Frequency
																		// ex) AAC / 48KHz -> 1000 * 1024 / 48000 = about 21mSec
		m_TotalReadSampleSize = 0;

		if( false == InitAudCapture( m_Channels, m_Frequency ) ) {
			FreeBuffer();
			NX_ASSERT( false );
			return ;
		}
		m_bInit = true;

#if( DUMP_PCM )
		outFp = fopen(PCM_DUMP_FILENAME, "wb+");
#endif
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_AudCaptureFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit ) {
		if( m_bRun )	Stop();

		CloseAudCapture();
		FreeBuffer();
		m_bInit = false;
	}

#if( DUMP_PCM )
	if( outFp ) {
		fclose(outFp);
		outFp = NULL;
	}		
#endif
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_AudCaptureFilter::Receive( CNX_Sample *pSample )
{
	NX_ASSERT( false );
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_AudCaptureFilter::ReleaseSample( CNX_Sample *pSample )
{
	m_SampleOutQueue.PushSample( pSample );
	m_pSemOut->Post();
	
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_AudCaptureFilter::Run( void )
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
int32_t	CNX_AudCaptureFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemOut->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void	CNX_AudCaptureFilter::AllocateBuffer( int32_t numOfBuffer )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( numOfBuffer <= MAX_BUFFER );

	m_SampleOutQueue.Reset();
	m_SampleOutQueue.SetQueueDepth( numOfBuffer );
	
	// Allocate Buffer Size
	// buffer size ( "PCM data is S16_LE" -> 16bit )
	// about 1sec buffer number ( numOfBuffer = (m_Frequency + m_Samples) / m_Samples )
	uint32_t unitBufferSize = m_Samples * 2 * m_Channels;
	m_pSampleBuffer = (uint8_t*)malloc( numOfBuffer * unitBufferSize );
	
	m_pSemOut->Init();
	if( NULL == m_pSampleBuffer ) {
		NX_ASSERT( m_pSampleBuffer );
		return;
	}
	
	for( int32_t i = 0; i < numOfBuffer; i++ ) {
		m_AudioSample[i].SetOwner( this );
		m_AudioSample[i].SetBuffer( m_pSampleBuffer + unitBufferSize * i, unitBufferSize);
		m_SampleOutQueue.PushSample( &m_AudioSample[i] );
		m_pSemOut->Post();
	}
	m_iNumOfBuffer = numOfBuffer;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()-- (m_iNumOfBuffer=%d)\n"), __func__, m_iNumOfBuffer) );
}

//------------------------------------------------------------------------------
void	CNX_AudCaptureFilter::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_SampleOutQueue.Reset();

	if( m_pSampleBuffer ) {
		free(m_pSampleBuffer);
		m_pSampleBuffer = NULL;
	}
	m_pSemOut->Post();		//	Send Dummy
	m_iNumOfBuffer = 0;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_AudCaptureFilter::GetSample( CNX_Sample **ppSample )
{
	NX_ASSERT( false );
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_AudCaptureFilter::GetDeliverySample( CNX_Sample **ppSample )
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
void	CNX_AudCaptureFilter::ThreadLoop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	CNX_VideoSample	*pSample = NULL;

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

	if( m_pRefClock ){
		m_PrevAudioSampleTime = m_pRefClock->GetCorrectTickCount();
	} else{
		m_PrevAudioSampleTime = NX_GetTickCount();
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

		if( false == CaptureAudioSample( pSample ) )
		{
			NxDbgMsg( NX_DBG_ERR, (TEXT("Audio Capture Error.\n")) );
			
			// Audio capture device reconfiguration. (Broken pipe case)
			CloseAudCapture();
			if( false == InitAudCapture( m_Channels, m_Frequency ) ) {
				NxDbgMsg( NX_DBG_ERR, (TEXT("InitAudCapture() failed!\n")) );
			}

			pSample->Unlock();
			continue;
		}

		int32_t nSampleCount = m_SampleOutQueue.GetSampleCount();
		if( nSampleCount <= 3 ) {
			NxDbgColorMsg( NX_DBG_VBS, (TEXT("SampleQueue is empty. ( SampleCount = %d )"), nSampleCount) );
		}
		
		m_pOutStatistics->CalculateFps();
		m_pOutStatistics->CalculateBufNumber( m_iNumOfBuffer - m_SampleOutQueue.GetSampleCount() );

		Deliver( pSample );
		pSample->Unlock();
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_AudCaptureFilter::ThreadMain( void *arg )
{
	CNX_AudCaptureFilter *pClass = (CNX_AudCaptureFilter *)arg;
	NX_ASSERT( NULL != pClass );

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
#define	AUD_CAP_DEVICE_NAME		"default"
//#define	AUD_CAP_DEVICE_NAME		"plug:dmix"
int32_t	CNX_AudCaptureFilter::InitAudCapture( uint32_t channels, uint32_t frequency )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	int32_t err;
	snd_pcm_hw_params_t *hw_params = NULL;
	NX_ASSERT( NULL==m_hAudCapture );

	if ((err = snd_pcm_open (&m_hAudCapture, AUD_CAP_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		NxErrMsg( (TEXT("cannot open audio device %s (%s)\n"), AUD_CAP_DEVICE_NAME, snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
	{
		NxErrMsg( (TEXT("cannot allocate hardware parameter structure (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_any (m_hAudCapture, hw_params)) < 0)
	{
		NxErrMsg( (TEXT("cannot initialize hardware parameter structure (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_access (m_hAudCapture, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		NxErrMsg( (TEXT("cannot set access type (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_format (m_hAudCapture, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
	{
		NxErrMsg( (TEXT("cannot set sample format (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_rate_near (m_hAudCapture, hw_params, &frequency, 0)) < 0)
	{
		NxErrMsg( (TEXT("cannot set sample rate (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params_set_channels (m_hAudCapture, hw_params, channels)) < 0)
	{
		NxErrMsg( (TEXT("cannot set channel count (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}

	if ((err = snd_pcm_hw_params (m_hAudCapture, hw_params)) < 0)
	{
		NxErrMsg( (TEXT("cannot set parameters (%s)\n"),snd_strerror (err)));
		goto error_exit;
	}
	snd_pcm_hw_params_free (hw_params);

	if( (err = snd_pcm_prepare( m_hAudCapture )) <0 )
	{
		NxErrMsg( (TEXT("cannot prepare audio interface for use (%s)\n"), snd_strerror(err)));

	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;

error_exit:
	if( hw_params )
		snd_pcm_hw_params_free (hw_params);
	
	if( m_hAudCapture ) {
		snd_pcm_close(m_hAudCapture);
		m_hAudCapture = NULL;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_AudCaptureFilter::CaptureAudioSample( CNX_MediaSample *pSample )
{
	uint8_t *pBuf;
	uint32_t bufSize;
	uint64_t timeStamp;
	int32_t totalReadSize = 0, readSize;

	if( NULL == m_hAudCapture ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("m_hAudCpature is NULL\n")) );
		NX_ASSERT( m_hAudCapture );
		return false;
	}

	if( true == pSample->GetBuffer( &pBuf, (int32_t*)&bufSize ) ) {
		bufSize /= ( 2 * m_Channels );
		while( bufSize > 0 )
		{
#if( BROKEN_PIPE_TEST )
			static int32_t broken_pipe_cnt = 0;
			broken_pipe_cnt++;
			if( !(broken_pipe_cnt % 500) ) {
				NxDbgMsg( NX_DBG_DEBUG, (TEXT("occur broken pipe\n")) );
				usleep(1000000);
			}
#endif
			readSize = snd_pcm_readi( m_hAudCapture, pBuf, bufSize );

			if( readSize < 0 ) {
				NxDbgMsg( NX_DBG_ERR, (TEXT("snd_pcm_readi() Failed. ( %s )\n"), snd_strerror(readSize)) );
				return false;
			}
			totalReadSize += readSize;
			bufSize -= (readSize);
			pBuf += (readSize * 2 * m_Channels);
		}
		
#if( DUMP_PCM )
		if( outFp ) {
			NxDbgMsg( NX_DBG_DEBUG, (TEXT("pBuf = %p, size = %d\n"), pBuf, totalReadSize) );
			fwrite( pBuf, 1, totalReadSize, outFp );
		}		
#endif
	}
	else {
		NxDbgMsg( NX_DBG_ERR, (TEXT("GetBuffer() Failed\n")) );
		return false;
	}

#if( TIMESTAMP_CORRECT )
	m_TotalReadSampleSize += totalReadSize;
	
	// Expected TimeStamp
	timeStamp = m_PrevAudioSampleTime + (m_TotalReadSampleSize * 1000) / m_Frequency;

	if( m_pRefClock ){
		int64_t Gap = timeStamp - m_pRefClock->GetCorrectTickCount();
		
		if( Gap > m_ClockCorrectThreshold ) {
			m_pRefClock->SetTickErrorTime( 3 );
			NxDbgMsg( NX_DBG_VBS, (TEXT("\tCorrect audio time stamp(+1, %lld)!!\n"), m_ClockCorrectThreshold) );
		}
		else if( Gap < -m_ClockCorrectThreshold ) {
			m_pRefClock->SetTickErrorTime( -3 );
			NxDbgMsg( NX_DBG_VBS, (TEXT("\tCorrect audio time stamp(-1, %lld)!!\n"), m_ClockCorrectThreshold) );
		}
	}
#else
	if( m_pRefClock ){
		timeStamp = m_pRefClock->GetCorrectTickCount();
	} else{
		timeStamp = NX_GetTickCount();
	}
#endif	

	pSample->SetTimeStamp( timeStamp );
	pSample->SetActualDataLength( totalReadSize * 2 * m_Channels );

	return true;
}

//------------------------------------------------------------------------------
void CNX_AudCaptureFilter::CloseAudCapture( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( NULL==m_hAudCapture ) {
		NX_ASSERT( m_hAudCapture );
		return;
	}
	snd_pcm_close( m_hAudCapture );
	m_hAudCapture = NULL;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t  CNX_AudCaptureFilter::GetStatistics( NX_FILTER_STATISTICS *pStatistics )
{
	return true;
}