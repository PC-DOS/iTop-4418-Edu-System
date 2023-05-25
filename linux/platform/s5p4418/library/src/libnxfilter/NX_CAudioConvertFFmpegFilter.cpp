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
#include <stdlib.h>
#include <string.h>

#include "NX_CAudioConvertFFmpegFilter.h"

#define INPUT_SAMPLE_QUE	42
#define OUTPUT_SAMPLE_QUE	42
//#define OUTPUT_SAMPLE_QUE	3
#define AVCODEC_MAX_AUDIO_FRAME_SIZE	192000


//////////////////////////////////////////////////////////////////////////////
//
//							Nexell AudioConvert Filter
//
//							----------
//			(input pin)   --|        |--- (out putpin)
//							----------
NX_CAudioConvertFFmpegFilter::NX_CAudioConvertFFmpegFilter(const char *filterName) :
	m_pInputPin(NULL),
	m_pOutputPin(NULL),
	m_pReSampleCtx(NULL)
{
	m_pInputPin = new NX_CAudioConvertInputPin(this);
	m_pOutputPin = new NX_CAudioConvertOutputPin(this);
	m_pInQueue = new NX_CSampleQueue(INPUT_SAMPLE_QUE);

	memset(m_cArrFilterName, 0, sizeof(m_cArrFilterName));
	memset(m_cArrFilterID, 0, sizeof(m_cArrFilterID));
	SetFilterName( filterName );

	m_CurrentVolume = 1;
	m_RequestVolume = 1;
	memset(&m_ResampleSt, 0, sizeof(m_ResampleSt));

	avcodec_init();

	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}
}

NX_CAudioConvertFFmpegFilter::~NX_CAudioConvertFFmpegFilter()
{
	if( m_pInputPin )	delete m_pInputPin;
	if( m_pOutputPin )	delete m_pOutputPin;
	if (m_pInQueue)		delete m_pInQueue;

	pthread_mutex_destroy(&m_hMutexThread);
}

NX_CBasePin *NX_CAudioConvertFFmpegFilter::GetPin(int Pos)
{
	if( PIN_DIRECTION_INPUT == Pos )
		return m_pInputPin;
	if( PIN_DIRECTION_OUTPUT == Pos )
		return m_pOutputPin;
	return NULL;
}

int32_t	NX_CAudioConvertFFmpegFilter::TransFormSample(NX_CSample *pSample)
{
	int32_t ret = -1;
	if (m_pInQueue)
	{
		pSample->AddRef();
		ret = m_pInQueue->PushSample(pSample);

		return ret;
	}
	return -1;
}

CBOOL NX_CAudioConvertFFmpegFilter::Run()
{
	if (CFALSE == m_bRunning)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] Run() ++\n");

		m_pInQueue->ResetQueue();
		if (m_pInputPin && m_pInputPin->IsConnected())
		{
			m_pInputPin->Active();
		}
		if (m_pOutputPin && m_pOutputPin->IsConnected())
		{
			m_pOutputPin->Active();
		}
		m_bPaused = CFALSE;
		m_bExitThread = CFALSE;

		if (pthread_create(&m_hThread, NULL, ThreadStub, this) < 0)
		{
			return CFALSE;
		}
		m_bRunning = CTRUE;

		if (m_pReSampleCtx)
			audio_resample_close(m_pReSampleCtx);

		m_ResampleSt.input_channels = m_AMediaType.Channels;
		m_ResampleSt.input_rate = m_AMediaType.Samplerate;
		m_ResampleSt.input_sample_fmt = (AVSampleFormat)m_AMediaType.FormatType;
//		m_ResampleSt.input_sample_fmt = AV_SAMPLE_FMT_int32_t;

		if (2 > m_AMediaType.Channels)
			m_ResampleSt.output_channels = m_AMediaType.Channels;
		else
			m_ResampleSt.output_channels = 2;

		m_ResampleSt.output_rate = m_AMediaType.Samplerate;
		m_ResampleSt.output_sample_fmt = AV_SAMPLE_FMT_S16; 

		m_ResampleSt.filter_length = 16; 
		m_ResampleSt.log2_phase_count = 10; 
		m_ResampleSt.linear = 0;
		m_ResampleSt.cutoff = 0.8;

		

#if 0
		printf("\n=================================================================\n");
		printf(" m_ResampleSt.input_channels = %d\n",	m_ResampleSt.input_channels);
		printf(" m_ResampleSt.input_rate = %d\n",		m_ResampleSt.input_rate);
		printf(" m_ResampleSt.sample_fmt_in = %d\n",	m_ResampleSt.input_sample_fmt);
		printf(" m_ResampleSt.output_channels = %d\n",	m_ResampleSt.output_channels);
		printf(" m_ResampleSt.output_rate = %d\n",		m_ResampleSt.output_rate);
		printf(" m_ResampleSt.sample_fmt_out = %d\n",	m_ResampleSt.output_sample_fmt);
		printf("\n=================================================================\n");
#endif

		m_pReSampleCtx = av_audio_resample_init(m_ResampleSt.output_channels, m_ResampleSt.input_channels,
												m_ResampleSt.output_rate, m_ResampleSt.input_rate,
												m_ResampleSt.output_sample_fmt, m_ResampleSt.input_sample_fmt,
												m_ResampleSt.filter_length, m_ResampleSt.log2_phase_count, m_ResampleSt.linear, m_ResampleSt.cutoff
												);

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] Run() --\n");
	}
//	NX_DbgMsg(DBG_MSG_ON, "[FilterName : %s, FilterID: %s ] Run() --\n", this->GetFilterName(), this->GetFilterId() );
	return NX_CBaseFilter::Run();
}

CBOOL NX_CAudioConvertFFmpegFilter::Stop()
{
	NX_DbgMsg( DBG_MSG_OFF, "[Filter|AConvert ] Stop() ++\n" );

	//	Set Thread End Command
	ExitThread();
	//	Send Queue Emd Message
	if (m_pInQueue)
		m_pInQueue->EndQueue();

	if( m_pInputPin && m_pInputPin->IsConnected() )
	{
		m_pInputPin->Inactive();
	}
	if( m_pOutputPin && m_pOutputPin->IsConnected() )
	{
		m_pOutputPin->Inactive();
	}

	if (CTRUE == m_bRunning)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert] Stop() : pthread_join() ++\n");
		m_bExitThread = CTRUE;
		pthread_join(m_hThread, NULL);
		m_hThread = 0;
		m_bRunning = CFALSE;
		m_pReSampleCtx = NULL;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert] Stop() : pthread_join() --\n");
	}

	if (m_pReSampleCtx)
		audio_resample_close(m_pReSampleCtx);

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] Stop() --\n");

	return NX_CBaseFilter::Stop();
}

CBOOL NX_CAudioConvertFFmpegFilter::Pause()
{
	return CTRUE;
}


CBOOL NX_CAudioConvertFFmpegFilter::Flush()
{
	
	return CTRUE;
}

void *NX_CAudioConvertFFmpegFilter::ThreadStub(void *pObj)
{
	if (NULL != pObj)
	{
		((NX_CAudioConvertFFmpegFilter*)pObj)->ThreadProc();
	}
	return (void*)(0);
}

int32_t NX_CAudioConvertFFmpegFilter::GetBitsPerSampleFormat(enum SampleFormat sample_fmt) {
	switch (sample_fmt) {
	case SAMPLE_FMT_U8:
		return 8;
	case SAMPLE_FMT_S16:
		return 16;
	case SAMPLE_FMT_S32:
	case SAMPLE_FMT_FLT:
		return 32;
	case SAMPLE_FMT_DBL:
		return 64;
	default:
		return 0;
	}
}

#define ADFILEWRITE	0
#if ADFILEWRITE	//File Write
static FILE	*hFile = NULL;
static int32_t count = 0;
static int32_t init = 0;
static int32_t i = 0;
#endif


void NX_CAudioConvertFFmpegFilter::ThreadProc()
{
	while(1)
	{
		NX_CSample *pOutSample = NULL;
		NX_CSample *pInSample = NULL;

		uint8_t *pInBuf = NULL;
		int16_t *pOutBuf = NULL;
		int32_t InSize = 0; 
		CBOOL Valid = CFALSE;
		int32_t OutSamples = 0;

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert]  ThreadProc() Start !!!\n");

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){
			NX_DbgMsg(DBG_MSG_ON, "[Filter|AConvert]  m_bExitThread = %d !!\n", m_bExitThread);
			pthread_mutex_unlock(&m_hMutexThread);
			break;
		}
		pthread_mutex_unlock(&m_hMutexThread);

//		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert]  m_pInQueue->GetSampleCount() = %d\n", m_pInQueue->GetSampleCount());
		if (m_pInQueue->PopSample(&pInSample) < 0)
		{
			printf(":%s:%s:Line(%d) : m_pInQueue->PopSample(&pInSample) < 0 !!!\n", __FILE__, __FUNCTION__, __LINE__);
			break;
		}	

		if (!pInSample){
			break;
		}

		if (0 != m_pOutputPin->GetDeliveryBuffer(&pOutSample)){
			printf(":%s:%s:Line(%d) :0 != m_pOutputPin->GetDeliveryBuffer(&pOutSample) !!!\n", __FILE__, __FUNCTION__, __LINE__);
			break;
		}

		//	Deliver End of stream
		if (pInSample->IsEndOfStream()){
			pOutSample->SetEndOfStream(CTRUE);
			m_pOutputPin->Deliver(pOutSample);
			pInSample->SetEndOfStream(CFALSE);
			pOutSample->Release();
			m_pEventNotify->SendEvent(EVENT_AUDIO_CONVERT_ERROR, 1);
			NX_DbgMsg(DBG_MSG_ON, "[Filter|AConvert ] Deliver EndofStream\n");
			break;
		}

#if ADFILEWRITE	//File Write
		if (init == 0)
		{
			hFile = fopen("/root/filter_adecoder_pcm.out", "wb");
			init = 1;
		}
#endif

		pInSample->GetPointer((uint32_t**)&pInBuf);
		InSize = pInSample->GetActualDataLength();
		pOutSample->GetPointer((uint32_t**)&pOutBuf);

		//	Copy Media Time
		int64_t MeidaTime = 0;
		pInSample->GetTime(&MeidaTime);
		Valid = pInSample->IsValid();
		pOutSample->SetTime(MeidaTime);

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] m_ResampleSt.input_sample_fmt = %d, m_ResampleSt.input_channels = %d, InSize = %d\n",
			m_ResampleSt.input_sample_fmt, m_ResampleSt.input_channels, InSize);
		OutSamples = audio_resample(m_pReSampleCtx, (int16_t*)pOutBuf, (int16_t*)((void*)pInBuf), InSize / (m_ResampleSt.input_channels * (GetBitsPerSampleFormat(m_ResampleSt.input_sample_fmt) / 8)));
		OutSamples = OutSamples * m_ResampleSt.output_channels * (GetBitsPerSampleFormat(m_ResampleSt.output_sample_fmt) / 8);


#if ADFILEWRITE	//File Write
		count++;
		fwrite(pOutBuf, 1, OutSamples, hFile);
		if (count == 500){
			printf("=====exit ==== \n");
			fclose(hFile);
			exit(1);
		}
#endif		

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] InSize = %d, OutSamples = %d\n", InSize, OutSamples);

		pInSample->Release();

		if (OutSamples > 0)
		{
			pOutSample->SetActualDataLength(OutSamples);
			pOutSample->SetValid(Valid);
			m_pOutputPin->Deliver(pOutSample);
		}

		if (pOutSample){
			pOutSample->Release();
		}

	} //while
}

CBOOL NX_CAudioConvertFFmpegFilter::SetVolume(int32_t Volume)
{
	m_RequestVolume = Volume;
	return CTRUE;
}

CBOOL NX_CAudioConvertFFmpegFilter::SetSWVolume(int16_t *PcmSample, int32_t SampleSize)
{
	int audio_volume = 0;
	short *p_samples = NULL;
	int L_tmp = 0;
	int i = 0;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] ++ m_CurrentVolume = %d, m_RequestVolume = %d\n", m_CurrentVolume, m_RequestVolume);

	audio_volume = m_RequestVolume * (float)2.56;
	if (audio_volume <= 256)
	{
		p_samples = (short *)PcmSample;
		for (i = 0; i<(int)(SampleSize / sizeof(short)); i++)
		{
			L_tmp = ((*p_samples) * audio_volume + 128) >> 8;
			if (L_tmp < -32768) L_tmp = -32768;
			if (L_tmp >  32767) L_tmp = 32767;
			*p_samples++ = (short)L_tmp;
		}
		m_CurrentVolume = m_RequestVolume;
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] -- m_CurrentVolume = %d, m_RequestVolume = %d\n", m_CurrentVolume, m_RequestVolume);

	return CTRUE;
}


CBOOL NX_CAudioConvertFFmpegFilter::PinInfo(NX_PinInfo *pPinInfo)
{
	pPinInfo->InPutNum = 1;
	pPinInfo->OutPutNum = 1;

	return CTRUE;
}

NX_CBasePin *NX_CAudioConvertFFmpegFilter::FindPin(int Pos)
{
	return GetPin(Pos);	
}

CBOOL NX_CAudioConvertFFmpegFilter::FindInterface(const char *InterfaceId, void **Interface)
{
//	if(!strcmp(InterfaceId, "IMediaControl")){
//		*Interface = (NX_IMediaControl *)(this);
//	}
//	else
	if (!strcmp(InterfaceId, "IAudioConvertControl")){
		*Interface = (NX_IAudioControl *)(this);
	}

	return 0;
}

int32_t NX_CAudioConvertFFmpegFilter::SetMediaType(void *Mediatype)
{

	memcpy(&m_AMediaType, Mediatype, sizeof(NX_AudioMediaType));

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] SetMediaType:  m_AMediaType: CodecID = %d, Samplerate = %d, CH = %d, sample_fmt = %d\n",
							m_AMediaType.CodecID, m_AMediaType.Samplerate, m_AMediaType.Channels, m_AMediaType.FormatType);

	return 0;
}

void NX_CAudioConvertFFmpegFilter::SetClockReference(NX_CClockRef *pClockRef)
{
	SetClockRef(pClockRef);
}

void NX_CAudioConvertFFmpegFilter::SetEventNotifi(NX_CEventNotifier *pEventNotifier)
{
	SetEventNotifier(pEventNotifier);
}




//#ifdef _DEBUG
#if 0
void NX_CTransFormFilter::DisplayVersion()
{
	int32_t Major, Minor, Revision;
	NX_DbgMsg( DBG_INFO, "  MP3 Decoder   : %d.%d.%d\n", Major, Minor, Revision );
}

void NX_CTransFormFilter::DisplayState()
{
}
void NX_CTransFormFilter::GetStatistics( uint32_t *AudInCnt, uint32_t *AudOutCnt )
{
}
void NX_CTransFormFilter::ClearStatistics()
{
	m_DbgInFrameCnt = 0;
	m_DbgOutFrameCnt= 0;
}
#endif	//	_DEBUG

//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							NX_CAudioConvertInputPin
//
NX_CAudioConvertInputPin::NX_CAudioConvertInputPin(NX_CAudioConvertFFmpegFilter *pOwner) :
	NX_CBaseInputPin( (NX_CBaseFilter*)pOwner )
{
}

int32_t NX_CAudioConvertInputPin::Receive(NX_CSample *pInSample)
{
	int32_t ret = -1;

	//return 0;	//

	if( NULL == m_pOwnerFilter && CTRUE != IsActive() )
		return -1;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert] Receive \n");

	ret = ((NX_CAudioConvertFFmpegFilter*)m_pOwnerFilter)->TransFormSample(pInSample);
	return ret;
}

int32_t NX_CAudioConvertInputPin::CheckMediaType(void *Mediatype)
{
	NX_MediaType *MType = (NX_MediaType *)Mediatype;

	if (AUDIOTYPE != MType->MeidaType)
	{
		NX_DbgMsg(DBG_MSG_ON, "[Filter|AConvert] AUDIOTYPE != MType->MeidaType \n");
		return -1;
	}

	NX_CAudioConvertFFmpegFilter *pOwner = (NX_CAudioConvertFFmpegFilter *)m_pOwnerFilter;

	pOwner->SetMediaType(&MType->AMediaType);

	memcpy(&pOwner->m_pOutputPin->MediaType, Mediatype, sizeof(NX_MediaType));

	return 0;
}


//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							NX_CAudioConvertOutputPin
//
NX_CAudioConvertOutputPin::NX_CAudioConvertOutputPin(NX_CAudioConvertFFmpegFilter *pOwner) :
	NX_CBaseOutputPin( (NX_CBaseFilter *)pOwner ),
	m_pSampleQ( NULL ),
	m_pSampleList( NULL )
{
	m_pSampleQ = new NX_CSampleQueue(OUTPUT_SAMPLE_QUE);
	AllocBuffers();
}

NX_CAudioConvertOutputPin::~NX_CAudioConvertOutputPin()
{
	if( m_pSampleQ ){
		delete m_pSampleQ;
	}
	DeallocBuffers();
}

CBOOL NX_CAudioConvertOutputPin::Active()
{
	if (CFALSE == m_bActive)
	{
		if (m_pSampleQ)
		{
			m_pSampleQ->ResetQueue();
			NX_CSample *pSample = m_pSampleList;

			for (int i = 0; i < OUTPUT_SAMPLE_QUE; i++)
			{
				if (NULL != pSample)
				{
					pSample->Reset();
					m_pSampleQ->PushSample(pSample);
					pSample = pSample->m_pNext;
				}
			}
		}
	}
	return NX_CBaseOutputPin::Active();
}

CBOOL NX_CAudioConvertOutputPin::Inactive()
{
	if( m_pSampleQ )	
		m_pSampleQ->EndQueue();
	if (CTRUE == m_bActive)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert] NX_CAudioConvertOutputPin Wait join ++\n");
		m_bActive = CFALSE;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert] NX_CAudioConvertOutputPin Wait join --\n");
	}
	return NX_CBaseOutputPin::Inactive();
}

int32_t	NX_CAudioConvertOutputPin::ReleaseSample(NX_CSample *pSample)
{
	m_pSampleQ->PushSample( pSample );

	return 0;
}

int32_t NX_CAudioConvertOutputPin::CheckMediaType(void *Mediatype)
{

	return 0;
}

int32_t NX_CAudioConvertOutputPin::GetDeliveryBuffer(NX_CSample **ppSample)
{	
	if (!IsActive()){
		return -1;
	}

	*ppSample = NULL;
	
	if( m_pSampleQ->GetSampleCount() < 0 ){
		return -1;
	}	

	m_pSampleQ->PopSample( ppSample );
	if( NULL != *ppSample )
	{
		(*ppSample)->AddRef();
	}

	if( !IsActive() )
	{
		if( NULL != *ppSample )
		{
			(*ppSample)->Release();
		}
		return -1;
	}
	return 0;
}


int32_t	NX_CAudioConvertOutputPin::AllocBuffers()
{
	uint32_t *pBuf;
	NX_CSample *pNewSample = NULL;
	NX_CSample *pOldSample = m_pSampleList;

	int i=0;

	pOldSample = new NX_CSample(this);
	pBuf = new uint32_t[AVCODEC_MAX_AUDIO_FRAME_SIZE / sizeof(uint32_t)];
	pOldSample->SetBuffer(pBuf, AVCODEC_MAX_AUDIO_FRAME_SIZE, sizeof(uint32_t));
	m_pSampleList = pOldSample;

	for (i = 1; i<OUTPUT_SAMPLE_QUE; i++)
	{
		pNewSample = new NX_CSample(this);
		pBuf = new uint32_t[AVCODEC_MAX_AUDIO_FRAME_SIZE / sizeof(uint32_t)];
		pNewSample->SetBuffer(pBuf, AVCODEC_MAX_AUDIO_FRAME_SIZE, sizeof(uint32_t));
		pOldSample->m_pNext = pNewSample;
		pOldSample = pNewSample;
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] Allocated Buffers = %d\n", i);
	return 0;
}

int32_t	NX_CAudioConvertOutputPin::DeallocBuffers()
{
	int k = 0;
	while(1)
	{
		if( NULL == m_pSampleList )
		{
			break;
		}
		NX_CSample *TmpSampleList = m_pSampleList;
		m_pSampleList = m_pSampleList->m_pNext;
		delete TmpSampleList;
		k++;
	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|AConvert ] Deallocated Buffers = %d\n", k);
	return 0;
}

//
//////////////////////////////////////////////////////////////////////////////

NX_IBaseFilter *CreateAudioConvertFilter(const char *filterName)
{
	return new NX_CAudioConvertFFmpegFilter(filterName);
}
