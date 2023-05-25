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
#ifndef __NX_CAudioConvertFFmpegFilter_h__
#define __NX_CAudioConvertFFmpegFilter_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"
#include "NX_IAudioControl.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C"{
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

class NX_CAudioConvertInputPin;
class NX_CAudioConvertOutputPin;

NX_IBaseFilter *CreateAudioConvertFilter(const char *filterName);

struct AVAudioConvert;
typedef struct AVAudioConvert AVAudioConvert;

typedef struct _ReSampleStruct{
	int32_t output_channels;
	int32_t input_channels;
	int32_t	output_rate;
	int32_t	input_rate;
	enum AVSampleFormat	output_sample_fmt;
	enum AVSampleFormat	input_sample_fmt;
	int32_t	filter_length;
	int32_t log2_phase_count;
	int32_t linear;
	double cutoff;
} ReSampleStruct;
//////////////////////////////////////////////////////////////////////////////
//
//						Nexell AudioConvert Filter
class NX_CAudioConvertFFmpegFilter :
	public NX_CBaseFilter, public NX_IBaseFilter, public NX_IAudioControl
{
public:
	NX_CAudioConvertFFmpegFilter(const char *filterName);
	virtual ~NX_CAudioConvertFFmpegFilter();
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
	virtual CBOOL			Run();
	virtual CBOOL			Stop();
	virtual CBOOL			Flush();
	virtual	NX_CBasePin		*GetPin(int Pos);
	//
	CBOOL					SetSWVolume(int16_t *pOutBuf, int32_t OutSamples);
	//IAudioControlControl
	virtual CBOOL			SetVolume(int32_t Volume);
	int32_t					TransFormSample(NX_CSample *pSample);
	int32_t					GetBitsPerSampleFormat(enum SampleFormat sample_fmt);
	//ffmpeg AudioConvert API
	AVAudioConvert			*av_audio_convert_alloc(enum AVSampleFormat out_fmt, int out_channels,
													enum AVSampleFormat in_fmt, int in_channels,
													const float *matrix, int flags);
	void					av_audio_convert_free(AVAudioConvert *ctx);
	int						av_audio_convert(AVAudioConvert *ctx,
											 void * const out[6], const int out_stride[6],
											 const void * const  in[6], const int  in_stride[6], int len);

	NX_CAudioConvertInputPin	*m_pInputPin;
	NX_CAudioConvertOutputPin	*m_pOutputPin;	
	NX_AudioMediaType			m_AMediaType;
	//ffmpeg AudioConvert API
	AVAudioConvert				*m_ctx;
	ReSampleContext				*m_pReSampleCtx;
	ReSampleStruct				m_ResampleSt;
	int32_t						m_RequestVolume;
	int32_t						m_CurrentVolume;
	//

#ifdef _DEBUG
	/////////////////////////////////////////////////////////////////////////////
	//
	//						Debug Informations
	//
public:
	void DisplayVersion();
	void DisplayState();
	void GetStatistics( uint32_t *AudInCnt, uint32_t *AudOutCnt );
	void ClearStatistics();
	uint32_t		m_DbgInFrameCnt;
	uint32_t		m_DbgOutFrameCnt;
	//



	/////////////////////////////////////////////////////////////////////////////
#endif

private: 
	NX_CAudioConvertFFmpegFilter(NX_CAudioConvertFFmpegFilter &Ref);
	NX_CAudioConvertFFmpegFilter &operator=(NX_CAudioConvertFFmpegFilter &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//						Nexell AudioConvert Input Pin
class NX_CAudioConvertInputPin :
	public NX_CBaseInputPin
{
public:
	NX_CAudioConvertInputPin(NX_CAudioConvertFFmpegFilter *pOwner);
	virtual ~NX_CAudioConvertInputPin(){}

	virtual int32_t Receive( NX_CSample *pInSample );
	virtual int32_t	CheckMediaType(void *Mediatype);
private:
	NX_CAudioConvertInputPin(NX_CAudioConvertInputPin &Ref);
	NX_CAudioConvertInputPin &operator=(NX_CAudioConvertInputPin &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell AudioConvert Output Pin
class NX_CAudioConvertOutputPin :
	public NX_CBaseOutputPin
{
public:
	NX_CAudioConvertOutputPin(NX_CAudioConvertFFmpegFilter *pOwner);
	virtual ~NX_CAudioConvertOutputPin();

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
	NX_CAudioConvertOutputPin(NX_CAudioConvertOutputPin &Ref);
	NX_CAudioConvertOutputPin &operator=(NX_CAudioConvertOutputPin &Ref);
};

//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__cplusplus

#endif	//	__NX_CAudioConvertFFmpegFilter_h__
