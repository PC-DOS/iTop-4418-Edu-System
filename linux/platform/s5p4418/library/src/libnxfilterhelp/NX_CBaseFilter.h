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
#ifndef	__NX_CBaseFitler_h__
#define	__NX_CBaseFitler_h__

#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "NX_MediaType.h"
#include "NX_EventMessage.h"
#include "NX_SystemCall.h"
#include "NX_DebugMsg.h"


#ifdef __cplusplus


class NX_CSample;
class NX_CBasePin;
class NX_CClockRef;

#define VIDEOTYPE	0
#define AUDIOTYPE	1

#define ON			1
#define OFF			0

//----------------------------------------------------------------------------
//		Semaphore
//----------------------------------------------------------------------------

class NX_CSemaphore{
public:
	NX_CSemaphore() :
		m_Value (1),
		m_Max (1),
		m_Init (0),
		m_bReset (CFALSE)
	{
		pthread_cond_init ( &m_hCond,  NULL );
		pthread_mutex_init( &m_hMutex, NULL );
	}
	NX_CSemaphore( int32_t Max, int32_t Init ) :
		m_Value (Init),
		m_Max (Max),
		m_Init (Init),
		m_bReset (CFALSE)
	{
		pthread_cond_init ( &m_hCond,  NULL );
		pthread_mutex_init( &m_hMutex, NULL );
	}
	~NX_CSemaphore()
	{
		ResetSignal();
		pthread_cond_destroy( &m_hCond );
		pthread_mutex_destroy( &m_hMutex );
	}

	enum {	MAX_SEM_VALUE = 1024	};
public:
	int32_t Post()
	{
		int32_t Ret = 0;
		pthread_mutex_lock( &m_hMutex );
		m_Value ++;
		pthread_cond_signal ( &m_hCond );
		if( m_bReset || m_Value<=0 ){
			Ret = -1;
		}
		pthread_mutex_unlock( &m_hMutex );
		return Ret;
	}
	int32_t Pend( int32_t nSec )
	{
		int32_t Ret = 0;
		pthread_mutex_lock( &m_hMutex );
		if( m_Value == 0 && !m_bReset ){
			timespec time;
			int32_t err;
			do{
#ifdef __linux__
				clock_gettime(CLOCK_REALTIME, &time);
#else
				time.tv_nsec = GetTickCount();	//GetTickCount => ms
				time.tv_sec = time.tv_nsec/1000;
				time.tv_nsec = time.tv_nsec * 1000;

#endif
				if( time.tv_nsec > (1000000 - nSec) ){
					time.tv_sec += 1;
					time.tv_nsec -= (1000000 - nSec);
				}else{
					time.tv_nsec += nSec;
				}
				err = pthread_cond_timedwait( &m_hCond, &m_hMutex, &time );

				if( m_bReset == CTRUE ){
					Ret = -1;
					break;
				}
			}while( ETIMEDOUT == err );
			m_Value --;
		}else if( m_Value < 0 || m_bReset ){
			Ret = -1;
		}else{
			m_Value --;
			Ret = 0;
		}
		pthread_mutex_unlock( &m_hMutex );
		return Ret;
	}
	void ResetSignal()
	{
		pthread_mutex_lock ( &m_hMutex );
		m_bReset = CTRUE;
		for( int i=0 ; i<m_Max ; i++ ){
			pthread_cond_broadcast( &m_hCond );
		}
		pthread_mutex_unlock( &m_hMutex );
	}
	void ResetValue()
	{
		pthread_mutex_lock( &m_hMutex );
		m_Value = m_Init;
		m_bReset = CFALSE;
		pthread_mutex_unlock( &m_hMutex );
	}

private:
	pthread_cond_t  m_hCond;
	pthread_mutex_t m_hMutex;
	int32_t				m_Value;
	int32_t				m_Max;
	int32_t				m_Init;
	CBOOL			m_bReset;
};

class NX_CMutex{
public:
	NX_CMutex()
	{
		pthread_mutex_init( &m_hMutex, NULL );
	}
	~NX_CMutex()
	{
		pthread_mutex_destroy( &m_hMutex );
	}
	void Lock()
	{
		pthread_mutex_lock( &m_hMutex );
	}
	void Unlock()
	{
		pthread_mutex_unlock( &m_hMutex );
	}
private:
	pthread_mutex_t		m_hMutex;
};

class NX_CAutoLock{
public:
	NX_CAutoLock( NX_CMutex &Lock ) :m_pLock(Lock){
		m_pLock.Lock();
	}
    ~NX_CAutoLock() {
        m_pLock.Unlock();
    };
protected:
	NX_CMutex& m_pLock;
private:
	NX_CAutoLock (const NX_CAutoLock &Ref);
	NX_CAutoLock &operator=(NX_CAutoLock &Ref);
};

class NX_CAutoLock2{
public:
	NX_CAutoLock2( NX_CMutex *Lock ){
		m_pLock = Lock;
		m_pLock->Lock();
	}
    ~NX_CAutoLock2() {
        m_pLock->Unlock();
    };
protected:
	NX_CMutex *m_pLock;
private:
	NX_CAutoLock2 (const NX_CAutoLock2 &Ref);
	NX_CAutoLock2 &operator=(NX_CAutoLock2 &Ref);
};

class NX_CEvent{
public:
	NX_CEvent()
	{
		pthread_cond_init( &m_hCond, NULL );
		pthread_mutex_init( &m_hMutex, NULL );
		m_bWaitExit = CFALSE;
	}

	~NX_CEvent()
	{
		pthread_cond_destroy( &m_hCond );
		pthread_mutex_destroy( &m_hMutex );
	}

	int32_t SendEvent()
	{
		pthread_mutex_lock( &m_hMutex );
		pthread_cond_signal( &m_hCond );
		pthread_mutex_unlock( &m_hMutex );
		return 0;
	}

	int32_t WaitEvent()
	{
		pthread_mutex_lock( &m_hMutex );
		pthread_cond_wait( &m_hCond, &m_hMutex );
		pthread_mutex_unlock( &m_hMutex );
		return 0;
	}

	void ResetEvent()
	{
		pthread_mutex_lock( &m_hMutex );
		pthread_cond_broadcast( &m_hCond );
		pthread_mutex_unlock( &m_hMutex );
	}

private:
	pthread_cond_t m_hCond;
	pthread_mutex_t m_hMutex;
	CBOOL			m_bWaitExit;
};

//////////////////////////////////////////////////////////////////////////////
//
//							Event Management Classes
//

//
//	Event Receiver
//

class NX_CEventReceiver{
public:
	NX_CEventReceiver(){}
	virtual ~NX_CEventReceiver(){}

	virtual void ProcessEvent( uint32_t EventCode, uint32_t Value ) = 0;

private:
	NX_CEventReceiver (const NX_CEventReceiver &Ref);
	NX_CEventReceiver &operator=(const NX_CEventReceiver &Ref);
};

//
//	Event Notifier
//
class NX_CEventNotifier
{
public:
	NX_CEventNotifier();
	virtual ~NX_CEventNotifier();

public:
	void SendEvent( uint32_t EventCode, uint32_t EventValue );
	void SetEventReceiver( NX_CEventReceiver *pReceiver );

private:
	NX_CEventReceiver	*m_pReceiver;
	NX_CMutex			m_EventLock;
private:
	NX_CEventNotifier (const NX_CEventNotifier &Ref);
	NX_CEventNotifier &operator=(const NX_CEventNotifier &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------
//	NX_CBaseFilter
//------------------------------------------------------------------------------

class	NX_CBaseFilter
{
public:
	NX_CBaseFilter():
		m_bLocked	   ( CFALSE ),
		m_pEventNotify ( NULL  ),
		m_pOutFilter   ( NULL  ),
		m_bRunning     ( CFALSE ),
		m_pRefClock    ( NULL  )
	{
	}
	NX_CBaseFilter( const char *filterName ):
		m_bLocked	   ( CFALSE ),
		m_pEventNotify ( NULL  ),
		m_pOutFilter   ( NULL  ),
		m_bRunning     ( CFALSE ),
		m_pRefClock    ( NULL  )
	{
		int32_t len = strlen((const char *)filterName);
		memcpy(m_cArrFilterName, filterName, len);
		m_cArrFilterName[len] = 0;
	}
	virtual ~NX_CBaseFilter(){}

	virtual CBOOL Run()		{	return CTRUE;	}
	virtual CBOOL Stop()	{	return CTRUE;	}
	virtual CBOOL Flush()	{	return CTRUE;	}
	virtual NX_CBasePin	*GetPin( int Pos ) = 0;

	CBOOL SetFilterId(int8_t *FilterId)	
	{	
		int32_t Len = 0;
		Len = strlen((const char *)FilterId);
		memcpy(m_cArrFilterID, FilterId, Len);
		m_cArrFilterID[Len] = 0;
		return CTRUE;	
	}
	int8_t *GetFilterId()	
	{	
		return m_cArrFilterID;
	}
	CBOOL SetFilterName(const char *FilterName)	
	{	
		int32_t Len = 0;
		Len = strlen((const char *)FilterName);
		memcpy(m_cArrFilterName, FilterName, Len);
		m_cArrFilterName[Len] = 0;
		return CTRUE;	
	}
	int8_t *GetFilterName()	
	{	
		return m_cArrFilterName;	
	}

	//	Clock Reference
	void GetClockRef(NX_CClockRef *pClockRef)
	{
		m_pRefClock = pClockRef;
	}
	void SetClockRef(NX_CClockRef *pClockRef)
	{
		m_pRefClock = pClockRef;
	}
	void SetEventNotifier( NX_CEventNotifier *pEventNotifier )
	{
		m_pEventNotify = pEventNotifier;
	}

public:
	NX_CMutex		m_CtrlLock;
	CBOOL			m_bLocked;
	NX_CEventNotifier *m_pEventNotify;
	int8_t		m_cArrFilterName[20];
	int8_t		m_cArrFilterID[20];

protected:
	NX_CBaseFilter *m_pOutFilter;	
	CBOOL			m_bRunning;
	NX_CClockRef	*m_pRefClock;
};

//------------------------------------------------------------------------------
//	NX_CBasePin
//------------------------------------------------------------------------------
typedef enum{
	PIN_DIRECTION_INPUT,
	PIN_DIRECTION_OUTPUT
}NX_PIN_DIRECTION;

typedef struct _NX_VideoMediaType{
	int32_t		MeidaType;					//VIDEOTYPE
	int32_t		Width;
	int32_t		Height;
	int32_t		Framerate;
	uint8_t			*pSeqData;
	int32_t		SeqDataSize;
	int32_t		CodecID;
	int32_t		Mp4Class;
	int32_t		VpuCodecType;
	int64_t			Duration;
}NX_VideoMediaType;

typedef struct _NX_AudioMediaType{
	int32_t		Samplerate;					//AUDIOTYPE
	int32_t		Channels;
	int32_t		Bitrate;
	uint8_t			*pSeqData;
	int32_t		SeqDataSize;
	int32_t		CodecID;
	int32_t		BlockAlign;
	int64_t			Duration;
	int32_t		FormatType;
}NX_AudioMediaType;

typedef struct _NX_MediaType{
	int32_t MeidaType;					//VIDEOTYPE:0, AUDIOTYPE:1
	NX_VideoMediaType VMediaType;
	NX_AudioMediaType AMediaType;
}NX_MediaType;


class	NX_CBasePin
{
public:
	NX_MediaType	MediaType;
	NX_CBasePin( NX_CBaseFilter *pOwnerFilter )
	{
		m_pOwnerFilter	= pOwnerFilter;
		m_pPartnerPin	= NULL;
		m_bActive		= CFALSE;		//	Pin Status : Inactive
	}
	virtual ~NX_CBasePin( void ){};

	virtual int32_t	CheckMediaType(void *Mediatype) = 0; //pure virtual
	
	CBOOL	GetMediaType(NX_MediaType **Mediatype)
	{
		*Mediatype = &MediaType;
		return CTRUE;
	}


	CBOOL	IsConnected()
	{
		return (NULL != m_pPartnerPin)?CTRUE:CFALSE;
	}

	CBOOL	Connect( NX_CBasePin *pPin )
	{

		if( NULL != pPin && NULL == m_pPartnerPin )
		{
			m_pPartnerPin = pPin;
		}
		return CTRUE;
	}

	CBOOL	Disconnect( )
	{
		if( m_pPartnerPin )
		{
			m_pPartnerPin = NULL;
		}
		return CTRUE;
	}

	virtual CBOOL Active()
	{
		m_Mutex.Lock();
		m_bActive = CTRUE;
		m_Mutex.Unlock();
		return CTRUE;
	}

	virtual CBOOL Inactive()
	{
		m_Mutex.Lock();
		m_bActive = CFALSE;
		m_Mutex.Unlock();
		return CTRUE;
	}

	virtual CBOOL IsActive()
	{
		m_Mutex.Lock();
		CBOOL State = m_bActive;
		m_Mutex.Unlock();
		return State;
	}


protected:
	NX_CBaseFilter		*m_pOwnerFilter;
	NX_CBasePin			*m_pPartnerPin;
	CBOOL				m_bActive;
	NX_CMutex			m_Mutex;
};


class	NX_CBaseOutputPin :
		public NX_CBasePin
{
public:
	NX_CBaseOutputPin( NX_CBaseFilter *pOwnerFilter );
	virtual ~NX_CBaseOutputPin( ){};

	virtual	int32_t		Deliver( NX_CSample *pSample );
	virtual	int32_t		ReleaseSample( NX_CSample *pSample )    = 0;	//	Pure Virtual
	virtual int32_t		GetDeliveryBuffer(NX_CSample **pSample) = 0;	//	Pure Virtual

protected:
	NX_PIN_DIRECTION		m_Direction;
};

class	NX_CBaseInputPin :
		public NX_CBasePin
{
public:
	NX_CBaseInputPin( NX_CBaseFilter *pOwnerFilter );
	virtual ~NX_CBaseInputPin( ){};

	virtual	int32_t		Receive( NX_CSample *pSample ) = 0;
protected:
	NX_PIN_DIRECTION		m_Direction;
};



//------------------------------------------------------------------------------
//	NX_CSample
//------------------------------------------------------------------------------
class	NX_CSample
{
public:
	NX_CSample( NX_CBaseOutputPin *pOwner )
	{
		m_pOwner 		= pOwner;
		m_iRefCount		= 0;
		m_TimeStamp		= -1;
		m_pBuf			= NULL;
		m_BufSize		= 0;
		m_ActualBufSize	= 0;
		m_AlignedByte	= 0;
		m_pNext			= NULL;
		m_bDiscontinuity= CFALSE;
		m_bSyncPoint	= CFALSE;
		m_bValid		= CFALSE;
	}

	virtual ~NX_CSample( void ){
		if( m_pBuf )
			delete[] m_pBuf;
	}

	int32_t 	AddRef( void )
	{
		NX_CAutoLock lock( m_Lock );
		m_iRefCount++;
		return m_iRefCount;
	}
	
	int32_t 	Release( void )
	{
		NX_CAutoLock lock( m_Lock );
		if( m_iRefCount > 0 )
		{
			m_iRefCount--;
			if( m_iRefCount == 0 )
			{
				m_pOwner->ReleaseSample( this );
				return m_iRefCount;
			}
		}
		return m_iRefCount;
	}

	void	SetTime( int64_t TimeStamp )
	{
		m_TimeStamp = TimeStamp;
	}
	
	int32_t		GetTime( int64_t *TimeStamp )
	{
		if( m_TimeStamp == -1 )
			return -1;
		*TimeStamp = m_TimeStamp;
		return 0;		//	NO ERROR
	}

	void	SetBuffer( uint32_t *pBuf, int32_t Size, int32_t Align )
	{
		m_pBuf = pBuf;
		m_BufSize = Size;
		m_AlignedByte = Align;
	}

	int32_t GetPointer( uint32_t **ppBuf )
	{
		if( NULL != m_pBuf )
		{
			*ppBuf = (uint32_t*)m_pBuf;
			return 0;
		}
		return -1;
	}

	int32_t SetActualDataLength ( int32_t Size )
	{
		if( m_BufSize >= Size )
		{
			m_ActualBufSize = Size;
			return 0;
		}
		return -1;
	}

	int32_t GetActualDataLength()
	{
		return m_ActualBufSize;
	}

	int32_t GetRefCount()
	{
		return m_iRefCount;
	}

	CBOOL IsSyncPoint()
	{
		return m_bSyncPoint;
	}

	void SetSyncPoint( CBOOL SyncPoint )
	{
		m_bSyncPoint = SyncPoint;
	}

	void Reset()
	{
		m_iRefCount		= 0;
		m_TimeStamp		= -1;
		m_bDiscontinuity= CFALSE;
		m_bSyncPoint	= CFALSE;
		m_bValid		= CFALSE;
		m_bEndofStream	= CFALSE;
	}

	void SetDiscontinuity( CBOOL Discontinuity )
	{
		m_bDiscontinuity = Discontinuity;
	}

	CBOOL IsDiscontinuity( )
	{
		return m_bDiscontinuity;
	}

	CBOOL IsValid()
	{
		return m_bValid;
	}

	void SetValid( CBOOL Valid )
	{
		m_bValid = Valid;
	}

	void SetEndOfStream( CBOOL EndOfStream )
	{
		m_bEndofStream = EndOfStream;
	}

	CBOOL IsEndOfStream()
	{
		return m_bEndofStream;
	}

public:
	NX_CSample *m_pNext;

protected:
	NX_CBaseOutputPin	*m_pOwner;
	int32_t m_iRefCount;
	int64_t	m_TimeStamp;
	uint32_t *m_pBuf;
	int32_t m_BufSize;
	int32_t m_ActualBufSize;
	int32_t m_AlignedByte;
	CBOOL m_bDiscontinuity;
	CBOOL m_bSyncPoint;
	CBOOL m_bValid;
	CBOOL m_bEndofStream;
	NX_CMutex m_Lock;

private:
	NX_CSample (const NX_CSample &Ref);
	NX_CSample &operator=(const NX_CSample &Ref);
};

//------------------------------------------------------------------------------
//	NX_CSampleQueue
//------------------------------------------------------------------------------

class	NX_CSampleQueue
{
public:
	NX_CSampleQueue( int32_t MaxQueueCnt ) :
		m_iHeadIndex( 0 ),
		m_iTailIndex( 0 ),
		m_iSampleCount( 0 ),
		m_MaxQueueCnt( MaxQueueCnt ),
		m_SemPush( NULL ),
		m_SemPop( NULL ),
		m_EndQueue(CFALSE)
	{
		m_MaxQueueCnt = MaxQueueCnt;
		m_SemPush = new NX_CSemaphore( MaxQueueCnt, MaxQueueCnt );	//	available buffer
		m_SemPop  = new NX_CSemaphore( MaxQueueCnt, 0           );	//	
		pthread_mutex_init( &m_ValueMutex, NULL );
		assert( m_SemPush != NULL );
		assert( m_SemPop != NULL );
	}

	virtual ~NX_CSampleQueue( void )
	{
		if( m_SemPush ){
			m_SemPush->ResetSignal();
			delete m_SemPush;
		}
		if( m_SemPop ){
			m_SemPop->ResetSignal();
			delete m_SemPop;
		}
		pthread_mutex_destroy( &m_ValueMutex );
	}
	
public:	
	virtual int32_t	PushSample( NX_CSample *pSample )
	{
		pthread_mutex_lock(&m_ValueMutex);
		if( m_EndQueue ){	pthread_mutex_unlock(&m_ValueMutex);	return -1;}
		pthread_mutex_unlock(&m_ValueMutex);
		if( m_SemPush->Pend( 300000 ) != 0 )
			return -1;
//		printf("===PushSample m_iTailIndex = %d\n", m_iTailIndex);
		pthread_mutex_lock( &m_ValueMutex );
		m_pSamplePool[m_iTailIndex] = pSample;
		m_iTailIndex = (m_iTailIndex+1) % m_MaxQueueCnt;
		m_iSampleCount++;
		pthread_mutex_unlock( &m_ValueMutex );

		return m_SemPop->Post();
	}

	virtual int32_t	PopSample( NX_CSample **ppSample )
	{
		pthread_mutex_lock(&m_ValueMutex);
		if( m_EndQueue ){	pthread_mutex_unlock(&m_ValueMutex);	return -1;}
//		printf("===PopSample m_iHeadIndex = %d, m_EndQueue = %d\n", m_iHeadIndex, m_EndQueue);
		pthread_mutex_unlock(&m_ValueMutex);
		if( m_SemPop->Pend( 300000 ) != 0 )
			return -1;
//		printf("===PopSample m_iTailIndex = %d\n", m_iTailIndex);
		pthread_mutex_lock( &m_ValueMutex );
		*ppSample = m_pSamplePool[m_iHeadIndex];
		m_iHeadIndex = (m_iHeadIndex+1) % m_MaxQueueCnt;
		m_iSampleCount--;
		pthread_mutex_unlock( &m_ValueMutex );

		return m_SemPush->Post();
	}

	int	GetSampleCount( void )
	{
		int Count;
		pthread_mutex_lock( &m_ValueMutex );
		Count = m_iSampleCount;
		pthread_mutex_unlock( &m_ValueMutex );
		return Count;
	}

	void EndQueue()
	{
		pthread_mutex_lock(&m_ValueMutex);
		m_EndQueue = CTRUE;
		m_SemPush->ResetSignal();
		m_SemPop->ResetSignal();
		pthread_mutex_unlock(&m_ValueMutex);
	}

	void ResetQueue()
	{
		pthread_mutex_lock(&m_ValueMutex);
		m_EndQueue = CFALSE;
		m_SemPush->ResetValue();
		m_SemPop->ResetValue();
		m_iHeadIndex = 0;
		m_iTailIndex = 0;
		m_iSampleCount = 0;
		pthread_mutex_unlock(&m_ValueMutex);
	}

private:
	enum { SAMPLE_QUEUE_COUNT = 128 };
	NX_CSample *m_pSamplePool[SAMPLE_QUEUE_COUNT];
	int32_t	m_iHeadIndex, m_iTailIndex, m_iSampleCount;
	int32_t m_MaxQueueCnt;
	NX_CSemaphore	*m_SemPush;
	NX_CSemaphore	*m_SemPop;
	pthread_mutex_t	m_ValueMutex;
	CBOOL m_EndQueue;
};

//
//	Time Resolution :  1/1000 seconds
//
class NX_CClockRef
{
public:
	NX_CClockRef()
		: m_bSetRefTime(CFALSE)
		, m_StartTime(0)
		, m_MediaDeltaTime(0)
		, m_PauseDeltaTime(0)
		, m_bStartPuaseTime(CFALSE)
	{
	}
	virtual ~NX_CClockRef() {}

public:
	void SetReferenceTime( int64_t StartTime )
	{
		NX_CAutoLock lock(m_Mutex);
		m_bSetRefTime  = CTRUE;
		m_StartTime    = StartTime;
		m_RefStartTime = NX_GetTickCount();
	}

	int32_t GetCurrTime( int64_t *CurTime )
	{
		NX_CAutoLock lock(m_Mutex);

		int32_t Ret = -1;
		if( CFALSE == m_bSetRefTime ){
			*CurTime = -1;
		}else{
			//
			//	Step 1. Find Reference Time : NX_GetTickCount() - m_RefStartTime
			//	Step 2. Adjust Reference Time to Seek Start Time
			//	Step 3. Adjust Reference Time into m_MediaDeltaTime
			//	Step 4. Adjust Reference Time into m_PauseDeltaTime
			//	
			*CurTime = (NX_GetTickCount() - m_RefStartTime) + m_StartTime + m_MediaDeltaTime - m_PauseDeltaTime;
			Ret = 0;
		}
		return Ret;
	}

	int32_t GetMediaTime( int64_t *CurTime )
	{
		NX_CAutoLock lock(m_Mutex);
		int32_t Ret = -1;
		if( CFALSE == m_bSetRefTime ){
			*CurTime = -1;
		}else{
			*CurTime = (NX_GetTickCount() - m_RefStartTime) + m_StartTime + m_MediaDeltaTime - m_PauseDeltaTime;
			if( m_bStartPuaseTime ){
				int64_t tmpTime = NX_GetTickCount();
				*CurTime -= (tmpTime-m_PauseTime);
			}
			Ret = 0;
		}
		return Ret;
	}

	void AdjustMediaTime( int64_t Gap ){
		NX_CAutoLock lock(m_Mutex);
		m_MediaDeltaTime += Gap;
	}

	void PauseRefTime( void ){
		NX_CAutoLock lock(m_Mutex);
		if( m_bStartPuaseTime == CFALSE )
		{
			m_PauseTime = NX_GetTickCount();
			printf("=====m_PauseTime %lld\n", m_PauseTime);
			m_bStartPuaseTime = CTRUE;
		}
	}

	void ResumeRefTime( void ){
		NX_CAutoLock lock(m_Mutex);
		if( m_bStartPuaseTime == CTRUE )
		{
			m_ResumeTime = NX_GetTickCount();
			m_PauseDeltaTime += (m_ResumeTime-m_PauseTime);
			//printf("===m_PauseDeltaTime = %lld\n");
			m_bStartPuaseTime = CFALSE;
		}
	}

	void Reset(){
		NX_CAutoLock lock(m_Mutex);
		m_bSetRefTime = CFALSE;
		m_StartTime = 0;
		m_MediaDeltaTime = 0;
		m_PauseDeltaTime = 0;
	}

public:
	CBOOL m_bSetRefTime;

	int64_t	m_StartTime;
	int64_t m_RefStartTime;

	int64_t	m_MediaDeltaTime;
	int64_t m_PauseDeltaTime;

	CBOOL m_bStartPuaseTime;
	int64_t m_PauseTime;
	int64_t m_ResumeTime;

	NX_CMutex m_Mutex;
};

#endif //	__cplusplus

#endif // __NX_CBaseFilter_h__
