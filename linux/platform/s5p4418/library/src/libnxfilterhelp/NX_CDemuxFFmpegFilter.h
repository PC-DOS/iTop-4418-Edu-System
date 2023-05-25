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
#ifndef __NX_CDemuxFFmpegFilter_h__
#define __NX_CDemuxFFmpegFilter_h__

#ifdef __cplusplus

#include "NX_MediaType.h"
#include "NX_TypeDefind.h"

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"
#include "NX_IDemuxControl.h"

#include <nx_fourcc.h>
#include <vpu_types.h>
#include <nx_video_api.h>
#include <nx_alloc_mem.h>

#ifdef __cplusplus
extern "C"{
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

#if 0
for (i = 0; i < DEMUX_MEDIA_MAX; i++)
{
	m_DemuxMediaInfo.VideoInfo[i].iMp4Class = -1;
	m_DemuxMediaInfo.VideoInfo[i].iVideoCodecID = -1;
	m_DemuxMediaInfo.VideoInfo[i].iVpuCodecType = -1;
	m_DemuxMediaInfo.VideoInfo[i].VideoTrackInfo.VideoPin = -1;
	m_DemuxMediaInfo.VideoInfo[i].VideoTrackInfo.StreamID = -1;
	m_DemuxMediaInfo.AudioInfo[i].AudioTrackInfo.AudioPin = -1;
	m_DemuxMediaInfo.AudioInfo[i].AudioTrackInfo.StreamID = -1;
}

#endif

#define	ENABLE_THEORA		1

#if	(ENABLE_THEORA)
#include "theoraparser/include/theora_parser.h"
//Theora specific display information
typedef struct {
	int frameWidth;
	int frameHeight;
	int picWidth;
	int picHeight;
	int picOffsetX;
	int picOffsetY;
} ThoScaleInfo;
#endif


#define	RCV_V2

#define	NX_MAX_NUM_SPS		3
#define	NX_MAX_SPS_SIZE		1024
#define	NX_MAX_NUM_PPS		3
#define	NX_MAX_PPS_SIZE		1024
typedef struct {
	int				version;
	int				profile_indication;
	int				compatible_profile;
	int				level_indication;
	int				nal_length_size;
	int				num_sps;
	int				sps_length[NX_MAX_NUM_SPS];
	unsigned char	sps_data[NX_MAX_NUM_SPS][NX_MAX_SPS_SIZE];
	int				num_pps;
	int				pps_length[NX_MAX_NUM_PPS];
	unsigned char	pps_data[NX_MAX_NUM_PPS][NX_MAX_PPS_SIZE];
} NX_AVCC_TYPE;


#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif

#define PUT_LE32(_p, _var) \
	*_p++ = (unsigned char)((_var) >> 0);  \
	*_p++ = (unsigned char)((_var) >> 8);  \
	*_p++ = (unsigned char)((_var) >> 16); \
	*_p++ = (unsigned char)((_var) >> 24);

#define PUT_BE32(_p, _var) \
	*_p++ = (unsigned char)((_var) >> 24);  \
	*_p++ = (unsigned char)((_var) >> 16);  \
	*_p++ = (unsigned char)((_var) >> 8); \
	*_p++ = (unsigned char)((_var) >> 0);

#define PUT_LE16(_p, _var) \
	*_p++ = (unsigned char)((_var) >> 0);  \
	*_p++ = (unsigned char)((_var) >> 8);

#define PUT_BE16(_p, _var) \
	*_p++ = (unsigned char)((_var) >> 8);  \
	*_p++ = (unsigned char)((_var) >> 0);

#include <pthread.h>

#define		MAX_AUDIO_NUM_BUF		42		//	NX_CSampleQueue의 Max Size가 32로 되어 있기 때문에 이 값을 증가 시킬 때는 반드시
#define		MAX_VIDEO_NUM_BUF		30		//	NX_CSampleQueue class의 SAMPLE_QUEUE_COUNT 값보다 작은 값을 사용하여야 한다.

#define		MAX_VIDEO_SIZE_BUF		(3*1024*1024)		//	3MB
#define		MAX_AUDIO_SIZE_BUF		(AVCODEC_MAX_AUDIO_FRAME_SIZE)		


//#define		NX_DEMUX_IN_BUF_SIZE	(1024*128)
#define		NX_DEMUX_IN_BUF_SIZE	(1024*1024*4)
#define		NX_DEMUX_FILE_READ_SIZE	(128*1024)
#define		NX_MAX_PATH				(1024)

#define		NX_MAX_PIN				(5)	
#define		NX_MAX_STREAM			(5)	

#define		DATATYPE_VIDEO	0
#define		DATATYPE_AUDIO	1
#define		DATATYPE_USER	2

//
typedef struct _DemuxVideoTrack {
	int32_t		VideoPin;
	int32_t		StreamIDX;
} DemuxVideoTrack;
typedef struct _DemuxAudioTrack {
	int32_t		AudioPin;
	int32_t		StreamIDX;
} DemuxAudioTrack;
typedef struct _DemuxAudioInfo {
	DemuxAudioTrack	AudioTrackInfo;
	CodecID			AudioCodecID;
	int32_t			AudioCodecName;
	int32_t			iSampleRate;
	int32_t			iChannels;
	int32_t			iSampleFmt;
	int32_t			iBitrate;
	int32_t			iBlockAlign;
	AVStream		*pAudioStream;
	int32_t			m_SeqSize;
	uint8_t			*m_SeqData;
} DemuxAudioInfo;
typedef struct _DemuxVideoInfo {
	DemuxVideoTrack	VideoTrackInfo;
	int32_t			iVideoCodecID;
	int32_t			iVideoCodecName;
	int32_t			iVpuCodecType;
	int32_t			iMp4Class;
	int32_t			iFrameRate;
	int32_t			iWidth;
	int32_t			iHeight;
	AVStream		*pVideoStream;
	int32_t			m_SeqSize;
	uint8_t			*m_SeqData;
} DemuxVideoInfo;

#define DEMUX_MEDIA_MAX		5
typedef struct _DemuxMediaInfo{
	int32_t			AudioTrackTotNum;
	DemuxAudioInfo	AudioInfo[DEMUX_MEDIA_MAX];
	int32_t			VideoTrackTotNum;
	DemuxVideoInfo	VideoInfo[DEMUX_MEDIA_MAX];
	int32_t			DataTrackTotNum;
	int64_t			Duration;
} DemuxMediaInfo;
//

class NX_CDemuxOutputPin;

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell Demxer Filter

NX_IBaseFilter *CreateDemuxFilter(const char *filterName);


#define DEMUX_HANDLE void*

class NX_CDemuxFFmpegFilter :
	public NX_CBaseFilter, public NX_IBaseFilter, public NX_IDemuxControl
{
public:
	NX_CDemuxFFmpegFilter(const char *filterName);
	virtual ~NX_CDemuxFFmpegFilter();

public:
	//IBaseFilter Pure Virtual
	virtual CBOOL		PinInfo(NX_PinInfo *PinInfo);
	virtual CBOOL		FindInterface(const char *InterfaceId, void **Interface);
	virtual NX_CBasePin	*FindPin(int Pos);
	virtual void		SetClockReference(NX_CClockRef *pClockRef);
	virtual void		SetEventNotifi(NX_CEventNotifier *pEventNotifier);
	virtual CBOOL		Pause(void);
	virtual int32_t		GetMediaInfo(MEDIA_INFO * pMediaInfo);
	//	overide NX_CBaseFilter
	virtual CBOOL		Run(void);
	virtual CBOOL		Stop(void);
	virtual NX_CBasePin	*GetPin(int Pos);
	//IDemuxControl 
	virtual int32_t		SetFileName(const char* pBuf);
	virtual CBOOL		PinCreate();
	virtual CBOOL		SelectPin(int32_t VPin, int32_t Apin);
	virtual CBOOL		PinInfo(PinType *pininfo);
	virtual CBOOL		GetMediaType(void *MediaType, int32_t Type);
	virtual CBOOL		SeekStream(int64_t seekTime);

	typedef enum { DEMUX_MODE_LIVE, DEMUX_MODE_FILE } DEMUXER_MODE;

	//	Thread
	pthread_t			m_hThread;
	CBOOL				m_bExitThread;
	pthread_mutex_t		m_hMutexThread;

	static void *ThreadStub(void *pObj);
	void ThreadProcFile();
	void ExitThread(){
		pthread_mutex_lock(&m_hMutexThread);
		m_bExitThread = CTRUE;
		pthread_mutex_unlock(&m_hMutexThread);
	}
	void DeliverEndOfStream(void);
	int32_t				FFmpegOpen();
	CBOOL				VideoPacketParsing(AVPacket *pkt, int32_t *Size, int32_t *IsKey, uint8_t *buffer);
	CBOOL				AudioPacketParsing(AVPacket *pkt, int32_t *Size, uint8_t *buffer);
	int32_t				ScanFile(void);
	CBOOL				Flush();
	
	DEMUX_HANDLE		m_hTSDemux;
	NX_CDemuxOutputPin	*m_pAudioOutPin;	
	NX_CDemuxOutputPin	*m_pVideoOutPin;	
	NX_CDemuxOutputPin	*m_pAudioOut[NX_MAX_PIN];				
	NX_CDemuxOutputPin	*m_pVideoOut[NX_MAX_PIN];		
	MEDIA_INFO			m_MediaInfo;	
	int64_t				m_TimeStamp;
	int32_t				m_APinNum;
	int32_t				m_VPinNum;
	int32_t				m_VPinSelect;
	int32_t				m_VpuCodecType[NX_MAX_STREAM];
	int32_t				m_Mp4Class[NX_MAX_STREAM];
	int32_t				m_Width[NX_MAX_STREAM];
	int32_t				m_Height[NX_MAX_STREAM];
	int32_t				m_VideoCodecID[NX_MAX_STREAM];
	AVStream			*m_VideoStream[NX_MAX_STREAM];
	int32_t				m_VideoStreamIdx[NX_MAX_STREAM];
	int32_t				m_APinSelect;
	CodecID				m_AudioCodecID[NX_MAX_STREAM];
	int32_t				m_SampleRate[NX_MAX_STREAM];
	int32_t				m_Channels[NX_MAX_STREAM];
	int32_t				m_SampleFmt[NX_MAX_STREAM];	
	AVStream			*m_AudioStream[NX_MAX_STREAM];
	int32_t				m_AudioStreamIdx[NX_MAX_STREAM];
	int32_t				m_SeqSize;
	NX_AVCC_TYPE		m_H264AvcCHeader;		//	for h.264
	unsigned char		m_SeqData[1024 * 4];
	NX_VideoMediaType	m_VMediaType;
	NX_AudioMediaType	m_AMediaType;
	PinType				m_SetPin;
	CBOOL				m_bAudioDis;
	CBOOL				m_bVideoDis;
	//FFMPEG
	AVFormatContext		*m_pFmtCtx;
	AVCodecContext		*m_pVideoDecCtx;
	AVCodecContext		*m_pAudioDecCtx;
	AVStream			*m_pVideoStream;
	AVStream			*m_pAudioStream;
	AVPacket			m_Pkt;
#if	(ENABLE_THEORA)
	//	for theora
	tho_parser_t*		m_TheoraParser;
#endif
	//
	//	Stream Information	//
	int64_t				m_StreamStartTime;
	int64_t				m_StreamEndTime;
	int64_t				m_StreamDuration;
	int64_t				m_DeltaTime;
	CBOOL				m_bFindNext;
	CBOOL				m_FoundKey;
	int32_t				m_Init;
	uint32_t			m_pInBufPos;
	uint8_t				m_pInBuf[NX_DEMUX_IN_BUF_SIZE];
	//	Working Mode
	DEMUXER_MODE		m_DemuxMode;
	CBOOL				m_bSetFirstTime;
	//	Time Stamp Module
	int64_t				m_PrevTimeStamp;
	int64_t				m_TimeOffset;
	//	Streaming Param
	CBOOL				m_bExternalForcedResync;
	//	Buffering Time
	int64_t				m_BufferingTime;
	int64_t				m_MaxBufferingTime;
	char				m_FileName[NX_MAX_PATH];
	DemuxMediaInfo		m_DemuxMediaInfo;
	/////////////////////////////////////////////////////////////////////////////
	//
	//						File Operation
	//
public:
	CBOOL		ReadStream(uint8_t *pBuf, int32_t readSize, int32_t *actualReadSize);
	int64_t		GetFileLength(void);
	void		dumpdata(void *data, int len, const char *msg);
	int			fourCCToMp4Class(unsigned int fourcc);
	int			fourCCToCodStd(unsigned int fourcc);
	int			codecIdToMp4Class(int codec_id);
	int			codecIdToCodStd(int codec_id);
	int			codecIdToFourcc(int codec_id);
	int			GetSequenceInformation(NX_AVCC_TYPE *h264Hader, AVStream *stream, unsigned char *buffer, int size);
	int			PasreAVCStream(AVPacket *pkt, int nalLengthSize, unsigned char *buffer, int outBufSize);
	int			MakeRvStream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize);
	int			MakeVC1Stream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize);
	int			MakeDIVX3Stream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize);
	int			MakeVP8Stream(AVPacket *pkt, AVStream *stream, unsigned char *buffer, int outBufSize);
	int			NX_ParseSpsPpsFromAVCC(unsigned char *extraData, int extraDataSize, NX_AVCC_TYPE *avcCInfo);
	void		NX_MakeH264StreamAVCCtoANNEXB(NX_AVCC_TYPE *avcc, unsigned char *pBuf, int *size);
	int			CodecIdToVpuType(int codecId, unsigned int fourcc);


	//
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	//
	//						Events
	//
public:
	void					RequestResetTimeBase(){ m_bExternalForcedResync = CTRUE; }
	//
	/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	/////////////////////////////////////////////////////////////////////////////
	//
	//						Debug Informations
	//
public:
	void DisplayVersion();
	void DisplayState();
	void GetStatistics(uint32_t *ReadCnt, uint32_t *ReaderLock, uint32_t *VidCnt, uint32_t *VidDis, uint32_t *AudCnt, uint32_t *AudDis);
	void ClearStatistics();
	uint32_t			m_DbgVideoESCnt;		//	Parsed Video Elementary Stream
	uint32_t			m_DbgAudioESCnt;		//	Parsed Audio Elementary Stream
	uint32_t			m_DbgVideoDisCnt;		//	Video Stream's Discontinuity Counter
	uint32_t			m_DbgAudioDisCnt;		//	Audio Stream's Discontinuity Counter
	uint32_t			m_DbgPmtPid;
	//
	/////////////////////////////////////////////////////////////////////////////
#endif

private:
	NX_CDemuxFFmpegFilter(NX_CDemuxFFmpegFilter &Ref);
	NX_CDemuxFFmpegFilter &operator=(NX_CDemuxFFmpegFilter &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//						Parser Output Pin Class
//
typedef enum{ OUT_PIN_TYPE_VIDEO, OUT_PIN_TYPE_AUDIO } NX_OUT_PIN_TYPE;
class NX_CDemuxOutputPin :
	public NX_CBaseOutputPin
{
public:
	NX_CDemuxOutputPin(NX_CBaseFilter *pOwnerFilter, NX_OUT_PIN_TYPE PinType);
	virtual ~NX_CDemuxOutputPin();

	//	Implementation Pure Virtual Functions
public:
	virtual int32_t		ReleaseSample(NX_CSample *pSample);
	virtual int32_t		GetDeliveryBuffer(NX_CSample **pSample);
	virtual int32_t		CheckMediaType(void *Mediatype);

public:
	//	Stream Pumping Thread
	pthread_t		m_hThread;
	CBOOL			m_bExitThread;
	pthread_mutex_t	m_hMutexThread;
	static void		*ThreadStub(void *pObj);
	void ThreadProc();
	void ExitThread(){
		pthread_mutex_lock(&m_hMutexThread);
		m_bExitThread = CTRUE;
		pthread_mutex_unlock(&m_hMutexThread);
	}

	//	Override
	CBOOL			Active();
	CBOOL			Inactive();
	//	Alloc Buffer
	int32_t			AllocBuffers();
	int32_t			DeallocBuffers();
	NX_OUT_PIN_TYPE	m_PinType;
	//	Media Sample Queue
	NX_CSampleQueue	*m_pInQueue;		//	Input Free Queue
	NX_CSampleQueue	*m_pOutQueue;		//	Output Queue
	NX_CSample		*m_pSampleList;

private:
	NX_CDemuxOutputPin(NX_CDemuxOutputPin &Ref);
	NX_CDemuxOutputPin &operator=(NX_CDemuxOutputPin &Ref);
};

//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__cplusplus

#endif	//	__NX_CDemuxFFmpegFilter_h__
