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
#include <unistd.h>
#include "NX_CAudioDecoderFFmpegFilter.h"
#include "NX_CodecID.h"


typedef struct {
	CodecID codec_id;
	int32_t codec_name;
} CodTab;
#define CODE_ID_MAX		86

static CodTab codec_tab[CODE_ID_MAX] = {
	{ CODEC_ID_H264, CODEC_H264 },
	{ CODEC_ID_H263, CODEC_H263 },
	{ CODEC_ID_MPEG1VIDEO, CODEC_MPEG1VIDEO },
	{ CODEC_ID_MPEG2VIDEO, CODEC_MPEG2VIDEO },
	{ CODEC_ID_MPEG4, CODEC_MPEG4 },
	{ CODEC_ID_MSMPEG4V3, CODEC_MSMPEG4V3 },
	{ CODEC_ID_FLV1, CODEC_FLV1 },
	{ CODEC_ID_WMV1, CODEC_WMV1 },
	{ CODEC_ID_WMV2, CODEC_WMV2 },
	{ CODEC_ID_WMV3, CODEC_WMV3 },
	{ CODEC_ID_VC1, CODEC_VC1 },
	{ CODEC_ID_RV30, CODEC_RV30 },
	{ CODEC_ID_RV40, CODEC_RV40 },
	{ CODEC_ID_THEORA, CODEC_THEORA },
	{ CODEC_ID_VP8, CODEC_VP8 },

	{ CODEC_ID_RA_144, CODEC_RA_144 },
	{ CODEC_ID_RA_288, CODEC_RA_288 },
	{ CODEC_ID_MP2, CODEC_MP2 },
	{ CODEC_ID_MP3, CODEC_MP3 },
	{ CODEC_ID_AAC, CODEC_AAC },
	{ CODEC_ID_AC3, CODEC_AC3 },
	{ CODEC_ID_DTS, CODEC_DTS },
	{ CODEC_ID_VORBIS, CODEC_VORBIS },
	{ CODEC_ID_WMAV1, CODEC_WMAV1 },
	{ CODEC_ID_WMAV2, CODEC_WMAV2 },
	{ CODEC_ID_WMAPRO, CODEC_WMAPRO },
	{ CODEC_ID_FLAC, CODEC_FLAC },
	{ CODEC_ID_COOK, CODEC_COOK },
	{ CODEC_ID_APE, CODEC_APE },
	{ CODEC_ID_AAC_LATM, CODEC_AAC_LATM },

	/* various PCM "codecs" */
	{ CODEC_ID_PCM_S16LE,			CODEC_PCM_S16LE		},
	{ CODEC_ID_PCM_S16BE,			CODEC_PCM_S16BE		},		
	{ CODEC_ID_PCM_U16LE,			CODEC_PCM_U16LE		},
	{ CODEC_ID_PCM_U16BE,			CODEC_PCM_U16BE		},
	{ CODEC_ID_PCM_S8,				CODEC_PCM_S8		},
	{ CODEC_ID_PCM_U8,				CODEC_PCM_U8		},
	{ CODEC_ID_PCM_MULAW,			CODEC_PCM_MULAW		},
	{ CODEC_ID_PCM_ALAW,			CODEC_PCM_ALAW		},
	{ CODEC_ID_PCM_S32LE,			CODEC_PCM_S32LE		},
	{ CODEC_ID_PCM_S32BE,			CODEC_PCM_S32BE		},
	{ CODEC_ID_PCM_U32LE,			CODEC_PCM_U32LE		},
	{ CODEC_ID_PCM_U32BE,			CODEC_PCM_U32BE		},
	{ CODEC_ID_PCM_S24LE,			CODEC_PCM_S24LE		},
	{ CODEC_ID_PCM_S24BE,			CODEC_PCM_S24BE		},
	{ CODEC_ID_PCM_U24LE,			CODEC_PCM_U24LE		},
	{ CODEC_ID_PCM_U24BE,			CODEC_PCM_U24BE		},
	{ CODEC_ID_PCM_S24DAUD,			CODEC_PCM_S24DAUD	},
	{ CODEC_ID_PCM_ZORK,			CODEC_PCM_ZORK		},
	{ CODEC_ID_PCM_S16LE_PLANAR,	CODEC_PCM_S16LE_PLANAR},
	{ CODEC_ID_PCM_DVD,				CODEC_PCM_DVD		},
	{ CODEC_ID_PCM_F32BE,			CODEC_PCM_F32BE		},
	{ CODEC_ID_PCM_F32LE,			CODEC_PCM_F32LE		},
	{ CODEC_ID_PCM_F64BE,			CODEC_PCM_F64BE		},
	{ CODEC_ID_PCM_F64LE,			CODEC_PCM_F64LE		},
	{ CODEC_ID_PCM_BLURAY,			CODEC_PCM_BLURAY	},
	{ CODEC_ID_PCM_LXF,				CODEC_PCM_LXF		},
	{ CODEC_ID_S302M,				CODEC_S302M			},

	/* various ADPCM codecs */
	{ CODEC_ID_ADPCM_IMA_QT,		CODEC_ADPCM_IMA_QT	},
	{ CODEC_ID_ADPCM_IMA_WAV,		CODEC_ADPCM_IMA_WAV	},
	{ CODEC_ID_ADPCM_IMA_DK3,		CODEC_ADPCM_IMA_DK3	},
	{ CODEC_ID_ADPCM_IMA_DK4,		CODEC_ADPCM_IMA_DK4	},
	{ CODEC_ID_ADPCM_IMA_WS, CODEC_ADPCM_IMA_WS },
	{ CODEC_ID_ADPCM_IMA_SMJPEG, CODEC_ADPCM_IMA_SMJPEG },
	{ CODEC_ID_ADPCM_MS, CODEC_ADPCM_MS },
	{ CODEC_ID_ADPCM_4XM, CODEC_ADPCM_4XM },
	{ CODEC_ID_ADPCM_XA, CODEC_ADPCM_XA },
	{ CODEC_ID_ADPCM_ADX, CODEC_ADPCM_ADX },
	{ CODEC_ID_ADPCM_EA, CODEC_ADPCM_EA },
	{ CODEC_ID_ADPCM_G726, CODEC_ADPCM_G726 },
	{ CODEC_ID_ADPCM_CT, CODEC_ADPCM_CT },
	{ CODEC_ID_ADPCM_SWF, CODEC_ADPCM_SWF },
	{ CODEC_ID_ADPCM_YAMAHA, CODEC_ADPCM_YAMAHA },
	{ CODEC_ID_ADPCM_SBPRO_4, CODEC_ADPCM_SBPRO_4 },
	{ CODEC_ID_ADPCM_SBPRO_3, CODEC_ADPCM_SBPRO_3 },
	{ CODEC_ID_ADPCM_SBPRO_2, CODEC_ADPCM_SBPRO_2 },
	{ CODEC_ID_ADPCM_THP, CODEC_ADPCM_THP },
	{ CODEC_ID_ADPCM_IMA_AMV, CODEC_ADPCM_IMA_AMV },
	{ CODEC_ID_ADPCM_EA_R1, CODEC_ADPCM_EA_R1 },
	{ CODEC_ID_ADPCM_EA_R3, CODEC_ADPCM_EA_R3 },
	{ CODEC_ID_ADPCM_EA_R2, CODEC_ADPCM_EA_R2 },
	{ CODEC_ID_ADPCM_IMA_EA_SEAD, CODEC_ADPCM_IMA_EA_SEAD },
	{ CODEC_ID_ADPCM_IMA_EA_EACS, CODEC_ADPCM_IMA_EA_EACS },
	{ CODEC_ID_ADPCM_EA_XAS, CODEC_ADPCM_EA_XAS },
	{ CODEC_ID_ADPCM_EA_MAXIS_XA, CODEC_ADPCM_EA_MAXIS_XA },
	{ CODEC_ID_ADPCM_IMA_ISS, CODEC_ADPCM_IMA_ISS },
	{ CODEC_ID_ADPCM_G722, CODEC_ADPCM_G722 },
};


//////////////////////////////////////////////////////////////////////////////
//
//							Nexell AudioDecoder Filter
//
//							----------
//			(input pin)   --|        |--- (out putpin)
//							----------
#define		MAX_AUDIO_NUM_BUF		42
NX_CAudioDecoderFFmpegFilter::NX_CAudioDecoderFFmpegFilter(const char *filterName) :
	m_pInputPin(NULL),
	m_pOutputPin(NULL),
	m_Discontinuity(CFALSE),
	m_Init(0),
	m_ExtData(NULL)
{
	m_pInputPin = new NX_CAudioDecoderInputPin(this);
	m_pInQueue = new NX_CSampleQueue(MAX_AUDIO_NUM_BUF);
	m_pOutputPin = new NX_CAudioDecoderOutputPin(this);

	memset(m_cArrFilterName, 0, sizeof(m_cArrFilterName));
	memset(m_cArrFilterID, 0, sizeof(m_cArrFilterID));
	SetFilterName(filterName);

	av_register_all();

	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}

}

NX_CAudioDecoderFFmpegFilter::~NX_CAudioDecoderFFmpegFilter()
{
	if( m_pInputPin ) delete m_pInputPin;
	if( m_pOutputPin ) delete m_pOutputPin;
	if (m_pInQueue)		delete m_pInQueue;

	if (m_ExtData)
		free(m_ExtData);

	pthread_mutex_destroy(&m_hMutexThread);
}

NX_CBasePin *NX_CAudioDecoderFFmpegFilter::GetPin(int Pos)
{
	if( PIN_DIRECTION_INPUT == Pos )
		return m_pInputPin;
	if( PIN_DIRECTION_OUTPUT == Pos )
		return m_pOutputPin;
	return NULL;
}

int32_t	NX_CAudioDecoderFFmpegFilter::DecoderSample(NX_CSample *pSample)
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

CBOOL NX_CAudioDecoderFFmpegFilter::Run()
{
	if (CFALSE == m_bRunning)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] Run() ++\n");
		if (m_Init == 0)
		{
			m_hAudioCodec = avcodec_find_decoder(m_CodecID);
			if (m_hAudioCodec == NULL)
				return -1;
			// Default Codec Configuration
			m_Avctx = avcodec_alloc_context();
						
			m_Avctx->request_channels = 0;
			m_Avctx->channels = m_channels;
			m_Avctx->sample_rate = m_Samplerate;
			m_Avctx->block_align = m_AMediaType.BlockAlign;
			m_Avctx->bit_rate = m_AMediaType.Bitrate;

			m_Avctx->extradata = (uint8_t *)m_ExtData;
			m_Avctx->extradata_size = m_ExtSize;
			NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] m_Avctx->request_channels = %d, m_Avctx->channels = %d, m_Avctx->sample_rate = %d, m_Avctx->block_align = %d, m_Avctx->extradata_size = %d\n",
										m_Avctx->request_channels, m_Avctx->channels, m_Avctx->sample_rate, m_Avctx->block_align, m_Avctx->extradata_size);
			
			if (avcodec_open(m_Avctx, m_hAudioCodec) < 0){
				NX_ErrMsg( ("[Filter|ADec ] avcodec_open() failed.\n") );
			}

			m_Init = 1;
		}

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

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] Run() --\n");
	}
	//NX_DbgMsg(DBG_MSG_OFF, "[FilterName : %s, FilterID: %s ] Run() --\n", this->GetFilterName(), this->GetFilterId());
	return NX_CBaseFilter::Run();
}

CBOOL NX_CAudioDecoderFFmpegFilter::Stop()
{
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] Stop() ++\n");

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
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec] Stop() : pthread_join() ++\n");
		pthread_join(m_hThread, NULL);
		m_hThread = 0;
		m_bRunning = CFALSE;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec] Stop() : pthread_join() --\n");
	}
	AudioClose();
	m_Init = 0;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] Stop() --\n");
	return NX_CBaseFilter::Stop();
}

CBOOL NX_CAudioDecoderFFmpegFilter::Pause()
{
	return CTRUE;
}

void NX_CAudioDecoderFFmpegFilter::AudioClose()
{
	if (m_Avctx){
		if (m_AMediaType.pSeqData) {
			//free(m_AMediaType.pSeqData);
			//m_AMediaType.pSeqData = NULL;
		}
		avcodec_close(m_Avctx);
		av_free(m_Avctx);
		m_Avctx = NULL;
	}
}

CBOOL NX_CAudioDecoderFFmpegFilter::Flush()
{
	return CTRUE;
}

void *NX_CAudioDecoderFFmpegFilter::ThreadStub(void *pObj)
{
	if (NULL != pObj)
	{
		((NX_CAudioDecoderFFmpegFilter*)pObj)->ThreadProc();
	}
	return (void*)(0);
}

int32_t NX_CAudioDecoderFFmpegFilter::GetBitsPerSampleFormat(enum SampleFormat sample_fmt) {
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
static FILE					*hFile = NULL;
static int count = 0;
static int init = 0;
static int i = 0;
#endif

void NX_CAudioDecoderFFmpegFilter::ThreadProc()
{
	NX_CSample *pOutSample = NULL;
	NX_CSample *pInSample = NULL;

	AVPacket avpkt;
	int outSize = 0, inSize = 0, usedByte = 0, inData = 0;
	int outSize_tmp = 0;
	int remain_data = 0;
	int64_t MeidaTime = 0;

	while (1)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec]  ThreadProc Start !!\n");

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){
			NX_DbgMsg(DBG_MSG_ON, "[Filter|ADec]  m_bExitThread = %d !!\n", m_bExitThread);
			pthread_mutex_unlock(&m_hMutexThread);
			break;
		}
		pthread_mutex_unlock(&m_hMutexThread);

		if (remain_data == 0){
			pInSample = NULL;
		}

		if (remain_data == 0){
			if (m_pInQueue->PopSample(&pInSample) < 0)
			{
				NX_DbgMsg(DBG_MSG_ON, "[Filter|ADec]  m_pInQueue->PopSample(&pInSample) < 0 !!\n");
				break;
			}

			if (!pInSample)
				break;
		}

		if (0 != m_pOutputPin->GetDeliveryBuffer(&pOutSample)){
			NX_DbgMsg(DBG_MSG_ON, "[Filter|ADec]  0 != m_pOutputPin->GetDeliveryBuffer(&pOutSample) !!\n");
			break;
		}
		
		if (remain_data == 0){
			av_init_packet(&avpkt);

			if (pInSample->IsDiscontinuity()){
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec] Discontinuity!!!\n");
				m_Discontinuity = CTRUE;
			}
		}

		if (remain_data == 0){
			//	Deliver End of stream
			if (pInSample->IsEndOfStream()){
				pOutSample->SetEndOfStream(CTRUE);
				m_pOutputPin->Deliver(pOutSample);
				pInSample->SetEndOfStream(CFALSE);
				pOutSample->Release();
				m_pEventNotify->SendEvent(EVENT_AUDIO_DECODER_ERROR, 1);
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] Deliver EndofStream\n");
				break;
			}
		}

#if ADFILEWRITE	//File Write
		if (init == 0)
		{
			hFile = fopen("/root/filter_pcm.out", "wb");
			init = 1;
		}
#endif

		uint8_t *pInBuf = NULL;
		int16_t *pOutBuf = NULL;
		CBOOL Valid = CFALSE;
		if (remain_data == 0){
			pInSample->GetPointer((uint32_t**)&pInBuf);
			inSize = pInSample->GetActualDataLength();
			avpkt.data = pInBuf;
			avpkt.size = inSize;
		}

		pOutSample->GetPointer((uint32_t**)&pOutBuf);

		//	Copy Media Time
		if (remain_data == 0){
			pInSample->GetTime(&MeidaTime);
			Valid = pInSample->IsValid();
		}
		pOutSample->SetTime(MeidaTime);


		outSize = 0;
		outSize_tmp = 0;
		do{
			//		outSize = allocSize;
			outSize_tmp = AVCODEC_MAX_AUDIO_FRAME_SIZE;
			usedByte = avcodec_decode_audio3(m_Avctx, (int16_t*)pOutBuf, &outSize_tmp, &avpkt);
			NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec ] inSize = %d, usedByte = %d, outSize = %d, sample_fmt = %d\n", inSize, usedByte, outSize_tmp, m_Avctx->sample_fmt);
			if (usedByte <= 0){
				break;
			}
			//	Update In Buffer
			inSize -= usedByte;
			inData += usedByte;

			avpkt.data = avpkt.data + usedByte;
			avpkt.size = avpkt.size - usedByte;

			outSize = outSize_tmp;
			if (outSize <= 0)
				continue;
			if (outSize > 0)
				break;

		} while (inSize > 0);

#if ADFILEWRITE
		count++;
		fwrite(pOutBuf, 1, OutSamples, hFile);
		if (count == 230){
			printf("=====exit ==== \n");
			fclose(hFile);
			exit(1);
		}
#endif		

		if ((avpkt.size == 0) || (usedByte < 0))
			remain_data = 0;
		else
			remain_data = 1;

		if (remain_data == 0){
			pInSample->Release();
		}

		if (outSize <= 0){
			if (pOutSample)
				pOutSample->Release();
			//	??? Exception
			continue;
		}

		outSize_tmp = outSize / (GetBitsPerSampleFormat((SampleFormat)m_AMediaType.FormatType) / 8);
		outSize_tmp = m_AMediaType.Channels;

		pOutSample->SetActualDataLength(outSize);
		pOutSample->SetValid(Valid);
		pOutSample->SetDiscontinuity(m_Discontinuity);
		if (m_Discontinuity){
			m_Discontinuity = CFALSE;
		}
		m_pOutputPin->Deliver(pOutSample);

		if (pOutSample){
			pOutSample->Release();
		}
	} //while
}


CBOOL NX_CAudioDecoderFFmpegFilter::PinInfo(NX_PinInfo *pPinInfo)
{
	pPinInfo->InPutNum = 1;
	pPinInfo->OutPutNum = 1;
	return 0;
}

NX_CBasePin *NX_CAudioDecoderFFmpegFilter::FindPin(int Pos)
{
	return GetPin(Pos);	
}

CBOOL NX_CAudioDecoderFFmpegFilter::FindInterface(const char *InterfaceId, void **Interface)
{
//	if(!strcmp(InterfaceId, "IMediaControl")){
//		*Interface = (NX_IMediaControl *)(this);
//	}

	return 0;
}

int32_t NX_CAudioDecoderFFmpegFilter::SetMediaType(void *Mediatype)
{
	int32_t j = 0;
	memcpy(&m_AMediaType, Mediatype, sizeof(NX_AudioMediaType));

	for (j = 0; j < CODE_ID_MAX; j++){
		if (m_AMediaType.CodecID == codec_tab[j].codec_name){
			m_CodecID = codec_tab[j].codec_id;
			break;
		}
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec] SetMediaType:  m_AMediaType: CodecID = %d, Samplerate = %d, CH = %d, sample_fmt = %d\n",
		m_AMediaType.CodecID, m_AMediaType.Samplerate, m_AMediaType.Channels, m_AMediaType.FormatType);

	m_Samplerate = m_AMediaType.Samplerate;
	m_channels = m_AMediaType.Channels;

	if (m_ExtData)
		free(m_ExtData);
	m_ExtSize = m_AMediaType.SeqDataSize;
	m_ExtData = (unsigned char*)malloc(m_ExtSize);
	memcpy(m_ExtData, m_AMediaType.pSeqData, m_AMediaType.SeqDataSize);

	return 0;
}

void NX_CAudioDecoderFFmpegFilter::SetClockReference(NX_CClockRef *pClockRef)
{
	SetClockRef(pClockRef);
}

void NX_CAudioDecoderFFmpegFilter::SetEventNotifi(NX_CEventNotifier *pEventNotifier)
{
	SetEventNotifier(pEventNotifier);
}



//#ifdef _DEBUG
#if 0
void NX_CMp3DecoderFilter::DisplayVersion()
{
	int32_t Major, Minor, Revision;
	mp3dec_version( &Major, &Minor, &Revision );
	NX_DbgMsg( DBG_INFO, "  MP3 Decoder   : %d.%d.%d\n", Major, Minor, Revision );
}

void NX_CMp3DecoderFilter::DisplayState()
{
}
void NX_CMp3DecoderFilter::GetStatistics( uint32_t *AudInCnt, uint32_t *AudOutCnt )
{
	*AudInCnt = m_DbgInFrameCnt;
	*AudOutCnt= m_DbgOutFrameCnt;
}
void NX_CMp3DecoderFilter::ClearStatistics()
{
	m_DbgInFrameCnt = 0;
	m_DbgOutFrameCnt= 0;
}
#endif	//	_DEBUG

//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							NX_CAudioDecoderInputPin
//
NX_CAudioDecoderInputPin::NX_CAudioDecoderInputPin(NX_CAudioDecoderFFmpegFilter *pOwner) :
	NX_CBaseInputPin( (NX_CBaseFilter*)pOwner )
{
}

int32_t NX_CAudioDecoderInputPin::Receive(NX_CSample *pInSample)
{
	int32_t ret = -1;

	//return 0;	//
	if( NULL == m_pOwnerFilter && CTRUE != IsActive() )
		return -1;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|ADec] Receive \n");

	ret = ((NX_CAudioDecoderFFmpegFilter*)m_pOwnerFilter)->DecoderSample(pInSample);
	return ret;
}


int32_t NX_CAudioDecoderInputPin::CheckMediaType(void *Mediatype)
{
	NX_MediaType *MType = (NX_MediaType *)Mediatype;
	NX_AudioMediaType *pType = NULL;
	int32_t i = 0;
	int32_t FindCodec = CFALSE;

	pType = &MType->AMediaType;

	if (AUDIOTYPE != MType->MeidaType)
		return -1;

	for (i = 0; i < CODE_ID_MAX; i++){
		if (pType->CodecID == codec_tab[i].codec_name){		
			FindCodec = CTRUE;
			break;
		}
	}

	if (CFALSE == FindCodec){
		NX_DbgMsg(DBG_MSG_ON, "[Filter|ADec ] Not Support CodecID = %d\n", pType->CodecID);
		return -1;
	}
	if ((pType->Samplerate < 8000) || (pType->Samplerate > 96000)){
		NX_DbgMsg(DBG_MSG_ON, "[Filter|ADec ] Not Support Samplerate = %d\n", pType->Samplerate);
		return -1;
	}
	else if ((pType->Channels < 1) || (pType->Channels > 6)){
		NX_DbgMsg(DBG_MSG_ON, "[Filter|ADec ] Not Support Channels = %d\n", pType->Channels);
		return -1;
	}

	NX_CAudioDecoderFFmpegFilter *pOwner = (NX_CAudioDecoderFFmpegFilter *)m_pOwnerFilter;

	pOwner->SetMediaType(&MType->AMediaType);	

	memcpy(&pOwner->m_pOutputPin->MediaType, Mediatype, sizeof(NX_MediaType));

	return 0;
}


//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							NX_CAudioDecoderOutputPin
//
NX_CAudioDecoderOutputPin::NX_CAudioDecoderOutputPin(NX_CAudioDecoderFFmpegFilter *pOwner) :
	NX_CBaseOutputPin( (NX_CBaseFilter *)pOwner ),
	m_pSampleQ( NULL ),
	m_pSampleList( NULL )
{
	AllocBuffers();
	m_pSampleQ = new NX_CSampleQueue(MAX_NUM_WAVE_OUT_BUF);
}

NX_CAudioDecoderOutputPin::~NX_CAudioDecoderOutputPin()
{
	if( m_pSampleQ ){
		delete m_pSampleQ;
	}
	DeallocBuffers();
}


CBOOL NX_CAudioDecoderOutputPin::Active()
{
	if (CFALSE == m_bActive)
	{
		if (m_pSampleQ)
		{
			m_pSampleQ->ResetQueue();
			NX_CSample *pSample = m_pSampleList;

			for (int i = 0; i < MAX_NUM_WAVE_OUT_BUF; i++)
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

CBOOL NX_CAudioDecoderOutputPin::Inactive()
{
	if( m_pSampleQ )	m_pSampleQ->EndQueue();
	if (CTRUE == m_bActive)
	{
		m_bActive = CFALSE;
	}
	return NX_CBaseOutputPin::Inactive();
}

int32_t	NX_CAudioDecoderOutputPin::ReleaseSample(NX_CSample *pSample)
{
//	printf("=====================   NX_CAudioDecoderOutputPin::ReleaseSample m_pSampleQ->GetSampleCount() ++ = %d\n", m_pSampleQ->GetSampleCount());
	m_pSampleQ->PushSample( pSample );
//	printf("=====================   NX_CAudioDecoderOutputPin::ReleaseSample m_pSampleQ->GetSampleCount() -- = %d\n", m_pSampleQ->GetSampleCount());
	return 0;
}

int32_t NX_CAudioDecoderOutputPin::CheckMediaType(void *Mediatype)
{

	return 0;
}

int32_t NX_CAudioDecoderOutputPin::GetDeliveryBuffer(NX_CSample **ppSample)
{	
	if( !IsActive() )
		return -1;

	*ppSample = NULL;
	
	if (m_pSampleQ->GetSampleCount() > 0){
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

int32_t	NX_CAudioDecoderOutputPin::AllocBuffers()
{
	uint32_t *pBuf;
	NX_CSample *pNewSample = NULL;
	NX_CSample *pOldSample = m_pSampleList;

	int k=0, i=0;

	pOldSample = new NX_CSample(this);
	pBuf = new uint32_t[MAX_SIZE_WAVE_OUT_BUF / sizeof(uint32_t)];
	pOldSample->SetBuffer(pBuf, MAX_SIZE_WAVE_OUT_BUF, sizeof(uint32_t));
	m_pSampleList = pOldSample;
	k++;

	for (i = 1; i<MAX_NUM_WAVE_OUT_BUF; i++)
	{
		pNewSample = new NX_CSample(this);
		pBuf = new uint32_t[MAX_SIZE_WAVE_OUT_BUF / sizeof(uint32_t)];
		pNewSample->SetBuffer(pBuf, MAX_SIZE_WAVE_OUT_BUF, sizeof(uint32_t));
		pOldSample->m_pNext = pNewSample;
		pOldSample = pNewSample;
		k++;
	}

	NX_DbgMsg( DBG_TRACE, "[Filter|ADec ] Allocated Buffers = %d\n", k );

	return 0;
}

int32_t	NX_CAudioDecoderOutputPin::DeallocBuffers()
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
//	NX_DbgMsg( DBG_TRACE, "[Filter|ADec ] Deallocated Buffers = %d\n", k );
	return 0;
}

//
//////////////////////////////////////////////////////////////////////////////


NX_IBaseFilter *CreateAudioDecoderFilter(const char *filterName)
{
	return new NX_CAudioDecoderFFmpegFilter(filterName);
}
