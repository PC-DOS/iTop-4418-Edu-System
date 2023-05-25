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
#ifndef __NX_CAudioRenderFilter_h__
#define __NX_CAudioRenderFilter_h__

#include <alsa/asoundlib.h>
#include "NX_MediaType.h"

#ifdef __cplusplus

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"

#include <nx_audio.h>

//#define		MAX_NUM_WAVE_OUT_BUF_AR		30
#define		MAX_NUM_WAVE_OUT_BUF_AR		3

enum {
	A_SAMPLE_FMT_NONE = -1,
	A_SAMPLE_FMT_U8,          ///< unsigned 8 bits
	A_SAMPLE_FMT_S16,         ///< signed 16 bits
	A_SAMPLE_FMT_S32,         ///< signed 32 bits
	A_SAMPLE_FMT_FLT,         ///< float
	A_SAMPLE_FMT_DBL,         ///< double
	A_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};


class NX_CWaveOut;
class NX_CAudioRenderInputPin;

NX_IBaseFilter *CreateAudioRenderFilter(const char *filterName);

//////////////////////////////////////////////////////////////////////////////
//
//					NX_CTimeGapQueue : does not suport thread safe.
//
#define	MAX_TIME_GAP_QUEUE	42
class NX_CTimeGapQueue
{
public:
	NX_CTimeGapQueue():
		m_TotalSize(0),
		m_WritePos (0),
		m_ReadPos  (0),
		m_CurSize  (0)
	{
	}
	virtual ~NX_CTimeGapQueue(){}

	void	Push( int32_t Gap ){
		m_TotalSize += Gap;
		m_Queue[m_WritePos] = Gap;
		m_WritePos = (m_WritePos+1) % MAX_TIME_GAP_QUEUE;
		m_CurSize ++;
	}
	void	Pop(){
		m_TotalSize -= m_Queue[m_ReadPos];
		m_ReadPos = (m_ReadPos+1) % MAX_TIME_GAP_QUEUE;
		m_CurSize --;
	}
	int32_t		GetCurSize(){
		return m_CurSize;
	}
	void	Flush(){
		m_TotalSize = m_WritePos = m_ReadPos = m_CurSize = 0;
	}
	int32_t		GetAverage(){ return m_TotalSize/MAX_TIME_GAP_QUEUE; }

private:
	int32_t		m_TotalSize;
	int32_t		m_WritePos;
	int32_t		m_ReadPos;
	int32_t		m_CurSize;
	int32_t		m_Queue[MAX_TIME_GAP_QUEUE];		

private:
	NX_CTimeGapQueue (NX_CTimeGapQueue &Ref);
	NX_CTimeGapQueue &operator=(NX_CTimeGapQueue &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Audio Renderer Filter
//
class NX_CAudioRenderFilter : 
	public NX_CBaseFilter, public NX_IBaseFilter
{
public:
	NX_CAudioRenderFilter(const char *filterName);
	virtual ~NX_CAudioRenderFilter();

public:
	int32_t			RenderSample( NX_CSample *pSample );

public:	//	Wave PCM Write Thread
	pthread_t			m_hThread;
	pthread_mutex_t		m_hMutexThread;
	CBOOL				m_bExitThread;
	CBOOL				m_bPaused;
	static void			*ThreadStub( void *pObj );
	void				ThreadProc();
	void ExitThread(){
		pthread_mutex_lock(&m_hMutexThread);
		m_bExitThread = CTRUE;
		pthread_mutex_unlock(&m_hMutexThread);
	}
	
	//	Override BaseFilter
	virtual CBOOL		Run();
	virtual CBOOL		Stop();
	virtual	NX_CBasePin	*GetPin(int Pos);
	//	
	void				Resume(void);

	//IBaseFilter Pure Virtual
	virtual	CBOOL		PinInfo(NX_PinInfo *PinInfo);
	virtual	NX_CBasePin	*FindPin(int Pos);
	virtual	CBOOL		FindInterface(const char *InterfaceId, void **Interface);
	virtual	void		SetClockReference(NX_CClockRef *pClockRef);
	virtual	void		SetEventNotifi(NX_CEventNotifier *pEventNotifier);
	virtual	CBOOL		Pause();
	//
	int32_t				SetMediaType(void *Mediatype);
	CBOOL				Flush();
	//

	NX_AudioMediaType		m_AMediaType;
	NX_CAudioRenderInputPin	*m_pInputPin;
	NX_CSampleQueue			*m_pInQueue;
	NX_CWaveOut				*m_pWaveOut;
	NX_AUDIO_HANDLE			m_hAudio;
	int32_t					m_Hdmi;

	//	Audio Buffers
	int32_t					m_NumAudioBuffer;
	int32_t					m_SizeAudioBuffer;
	int32_t					m_SamplingFrequency;
	int32_t					m_NumChannels;
	int32_t					m_FormatType;

	int64_t					m_PrevEndTime;
	int64_t					m_RefTime;
	CBOOL					m_ClockRefreshCnt;
	CBOOL					m_Discontinuity;
	int32_t					m_MaxSleepTime;
	NX_CTimeGapQueue		m_TimeGapQueue;
	void					ChangeBufferingTime( uint32_t BufferingTime );
	NX_CSemaphore			m_SemPause;

#ifdef _DEBUG
	/////////////////////////////////////////////////////////////////////////////
	//
	//						Debug Informations
	//
public:
	void DisplayVersion();
	void DisplayState();
	void GetStatistics(uint32_t *InPcmFrames, uint32_t *OutPcmFrames, uint32_t *DropPcmFrames, uint32_t *State);
	void ClearStatistics();
	uint32_t		m_DbgInFrameCnt;
	uint32_t		m_DbgOutFrameCnt;
	uint32_t		m_DbgDropFrameCnt;
	//
	/////////////////////////////////////////////////////////////////////////////
#endif


private:
	NX_CAudioRenderFilter (NX_CAudioRenderFilter &Ref);
	NX_CAudioRenderFilter &operator=(NX_CAudioRenderFilter &Ref);
};

//
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Audio Renderer Input Pin
//

class NX_CAudioRenderInputPin: 
	public NX_CBaseInputPin
{
public:
	NX_CAudioRenderInputPin( NX_CAudioRenderFilter *pOwnerFilter );
	virtual ~NX_CAudioRenderInputPin();

protected:
	virtual int32_t Receive( NX_CSample *pSample );
	virtual int32_t	CheckMediaType(void *Mediatype);
};

//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Alsa WaveOut Module
//
class NX_CWaveOut{
public:
	NX_CWaveOut();
	virtual ~NX_CWaveOut();

public:
	int32_t				WaveOutOpen(uint32_t SampleFreq, uint32_t Channeles, int32_t FormatType, uint32_t NumBuf, uint32_t SizeBuf, int32_t m_Hdmi);
	int32_t				WaveOutPlayBuffer( uint32_t *pBuf, int32_t Size, int32_t &Error );
	int32_t				WaveOutReset();							//	Stop Play & Reset Buffer
	void				WaveOutClose();
	void				WaveOutPrepare();
	int32_t				WaveOutCheckAvailable();
	int64_t				GetBufferedTime();
	void				WriteDummy( uint32_t Time );					//	Milli-Seconds
	int64_t				FindDuration( uint32_t Size );
private:
	uint8_t					m_Dummy[1024*4];
	snd_pcm_t*				m_hPlayback;
	snd_pcm_hw_params_t*	m_pHWParam;
	snd_pcm_status_t*		m_pStatus;
	uint32_t				m_Periods;
	uint32_t				m_PeriodBufSize;
	uint32_t				m_SamplingFreq;
	float					m_TimePerSample;
	int32_t					m_Channels;
	snd_pcm_uframes_t		m_MaxAvailBuffer;
};
//
//
//////////////////////////////////////////////////////////////////////////////


#endif	//	__cplusplus

#endif	//	__NX_CAudioRenderFilter_h__
