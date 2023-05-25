//------------------------------------------------------------------------------
//
//	Copyright (C) 2010 Nexell co., Ltd All Rights Reserved
//	Nexell Proprietary & Confidential
//
//	Module     : 
//	File       : 
//	Description:
//	Author     : RayPark
//	History    :
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h> 		// for open/close, getpid
#include <fcntl.h> 			// for O_RDWR
#include <sys/ioctl.h>	 	// for ioctl
#include <asm/unistd.h> 	// __NR_gettid
#include <sched.h> 			// schedule
#include <linux/sched.h> 	// SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH
#include <alsa/asoundlib.h>

#include "NX_SystemCall.h"
#include "NX_CAudioRenderFilter.h"
#include "NX_CodecID.h"


#define	DEFAULT_SAMPLE_FREQ	(44100)
#define	DEFAULT_CHANNELS	(2)
#define	TIME_RESOLUTION		(1000)

//////////////////////////////////////////////////////////////////////////////
//
//							NX_CRenderFilter
//
#define DEVNODE		"/dev/timer"

NX_CAudioRenderFilter::NX_CAudioRenderFilter(const char *filterName)
	: NX_CBaseFilter(filterName)
	, m_pInputPin(NULL)
	, m_Hdmi(-1)
	, m_NumAudioBuffer(16)
	, m_SizeAudioBuffer(1024 * 4)
	, m_SamplingFrequency( DEFAULT_SAMPLE_FREQ )
	, m_NumChannels(DEFAULT_CHANNELS)
	, m_MaxSleepTime(RENDERER_MAX_SLEEP_TIME)
{
	m_pInputPin = (NX_CAudioRenderInputPin*)new NX_CAudioRenderInputPin( this );
	m_pInQueue = new NX_CSampleQueue(MAX_NUM_WAVE_OUT_BUF_AR);
	m_pWaveOut = new NX_CWaveOut();

	memset(m_cArrFilterName, 0, sizeof(m_cArrFilterName));
	memset(m_cArrFilterID, 0, sizeof(m_cArrFilterID));

	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}

#ifdef _DEBUG
	m_DbgInFrameCnt = 0;
	m_DbgOutFrameCnt = 0;
	m_DbgDropFrameCnt= 0;
#endif
}

NX_CAudioRenderFilter::~NX_CAudioRenderFilter()
{
	if( m_pInputPin )
		delete m_pInputPin;
	if( m_pInQueue )
		delete m_pInQueue;
	if( m_pWaveOut )
		delete m_pWaveOut;

	pthread_mutex_destroy(&m_hMutexThread);
}


NX_CBasePin *NX_CAudioRenderFilter::GetPin( int32_t Pos )
{
	if( 0 == Pos && 0 != m_pInputPin )
	{
		return m_pInputPin;
	}
	return NULL;
}

int32_t	NX_CAudioRenderFilter::RenderSample( NX_CSample *pSample )
{
	//	Push Sample
	int32_t ret = 0;
	if( m_pInQueue ){
		pSample->AddRef();
//		printf("=== NX_CAudioRenderFilter RenderSample %d ++\n", m_pInQueue->GetSampleCount());
		ret = m_pInQueue->PushSample( pSample );
//		printf("=== NX_CAudioRenderFilter RenderSample %d -- \n", m_pInQueue->GetSampleCount());
		return ret;		
	}
	return 0;
}

CBOOL NX_CAudioRenderFilter::Run()
{
	if( CFALSE == m_bRunning )
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend ] Run() ++\n");

		if( m_pInQueue )
			m_pInQueue->ResetQueue();
		m_bExitThread = CFALSE;
		m_bPaused = CFALSE;

		if( pthread_create( &m_hThread, NULL, ThreadStub, this ) < 0 )
		{
			return CFALSE;
		}

		if( m_pInputPin && CTRUE==m_pInputPin->IsConnected() )
		{
			m_pInputPin->Active();
		}

		m_bRunning = CTRUE;

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend ] Run() --\n");
	}
	else if (CTRUE == m_bRunning)
	{
		if (CTRUE == m_bPaused){
			m_bPaused = CFALSE;
			m_pRefClock->ResumeRefTime();
		}
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend ] m_bPaused = %d --\n", m_bPaused);
	return CTRUE;
}

CBOOL NX_CAudioRenderFilter::Flush()
{

	return CTRUE;
}

CBOOL NX_CAudioRenderFilter::Stop()
{
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Stop() ++\n");

	//	Set Thread End Command
	ExitThread();
	//	Send Queue Emd Message
	if( m_pInQueue )
		m_pInQueue->EndQueue();

	//	Input Inactive
	if( m_pInputPin )
		m_pInputPin->Inactive();

	//	Wait Thread Exit
	if( m_bRunning ){
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Stop() : pthread_join() ++\n");
		pthread_join( m_hThread, NULL );
		m_hThread = 0;
		m_bRunning = CFALSE;
		m_Hdmi = -1;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Stop() : pthread_join() --\n");
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Stop() --\n");
	return CTRUE;
}

CBOOL NX_CAudioRenderFilter::Pause( void )
{
	m_bPaused = CTRUE;
	m_pRefClock->PauseRefTime();
	
	return CTRUE;
}

void NX_CAudioRenderFilter::Resume(void)
{
	m_bPaused = CFALSE;
	m_SemPause.Post();
}

void *NX_CAudioRenderFilter::ThreadStub( void *pObj )
{
	if( NULL != pObj )
	{
		((NX_CAudioRenderFilter*)pObj)->ThreadProc();
	}
	return (void*)(0);
}

#define ARFILEWRITE	0
#if ARFILEWRITE	//File Write
static FILE	*hFile = NULL;
static int32_t count = 0;
static int32_t init = 0;
static int32_t i = 0;
#endif

void NX_CAudioRenderFilter::ThreadProc()
{
	NX_CSample *pSample;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] ThreadProc() : Freq %d, Channels %d, NumBuf=%d, SizeBuf=%d\n", m_SamplingFrequency, m_NumChannels, m_NumAudioBuffer, m_SizeAudioBuffer);
	if (m_pWaveOut->WaveOutOpen((uint32_t)m_SamplingFrequency, m_NumChannels, m_FormatType, m_NumAudioBuffer, m_SizeAudioBuffer, m_Hdmi) < 0)
	{
		NX_ErrMsg( ("[Filter|ARend] NX_CAudioRenderFilter::ThreadProc : WaveOpen Failed!!!\n") );
	}

	m_PrevEndTime = -1;
	m_ClockRefreshCnt = 0;

#if ARFILEWRITE	//File Write
	if (init == 0)
	{
		hFile = fopen("/root/filter_arender_pcm.out", "wb");
		init = 1;
	}
#endif
	while(1)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] ThreadProc Start ++ !!!\n");

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){
			pthread_mutex_unlock(&m_hMutexThread);
			NX_DbgMsg(DBG_MSG_ON, "[Filter|ARend]  m_bExitThread = %d !!\n", m_bExitThread);
			break;
		}
		pthread_mutex_unlock(&m_hMutexThread);

		while (m_bPaused && !m_bExitThread)
		{
			NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] m_bPaused = %d\n", m_bPaused);
			usleep(3000);
		}

		pSample = NULL;
		if( m_pInQueue->PopSample( &pSample  ) < 0 )
			break;
		if( pSample->IsEndOfStream() ){			
			pSample->SetEndOfStream( CFALSE );
			pSample->Release();
			m_pEventNotify->SendEvent(EVENT_END_OF_STREAM, 0);
			NX_DbgMsg(DBG_MSG_ON, "[Filter|ARend] Deliver EndofStream\n");
			break;
		}		

		m_Discontinuity = pSample->IsDiscontinuity();

		if( pSample )
		{
			uint32_t *pBuf = NULL;
			int32_t Size, Error;
			int64_t InTime = 0, CurrRefTime = 0;

			if( m_PrevEndTime == -1 ){
				m_pRefClock->GetCurrTime(&CurrRefTime);
			}

			//	Get Input Information
			pSample->GetPointer( &pBuf );
			Size = pSample->GetActualDataLength();
#if ARFILEWRITE	//File Write
			count++;
			fwrite(pBuf, 1, Size, hFile);
			if (count == 400){
				printf("=====exit ==== \n");
				fclose(hFile);
				exit(1);
			}
#endif		
	

#ifdef _DEBUG
			m_DbgInFrameCnt++;
#endif


			Error = 0;

			m_pWaveOut->WaveOutPlayBuffer( pBuf, Size, Error );
			if( Error < 0 ){
				m_Discontinuity = CTRUE;
			}

			pSample->GetTime(&InTime);
			InTime = InTime - m_pWaveOut->GetBufferedTime();
			m_pRefClock->GetCurrTime(&m_RefTime);
			if (m_RefTime > InTime){
				int32_t Diff = 0;
				Diff = (int32_t)(m_RefTime - InTime);
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Diff < AUDIO_DUMMY_LIMIT: Diff = %d, m_RefTime = %lld,  InTime = %lld\n", Diff, m_RefTime, InTime);
				if (Diff < AUDIO_DUMMY_LIMIT)
				{
					if (Diff != 0 && Diff > 5){
						//	Wait Time
						NX_DbgMsg(DBG_MSG_OFF, "[Filter|Arend] dummy( %d )\n", Diff);
//						m_pWaveOut->WriteDummy(Diff);
					}
				}
			}
			else if (InTime >= m_RefTime)
			{
				int32_t Diff = 0;
				Diff = InTime - m_RefTime;
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Diff > AUDIO_DUMMY_LIMIT: Diff = %d, m_RefTime = %lld,  InTime = %lld\n", Diff, m_RefTime, InTime);
				if (Diff > AUDIO_DUMMY_LIMIT){
					m_pRefClock->AdjustMediaTime(Diff);
				}
			}						
			pSample->Release();
		}else{
			break;
		}

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] ThreadProc Start -- !!!\n");
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] WaveOutClose\n");

	if( m_pWaveOut ) 
		m_pWaveOut->WaveOutClose();

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] Exit renderer thread!!\n");
}
//
//////////////////////////////////////////////////////////////////////////////

void NX_CAudioRenderFilter::ChangeBufferingTime( uint32_t BufferingTime )
{
	m_MaxSleepTime = (int32_t)(BufferingTime);
}

/////////////////////////////////////////////////////////////////////////////////
//
//						Debug Informations
//

CBOOL NX_CAudioRenderFilter::PinInfo(NX_PinInfo *pPinInfo)
{
	pPinInfo->InPutNum = 1;
	pPinInfo->OutPutNum = 1;

	return 0;
}

NX_CBasePin *NX_CAudioRenderFilter::FindPin(int32_t Pos)
{
	return GetPin(Pos);
}

CBOOL NX_CAudioRenderFilter::FindInterface(const char *InterfaceId, void **Interface)
{
//	if (!strcmp(InterfaceId, "IMediaControl")){
//		*Interface = (NX_IMediaControl *)(this);
//	}

	return 0;
}

int32_t NX_CAudioRenderFilter::SetMediaType(void *Mediatype)
{

	memcpy(&m_AMediaType, Mediatype, sizeof(NX_AudioMediaType));

	NX_DbgMsg(DBG_MSG_ON, "[Filter|ARend] ++ m_AMediaType.CodecID = %d, m_AMediaType.Samplerate = %d, m_AMediaType.FormatType= %d, m_AMediaType.Channels = %d\n",
										     m_AMediaType.CodecID, m_AMediaType.Samplerate, m_AMediaType.FormatType, m_AMediaType.Channels);

	m_SamplingFrequency = m_AMediaType.Samplerate;
	if (m_AMediaType.Channels > 2)
		m_NumChannels = 2;
	else
		m_NumChannels = m_AMediaType.Channels;
//	m_FormatType = m_AMediaType.FormatType;
	m_FormatType = A_SAMPLE_FMT_S16;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ARend] -- m_SamplingFrequency = %d, m_NumChannels = %d, m_FormatType= %d\n",	m_SamplingFrequency, m_NumChannels, m_FormatType);
	
	return 0;
}

void NX_CAudioRenderFilter::SetClockReference(NX_CClockRef *pClockRef)
{
	SetClockRef(pClockRef);
}

void NX_CAudioRenderFilter::SetEventNotifi(NX_CEventNotifier *pEventNotifier)
{
	SetEventNotifier(pEventNotifier);
}


//
//////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////////
//
//						Debug Informations
//
void NX_CAudioRenderFilter::DisplayVersion()
{
}

void NX_CAudioRenderFilter::DisplayState()
{
}

void NX_CAudioRenderFilter::GetStatistics(uint32_t *InPcmFrames, uint32_t *OutPcmFrames, uint32_t *DropPcmFrames, uint32_t *State)
{
	*InPcmFrames   = m_DbgInFrameCnt ;
	*OutPcmFrames  = m_DbgOutFrameCnt;
	*DropPcmFrames = m_DbgDropFrameCnt;
	*State         = 0;
}

void NX_CAudioRenderFilter::ClearStatistics()
{
	m_DbgInFrameCnt = 0;
	m_DbgOutFrameCnt= 0;
	m_DbgDropFrameCnt = 0;
}
//
/////////////////////////////////////////////////////////////////////////////////
#endif



//////////////////////////////////////////////////////////////////////////////
//
//						MES Audio CRender Input Pin Class
//

NX_CAudioRenderInputPin::NX_CAudioRenderInputPin(	NX_CAudioRenderFilter *pOwnerFilter ) :
	NX_CBaseInputPin( (NX_CBaseFilter*)pOwnerFilter )
{
}
NX_CAudioRenderInputPin::~NX_CAudioRenderInputPin()
{
}

int32_t NX_CAudioRenderInputPin::Receive( NX_CSample *pSample )
{
	//return 0;	//
	if( !m_pOwnerFilter )
		return 0;

	if( IsConnected() && IsActive() ){
		return ((NX_CAudioRenderFilter*)m_pOwnerFilter)->RenderSample( pSample );
	}
	return 0;
}

int32_t NX_CAudioRenderInputPin::CheckMediaType(void *Mediatype)
{
	NX_AudioMediaType *pType = NULL;
	NX_MediaType *MType = (NX_MediaType *)Mediatype;

	pType = &MType->AMediaType;

	if (AUDIOTYPE != MType->MeidaType)
		return -1;

	if ((pType->Samplerate < 8000) || (pType->Samplerate > 96000)){
		NX_ErrMsg(("[Filter|Arend] Not Support Samplerate = %d\n", pType->Samplerate));
		return -1;
	}
	else if ((pType->Channels < 1) || (pType->Channels > 6)){
		NX_ErrMsg(("[Filter|Arend] Not Support Channels = %d\n", pType->Channels));
		return -1;
	}

	NX_CAudioRenderFilter *pOwner = (NX_CAudioRenderFilter *)m_pOwnerFilter;

	pOwner->SetMediaType(&MType->AMediaType);

	return 0;
}
//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Win32 WaveOut Module
//
NX_CWaveOut::NX_CWaveOut()
{
	m_hPlayback = NULL;
	m_pHWParam = NULL;
	m_MaxAvailBuffer = 0;
	m_SamplingFreq = DEFAULT_SAMPLE_FREQ;
	m_TimePerSample = ((double)TIME_RESOLUTION)/((double)DEFAULT_SAMPLE_FREQ);
	memset( m_Dummy, 0, sizeof(m_Dummy) );
}
NX_CWaveOut::~NX_CWaveOut()
{
	WaveOutClose();
}


int32_t NX_CWaveOut::WaveOutOpen(uint32_t SampleFreq, uint32_t Channels, int32_t FormatType, uint32_t NumBuf, uint32_t SizeBuf, int32_t Hdmi)
{
	int32_t Ret = 0;
	unsigned int SampleFormat = -1;

	m_PeriodBufSize	= SizeBuf;
	m_Periods		= NumBuf;
	m_SamplingFreq	= SampleFreq;
	m_Channels		= Channels;

	if( 0 != m_SamplingFreq ){
		m_TimePerSample = ((double)TIME_RESOLUTION)/((double)m_SamplingFreq);
	}

	if (ON != Hdmi){
		//	if( (Ret=snd_pcm_open(&m_hPlayback, "plug:dmix", SND_PCM_STREAM_PLAYBACK, 0)) < 0 ){
		if ( (Ret = snd_pcm_open(&m_hPlayback, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0){
			NX_ErrMsg(("[BB|ARend] WaveOutOpen : snd_pcm_open failed!!!(%d)\n", Ret));
			m_hPlayback = NULL;
			return -1;
		}

	}
	else if (ON == Hdmi){
		if ( (Ret = snd_pcm_open(&m_hPlayback, "default:CARD=SPDIFTranscieve", SND_PCM_STREAM_PLAYBACK, 0)) < 0){	//hdmi
			NX_ErrMsg(("[BB|ARend] WaveOutOpen : snd_pcm_open failed!!!(%d)\n", Ret));
			m_hPlayback = NULL;
			return -1;
		}
	}

	if( 0 != (Ret=snd_pcm_hw_params_malloc( &m_pHWParam )) )
	{
		NX_ErrMsg( ("[BB|ARend] WaveOutOpen : snd_pcm_hw_params_malloc failed!!!(%d)\n", Ret) );
	}
	snd_pcm_hw_params_any( m_hPlayback, m_pHWParam );

	if( snd_pcm_hw_params_set_access(m_hPlayback, m_pHWParam, SND_PCM_ACCESS_RW_INTERLEAVED) < 0 ){
		NX_ErrMsg( ("[BB|ARend] WaveOutOpen : snd_pcm_hw_params_set_access failed!!!\n") );
	}

	switch (FormatType) {
	case SAMPLE_FOMAT_U8:
		SampleFormat = SND_PCM_FORMAT_U8;
		break;
	case SAMPLE_FOMAT_S16:
		SampleFormat = SND_PCM_FORMAT_S16_LE;
		break;
	case SAMPLE_FOMAT_S32:
		SampleFormat = SND_PCM_FORMAT_S32_LE;
		break;
	case SAMPLE_FOMAT_FLT:
		SampleFormat = SND_PCM_FORMAT_FLOAT_LE;
		break;
	case SAMPLE_FOMAT_DBL:
		SampleFormat = SND_PCM_FORMAT_FLOAT64_LE;
		break;
	default:
		SampleFormat = 0;
	}


	//	PCM Signed Little Endian
//	if( snd_pcm_hw_params_set_format ( m_hPlayback, m_pHWParam, SND_PCM_FORMAT_S16_LE ) < 0 ){
	if (snd_pcm_hw_params_set_format(m_hPlayback, m_pHWParam, (snd_pcm_format_t)SampleFormat) < 0){
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_set_format failed!!!\n") );
	}

	Ret = snd_pcm_hw_params_set_rate_near(m_hPlayback, m_pHWParam, &m_SamplingFreq, 0);				//	Set Sampling Freq
	if( Ret < 0 )	NX_ErrMsg(("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_set_rate_near failed!!!\n"));
	if( SampleFreq != m_SamplingFreq )	NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : Warnning : snd_pcm_hw_params_set_rate_near(%d,%d)!!!\n", Ret, m_SamplingFreq));

	//	Set Channels
	if( snd_pcm_hw_params_set_channels( m_hPlayback, m_pHWParam, Channels ) < 0 ){
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_set_format failed!!!\n") );
	}

	if( snd_pcm_hw_params_set_periods_near( m_hPlayback, m_pHWParam, &m_Periods, 0 ) < 0 )
	{
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_set_periods_near failed!!!\n") );
	}else{
		snd_pcm_hw_params_set_periods( m_hPlayback, m_pHWParam, m_Periods, 0 );
	}

	if( snd_pcm_hw_params_set_period_size_near( m_hPlayback, m_pHWParam, (snd_pcm_uframes_t*)&m_PeriodBufSize, 0 ) < 0 )
	{
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_set_period_size_near failed!!!\n") );
	}else{
		snd_pcm_hw_params_set_period_size( m_hPlayback, m_pHWParam, m_PeriodBufSize, 0 );
	}

	m_MaxAvailBuffer = m_PeriodBufSize * m_Periods;

	if( snd_pcm_hw_params_set_buffer_size_near(m_hPlayback, m_pHWParam, &m_MaxAvailBuffer) < 0 ){
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_set_buffer_size failed!!!\n") );
	}

	if( snd_pcm_hw_params( m_hPlayback, m_pHWParam ) < 0 ){
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params failed!!!\n") );
	}

	if( snd_pcm_hw_params_get_buffer_size(m_pHWParam, &m_MaxAvailBuffer) < 0 ){
		NX_ErrMsg( ("[Filter|ARend] WaveOutOpen : snd_pcm_hw_params_get_buffer_size failed!!!\n") );
	}

	NX_DbgMsg( DBG_TRACE, "[Filter|ARend] >>>>>  PeriodBufSize(%d), Periods(%d), Channel(%d), Max(%d)  <<<<<\n", m_PeriodBufSize, m_Periods, m_Channels, m_MaxAvailBuffer );

	if ((Ret = snd_pcm_prepare(m_hPlayback)) < 0) {
		NX_ErrMsg( ("[Filter|ARend] Alsa : Prepare error: %s\n", snd_strerror(Ret)) );
	}
	return Ret;
}

int32_t NX_CWaveOut::WaveOutPlayBuffer( uint32_t *pBuf, int32_t Size, int32_t &Error )
{
	int32_t Written;
	if( !m_hPlayback )		return -1;

	Error = snd_pcm_avail(m_hPlayback );
	if( Error < 0 ){
		NX_DbgMsg( DBG_MSG_ON, "[Filter|ARend] ===>>> ALSA Error(%d) <<<===\n", Error );
		snd_pcm_recover( m_hPlayback, Error, 1 );
	}
	Written = snd_pcm_writei(m_hPlayback, pBuf, Size>>m_Channels);
	if( Written < 0 )
	{
		snd_pcm_recover( m_hPlayback, Written, 1 );
		NX_DbgMsg(DBG_MSG_ON, "[Filter|ARend] ALSA Error(%d)\n", Written);
		return -1;
	}
	return 0;
}

void NX_CWaveOut::WriteDummy( uint32_t Time )
{
	int32_t Err;
	uint32_t Mad, Remain, DummyBufSize, SampleSize;

	if( NULL==m_hPlayback || 0==Time )	return;
	SampleSize = (Time * m_SamplingFreq) / TIME_RESOLUTION;
	if( SampleSize  == 0 ) return;

	DummyBufSize = sizeof(m_Dummy)>>m_Channels;
	Mad = SampleSize/DummyBufSize;
	Remain = SampleSize%DummyBufSize;

	for( uint32_t i=0; i<Mad ; i++ )
	{
		if( (Err = snd_pcm_writei( m_hPlayback, m_Dummy, DummyBufSize )) < 0 ){
			snd_pcm_recover( m_hPlayback, Err, 1 );
			return;
		}
	}
	if( Remain )
	{
		if( (Err = snd_pcm_writei( m_hPlayback, m_Dummy, Remain )) < 0 ){
			snd_pcm_recover( m_hPlayback, Err, 1 );
			return;
		}
	}
}

int64_t NX_CWaveOut::GetBufferedTime()
{
	snd_pcm_sframes_t avail;
	if( NULL==m_hPlayback )		return 0;

	if( (avail = snd_pcm_avail( m_hPlayback )) <= 0 ){
		avail = 0;
	}
	return (int64_t)( (m_MaxAvailBuffer - avail) * m_TimePerSample );
}

int64_t NX_CWaveOut::FindDuration( uint32_t Size )
{
	if( NULL==m_hPlayback || 0==m_Channels )		return 0;
	//	CHECKME
	Size = Size / 2 / m_Channels;
	return (int64_t)( ((float)Size) * m_TimePerSample );
}

int32_t NX_CWaveOut::WaveOutReset()
{
	if( !m_hPlayback )		return -1;
	return 0;
}
void NX_CWaveOut::WaveOutClose()
{
	if( m_hPlayback )
	{
		//	stop playing & drop pending frames
		snd_pcm_drop(m_hPlayback);
		snd_pcm_prepare( m_hPlayback );
		snd_pcm_close(m_hPlayback);
		m_hPlayback = NULL;
	}

	if( m_pHWParam )
	{
		snd_pcm_hw_params_free (m_pHWParam);
		//	Dealloc
		m_pHWParam = NULL;
	}
	return ;
}

int32_t NX_CWaveOut::WaveOutCheckAvailable()
{
	return 0;
}


NX_IBaseFilter *CreateAudioRenderFilter(const char *filterName)
{
	return new NX_CAudioRenderFilter(filterName);
}
//
//
//////////////////////////////////////////////////////////////////////////////
