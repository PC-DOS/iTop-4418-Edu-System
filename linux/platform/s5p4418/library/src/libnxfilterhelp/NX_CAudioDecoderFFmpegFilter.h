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
#ifndef __NX_CAudioDecoderFilter_h__
#define __NX_CAudioDecoderFilter_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"


#ifdef __cplusplus
extern "C"{
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/opt.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

#include <pthread.h>

#define		MAX_NUM_WAVE_OUT_BUF		30
//#define		MAX_NUM_WAVE_OUT_BUF		2
#define		MAX_SIZE_WAVE_OUT_BUF		AVCODEC_MAX_AUDIO_FRAME_SIZE

class NX_CAudioDecoderInputPin;
class NX_CAudioDecoderOutputPin;

NX_IBaseFilter *CreateAudioDecoderFilter(const char *filterName);

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell AudioDecoder Filter
class NX_CAudioDecoderFFmpegFilter :
	public NX_CBaseFilter, public NX_IBaseFilter
{
public:
	NX_CAudioDecoderFFmpegFilter(const char *filterName);
	virtual ~NX_CAudioDecoderFFmpegFilter();
public:

	pthread_t			m_hThread;
	pthread_mutex_t		m_hMutexThread;
	CBOOL				m_bExitThread;
	CBOOL				m_bPaused;
	static void			*ThreadStub(void *pObj);
	void				ThreadProc();
	void ExitThread(){
		pthread_mutex_lock(&m_hMutexThread);
		m_bExitThread = CTRUE;
		pthread_mutex_unlock(&m_hMutexThread);
	}
	NX_CSampleQueue		*m_pInQueue;

	int32_t	DecoderSample(NX_CSample *pSample);

	//IBaseFilter Pure Virtual
	virtual CBOOL			PinInfo(NX_PinInfo *PinInfo);
	virtual NX_CBasePin		*FindPin(int Pos);
	virtual CBOOL			FindInterface(const char *InterfaceId, void **Interface);
	virtual void			SetClockReference(NX_CClockRef *pClockRef);
	virtual void			SetEventNotifi(NX_CEventNotifier *pEventNotifier);
	virtual CBOOL			Pause();
	//
	int32_t					SetMediaType(void *Mediatype);
	//	Override BaseFilter
	virtual	CBOOL			Run();
	virtual	CBOOL			Stop();
	virtual CBOOL			Flush();
	virtual	NX_CBasePin		*GetPin(int Pos);
	//
	int32_t GetBitsPerSampleFormat(enum SampleFormat sample_fmt);
	void AudioClose();

	NX_CAudioDecoderInputPin	*m_pInputPin;
	NX_CAudioDecoderOutputPin	*m_pOutputPin;
	CBOOL						m_Discontinuity;
	//	FFMPEG codec context
	int32_t						m_Init;
	CodecID						m_CodecID;
	AVCodec						*m_hAudioCodec;
	AVCodecContext				*m_Avctx;
	NX_AudioMediaType			m_AMediaType;
	int32_t						m_Samplerate;
	int32_t						m_channels;
	int32_t						m_SampleFmt;
	int32_t						m_bitrate;
	unsigned char				*m_ExtData;
	int32_t						m_ExtSize;
	
#ifdef _DEBUG
	/////////////////////////////////////////////////////////////////////////////
	//
	//						Debug Informations
	//
public:
	void DisplayVersion();
	void DisplayState();
	void GetStatistics( U32 *AudInCnt, U32 *AudOutCnt );
	void ClearStatistics();
	U32		m_DbgInFrameCnt;
	U32		m_DbgOutFrameCnt;
	//



	/////////////////////////////////////////////////////////////////////////////
#endif

private:
	NX_CAudioDecoderFFmpegFilter(NX_CAudioDecoderFFmpegFilter &Ref);
	NX_CAudioDecoderFFmpegFilter &operator=(NX_CAudioDecoderFFmpegFilter &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//						Nexell AudioDecoder Input Pin
class NX_CAudioDecoderInputPin :
	public NX_CBaseInputPin
{
public:
	NX_CAudioDecoderInputPin(NX_CAudioDecoderFFmpegFilter *pOwner);
	virtual ~NX_CAudioDecoderInputPin(){}

	virtual int32_t Receive( NX_CSample *pInSample );
	virtual int32_t	CheckMediaType(void *Mediatype);
private:
	NX_CAudioDecoderInputPin(NX_CAudioDecoderInputPin &Ref);
	NX_CAudioDecoderInputPin &operator=(NX_CAudioDecoderInputPin &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell AudioDecoder Output Pin
class NX_CAudioDecoderOutputPin :
	public NX_CBaseOutputPin
{
public:
	NX_CAudioDecoderOutputPin(NX_CAudioDecoderFFmpegFilter *pOwner);
	virtual ~NX_CAudioDecoderOutputPin();

	//	Implementation Pure Virtual Functions
public:
	virtual int32_t		ReleaseSample( NX_CSample *pSample );
	virtual int32_t		GetDeliveryBuffer(NX_CSample **pSample);
	virtual int32_t		CheckMediaType(void *Mediatype);
	virtual CBOOL		Active();
	virtual CBOOL		Inactive();
	//	Alloc Buffer
	int32_t				AllocBuffers();
	int32_t				DeallocBuffers();
	//	Media Sample Queue
	NX_CSampleQueue		*m_pSampleQ;
	NX_CSample			*m_pSampleList;
private:
	NX_CAudioDecoderOutputPin(NX_CAudioDecoderOutputPin &Ref);
	NX_CAudioDecoderOutputPin &operator=(NX_CAudioDecoderOutputPin &Ref);
};

//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__cplusplus

#endif	//	__NX_CAudioDecoderFilter_h__
