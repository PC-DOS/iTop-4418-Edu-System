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
#ifndef __NX_CVideoDecoderFilter_h__
#define __NX_CVideoDecoderFilter_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"
//#include "NX_IMediaControl.h"

#include <nx_fourcc.h>
#include <vpu_types.h>
#include <nx_video_api.h>
#include <nx_alloc_mem.h>


#include <pthread.h>

//	Config Media Sample Buffer
#define		MAX_NUM_VIDEO_BUF			(10)						
//#define		MAX_NUM_VIDEO_BUF			(30)					
#define		MIN_NUM_VIDEO_BUF			(5)						
#define		MAX_NUM_VIDEO_OUT_BUF		(3)

//	Output Timestamp
#define	NX_MAX_BUF				32
struct OutBufferTimeInfo{
	int64_t 			timestamp;
	uint32_t			flag;
};
//



class NX_CVideoDecoderInputPin;
class NX_CVideoDecoderOutputPin;

NX_IBaseFilter *CreateVideoDecoderFilter(const char *filterName);

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell VideoDecoder Filter

#define	MAX_INPUT_BUF_SIZE		(1024*1024*3)
class NX_CVideoDecoderFilter :
	public NX_CBaseFilter, public NX_IBaseFilter
{
public:
	NX_CVideoDecoderFilter(const char *filterName);
	virtual ~NX_CVideoDecoderFilter();
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
	virtual CBOOL		PinInfo(NX_PinInfo *PinInfo);
	virtual NX_CBasePin	*FindPin(int Pos);
	virtual CBOOL		FindInterface(const char *InterfaceId, void **Interface);
	virtual void		SetClockReference(NX_CClockRef *pClockRef);
	virtual void		SetEventNotifi(NX_CEventNotifier *pEventNotifier);
	virtual CBOOL		Pause();
	//

	int32_t SetMediaType(void *Mediatype);
	void CloseDecoder(NX_VID_DEC_HANDLE hCodec);
	int FlushDecoder(NX_VID_DEC_HANDLE hCodec);
	void InitVideoTimeStamp();
	void PushVideoTimeStamp(int64_t timestamp, uint32_t flag);
	int32_t PopVideoTimeStamp(int64_t *timestamp, uint32_t *flag);

	NX_VID_DEC_HANDLE m_hCodec;

	//	Override  BaseFilter
	virtual CBOOL			Run();
	virtual CBOOL			Stop();
	virtual CBOOL			Flush();
	virtual	NX_CBasePin		*GetPin(int Pos);	//

	NX_CVideoDecoderInputPin	*m_pInputPin;
	NX_CVideoDecoderOutputPin	*m_pOutputPin;
	CBOOL						m_Discontinuity;
	unsigned char				m_VideoMaxBuf[MAX_INPUT_BUF_SIZE];
	int32_t						m_Width;
	int32_t						m_Height;
	int32_t						m_VpuCodecType;
	int32_t						m_Mp4Class;
	unsigned char				m_SeqData[1024 * 4];
	int32_t						m_SeqDataSize;
	NX_VideoMediaType			m_VMediaType;
	int32_t						m_bInit;
	struct OutBufferTimeInfo	m_OutTimeStamp[NX_MAX_BUF];	
	int32_t						m_iInFlag;
	int32_t						m_iOutFlag;

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
	NX_CVideoDecoderFilter(NX_CVideoDecoderFilter &Ref);
	NX_CVideoDecoderFilter &operator=(NX_CVideoDecoderFilter &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//						Nexell VideoDecoder Input Pin
class NX_CVideoDecoderInputPin :
	public NX_CBaseInputPin
{
public:
	NX_CVideoDecoderInputPin(NX_CVideoDecoderFilter *pOwner);
	virtual ~NX_CVideoDecoderInputPin(){}

	virtual int32_t Receive( NX_CSample *pInSample );
	virtual int32_t	CheckMediaType(void *Mediatype);
private:
	NX_CVideoDecoderInputPin(NX_CVideoDecoderInputPin &Ref);
	NX_CVideoDecoderInputPin &operator=(NX_CVideoDecoderInputPin &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell VideoDecoder Output Pin
class NX_CVideoDecoderOutputPin :
	public NX_CBaseOutputPin
{
public:
	NX_CVideoDecoderOutputPin(NX_CVideoDecoderFilter *pOwner);
	virtual ~NX_CVideoDecoderOutputPin();

	//	Implementation Pure Virtual Functions
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

	virtual int32_t		ReleaseSample( NX_CSample *pSample );
	virtual int32_t		GetDeliveryBuffer(NX_CSample **pSample);
	virtual int32_t		CheckMediaType(void *Mediatype);
	virtual CBOOL		Active();
	virtual CBOOL		Inactive();
	//	Alloc Buffer
	int32_t	AllocBuffers();
	int32_t	DeallocBuffers();
	int32_t	SetHandle(NX_VID_DEC_HANDLE hCodec);

	NX_VID_DEC_HANDLE	m_hCodec;	
	//	Media Sample Queue
	NX_CSampleQueue		*m_pSampleQ;
	NX_CSample			*m_pSampleList;
	NX_CMutex			m_VideoMutex;
private:
	NX_CVideoDecoderOutputPin(NX_CVideoDecoderOutputPin &Ref);
	NX_CVideoDecoderOutputPin &operator=(NX_CVideoDecoderOutputPin &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__cplusplus

#endif	//	__NX_CVideoDecoderFilter_h__
