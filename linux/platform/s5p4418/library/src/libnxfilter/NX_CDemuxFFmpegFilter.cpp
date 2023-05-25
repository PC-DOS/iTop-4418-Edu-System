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
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "NX_CDemuxFFmpegFilter.h"
#include "NX_CodecID.h"

typedef struct {
	CodecID codec_id;
	int32_t codec_name;
} CodTab;
#define CODE_ID_MAX		86
static CodTab codec_tab[CODE_ID_MAX] = {
	{ CODEC_ID_H264,				CODEC_H264			},
	{ CODEC_ID_H263,				CODEC_H263			},
	{ CODEC_ID_MPEG1VIDEO,			CODEC_MPEG1VIDEO	},
	{ CODEC_ID_MPEG2VIDEO,			CODEC_MPEG2VIDEO	},
	{ CODEC_ID_MPEG4,				CODEC_MPEG4			},
	{ CODEC_ID_MSMPEG4V3,			CODEC_MSMPEG4V3		},
	{ CODEC_ID_FLV1,				CODEC_FLV1			},
	{ CODEC_ID_WMV1,				CODEC_WMV1			},
	{ CODEC_ID_WMV2,				CODEC_WMV2			},
	{ CODEC_ID_WMV3,				CODEC_WMV3			},
	{ CODEC_ID_VC1,					CODEC_VC1			},
	{ CODEC_ID_RV30,				CODEC_RV30			},
	{ CODEC_ID_RV40,				CODEC_RV40			},
	{ CODEC_ID_THEORA,				CODEC_THEORA		},
	{ CODEC_ID_VP8,					CODEC_VP8			},

	{ CODEC_ID_RA_144,				CODEC_RA_144		},
	{ CODEC_ID_RA_288,				CODEC_RA_288		},
	{ CODEC_ID_MP2,					CODEC_MP2			},
	{ CODEC_ID_MP3,					CODEC_MP3			},
	{ CODEC_ID_AAC,					CODEC_AAC			},
	{ CODEC_ID_AC3,					CODEC_AC3			},
	{ CODEC_ID_DTS,					CODEC_DTS			},
	{ CODEC_ID_VORBIS,				CODEC_VORBIS		},
	{ CODEC_ID_WMAV1,				CODEC_WMAV1			},
	{ CODEC_ID_WMAV2,				CODEC_WMAV2			},
	{ CODEC_ID_WMAPRO,				CODEC_WMAPRO		},
	{ CODEC_ID_FLAC,				CODEC_FLAC			},
	{ CODEC_ID_COOK,				CODEC_COOK			},
	{ CODEC_ID_APE,					CODEC_APE			},
	{ CODEC_ID_AAC_LATM,			CODEC_AAC_LATM		},

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
	{ CODEC_ID_ADPCM_IMA_WS,		CODEC_ADPCM_IMA_WS	},
	{ CODEC_ID_ADPCM_IMA_SMJPEG,	CODEC_ADPCM_IMA_SMJPEG},
	{ CODEC_ID_ADPCM_MS,			CODEC_ADPCM_MS		},
	{ CODEC_ID_ADPCM_4XM,			CODEC_ADPCM_4XM		},
	{ CODEC_ID_ADPCM_XA,			CODEC_ADPCM_XA		},
	{ CODEC_ID_ADPCM_ADX,			CODEC_ADPCM_ADX		},
	{ CODEC_ID_ADPCM_EA,			CODEC_ADPCM_EA		},
	{ CODEC_ID_ADPCM_G726,			CODEC_ADPCM_G726	},
	{ CODEC_ID_ADPCM_CT,			CODEC_ADPCM_CT		},
	{ CODEC_ID_ADPCM_SWF,			CODEC_ADPCM_SWF		},
	{ CODEC_ID_ADPCM_YAMAHA,		CODEC_ADPCM_YAMAHA	},
	{ CODEC_ID_ADPCM_SBPRO_4,		CODEC_ADPCM_SBPRO_4	},
	{ CODEC_ID_ADPCM_SBPRO_3,		CODEC_ADPCM_SBPRO_3	},
	{ CODEC_ID_ADPCM_SBPRO_2,		CODEC_ADPCM_SBPRO_2	},
	{ CODEC_ID_ADPCM_THP,			CODEC_ADPCM_THP		},
	{ CODEC_ID_ADPCM_IMA_AMV,		CODEC_ADPCM_IMA_AMV	},
	{ CODEC_ID_ADPCM_EA_R1,			CODEC_ADPCM_EA_R1	},
	{ CODEC_ID_ADPCM_EA_R3,			CODEC_ADPCM_EA_R3	},
	{ CODEC_ID_ADPCM_EA_R2,			CODEC_ADPCM_EA_R2	},
	{ CODEC_ID_ADPCM_IMA_EA_SEAD,	CODEC_ADPCM_IMA_EA_SEAD},
	{ CODEC_ID_ADPCM_IMA_EA_EACS,	CODEC_ADPCM_IMA_EA_EACS},
	{ CODEC_ID_ADPCM_EA_XAS,		CODEC_ADPCM_EA_XAS	},
	{ CODEC_ID_ADPCM_EA_MAXIS_XA,	CODEC_ADPCM_EA_MAXIS_XA},
	{ CODEC_ID_ADPCM_IMA_ISS,		CODEC_ADPCM_IMA_ISS	},
	{ CODEC_ID_ADPCM_G722,			CODEC_ADPCM_G722	},
};


//////////////////////////////////////////////////////////////////////////////
//
//							NX_CDemuxFilter
//

NX_CDemuxFFmpegFilter::NX_CDemuxFFmpegFilter(const char *filterName) :
	m_StreamStartTime(0),
	m_StreamEndTime(-1),
	m_StreamDuration(0),
	m_DeltaTime(0),
	m_bFindNext(CFALSE),
	m_pInBufPos(0),
	m_DemuxMode(DEMUX_MODE_FILE),	//	Set Default Running Mode : File Mode
	m_BufferingTime(DEFAULT_BUFFERING_TIME),
	m_MaxBufferingTime(STREAM_READER_MAX_DELAY)
{
	m_APinNum = 0;
	m_VPinNum = 0;


	//ffmpeg
	m_pFmtCtx = NULL;
	m_pVideoDecCtx = NULL;
	m_pAudioDecCtx = NULL;
	m_pVideoStream = NULL;
	m_pAudioStream = NULL;

	m_Init = 0;

	memset(&m_DemuxMediaInfo, 0, sizeof(DemuxMediaInfo));

	//	FFMPEG
	av_register_all();

	memset(m_cArrFilterName, 0, sizeof(m_cArrFilterName));
	memset(m_cArrFilterID, 0, sizeof(m_cArrFilterID));
	SetFilterName(filterName);

	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}
}

NX_CDemuxFFmpegFilter::~NX_CDemuxFFmpegFilter()
{
	int32_t i = 0;

	for (i = 0; i < m_SetPin.OutPinNum.VPinNum; i++){
		if (m_pVideoOut[i])
			delete m_pVideoOut[i];
	}
	for (i = 0; i < m_SetPin.OutPinNum.APinNum; i++){
		if (m_pAudioOut[i])
			delete m_pAudioOut[i];
	}

	if (m_VMediaType.pSeqData)
		free(m_VMediaType.pSeqData);

	if (m_AMediaType.pSeqData)
		free(m_AMediaType.pSeqData);

	for (i = 0; i < m_DemuxMediaInfo.VideoTrackTotNum; i++)
	{
		if (m_DemuxMediaInfo.VideoInfo[i].m_SeqData)
			free(m_DemuxMediaInfo.VideoInfo[i].m_SeqData);
	}
	for (i = 0; i < m_DemuxMediaInfo.AudioTrackTotNum; i++)
	{
		if (m_DemuxMediaInfo.AudioInfo[i].m_SeqData)
			free(m_DemuxMediaInfo.AudioInfo[i].m_SeqData);
	}



	pthread_mutex_destroy(&m_hMutexThread);
}

int32_t NX_CDemuxFFmpegFilter::FFmpegOpen()
{
	int32_t i = 0;
	AVInputFormat *iformat = NULL;
	AVStream *stream = NULL;
	int32_t VPinNum = 0;
	int32_t APinNum = 0;

	m_pFmtCtx = avformat_alloc_context();

	m_pFmtCtx->flags |= CODEC_FLAG_TRUNCATED;

	/* open input file, and allocate format context */
	if (avformat_open_input(&m_pFmtCtx, (const char *)m_FileName, iformat, NULL) < 0) {
		printf("Could not open source file %s\n", m_FileName);
		return -1;
	}

	/* fill the streams in the format context */
	if (av_find_stream_info(m_pFmtCtx) < 0)
	{
		av_close_input_file(m_pFmtCtx);
		return -1;
	}

	for (i = 0; i < DEMUX_MEDIA_MAX; i++)
	{
		m_DemuxMediaInfo.VideoInfo[i].VideoTrackInfo.StreamIDX = -1;
		m_DemuxMediaInfo.AudioInfo[i].AudioTrackInfo.StreamIDX = -1;
	}

	VPinNum = 0;
	APinNum = 0;

	for (i = 0; i < (int)m_pFmtCtx->nb_streams; i++)
	{
		stream = m_pFmtCtx->streams[i];
		if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			m_DemuxMediaInfo.VideoInfo[VPinNum].VideoTrackInfo.StreamIDX = i;
			m_DemuxMediaInfo.VideoInfo[VPinNum].pVideoStream = stream;
			VPinNum++;
		}
		else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			m_DemuxMediaInfo.AudioInfo[APinNum].AudioTrackInfo.StreamIDX = i;
			m_DemuxMediaInfo.AudioInfo[APinNum].pAudioStream = stream;
			APinNum++;
		}
	}
	return 0;
}

CBOOL NX_CDemuxFFmpegFilter::Flush()
{

	return CTRUE;
}

CBOOL NX_CDemuxFFmpegFilter::Run()
{
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux ] Run() ++\n");

	if (m_pVideoOutPin && m_pVideoOutPin->IsConnected())	
	{
		m_bVideoDis = CTRUE;
		m_pVideoOutPin->Active();
	}

	if (m_pAudioOutPin && m_pAudioOutPin->IsConnected()){
		m_bAudioDis = CTRUE;
		m_pAudioOutPin->Active();
	}

	if (CFALSE == m_bRunning)
	{
		if (0 == m_Init){
			FFmpegOpen();
			m_Init = 1;
		}

		m_bExitThread = CFALSE;
		m_bSetFirstTime = CFALSE;
		if (pthread_create(&m_hThread, NULL, ThreadStub, this) < 0)
		{
			return CFALSE;
		}
		m_bRunning = CTRUE;
	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux ] Run() --\n");
	return CTRUE;
}

CBOOL NX_CDemuxFFmpegFilter::Stop()
{
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Stop() ++\n");

	ExitThread();

	if (CTRUE == m_bRunning)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Video Out InActive\n");
		if (m_pVideoOutPin && m_pVideoOutPin->IsConnected())
			m_pVideoOutPin->Inactive();

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Audio Out InActive\n");
		if (m_pAudioOutPin && m_pAudioOutPin->IsConnected())
			m_pAudioOutPin->Inactive();

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Main Thread Join ++\n");
		//	Wait Thread Exit
		pthread_join(m_hThread, NULL);
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Main Thread Join --\n");
		m_hThread = 0;
		m_bRunning = CFALSE;

		m_Init = 0;

		if (m_pFmtCtx) {
			av_close_input_file(m_pFmtCtx);
			m_pFmtCtx = NULL;
		}
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Stop() --\n");
	return CTRUE;
}

CBOOL NX_CDemuxFFmpegFilter::Pause()
{
	return CTRUE;
}

NX_CBasePin	*NX_CDemuxFFmpegFilter::GetPin(int Pos)
{
	if (Pos == VIDEOTYPE)
		return m_pVideoOutPin;
	else if (AUDIOTYPE == Pos)
		return m_pAudioOutPin;

	return NULL;
}

void *NX_CDemuxFFmpegFilter::ThreadStub(void *pObj)
{
	((NX_CDemuxFFmpegFilter*)pObj)->ThreadProcFile();
	return (void*)0;
}

void NX_CDemuxFFmpegFilter::ThreadProcFile()
{
	int32_t read_size = 0;
	m_pInBufPos = 0;
	CBOOL Ret = 0;

	while (1)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux]  NX_CDemuxFFmpegFilter: ThreadProc Start ++ !!!\n");

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){ 
			pthread_mutex_unlock(&m_hMutexThread);	
			NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux]  m_bExitThread = %d !!\n", m_bExitThread);
			break;
		}
		pthread_mutex_unlock(&m_hMutexThread);

		Ret = ReadStream(m_pInBuf + m_pInBufPos, m_pInBufPos, &read_size);
		if (CFALSE == Ret){
			NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux] Warring ReadStream: Ret = %d, read_size = %d !!\n", Ret, read_size);
		}

		if (read_size < 0){
			m_pInBufPos = 0;
			m_bVideoDis = CTRUE;
			m_bAudioDis = CTRUE;

			//	Deliver End of Stream
			DeliverEndOfStream();
			m_pEventNotify->SendEvent(EVENT_DEMUX_ERROR, 1);
			NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux] End of file.\n");
			break;
		}

		m_pInBufPos = 0;

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux]  NX_CDemuxFFmpegFilter: ThreadProc Start --  !!!\n");
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Exit main parsing thread(File)!!\n");
}

void NX_CDemuxFFmpegFilter::DeliverEndOfStream(void)
{
	NX_CDemuxOutputPin *pOutPin;

	for (int i = 0; i<2; i++)
	{
		if (i == 0)	pOutPin = m_pVideoOutPin;
		else if (i == 1)	pOutPin = m_pAudioOutPin;

		if (pOutPin && pOutPin->IsConnected() && pOutPin->IsActive())
		{
			NX_CSample *pSample = NULL;
			pOutPin->m_pInQueue->PopSample(&pSample);
			if (pSample)
			{
				//	Deliver End of Stream
				pSample->Release();
				pSample->SetActualDataLength(0);
				pSample->SetEndOfStream(CTRUE);
				pOutPin->m_pOutQueue->PushSample(pSample);
			}
		}
	}
}

int32_t NX_CDemuxFFmpegFilter::SetFileName(const char *filename)
{
	int32_t ret = -1;
	m_DemuxMode = DEMUX_MODE_FILE;

	if (NULL == filename || (NX_MAX_PATH - 1) < strlen((const char *)filename))
		return -1;
	strcpy((char *)m_FileName, (const char *)filename);

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] SetFileName : %s\n", m_FileName);

	//	Parsing Stream Information
	ret = ScanFile();

	if (ret < 0)
		NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux] ScanFile: ret = %d\n", ret);

	return ret;
}

CBOOL NX_CDemuxFFmpegFilter::GetMediaType(void *MediaType, int32_t Type)
{
	if (VIDEOTYPE == Type){
		memcpy(MediaType, &m_VMediaType, sizeof(_NX_VideoMediaType));
	}
	else if (AUDIOTYPE == Type){
		memcpy(MediaType, &m_AMediaType, sizeof(_NX_AudioMediaType));
	}
	else{
		return CFALSE;
	}
	
	return CTRUE;
}

int32_t NX_CDemuxFFmpegFilter::PinCreate()
{
	int32_t i = 0;

	for (i = 0; i < m_VPinNum; i++)
		m_pVideoOut[i] = new NX_CDemuxOutputPin(this, OUT_PIN_TYPE_VIDEO);
	for (i = 0; i < m_APinNum; i++)
		m_pAudioOut[i] = new NX_CDemuxOutputPin(this, OUT_PIN_TYPE_AUDIO);

	return 0;
}

int32_t NX_CDemuxFFmpegFilter::SelectPin(int32_t VPin, int32_t Apin)
{
	NX_MediaType	*MediaType = NULL;

	if (m_VPinNum >= 1){		
		m_pVideoOutPin = m_pVideoOut[VPin - 1];
		m_VPinSelect = VPin - 1;
	}
	
	if (m_VPinNum >= 1)
		if (VPin > 0)
		{
			m_pVideoOutPin->GetMediaType(&MediaType);
			memset(MediaType, 0, sizeof(NX_MediaType));
			MediaType->MeidaType = VIDEOTYPE;
			memcpy(&MediaType->VMediaType, &m_VMediaType, sizeof(NX_VideoMediaType));
		}

	if (m_APinNum >= 1){
		m_pAudioOutPin = m_pAudioOut[Apin - 1];
		m_APinSelect = Apin - 1;
	}
	if (m_APinNum >= 1)
		if (Apin > 0)
		{
			m_pAudioOutPin->GetMediaType(&MediaType);
			memset(MediaType, 0, sizeof(NX_MediaType));
			MediaType->MeidaType = AUDIOTYPE;
			memcpy(&MediaType->AMediaType, &m_AMediaType, sizeof(NX_AudioMediaType));
		}

	return 0;
}

int32_t NX_CDemuxFFmpegFilter::PinInfo(PinType *pininfo)
{
	pininfo->OutPinNum.APinNum = m_APinNum;
	pininfo->OutPinNum.VPinNum = m_VPinNum;

	return 0;
}

#define DEMUX_ON	1
#define DEMUX_OFF	0

int32_t	NX_CDemuxFFmpegFilter::ScanFile(void)
{

	int32_t i = 0, j = 0;

	int32_t video_stream_idx = -1;
	int32_t audio_stream_idx = -1;
	AVInputFormat *iformat = NULL;
	AVCodec *video_codec = NULL;
	AVCodec *audio_codec = NULL;
	AVStream *stream = NULL;
	AVCodecContext *dec = NULL;

	m_pFmtCtx = avformat_alloc_context();

	m_pFmtCtx->flags |= CODEC_FLAG_TRUNCATED;

	/* open input file, and allocate format context */
	if (avformat_open_input(&m_pFmtCtx, (const char *)m_FileName, iformat, NULL) < 0) {
		printf("Could not open source file %s\n", m_FileName);
		return -1;
	}
	
	/* fill the streams in the format context */
	if (av_find_stream_info(m_pFmtCtx) < 0)
	{
		av_close_input_file(m_pFmtCtx);
		return -1;
	}

//	av_dump_format(m_pFmtCtx, 0, m_FileName, 0);	

	for (i = 0; i < DEMUX_MEDIA_MAX; i++)
	{
		m_DemuxMediaInfo.VideoInfo[i].iMp4Class = -1;
		m_DemuxMediaInfo.VideoInfo[i].iVideoCodecID = -1;
		m_DemuxMediaInfo.VideoInfo[i].iVpuCodecType = -1;
		m_DemuxMediaInfo.VideoInfo[i].VideoTrackInfo.VideoPin = -1;
		m_DemuxMediaInfo.VideoInfo[i].VideoTrackInfo.StreamIDX = -1;
		m_DemuxMediaInfo.AudioInfo[i].AudioTrackInfo.AudioPin = -1;
		m_DemuxMediaInfo.AudioInfo[i].AudioTrackInfo.StreamIDX = -1;
	}


	m_VPinNum = 0;
	m_APinNum = 0;

	memset(&m_MediaInfo, 0, sizeof(MEDIA_INFO));
	m_DemuxMediaInfo.Duration = m_pFmtCtx->duration / 1000;

	for (i = 0; i<(int)m_pFmtCtx->nb_streams; i++)
	{
		stream = m_pFmtCtx->streams[i];
		if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{			

			m_DemuxMediaInfo.VideoTrackTotNum++;
			m_DemuxMediaInfo.VideoInfo[m_VPinNum].VideoTrackInfo.VideoPin = m_VPinNum;
			m_DemuxMediaInfo.VideoInfo[m_VPinNum].iWidth = stream->codec->width;
			m_DemuxMediaInfo.VideoInfo[m_VPinNum].iHeight = stream->codec->height;
			m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVideoCodecID = stream->codec->codec_id;

			if (!(video_codec = avcodec_find_decoder(stream->codec->codec_id)))
			{
				printf("Unsupported codec (id=%d) for input stream %d\n", stream->codec->codec_id, stream->index);
				return -1;
			}

			if (avcodec_open(stream->codec, video_codec)<0)
			{
				printf("Error while opening codec for input stream %d\n", stream->index);
				return -1;
			}
			else
			{
				if (video_stream_idx == -1)
				{
					m_DemuxMediaInfo.VideoInfo[m_VPinNum].VideoTrackInfo.StreamIDX = i;
					m_DemuxMediaInfo.VideoInfo[m_VPinNum].iFrameRate = 0;
					m_DemuxMediaInfo.VideoInfo[m_VPinNum].pVideoStream = stream;
					m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVpuCodecType = CodecIdToVpuType(stream->codec->codec_id, stream->codec->codec_tag);
					if (m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVpuCodecType < 0)
						return -1;
					m_DemuxMediaInfo.VideoInfo[m_VPinNum].iMp4Class = fourCCToMp4Class(stream->codec->codec_tag);
					if (m_DemuxMediaInfo.VideoInfo[m_VPinNum].iMp4Class == -1)
						m_DemuxMediaInfo.VideoInfo[m_VPinNum].iMp4Class = codecIdToMp4Class(stream->codec->codec_id);
					NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux ] codec_id = %d, width = %d, height = %d, vpuCodecType = %d, mp4Class = %d\n", stream->codec->codec_id, stream->codec->width, stream->codec->height, m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVpuCodecType, m_DemuxMediaInfo.VideoInfo[m_VPinNum].iMp4Class);


#if (ENABLE_THEORA)
					//	Intialize Theora Parser
					if (m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVpuCodecType == NX_THEORA_DEC)
					{
						theora_parser_init((void**)&m_TheoraParser);
					}

#endif
					m_DemuxMediaInfo.VideoInfo[m_VPinNum].m_SeqSize = GetSequenceInformation(&m_H264AvcCHeader, stream, m_SeqData, sizeof(m_SeqData));

					m_DemuxMediaInfo.VideoInfo[m_VPinNum].m_SeqData = (uint8_t *)malloc(m_DemuxMediaInfo.VideoInfo[m_VPinNum].m_SeqSize);
					memcpy(m_DemuxMediaInfo.VideoInfo[m_VPinNum].m_SeqData, m_SeqData, m_DemuxMediaInfo.VideoInfo[m_VPinNum].m_SeqSize);
					for (j = 0; j < CODE_ID_MAX; j++){
						if (m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVideoCodecID == codec_tab[j].codec_id){
							m_DemuxMediaInfo.VideoInfo[m_VPinNum].iVideoCodecName = codec_tab[j].codec_name;
							break;
						}
					}
				}
				else
				{
					avcodec_close(stream->codec);
				}
				video_stream_idx = -1;
				m_VPinNum++;
			}
		}
		else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{		
			dec = stream->codec;

			m_DemuxMediaInfo.AudioTrackTotNum++;
			m_DemuxMediaInfo.AudioInfo[m_APinNum].AudioTrackInfo.AudioPin = m_APinNum;
			m_DemuxMediaInfo.AudioInfo[m_APinNum].iSampleRate = stream->codec->sample_rate;
			m_DemuxMediaInfo.AudioInfo[m_APinNum].iChannels = stream->codec->channels;
			m_DemuxMediaInfo.AudioInfo[m_APinNum].AudioCodecID = stream->codec->codec_id;
			m_DemuxMediaInfo.AudioInfo[m_APinNum].iSampleFmt = stream->codec->sample_fmt;			

			//
			NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux ] codec (id=%d) for input stream %d, block_align = %d, sample_fme = %d\n", stream->codec->codec_id, stream->index, stream->codec->block_align, stream->codec->sample_fmt);
			if (!(audio_codec = avcodec_find_decoder(stream->codec->codec_id)))
			{
				printf("Unsupported codec (id=%d) for input stream %d\n", stream->codec->codec_id, stream->index);
				return -1;
			}

			if (avcodec_open(stream->codec, audio_codec)<0)
			{
				printf("Error while opening codec for input stream %d\n", stream->index);
				return -1;
			}
			else
			{
				if (audio_stream_idx == -1)
				{
					m_DemuxMediaInfo.AudioInfo[m_APinNum].AudioTrackInfo.StreamIDX = i;
					m_DemuxMediaInfo.AudioInfo[m_APinNum].pAudioStream = stream;

					for (j = 0; j < CODE_ID_MAX; j++){
						if (m_DemuxMediaInfo.AudioInfo[m_APinNum].AudioCodecID == codec_tab[j].codec_id){
							m_DemuxMediaInfo.AudioInfo[m_APinNum].AudioCodecName = codec_tab[j].codec_name;
							break;
						}
					}
					m_DemuxMediaInfo.AudioInfo[m_APinNum].m_SeqSize = dec->extradata_size;
					m_DemuxMediaInfo.AudioInfo[m_APinNum].m_SeqData = (uint8_t *)malloc(m_DemuxMediaInfo.AudioInfo[m_APinNum].m_SeqSize);
					memcpy(m_DemuxMediaInfo.AudioInfo[m_APinNum].m_SeqData, dec->extradata, m_DemuxMediaInfo.AudioInfo[m_APinNum].m_SeqSize);
					
					m_DemuxMediaInfo.AudioInfo[m_APinNum].iBitrate = dec->bit_rate;
					m_DemuxMediaInfo.AudioInfo[m_APinNum].iBlockAlign = dec->block_align;
				}
				else
				{
					avcodec_close(stream->codec);
				}
				audio_stream_idx = -1;
				m_APinNum++;
			}
		}
		else if (stream->codec->codec_type == AVMEDIA_TYPE_DATA)
		{
			m_DemuxMediaInfo.DataTrackTotNum++;
		}
	}
	if (m_pFmtCtx) {
		av_close_input_file(m_pFmtCtx);
		m_pFmtCtx = NULL;
	}

	m_MediaInfo.VideoTrackTotNum = m_DemuxMediaInfo.VideoTrackTotNum;
	m_MediaInfo.Duration = m_DemuxMediaInfo.Duration;
	for (i = 0; i <m_DemuxMediaInfo.VideoTrackTotNum; i++)
	{
		m_MediaInfo.VideoInfo[i].Height = m_DemuxMediaInfo.VideoInfo[i].iHeight;
		m_MediaInfo.VideoInfo[i].Width = m_DemuxMediaInfo.VideoInfo[i].iWidth;
		m_MediaInfo.VideoInfo[i].VCodecID = m_DemuxMediaInfo.VideoInfo[i].iVideoCodecID;
		m_MediaInfo.VideoInfo[i].VideoTrackNum = m_DemuxMediaInfo.VideoInfo[i].VideoTrackInfo.VideoPin;
	}

	m_MediaInfo.AudioTrackTotNum = m_DemuxMediaInfo.AudioTrackTotNum;

	for (i = 0; i <m_DemuxMediaInfo.AudioTrackTotNum; i++)
	{
		m_MediaInfo.AudioInfo[i].samplerate = m_DemuxMediaInfo.AudioInfo[i].iSampleRate;
		m_MediaInfo.AudioInfo[i].channels = m_DemuxMediaInfo.AudioInfo[i].iChannels;
		m_MediaInfo.AudioInfo[i].ACodecID = m_DemuxMediaInfo.AudioInfo[i].AudioCodecID;
		m_MediaInfo.AudioInfo[i].AudioTrackNum = m_DemuxMediaInfo.AudioInfo[i].AudioTrackInfo.AudioPin;
	}

	m_VPinSelect = 0;
	m_APinSelect = 0;
	memset(&m_VMediaType, 0, sizeof(m_VMediaType));
	m_VMediaType.MeidaType = VIDEOTYPE;
	m_VMediaType.CodecID = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].iVideoCodecName;
	m_VMediaType.VpuCodecType = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].iVpuCodecType;
	m_VMediaType.Mp4Class = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].iMp4Class;
	m_VMediaType.Width = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].iWidth;
	m_VMediaType.Height = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].iHeight;
	m_VMediaType.Duration = m_DemuxMediaInfo.Duration;
	m_VMediaType.Framerate = 0;
	m_VMediaType.pSeqData = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].m_SeqData;
	m_VMediaType.SeqDataSize = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].m_SeqSize;

	memset(&m_AMediaType, 0, sizeof(m_AMediaType));
	m_AMediaType.CodecID = m_DemuxMediaInfo.AudioInfo[m_APinSelect].AudioCodecName;
	m_AMediaType.Samplerate = m_DemuxMediaInfo.AudioInfo[m_APinSelect].iSampleRate;
	m_AMediaType.Channels = m_DemuxMediaInfo.AudioInfo[m_APinSelect].iChannels;
	m_AMediaType.Bitrate = m_DemuxMediaInfo.AudioInfo[m_APinSelect].iBitrate;
	m_AMediaType.BlockAlign = m_DemuxMediaInfo.AudioInfo[m_APinSelect].iBlockAlign;
	m_AMediaType.Duration = m_DemuxMediaInfo.Duration;
	m_AMediaType.FormatType = m_DemuxMediaInfo.AudioInfo[m_APinSelect].iSampleFmt;
	m_AMediaType.pSeqData = m_DemuxMediaInfo.AudioInfo[m_APinSelect].m_SeqData;
	m_AMediaType.SeqDataSize = m_DemuxMediaInfo.AudioInfo[m_APinSelect].m_SeqSize;

	return 0;
}

int32_t	NX_CDemuxFFmpegFilter::GetMediaInfo(MEDIA_INFO * pMediaInfo)
{

	memcpy(pMediaInfo, &m_MediaInfo, sizeof(MEDIA_INFO));

	return 0;
}

CBOOL NX_CDemuxFFmpegFilter::SeekStream(int64_t seekTime)	//millsecend
{
	unsigned int sec;
	int64_t tm;

	if (0 == m_Init){
		FFmpegOpen();
		m_Init = 1;
	}

	AVStream *stream;
	int index;
	int seekret;

	index = av_find_default_stream_index(m_pFmtCtx);
	printf("default stream index %d, %d", index, m_AudioStreamIdx[0]);
	if (index < 0)
		return -1;

	/* get the stream for seeking */
	stream = m_pFmtCtx->streams[index];
	sec = seekTime / 1000;
	tm = (int64_t)(sec * AV_TIME_BASE);

	tm = av_rescale_q(tm, AV_TIME_BASE_Q, stream->time_base);
	{
		int keyframeidx;
		int64_t fftarget;
		int64_t target;

		/* search in the index for the previous keyframe */
		keyframeidx = av_index_search_timestamp(stream, tm, AVSEEK_FLAG_BACKWARD);

		printf( "keyframeidx: %d", keyframeidx);

		if (keyframeidx >= 0) {
			fftarget = stream->index_entries[keyframeidx].timestamp;
			printf("\n===fftarget = %lld\n", fftarget);
			{
				target = av_rescale_q(fftarget, AV_TIME_BASE_Q, stream->time_base);
				printf("\n===target = %lld\n", target);
			}

		}

	}

	if ((seekret =	av_seek_frame(m_pFmtCtx, index, tm,	AVSEEK_FLAG_BACKWARD)) < 0)
		return -1;

	return CTRUE;
}

//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//						File Handling Module
//

//


//--------------------------------------------------------------------------------
//  CodecInfo
typedef struct {
	int codStd;
	int mp4Class;
	int codec_id;
	unsigned int fourcc;
} CodStdTab;

#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif

static const CodStdTab codstd_tab[] = {
	{ NX_AVC_DEC, 0, CODEC_ID_H264, MKTAG('H', '2', '6', '4') },
	{ NX_AVC_DEC, 0, CODEC_ID_H264, MKTAG('X', '2', '6', '4') },
	{ NX_AVC_DEC, 0, CODEC_ID_H264, MKTAG('A', 'V', 'C', '1') },
	{ NX_AVC_DEC, 0, CODEC_ID_H264, MKTAG('V', 'S', 'S', 'H') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('H', '2', '6', '3') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('X', '2', '6', '3') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('T', '2', '6', '3') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('L', '2', '6', '3') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('V', 'X', '1', 'K') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('Z', 'y', 'G', 'o') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('H', '2', '6', '3') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('I', '2', '6', '3') },	/* intel h263 */
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('H', '2', '6', '1') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('U', '2', '6', '3') },
	{ NX_H263_DEC, 0, CODEC_ID_H263, MKTAG('V', 'I', 'V', '1') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('F', 'M', 'P', '4') },
	{ NX_MP4_DEC, 5, CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', 'X') },	// DivX 4
	{ NX_MP4_DEC, 1, CODEC_ID_MPEG4, MKTAG('D', 'X', '5', '0') },
	{ NX_MP4_DEC, 2, CODEC_ID_MPEG4, MKTAG('X', 'V', 'I', 'D') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('M', 'P', '4', 'S') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('M', '4', 'S', '2') },	//MPEG-4 version 2 simple profile
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG(4, 0, 0, 0) },	/* some broken avi use this */
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', '1') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('B', 'L', 'Z', '0') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('M', 'P', '4', 'V') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('U', 'M', 'P', '4') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('W', 'V', '1', 'F') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('S', 'E', 'D', 'G') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('R', 'M', 'P', '4') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('3', 'I', 'V', '2') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('F', 'F', 'D', 'S') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('F', 'V', 'F', 'W') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('D', 'C', 'O', 'D') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('M', 'V', 'X', 'M') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('P', 'M', '4', 'V') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('S', 'M', 'P', '4') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('D', 'X', 'G', 'M') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('V', 'I', 'D', 'M') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('M', '4', 'T', '3') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('G', 'E', 'O', 'X') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('H', 'D', 'X', '4') }, /* flipped video */
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('D', 'M', 'K', '2') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('D', 'I', 'G', 'I') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('I', 'N', 'M', 'C') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('E', 'P', 'H', 'V') }, /* Ephv MPEG-4 */
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('E', 'M', '4', 'A') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('M', '4', 'C', 'C') }, /* Divio MPEG-4 */
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('S', 'N', '4', '0') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('V', 'S', 'P', 'X') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('U', 'L', 'D', 'X') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('G', 'E', 'O', 'V') },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG('S', 'I', 'P', 'P') }, /* Samsung SHR-6040 */
	{ NX_DIV3_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '3') }, /* default signature when using MSMPEG4 */
	{ NX_DIV3_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('M', 'P', '4', '3') },
	{ NX_DIV3_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('M', 'P', 'G', '3') },
	{ NX_MP4_DEC, 1, CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '5') },
	{ NX_MP4_DEC, 1, CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '6') },
	{ NX_MP4_DEC, 5, CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '4') },
	{ NX_DIV3_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('D', 'V', 'X', '3') },
	{ NX_DIV3_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('A', 'P', '4', '1') },	//Another hacked version of Microsoft's MP43 codec. 
	{ NX_MP4_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('C', 'O', 'L', '1') },
	{ NX_MP4_DEC, 0, CODEC_ID_MSMPEG4V3, MKTAG('C', 'O', 'L', '0') },	// not support ms mpeg4 v1, 2    
	{ NX_MP4_DEC, 256, CODEC_ID_FLV1, MKTAG('F', 'L', 'V', '1') }, /* Sorenson spark */
	{ NX_VC1_DEC, 0, CODEC_ID_WMV1, MKTAG('W', 'M', 'V', '1') },
	{ NX_VC1_DEC, 0, CODEC_ID_WMV2, MKTAG('W', 'M', 'V', '2') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG1VIDEO, MKTAG('M', 'P', 'G', '1') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG1VIDEO, MKTAG('M', 'P', 'G', '2') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('M', 'P', 'G', '2') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('M', 'P', 'E', 'G') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG1VIDEO, MKTAG('M', 'P', '2', 'V') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG1VIDEO, MKTAG('P', 'I', 'M', '1') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('P', 'I', 'M', '2') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG1VIDEO, MKTAG('V', 'C', 'R', '2') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG1VIDEO, MKTAG(1, 0, 0, 16) },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG(2, 0, 0, 16) },
	{ NX_MP4_DEC, 0, CODEC_ID_MPEG4, MKTAG(4, 0, 0, 16) },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('D', 'V', 'R', ' ') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('M', 'M', 'E', 'S') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('L', 'M', 'P', '2') }, /* Lead MPEG2 in avi */
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('S', 'L', 'I', 'F') },
	{ NX_MP2_DEC, 0, CODEC_ID_MPEG2VIDEO, MKTAG('E', 'M', '2', 'V') },
	{ NX_VC1_DEC, 0, CODEC_ID_WMV3, MKTAG('W', 'M', 'V', '3') },
	{ NX_VC1_DEC, 0, CODEC_ID_VC1, MKTAG('W', 'V', 'C', '1') },
	{ NX_VC1_DEC, 0, CODEC_ID_VC1, MKTAG('W', 'M', 'V', 'A') },

	{ NX_RV_DEC, 0, CODEC_ID_RV30, MKTAG('R', 'V', '3', '0') },
	{ NX_RV_DEC, 0, CODEC_ID_RV40, MKTAG('R', 'V', '4', '0') },
	{ NX_THEORA_DEC, 0, CODEC_ID_THEORA, MKTAG('T', 'H', 'E', 'O') },
	{ NX_VP8_DEC, 0, CODEC_ID_VP8, MKTAG('V', 'P', '8', '0') }
#if 0
	{ STD_AVS, 0, CODEC_ID_CAVS, MKTAG('C', 'A', 'V', 'S') },
	{ STD_AVS, 0, CODEC_ID_AVS, MKTAG('A', 'V', 'S', '2') },
	{ STD_VP3, 0, CODEC_ID_VP3, MKTAG('V', 'P', '3', '0') },
	{ STD_VP3, 0, CODEC_ID_VP3, MKTAG('V', 'P', '3', '1') },
#endif
};

void NX_CDemuxFFmpegFilter::dumpdata(void *data, int len, const char *msg)
{
	int i = 0;
	unsigned char *byte = (unsigned char *)data;
	printf("Dump Data : %s", msg);
	for (i = 0; i<len; i++)
	{
		if (i % 32 == 0)	printf("\n\t");
		printf("%.2x", byte[i]);
		if (i % 4 == 3) printf(" ");
	}
	printf("\n");
}


int NX_CDemuxFFmpegFilter::fourCCToMp4Class(unsigned int fourcc)
{
	unsigned int i;
	int mp4Class = -1;
	char str[5];

	str[0] = toupper((char)fourcc);
	str[1] = toupper((char)(fourcc >> 8));
	str[2] = toupper((char)(fourcc >> 16));
	str[3] = toupper((char)(fourcc >> 24));
	str[4] = '\0';

	for (i = 0; i<sizeof(codstd_tab) / sizeof(codstd_tab[0]); i++)
	{
		if (codstd_tab[i].fourcc == (unsigned int)MKTAG(str[0], str[1], str[2], str[3]))
		{
			mp4Class = codstd_tab[i].mp4Class;
			break;
		}
	}

	return mp4Class;
}

int NX_CDemuxFFmpegFilter::fourCCToCodStd(unsigned int fourcc)
{
	int codStd = -1;
	unsigned int i;

	char str[5];

	str[0] = toupper((char)fourcc);
	str[1] = toupper((char)(fourcc >> 8));
	str[2] = toupper((char)(fourcc >> 16));
	str[3] = toupper((char)(fourcc >> 24));
	str[4] = '\0';

	for (i = 0; i<sizeof(codstd_tab) / sizeof(codstd_tab[0]); i++)
	{
		if (codstd_tab[i].fourcc == (unsigned int)MKTAG(str[0], str[1], str[2], str[3]))
		{
			codStd = codstd_tab[i].codStd;
			break;
		}
	}

	return codStd;
}

int NX_CDemuxFFmpegFilter::codecIdToMp4Class(int codec_id)
{
	int mp4Class = -1;
	unsigned int i;

	for (i = 0; i<sizeof(codstd_tab) / sizeof(codstd_tab[0]); i++)
	{
		if (codstd_tab[i].codec_id == codec_id)
		{
			mp4Class = codstd_tab[i].mp4Class;
			break;
		}
	}

	return mp4Class;

}
int NX_CDemuxFFmpegFilter::codecIdToCodStd(int codec_id)
{
	int codStd = -1;
	unsigned int i;

	for (i = 0; i<sizeof(codstd_tab) / sizeof(codstd_tab[0]); i++)
	{
		if (codstd_tab[i].codec_id == codec_id)
		{
			codStd = codstd_tab[i].codStd;
			break;
		}
	}
	return codStd;
}

int NX_CDemuxFFmpegFilter::codecIdToFourcc(int codec_id)
{
	int fourcc = 0;
	unsigned int i;

	for (i = 0; i<sizeof(codstd_tab) / sizeof(codstd_tab[0]); i++)
	{
		if (codstd_tab[i].codec_id == codec_id)
		{
			fourcc = codstd_tab[i].fourcc;
			break;
		}
	}
	return fourcc;
}
//--------------------------------------------------------------------------------

int NX_CDemuxFFmpegFilter::GetSequenceInformation(NX_AVCC_TYPE *h264Hader, AVStream *stream, unsigned char *buffer, int size)
{
	unsigned char *pbHeader = buffer;
	enum CodecID codecId = stream->codec->codec_id;
	int fourcc;
	int frameRate = 0;
	int nMetaData = stream->codec->extradata_size;
	unsigned char *pbMetaData = stream->codec->extradata;
	int retSize = 0;
	unsigned int tag = stream->codec->codec_tag;

	if (stream->avg_frame_rate.den && stream->avg_frame_rate.num)
		frameRate = (int)((double)stream->avg_frame_rate.num / (double)stream->avg_frame_rate.den);
	if (!frameRate && stream->r_frame_rate.den && stream->r_frame_rate.num)
		frameRate = (int)((double)stream->r_frame_rate.num / (double)stream->r_frame_rate.den);

	if ((codecId == CODEC_ID_H264) && (stream->codec->extradata_size>0))
	{
		if (stream->codec->extradata[0] == 0x1)
		{
			NX_ParseSpsPpsFromAVCC(pbMetaData, nMetaData, h264Hader);
			NX_MakeH264StreamAVCCtoANNEXB(h264Hader, buffer, &retSize);
			return retSize;
		}
	}
	else if ((codecId == CODEC_ID_VC1))
	{
		retSize = nMetaData;
		memcpy(pbHeader, pbMetaData, retSize);
		//if there is no seq startcode in pbMetatData. VPU will be failed at seq_init stage.
		return retSize;
	}
	else if ((codecId == CODEC_ID_MSMPEG4V3))
	{
		switch (tag)
		{
		case MKTAG('D', 'I', 'V', '3'):
		case MKTAG('M', 'P', '4', '3'):
		case MKTAG('M', 'P', 'G', '3'):
		case MKTAG('D', 'V', 'X', '3'):
		case MKTAG('A', 'P', '4', '1'):
			if (!nMetaData)
			{
				PUT_LE32(pbHeader, MKTAG('C', 'N', 'M', 'V'));	//signature 'CNMV'
				PUT_LE16(pbHeader, 0x00);						//version
				PUT_LE16(pbHeader, 0x20);						//length of header in bytes
				PUT_LE32(pbHeader, MKTAG('D', 'I', 'V', '3'));	//codec FourCC
				PUT_LE16(pbHeader, stream->codec->width);		//width
				PUT_LE16(pbHeader, stream->codec->height);		//height
				PUT_LE32(pbHeader, stream->r_frame_rate.num);	//frame rate
				PUT_LE32(pbHeader, stream->r_frame_rate.den);	//time scale(?)
				PUT_LE32(pbHeader, stream->nb_index_entries);	//number of frames in file
				PUT_LE32(pbHeader, 0);							//unused
				retSize += 32;
			}
			else
			{
				PUT_BE32(pbHeader, nMetaData);
				retSize += 4;
				memcpy(pbHeader, pbMetaData, nMetaData);
				retSize += nMetaData;
			}
			return retSize;
		default:
			break;

		}
	}
	else if ((codecId == CODEC_ID_WMV1) || (codecId == CODEC_ID_WMV2) || (codecId == CODEC_ID_WMV3))
	{
#ifdef	RCV_V2	//	RCV_V2
		PUT_LE32(pbHeader, ((0xC5 << 24) | 0));
		retSize += 4; //version
		PUT_LE32(pbHeader, nMetaData);
		retSize += 4;

		memcpy(pbHeader, pbMetaData, nMetaData);
		pbHeader += nMetaData;
		retSize += nMetaData;

		PUT_LE32(pbHeader, stream->codec->height);
		retSize += 4;
		PUT_LE32(pbHeader, stream->codec->width);
		retSize += 4;
		PUT_LE32(pbHeader, 12);
		retSize += 4;
		PUT_LE32(pbHeader, 2 << 29 | 1 << 28 | 0x80 << 24 | 1 << 0);
		retSize += 4; // STRUCT_B_FRIST (LEVEL:3|CBR:1:RESERVE:4:HRD_BUFFER|24)
		PUT_LE32(pbHeader, stream->codec->bit_rate);
		retSize += 4; // hrd_rate
		PUT_LE32(pbHeader, frameRate);
		retSize += 4; // frameRate
#else	//RCV_V1
		PUT_LE32(pbHeader, (0x85 << 24) | 0x00);
		retSize += 4; //frames count will be here
		PUT_LE32(pbHeader, nMetaData);
		retSize += 4;
		memcpy(pbHeader, pbMetaData, nMetaData);
		pbHeader += nMetaData;
		retSize += nMetaData;
		PUT_LE32(pbHeader, stream->codec->height);
		retSize += 4;
		PUT_LE32(pbHeader, stream->codec->width);
		retSize += 4;
#endif
		return retSize;
	}
	else if ((stream->codec->codec_id == CODEC_ID_RV40 || stream->codec->codec_id == CODEC_ID_RV30)
		&& (stream->codec->extradata_size>0))
	{
		if (CODEC_ID_RV40 == stream->codec->codec_id)
		{
			fourcc = MKTAG('R', 'V', '4', '0');
		}
		else
		{
			fourcc = MKTAG('R', 'V', '3', '0');
		}
		retSize = 26 + nMetaData;
		PUT_BE32(pbHeader, retSize); //Length
		PUT_LE32(pbHeader, MKTAG('V', 'I', 'D', 'O')); //MOFTag
		PUT_LE32(pbHeader, fourcc); //SubMOFTagl
		PUT_BE16(pbHeader, stream->codec->width);
		PUT_BE16(pbHeader, stream->codec->height);
		PUT_BE16(pbHeader, 0x0c); //BitCount;
		PUT_BE16(pbHeader, 0x00); //PadWidth;
		PUT_BE16(pbHeader, 0x00); //PadHeight;
		PUT_LE32(pbHeader, frameRate);
		memcpy(pbHeader, pbMetaData, nMetaData);
		return retSize;
	}
	else if (stream->codec->codec_id == CODEC_ID_VP8)
	{
		PUT_LE32(pbHeader, MKTAG('D', 'K', 'I', 'F'));	//signature 'DKIF'
		PUT_LE16(pbHeader, 0x00);						//version
		PUT_LE16(pbHeader, 0x20);						//length of header in bytes
		PUT_LE32(pbHeader, MKTAG('V', 'P', '8', '0'));	//codec FourCC
		PUT_LE16(pbHeader, stream->codec->width);		//width
		PUT_LE16(pbHeader, stream->codec->height);		//height
		PUT_LE32(pbHeader, stream->r_frame_rate.num);	//frame rate
		PUT_LE32(pbHeader, stream->r_frame_rate.den);	//time scale(?)
		PUT_LE32(pbHeader, stream->nb_index_entries);	//number of frames in file
		PUT_LE32(pbHeader, 0);							//unused
		retSize += 32;
		return retSize;
	}
#if	(ENABLE_THEORA)
	else if (stream->codec->codec_id == CODEC_ID_THEORA)
	{
		ThoScaleInfo thoScaleInfo;
		tho_parser_t *thoParser = m_TheoraParser;
		thoParser->open(thoParser->handle, stream->codec->extradata, stream->codec->extradata_size, (int *)&thoScaleInfo);
		retSize = theora_make_stream((void *)m_TheoraParser->handle, buffer, 1);
		return retSize;
	}
#endif

	memcpy(buffer, stream->codec->extradata, stream->codec->extradata_size);

	return stream->codec->extradata_size;
}

int NX_CDemuxFFmpegFilter::PasreAVCStream(AVPacket *pkt, int nalLengthSize, unsigned char *buffer, int outBufSize)
{
	int nalLength;

	//	input
	unsigned char *inBuf = pkt->data;
	int inSize = pkt->size;
	int pos = 0;

	//	'avcC' format
	do{
		nalLength = 0;

		if (nalLengthSize == 2)
		{
			nalLength = inBuf[0] << 8 | inBuf[1];
		}
		else if (nalLengthSize == 3)
		{
			nalLength = inBuf[0] << 16 | inBuf[1] << 8 | inBuf[2];
		}
		else if (nalLengthSize == 4)
		{
			nalLength = inBuf[0] << 24 | inBuf[1] << 16 | inBuf[2] << 8 | inBuf[3];
		}
		else if (nalLengthSize == 1)
		{
			nalLength = inBuf[0];
		}

		inBuf += nalLengthSize;
		inSize -= nalLengthSize;

		if (0 == nalLength || inSize<(int)nalLength)
		{
			printf("Error : avcC type nal length error (nalLength = %d, inSize=%d, nalLengthSize=%d)\n", nalLength, inSize, nalLengthSize);
			return -1;
		}

		//	put nal start code
		buffer[pos + 0] = 0x00;
		buffer[pos + 1] = 0x00;
		buffer[pos + 2] = 0x00;
		buffer[pos + 3] = 0x01;
		pos += 4;

		//printf("buffer = %x, pos = %d, inBuf = %x, nalLength = %d\n", buffer, pos, inBuf, nalLength);
		memcpy(buffer + pos, inBuf, nalLength);
		pos += nalLength;

		inSize -= nalLength;
		inBuf += nalLength;
	} while (2<inSize);
	return pos;
}

int NX_CDemuxFFmpegFilter::MakeRvStream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize)
{
	unsigned char *p = pkt->data;
	int cSlice, nSlice;
	int i, val, offset;
	//	int has_st_code = 0;
	int size;

	cSlice = p[0] + 1;
	nSlice = pkt->size - 1 - (cSlice * 8);
	size = 20 + (cSlice * 8);

	PUT_BE32(buffer, nSlice);
	if (AV_NOPTS_VALUE == (unsigned long long)pkt->pts)
	{
		PUT_LE32(buffer, 0);
	}
	else
	{
		PUT_LE32(buffer, (int)((double)(pkt->pts / stream->time_base.den))); // milli_sec
	}
	PUT_BE16(buffer, stream->codec->frame_number);
	PUT_BE16(buffer, 0x02); //Flags
	PUT_BE32(buffer, 0x00); //LastPacket
	PUT_BE32(buffer, cSlice); //NumSegments
	offset = 1;
	for (i = 0; i < (int)cSlice; i++)
	{
		val = (p[offset + 3] << 24) | (p[offset + 2] << 16) | (p[offset + 1] << 8) | p[offset];
		PUT_BE32(buffer, val); //isValid
		offset += 4;
		val = (p[offset + 3] << 24) | (p[offset + 2] << 16) | (p[offset + 1] << 8) | p[offset];
		PUT_BE32(buffer, val); //Offset
		offset += 4;
	}

	memcpy(buffer, pkt->data + (1 + (cSlice * 8)), nSlice);
	size += nSlice;

	//printf("size = %6d, nSlice = %6d, cSlice = %4d, pkt->size=%6d, frameNumber=%d\n", size, nSlice, cSlice, pkt->size, stream->codec->frame_number );
	return size;
}

int NX_CDemuxFFmpegFilter::MakeVC1Stream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize)
{
	int size = 0;
	unsigned char *p = pkt->data;

	if (stream->codec->codec_id == CODEC_ID_VC1)
	{
		if (p[0] != 0 || p[1] != 0 || p[2] != 1) // check start code as prefix (0x00, 0x00, 0x01)
		{
			*buffer++ = 0x00;
			*buffer++ = 0x00;
			*buffer++ = 0x01;
			*buffer++ = 0x0D;
			size = 4;
			memcpy(buffer, pkt->data, pkt->size);
			size += pkt->size;
		}
		else
		{
			memcpy(buffer, pkt->data, pkt->size);
			size = pkt->size; // no extra header size, there is start code in input stream.
		}
	}
	else
	{
		PUT_LE32(buffer, pkt->size | ((pkt->flags & AV_PKT_FLAG_KEY) ? 0x80000000 : 0));
		size += 4;
#ifdef RCV_V2	//	RCV_V2
		if (AV_NOPTS_VALUE == (unsigned long long)pkt->pts)
		{
			PUT_LE32(buffer, 0);
		}
		else
		{
			PUT_LE32(buffer, (int)((double)(pkt->pts / stream->time_base.den))); // milli_sec
		}
		size += 4;
#endif
		memcpy(buffer, pkt->data, pkt->size);
		size += pkt->size;
	}
	return size;
}


int NX_CDemuxFFmpegFilter::MakeDIVX3Stream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize)
{
	int size = pkt->size;
	unsigned int tag = stream->codec->codec_tag;
	if (tag == MKTAG('D', 'I', 'V', '3') || tag == MKTAG('M', 'P', '4', '3') ||
		tag == MKTAG('M', 'P', 'G', '3') || tag == MKTAG('D', 'V', 'X', '3') || tag == MKTAG('A', 'P', '4', '1'))
	{
		PUT_LE32(buffer, pkt->size);
		PUT_LE32(buffer, 0);
		PUT_LE32(buffer, 0);
		size += 12;
	}
	memcpy(buffer, pkt->data, pkt->size);
	return size;
}

int NX_CDemuxFFmpegFilter::MakeVP8Stream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize)
{
	PUT_LE32(buffer, pkt->size);		//	frame_chunk_len
	PUT_LE32(buffer, 0);				//	time stamp
	PUT_LE32(buffer, 0);
	memcpy(buffer, pkt->data, pkt->size);
	return (pkt->size + 12);
}



int NX_CDemuxFFmpegFilter::NX_ParseSpsPpsFromAVCC(unsigned char *extraData, int extraDataSize, NX_AVCC_TYPE *avcCInfo)
{
	int pos = 0;
	int i;
	int length;
	if (1 != extraData[0] || 11>extraDataSize){
		printf("Error : Invalid \"avcC\" data\n");
		return -1;
	}

	//	Parser "avcC" format data
	avcCInfo->version = (int)extraData[pos];			pos++;
	avcCInfo->profile_indication = (int)extraData[pos];			pos++;
	avcCInfo->compatible_profile = (int)extraData[pos];			pos++;
	avcCInfo->level_indication = (int)extraData[pos];			pos++;
	avcCInfo->nal_length_size = (int)(extraData[pos] & 0x03) + 1;	pos++;
	//	parser SPSs
	avcCInfo->num_sps = (int)(extraData[pos] & 0x1f);	pos++;
	for (i = 0; i<avcCInfo->num_sps; i++){
		length = avcCInfo->sps_length[i] = (int)(extraData[pos] << 8) | extraData[pos + 1];
		pos += 2;
		if ((pos + length) > extraDataSize){
			printf("Error : extraData size too small(SPS)\n");
			return -1;
		}
		memcpy(avcCInfo->sps_data[i], extraData + pos, length);
		pos += length;
	}

	//	parse PPSs
	avcCInfo->num_pps = (int)extraData[pos];			pos++;
	for (i = 0; i<avcCInfo->num_pps; i++){
		length = avcCInfo->pps_length[i] = (int)(extraData[pos] << 8) | extraData[pos + 1];
		pos += 2;
		if ((pos + length) > extraDataSize){
			printf("Error : extraData size too small(PPS)\n");
			return -1;
		}
		memcpy(avcCInfo->pps_data[i], extraData + pos, length);
		pos += length;
	}
	return 0;
}

void NX_CDemuxFFmpegFilter::NX_MakeH264StreamAVCCtoANNEXB(NX_AVCC_TYPE *avcc, unsigned char *pBuf, int *size)
{
	int i;
	int pos = 0;
	for (i = 0; i<avcc->num_sps; i++)
	{
		pBuf[pos++] = 0x00;
		pBuf[pos++] = 0x00;
		pBuf[pos++] = 0x00;
		pBuf[pos++] = 0x01;
		memcpy(pBuf + pos, avcc->sps_data[i], avcc->sps_length[i]);
		pos += avcc->sps_length[i];
	}
	for (i = 0; i<avcc->num_pps; i++)
	{
		pBuf[pos++] = 0x00;
		pBuf[pos++] = 0x00;
		pBuf[pos++] = 0x00;
		pBuf[pos++] = 0x01;
		memcpy(pBuf + pos, avcc->pps_data[i], avcc->pps_length[i]);
		pos += avcc->pps_length[i];
	}
	*size = pos;
}

int NX_CDemuxFFmpegFilter::CodecIdToVpuType(int codecId, unsigned int fourcc)
{
	int vpuCodecType = -1;
	//printf("codecId = %d, fourcc=%c%c%c%c\n", codecId, fourcc, fourcc>>8, fourcc>>16, fourcc>>24);
	if (codecId == CODEC_ID_MPEG4 || codecId == CODEC_ID_FLV1)
	{
		vpuCodecType = NX_MP4_DEC;
	}
	else if (codecId == CODEC_ID_MSMPEG4V3)
	{
		switch (fourcc)
		{
		case MKTAG('D', 'I', 'V', '3'):
		case MKTAG('M', 'P', '4', '3'):
		case MKTAG('M', 'P', 'G', '3'):
		case MKTAG('D', 'V', 'X', '3'):
		case MKTAG('A', 'P', '4', '1'):
			vpuCodecType = NX_DIV3_DEC;
			break;
		default:
			vpuCodecType = NX_MP4_DEC;
			break;
		}
	}
	else if (codecId == CODEC_ID_H263 || codecId == CODEC_ID_H263P || codecId == CODEC_ID_H263I)
	{
		vpuCodecType = NX_H263_DEC;
	}
	else if (codecId == CODEC_ID_H264)
	{
		vpuCodecType = NX_AVC_DEC;
	}
	else if (codecId == CODEC_ID_MPEG2VIDEO)
	{
		vpuCodecType = NX_MP2_DEC;
	}
	else if ((codecId == CODEC_ID_WMV3) || (codecId == CODEC_ID_VC1))
	{
		vpuCodecType = NX_VC1_DEC;
	}
	else if ((codecId == CODEC_ID_RV30) || (codecId == CODEC_ID_RV40))
	{
		vpuCodecType = NX_RV_DEC;
	}
#if (ENABLE_THEORA)
	else if (codecId == CODEC_ID_THEORA)
	{
		vpuCodecType = NX_THEORA_DEC;
	}
#endif
	else if (codecId == CODEC_ID_VP8)
	{
		vpuCodecType = NX_VP8_DEC;
	}
	else
	{
		printf("Cannot support codecid(%d)\n", codecId);
		exit(-1);
	}
	return vpuCodecType;
}

//

CBOOL NX_CDemuxFFmpegFilter::VideoPacketParsing(AVPacket *pkt, int32_t *Size, int32_t *IsKey, uint8_t *buffer)
{
	AVStream *stream = m_DemuxMediaInfo.VideoInfo[m_VPinSelect].pVideoStream;
	enum CodecID codecId = stream->codec->codec_id;
	double timeStampRatio = 0;
		
	if (0 != (double)stream->time_base.den)
		timeStampRatio = (double)stream->time_base.num*1000. / (double)stream->time_base.den;
	*Size = 0;

	if (pkt->stream_index == stream->index)
	{
		//	check codec type		
		if (codecId == CODEC_ID_H264 && stream->codec->extradata_size > 0 && stream->codec->extradata[0] == 1)
		{
			*Size = PasreAVCStream(pkt, m_H264AvcCHeader.nal_length_size, buffer, 0);
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts * timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
		else if ((codecId == CODEC_ID_VC1) || (codecId == CODEC_ID_WMV1) || (codecId == CODEC_ID_WMV2) || (codecId == CODEC_ID_WMV3))
		{
			*Size = MakeVC1Stream(pkt, stream, buffer, 0);
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts * timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
		else if (codecId == CODEC_ID_RV30 || codecId == CODEC_ID_RV40)
		{
			*Size = MakeRvStream(pkt, stream, buffer, 0);
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts * timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
		else if (codecId == CODEC_ID_MSMPEG4V3)
		{
			*Size = MakeDIVX3Stream(pkt, stream, buffer, 0);
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts * timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
#if	(ENABLE_THEORA)
		else if (codecId == CODEC_ID_THEORA)
		{
			tho_parser_t *thoParser = m_TheoraParser;
			if (thoParser->read_frame(thoParser->handle, pkt->data, pkt->size) < 0)
			{
				printf("Theora Read Frame Failed!!!\n");
				exit(1);
			}
			*Size = theora_make_stream((void *)thoParser->handle, buffer, 3 /*Theora Picture Run*/);
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts*timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
#endif
		else if (codecId == CODEC_ID_VP8)
		{
			*Size = MakeVP8Stream(pkt, stream, buffer, 0);
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts * timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
		else
		{
			memcpy(buffer, pkt->data, pkt->size);
			*Size = pkt->size;
			*IsKey = (pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0;
			if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
				m_TimeStamp = pkt->pts * timeStampRatio;
			else
				m_TimeStamp = -1;
			return CTRUE;
		}
	}

	return CFALSE;
}


CBOOL NX_CDemuxFFmpegFilter::AudioPacketParsing(AVPacket *pkt, int32_t *Size, uint8_t *buffer)
{
	AVStream *stream = m_DemuxMediaInfo.AudioInfo[m_APinSelect].pAudioStream;
	double timeStampRatio = (double)stream->time_base.num*1000. / (double)stream->time_base.den;
	*Size = 0;

	memcpy(buffer, pkt->data, pkt->size);
	*Size = pkt->size;

	if ((unsigned long long)pkt->pts != AV_NOPTS_VALUE)
		m_TimeStamp = pkt->pts * timeStampRatio;
	else
		m_TimeStamp = -1;

	return CTRUE;
}

CBOOL NX_CDemuxFFmpegFilter::ReadStream(uint8_t *pBuf, int32_t readSize, int32_t *actualReadSize)
{

	int64_t PTS = (int64_t)(0);
	int64_t DiffTime;

	NX_CDemuxOutputPin *pOut = NULL;
	AVPacket Pkt;
	int32_t Size;
	int32_t IsKey;
	int Type = -1;
	CBOOL ret = CTRUE;

	av_init_packet(&Pkt);
	do{
		ret = av_read_frame(m_pFmtCtx, &Pkt);
		if (ret < 0){
			*actualReadSize = -1;
			return CFALSE;
		}
		if (ret >= 0)
		{
			if (Pkt.stream_index == m_DemuxMediaInfo.VideoInfo[m_VPinSelect].VideoTrackInfo.StreamIDX) {
				int res = 0;
				res = VideoPacketParsing(&Pkt, &Size, &IsKey, pBuf);
				Type = DATATYPE_VIDEO;
				av_free_packet(&Pkt);
				if (CTRUE == res)
					break;
			}
			else if (Pkt.stream_index == m_DemuxMediaInfo.AudioInfo[m_APinSelect].AudioTrackInfo.StreamIDX) {
				ret = AudioPacketParsing(&Pkt, &Size, pBuf);
				Type = DATATYPE_AUDIO;
				av_free_packet(&Pkt);
				if (CTRUE == ret)
					break;
			}
			else{
				Type = DATATYPE_USER;
				av_free_packet(&Pkt);
				break;
			}
		}
		else
		{
			//	EOS Detected
			if (AVERROR_EOF == ret){
				*actualReadSize = -1;
				NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux] AVERROR_EOF !!\n");
			}
			return CFALSE;
		}
	} while (1);
	*actualReadSize = ret;

	if (Type == DATATYPE_VIDEO)
	{
		pOut = m_pVideoOutPin;
	}
	else if (Type == DATATYPE_AUDIO)
	{
		pOut = m_pAudioOutPin;
	}
	else
	{
		//	Skip User Data
		return CTRUE;
	}
	
	if (m_bSetFirstTime == CFALSE)
	{
		//	
		m_PrevTimeStamp = 0;
		m_TimeOffset = m_StreamStartTime;

		m_pRefClock->SetReferenceTime(m_StreamStartTime);

		NX_DbgMsg(DBG_MSG_OFF, "======  SetReferenceTime( %d )\n", (int32_t)(m_StreamStartTime));

		m_bSetFirstTime = CTRUE;
	}
	else{
		int64_t CurrTime = 0;
		if (m_PrevTimeStamp > PTS){
			DiffTime = m_PrevTimeStamp - PTS;
			if (DiffTime > MEDIA_RESYNC_TIME){
				m_TimeOffset = PTS - CurrTime;
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Resync : m_PrevTimeStamp - PTS > %d\n", (int32_t)DiffTime);
			}
		}
		else{
			DiffTime = PTS - m_PrevTimeStamp;
			if (DiffTime > MEDIA_RESYNC_TIME){
				//m_pRefClock->GetCurrTime(&CurrTime);
				m_TimeOffset = PTS - CurrTime;
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Resync : PTS - m_PrevTimeStamp > %d\n", (int32_t)DiffTime);
			}
		}
	}

	m_PrevTimeStamp = PTS;

	PTS -= m_TimeOffset;			//	
	PTS += m_BufferingTime;			//	


	if (NULL == pOut || !pOut->IsConnected() || !pOut->IsActive())
	{
		//	Not Connected
		//printf(":%s:Line(%d) : pOut = %d, pOut->IsConnected() = %d, pOut->IsActive() = %d, Type = %d Not Connected \n", __FILE__, __LINE__, pOut, pOut->IsConnected(), pOut->IsActive(), Type);
		return CTRUE;
	}

	//	Send to Output Pin
	//
	//	Step 1. Get Sample From m_pInQueue.
	//	Step 2. Write Data
	//	Step 3. Push Sample to Output Queue
	//

	//	Step 1. Get Sample From m_pInQueue
	NX_CSample *pSample = NULL;
	pOut->m_pInQueue->PopSample(&pSample);
	if (pSample)
	{
		uint32_t *pDest = NULL;
		pSample->GetPointer(&pDest);
		memcpy((uint8_t*)pDest, pBuf, Size);
		pSample->SetActualDataLength(Size);
		pSample->SetTime(m_TimeStamp);

		//	TODO : Set Key Frame Flag
		//pSample->SetSyncPoint((GetMPEG4VopType(pEsInfo->pBuf, pEsInfo->BufSize) == 0) ? CTRUE : CFALSE);

		pSample->SetValid(CTRUE);
		//	Check Discontinuity for data validity
		if (Type == DATATYPE_VIDEO)
		{
			if (m_bVideoDis)
			{
				pSample->SetDiscontinuity(CTRUE);
				m_bVideoDis = CFALSE;
			}
			else
			{
				pSample->SetDiscontinuity(CFALSE);
			}
		}
		else
		{
			if (m_bAudioDis)
			{
				pSample->SetDiscontinuity(CTRUE);
				m_bAudioDis = CFALSE;
			}
			else
			{
				pSample->SetDiscontinuity(CFALSE);
			}
		}

		//
		//	Step 3. Push Sample to Output Queue
		//
		//printf(":%s:Line(%d) : pOut->m_pOutQueue->GetSampleCount = %d  ++ \n", __FILE__, __LINE__, pOut->m_pOutQueue->GetSampleCount());
		pOut->m_pOutQueue->PushSample(pSample);
		//printf(":%s:Line(%d) : pOut->m_pOutQueue->GetSampleCount = %d  -- \n", __FILE__, __LINE__, pOut->m_pOutQueue->GetSampleCount());
	}

	return CTRUE;
}

int64_t NX_CDemuxFFmpegFilter::GetFileLength()
{
	return -1;
}


CBOOL NX_CDemuxFFmpegFilter::PinInfo(NX_PinInfo *pPinInfo)
{
	pPinInfo->InPutNum = 0;		
	pPinInfo->OutPutNum = m_APinNum + m_VPinNum;
	return 0;
}

NX_CBasePin *NX_CDemuxFFmpegFilter::FindPin(int Pos)
{
	return GetPin(Pos);
}

CBOOL NX_CDemuxFFmpegFilter::FindInterface(const char *InterfaceId, void **Interface)
{
	if (!strcmp(InterfaceId, "IDemuxControl")){
		*Interface = (NX_IDemuxControl *)(this);
	}
//	else if (!strcmp(InterfaceId, "IMediaControl")){
//		*Interface = (NX_IMediaControl *)(this);
//	}

	return 0;
}

void NX_CDemuxFFmpegFilter::SetClockReference(NX_CClockRef *pClockRef)
{
	SetClockRef(pClockRef);
}

void NX_CDemuxFFmpegFilter::SetEventNotifi(NX_CEventNotifier *pEventNotifier)
{
	SetEventNotifier(pEventNotifier);
}

//
//////////////////////////////////////////////////////////////////////////////



#ifdef _DEBUG
//////////////////////////////////////////////////////////////////////////////
//
//								Debug Routine
//
void NX_CDemuxFFmpegFilter::DisplayVersion()
{
}
void NX_CDemuxFFmpegFilter::DisplayState()
{
}

void NX_CDemuxFFmpegFilter::GetStatistics(uint32_t *ReadCnt, uint32_t *ReaderLock, uint32_t *VidCnt, uint32_t *VidDis, uint32_t *AudCnt, uint32_t *AudDis)
{
	*ReaderLock = 0;
	*VidCnt = m_DbgVideoESCnt;
	*VidDis = m_DbgVideoDisCnt;
	*AudCnt = m_DbgAudioESCnt;
	*AudDis = m_DbgAudioDisCnt;
}

void NX_CDemuxFFmpegFilter::ClearStatistics()
{
	m_DbgVideoESCnt = 0;
	m_DbgAudioESCnt = 0;
	m_DbgVideoDisCnt = 0;
	m_DbgAudioDisCnt = 0;
}

//
//////////////////////////////////////////////////////////////////////////////
#endif


//////////////////////////////////////////////////////////////////////////////
//
//						Demux Output Pin Class
//

NX_CDemuxOutputPin::NX_CDemuxOutputPin(NX_CBaseFilter *pOwnerFilter, NX_OUT_PIN_TYPE pinType) :
	NX_CBaseOutputPin(pOwnerFilter),
	m_PinType(pinType),
	m_pInQueue(NULL),
	m_pOutQueue(NULL),
	m_pSampleList(NULL)
{
	if (OUT_PIN_TYPE_VIDEO == pinType)
	{		
		m_pInQueue = new NX_CSampleQueue(MAX_VIDEO_NUM_BUF);
		m_pOutQueue = new NX_CSampleQueue(MAX_VIDEO_NUM_BUF);
	}
	else
	{
		m_pInQueue = new NX_CSampleQueue(MAX_AUDIO_NUM_BUF);
		m_pOutQueue = new NX_CSampleQueue(MAX_AUDIO_NUM_BUF);
	}
	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}
	AllocBuffers();
}

NX_CDemuxOutputPin::~NX_CDemuxOutputPin()
{
	if (m_pInQueue)	delete m_pInQueue;
	if (m_pOutQueue)	delete m_pOutQueue;
	DeallocBuffers();
	pthread_mutex_destroy(&m_hMutexThread);
}

CBOOL NX_CDemuxOutputPin::Active()
{
	if (CFALSE == m_bActive)
	{
		m_bExitThread = CFALSE;
		//	Push Allocated Buffers

		m_pInQueue->ResetQueue();
		m_pOutQueue->ResetQueue();

		if (m_pInQueue){
			NX_CSample *pSample = m_pSampleList;
			while (1)
			{
				if (NULL == pSample)
					break;
				pSample->Reset();
				m_pInQueue->PushSample(pSample);
				pSample = pSample->m_pNext;
			}
		}
		//	

		if (pthread_create(&m_hThread, NULL, ThreadStub, this) < 0)
		{
			return CFALSE;
		}
		m_bActive = CTRUE;
	}
		
	return CTRUE;
	//return NX_CBaseOutputPin::Active();
}

CBOOL NX_CDemuxOutputPin::Inactive()
{
	//	Set Thread End Command
	ExitThread();
	m_pInQueue->EndQueue();
	m_pOutQueue->EndQueue();
	if (CTRUE == m_bActive)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] NX_CDemuxOutputPin Wait join ++\n");
		pthread_join(m_hThread, NULL);
		m_bActive = CFALSE;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] NX_CDemuxOutputPin Wait join --\n");
	}
	return CTRUE;
}

int32_t NX_CDemuxOutputPin::CheckMediaType(void *Mediatype)
{

	return 0;
}



int32_t NX_CDemuxOutputPin::ReleaseSample(NX_CSample *pSample)
{
	if (m_pInQueue)
	{
		return m_pInQueue->PushSample(pSample);
	}
	return 0;
}


int32_t NX_CDemuxOutputPin::GetDeliveryBuffer(NX_CSample **ppSample)
{
	*ppSample = NULL;
	//printf(":%s:Line(%d) : GetDeliveryBuffer: m_pOutQueue->PopSample = %d  ++ \n", __FILE__, __LINE__, m_pOutQueue->GetSampleCount());
	if (m_pOutQueue->PopSample(ppSample) < 0)
	{
		printf(":%s:Line(%d) : GetDeliveryBuffer: Not Sample \n", __FILE__, __LINE__);
		return -1;
	}
	//printf(":%s:Line(%d) : GetDeliveryBuffer: m_pOutQueue->PopSample = %d  -- \n", __FILE__, __LINE__, m_pOutQueue->GetSampleCount());
	if ((*ppSample) != NULL)
	{
		(*ppSample)->AddRef();
	}
	return 0;
}

void *NX_CDemuxOutputPin::ThreadStub(void *pObj)
{
	((NX_CDemuxOutputPin*)pObj)->ThreadProc();
	return (void*)0;
}

void NX_CDemuxOutputPin::ThreadProc()
{
	NX_CSample *pOutSample = NULL;
		
	while (1)	
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux]  NX_CDemuxOutputPin: ThreadProc Start ++ !!!\n");

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){ 
			NX_DbgMsg(DBG_MSG_ON, "[Filter|Demux]  m_bExitThread = %d !!\n", m_bExitThread);
			pthread_mutex_unlock(&m_hMutexThread);	
			break; 
		}
		pthread_mutex_unlock(&m_hMutexThread);

		pOutSample = NULL;

		if (GetDeliveryBuffer(&pOutSample) < 0){
			printf(":%s:Line(%d) : GetDeliveryBuffer Break \n", __FILE__, __LINE__);
			break;
		}

		//	Fill Buffer
		if (pOutSample)
		{
			//printf(":%s:Line(%d) ++ : pOutSample->Release(): pOutSample->GetRefCount()  (%d)\n", __FILE__, __LINE__, pOutSample->GetRefCount());
			if (Deliver(pOutSample) < 0){
				break;
			}
			pOutSample->Release();
		}

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux]  NX_CDemuxOutputPin :ThreadProc Start --!!!\n");
	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Demux] Exit output pin thread!!(%d)\n", m_PinType);
}


//
//		Allocate Media Sample & Buffer
//
int32_t	NX_CDemuxOutputPin::AllocBuffers()
{
	int i = 0;
	int32_t SizeBuf;
	int32_t NumBuf;
	uint32_t *pBuf;
	NX_CSample *pNewSample = NULL;
	NX_CSample *pOldSample = m_pSampleList;

	if (OUT_PIN_TYPE_VIDEO == m_PinType)
	{
		SizeBuf = MAX_VIDEO_SIZE_BUF;
		NumBuf = MAX_VIDEO_NUM_BUF;
	}
	else
	{
		SizeBuf = MAX_AUDIO_SIZE_BUF;
		NumBuf = MAX_AUDIO_NUM_BUF;
		NumBuf = 2;
	}

	//
	//	Step 1. Allocate First Media Sample
	//

	//	Make New Sample
	pOldSample = new NX_CSample(this);
	//	Allocate Buffer
	pBuf = new uint32_t[SizeBuf / sizeof(uint32_t)];
	//	Link Sample to Buffer
	pOldSample->SetBuffer(pBuf, SizeBuf, sizeof(uint32_t));
	m_pSampleList = pOldSample;

	//	STep 2. Allocate Other Media Samples
	for (i = 1; i<NumBuf; i++)
	{
		//	Make New Sample
		pNewSample = new NX_CSample(this);
		//	Allocate Buffer
		pBuf = new uint32_t[SizeBuf / sizeof(uint32_t)];
		//	Link Sample to Buffer
		pNewSample->SetBuffer(pBuf, SizeBuf, sizeof(uint32_t));
		//	Make List
		pOldSample->m_pNext = pNewSample;
		pOldSample = pNewSample;
	}
	NX_DbgMsg(DBG_TRACE, "[Filter|Demux] Allocated Buffers = %d,%d\n", m_PinType, i);
	return 0;
}

int32_t	NX_CDemuxOutputPin::DeallocBuffers()
{
	int k = 0;
	while (1)
	{
		if (NULL == m_pSampleList)
			break;
		NX_CSample *TmpSampleList = m_pSampleList;
		m_pSampleList = m_pSampleList->m_pNext;
		delete TmpSampleList;
		k++;
	}
	NX_DbgMsg(DBG_TRACE, "[Filter|Demux] Deallocated Buffers = %d,%d\n", m_PinType, k);
	return 0;
}

//
//////////////////////////////////////////////////////////////////////////////

NX_IBaseFilter *CreateDemuxFilter(const char *filterName)
{
	return new NX_CDemuxFFmpegFilter(filterName);
}
