//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		: 
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __CNX_BASEFILTER_H__
#define __CNX_BASEFILTER_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <NX_FilterSysApi.h>
#include <nx_alloc_mem.h>
#include <pthread.h>

#ifdef __cplusplus

//------------------------------------------------------------------------------
//	CNX_BaseFilter
//------------------------------------------------------------------------------
class	CNX_Sample;
class	INX_EventNotify;
class	CNX_AutoLock;
class	CNX_RefClock;

class	CNX_BaseFilter
{
public:
	CNX_BaseFilter()
	{
		m_pOutFilter = NULL;
		m_pNotify = NULL;
		m_pRefClock = NULL;
	}

	virtual ~CNX_BaseFilter() {}

public:
	virtual	int32_t	Receive( CNX_Sample *pSample ) = 0;
	virtual int32_t	ReleaseSample( CNX_Sample *pSample ) = 0;

	virtual void	Connect( CNX_BaseFilter *pFilter )
	{
		m_pOutFilter = pFilter;
	}

	virtual void	Disconnect( void )
	{
		m_pOutFilter = NULL;
	}

	virtual	int32_t	Run( void ) 	{ return true; }
	virtual	int32_t	Stop( void ) 	{ return true; }
	virtual void	SetNotifier( INX_EventNotify *pNotify ){ if( pNotify )	m_pNotify = pNotify; }

protected:
	virtual int32_t	Deliver( CNX_Sample *pSample )
	{
		if( NULL != m_pOutFilter )
		{
			return m_pOutFilter->Receive( pSample );
		}

		return false;
	}

protected:
	CNX_BaseFilter *m_pOutFilter;
	INX_EventNotify *m_pNotify;
	CNX_RefClock	*m_pRefClock;
};


//------------------------------------------------------------------------------
//	CNX_Sample
//------------------------------------------------------------------------------
class	CNX_Sample
{
public:
	CNX_Sample( CNX_BaseFilter *pOwner )
	{
		m_pOwner 		= pOwner;
		m_iRefCount		= 0;
		m_qwTimeStamp	= 0;
		pthread_mutex_init( &m_hSampleLock, NULL );
	}

	virtual ~CNX_Sample( void )
	{
		pthread_mutex_destroy( &m_hSampleLock );
	}

	virtual void		SetOwner( CNX_BaseFilter *pOwner )
	{
		m_pOwner = pOwner;
	}

	virtual void 		Lock( void )
	{
		pthread_mutex_lock( &m_hSampleLock );
		m_iRefCount++;
		pthread_mutex_unlock( &m_hSampleLock );
	}

	virtual void 		Unlock( void )
	{
		pthread_mutex_lock( &m_hSampleLock );
		if( 0 < m_iRefCount )
		{
			m_iRefCount--;
			if( 0 == m_iRefCount )
			{
				pthread_mutex_unlock( &m_hSampleLock );
				if( NULL != m_pOwner )		m_pOwner->ReleaseSample( this );
				return;
			}
		}
		pthread_mutex_unlock( &m_hSampleLock );
	}

	virtual	void		SetTimeStamp( uint64_t timestamp )
	{
		m_qwTimeStamp = timestamp;
	}

	virtual	uint64_t	GetTimeStamp( void )
	{
		return m_qwTimeStamp;
	}

	virtual int32_t		GetRefCount( void )
	{
		return m_iRefCount;
	}

protected:
	CNX_BaseFilter	*m_pOwner;
	int32_t			m_iRefCount;
	uint64_t		m_qwTimeStamp;
	pthread_mutex_t m_hSampleLock;
};


//------------------------------------------------------------------------------
//	CNX_MediaSample
//	; General media sample
//------------------------------------------------------------------------------
enum {
	FLAGS_WRITING_NONE = 0,
	FLAGS_WRITING_NORMAL_START,
	FLAGS_WRITING_NORMAL_STOP,
	FLAGS_WRITING_EVENT_START,
	FLAGS_WRITING_EVENT_STOP,
};

class	CNX_MediaSample
		: public CNX_Sample
{
public:
	CNX_MediaSample() 
		: CNX_Sample( NULL )
		, m_pBuf( NULL )
		, m_BufSize( 0 )
		, m_ActualBufSize(0)
		, m_bDiscontinuity( true )
		, m_bSyncPoint( false )
	{
	}
	CNX_MediaSample( CNX_BaseFilter *pOwner, uint8_t *pBuf, int32_t bufSize )
		: CNX_Sample( pOwner )
		, m_pBuf( pBuf )
		, m_BufSize( bufSize )
		, m_ActualBufSize(0)
		, m_bDiscontinuity( true )
		, m_bSyncPoint( false )
		, m_Flag( 0 )
	{
	}

public:
	virtual void	SetBuffer( uint8_t* pBuf, int32_t size )
	{
		m_pBuf = pBuf;
		m_BufSize = size;
		m_ActualBufSize = 0;		//	Reset actual data size
	}
	virtual int32_t	GetBuffer( uint8_t** ppBuf, int32_t *size )
	{
		if( NULL != m_pBuf )
		{
			*ppBuf = m_pBuf;
			*size = m_BufSize ;
			return true;
		}
		return false;
	}
	virtual int32_t	GetActualDataLength( void )
	{
		return m_ActualBufSize;
	}
	virtual int32_t	SetActualDataLength( int32_t size )
	{
		if( m_BufSize >= size )
		{
			m_ActualBufSize = size;
			return true;
		}
		return false;
	}
	virtual int32_t	IsDiscontinuity( void )
	{
		return m_bDiscontinuity;
	}
	virtual void	SetDiscontinuity( int32_t discontinuity )
	{
		m_bDiscontinuity = discontinuity;
	}
	virtual int32_t	IsSyncPoint( void )
	{
		return m_bSyncPoint;
	}
	virtual void	SetSyncPoint( int32_t syncPoint )
	{
		m_bSyncPoint = syncPoint;
	}
	virtual int32_t	GetSyncPoint( void )
	{
		return m_bSyncPoint;
	}
	virtual void 	SetFlags( int32_t flag )
	{
		m_Flag = flag;
	}
	virtual int32_t GetFlags( void )
	{
		return m_Flag;
	}
private:
	uint8_t		*m_pBuf;
	int32_t		m_BufSize;
	int32_t		m_ActualBufSize;
	int32_t		m_bDiscontinuity;
	int32_t		m_bSyncPoint;
	int32_t		m_Flag;

private:
	CNX_MediaSample (const CNX_MediaSample &Ref);
	CNX_MediaSample &operator=(const CNX_MediaSample &Ref);
};


//------------------------------------------------------------------------------
//	CNX_VideoSample
//------------------------------------------------------------------------------
class	CNX_VideoSample
		: public CNX_MediaSample
{
public:
	CNX_VideoSample() 
	{
		m_pVideoMemory = NULL;
	}

	virtual ~CNX_VideoSample() {}

public:
	virtual void	SetVideoMemory(NX_VID_MEMORY_INFO *pVideoMemory)
	{
		m_pVideoMemory = pVideoMemory;
	}

	virtual NX_VID_MEMORY_INFO* GetVideoMemory(void)
	{
		return m_pVideoMemory;
	}

	virtual void	SetVideoMemoryIndex( int32_t index )
	{
		m_iVideoMemroyIndex = index;
	}

	virtual int32_t	GetVideoMemoryIndex( void )
	{
		return m_iVideoMemroyIndex;
	}

private:
	NX_VID_MEMORY_INFO	*m_pVideoMemory;
	int32_t				m_iVideoMemroyIndex;

private:
	CNX_VideoSample (const CNX_VideoSample &Ref);
	CNX_VideoSample &operator=(const CNX_VideoSample &Ref);
};


//------------------------------------------------------------------------------
//	CNX_MuxerSample
//	; General media sample
//------------------------------------------------------------------------------
class	CNX_MuxerSample
		: public CNX_MediaSample
{
public:
	CNX_MuxerSample(){}
	virtual ~CNX_MuxerSample(){}
	void SetDataType( uint32_t dataType )
	{
		m_DataType = dataType;
	}
	uint32_t	GetDataType()
	{
		return m_DataType;
	}

	enum {	
		DATA_VIDEO0	= 0x00,
		DATA_VIDEO1	= 0x01,
		DATA_AUDIO0	= 0x02,	
		DATA_USER0	= 0x03 };

private:
	uint32_t		m_DataType;
private:
	CNX_MuxerSample (const CNX_MuxerSample &Ref);
	CNX_MuxerSample &operator=(const CNX_MuxerSample &Ref);
};


//------------------------------------------------------------------------------
//	CNX_SampleQueue
//------------------------------------------------------------------------------
class	CNX_SampleQueue
{
public:
	CNX_SampleQueue( )
	{
		m_iHeadIndex 	= 0;
		m_iTailIndex 	= 0;
		m_iSampleCount  = 0;
		m_iQueueDepth	= SAMPLE_QUEUE_COUNT;
		pthread_mutex_init( &m_hSampleQLock, NULL );
	}
	virtual ~CNX_SampleQueue( )
	{
		pthread_mutex_destroy( &m_hSampleQLock );
	};
public:
	virtual int32_t	PushSample( CNX_Sample *pSample )
	{
		pthread_mutex_lock( &m_hSampleQLock );

		if( m_iQueueDepth <= m_iSampleCount ) {
			// printf("%s(): SampleQueue is full.\n", __func__);
			pthread_mutex_unlock( &m_hSampleQLock );
			return false;
		}

		m_pSamplePool[m_iTailIndex] = pSample;
		m_iTailIndex = (m_iTailIndex+1) % SAMPLE_QUEUE_COUNT;
		m_iSampleCount++;

		pthread_mutex_unlock( &m_hSampleQLock );
		return true;
	}

	virtual int32_t	PopSample( CNX_Sample **ppSample )
	{
		pthread_mutex_lock( &m_hSampleQLock );

		if( m_iSampleCount <= 0 ) {
			// printf("%s(): SampleQueue is empty.\n", __func__);
			pthread_mutex_unlock( &m_hSampleQLock );
			return false;
		}

		*ppSample = m_pSamplePool[m_iHeadIndex];
		m_iHeadIndex = (m_iHeadIndex+1) % SAMPLE_QUEUE_COUNT;
		m_iSampleCount--;

		pthread_mutex_unlock( &m_hSampleQLock );
		return true;
	}
	virtual int32_t	IsReady( void )
	{
		pthread_mutex_lock( &m_hSampleQLock );
		if( m_iSampleCount > 0 ) {
			pthread_mutex_unlock( &m_hSampleQLock );
			return true;
		}
		pthread_mutex_unlock( &m_hSampleQLock );
		return false;
	}
	virtual void	Reset( void )
	{
		pthread_mutex_lock( &m_hSampleQLock );
		m_iHeadIndex 	= 0;
		m_iTailIndex 	= 0;
		m_iSampleCount 	= 0;
		pthread_mutex_unlock( &m_hSampleQLock );
	}
	virtual	int32_t	GetSampleCount( void )
	{
		int32_t nSampleCount = 0;
		pthread_mutex_lock( &m_hSampleQLock );
		nSampleCount = m_iSampleCount;
		pthread_mutex_unlock( &m_hSampleQLock );
		return nSampleCount;
	}
	virtual	void	SetQueueDepth( int32_t depth )
	{
		pthread_mutex_lock( &m_hSampleQLock );
		m_iQueueDepth = depth;
		pthread_mutex_unlock( &m_hSampleQLock );
	}
	virtual int32_t GetMaxSampleCount( void )
	{
		int32_t depth;
		pthread_mutex_lock( &m_hSampleQLock );
		depth = m_iQueueDepth;
		pthread_mutex_unlock( &m_hSampleQLock );
		return depth;
	}
private:
	enum { SAMPLE_QUEUE_COUNT = 256 + 128};
	CNX_Sample *m_pSamplePool[SAMPLE_QUEUE_COUNT];
	int32_t	m_iHeadIndex, m_iTailIndex, m_iSampleCount;
	int32_t m_iQueueDepth;
	pthread_mutex_t m_hSampleQLock;
private:
	CNX_SampleQueue (const CNX_SampleQueue &Ref);
	CNX_SampleQueue &operator=(const CNX_SampleQueue &Ref);
};	


//------------------------------------------------------------------------------
//	INX_EventNotify
//------------------------------------------------------------------------------
class INX_EventNotify
{
public:
	virtual ~INX_EventNotify(){}
	virtual void EventNotify( uint32_t eventCode, void *pEventData, uint32_t dataLength ) = 0;
};


//----------------------------------------------------------------------------
//		Semaphore
//----------------------------------------------------------------------------
class CNX_Semaphore
{
public:
	CNX_Semaphore( int32_t Max, int32_t InitVal ) :
		m_Value (InitVal),
		m_Max (Max),
		m_Init (InitVal),
		m_bReset (false)
	{
		pthread_cond_init ( &m_hCond,  NULL );
		pthread_mutex_init( &m_hMutex, NULL );
	}
	~CNX_Semaphore()
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
		if( m_bReset || m_Value <= 0 ) {
			Ret = -1;
		}
		pthread_mutex_unlock( &m_hMutex );
		return Ret;
	}
	int32_t Pend()
	{
		int32_t Ret = 0;
		pthread_mutex_lock( &m_hMutex );
		if( m_Value == 0 && !m_bReset ){
			Ret = pthread_cond_wait( &m_hCond, &m_hMutex );
			m_Value--;
		}
		else if( m_Value < 0 || m_bReset ) {
			Ret = -1;
		}
		else {
			m_Value--;
			Ret = 0;
		}
		pthread_mutex_unlock( &m_hMutex );
		return Ret;
	}
	void ResetSignal()
	{
		pthread_mutex_lock ( &m_hMutex );
		m_bReset = true;
		for( int32_t i=0 ; i<m_Max ; i++ )
			pthread_cond_signal( &m_hCond );
		pthread_mutex_unlock( &m_hMutex );
	}
	void Init()
	{
		pthread_mutex_lock( &m_hMutex );
		m_Value = m_Init;
		m_bReset = false;
		pthread_mutex_unlock( &m_hMutex );
	}
	// int32_t GetValue() // for debug
	// {
	// 	int32_t value = 0;
	// 	pthread_mutex_lock( &m_hMutex );
	// 	value = m_Value;
	// 	pthread_mutex_unlock( &m_hMutex );
	// 	return value;	
	// }
private:
	pthread_cond_t  m_hCond;
	pthread_mutex_t m_hMutex;
	int32_t			m_Value;
	int32_t			m_Max;
	int32_t			m_Init;
	int32_t			m_bReset;
};


//------------------------------------------------------------------------------
//	CNX_BufferQueue
//------------------------------------------------------------------------------
class	CNX_BufferQueue
{
public:
	CNX_BufferQueue( void )
	{
		m_iHeadIndex 	= 0;
		m_iTailIndex 	= 0;
		m_iSampleCount  = 0;
		m_iQueueDepth	= MAX_QUEUE_COUNT;
		pthread_mutex_init( &m_hQLock, NULL );
	}

	virtual ~CNX_BufferQueue( void )
	{
		pthread_mutex_destroy( &m_hQLock );
	}
	
public:
	virtual void	Push( void *pSample, int32_t size )
	{
		pthread_mutex_lock( &m_hQLock );

		m_pBuffer[m_iTailIndex] = pSample;
		m_BufferSize[m_iTailIndex] = size;
		m_iTailIndex = (m_iTailIndex+1) % MAX_QUEUE_COUNT;
		m_iSampleCount++;

		pthread_mutex_unlock( &m_hQLock );
	}

	virtual void	Pop( void **ppSample, int32_t *size )
	{
		pthread_mutex_lock( &m_hQLock );

		*ppSample = m_pBuffer[m_iHeadIndex];
		*size = m_BufferSize[m_iHeadIndex];
		m_iHeadIndex = (m_iHeadIndex+1) % MAX_QUEUE_COUNT;
		m_iSampleCount--;

		pthread_mutex_unlock( &m_hQLock );
	}

	virtual int32_t	IsReady( void )
	{
		pthread_mutex_lock( &m_hQLock );
		if( m_iSampleCount > 0 ) {
			pthread_mutex_unlock( &m_hQLock );
			return true;
		}
		pthread_mutex_unlock( &m_hQLock );
		return false;
	}

	virtual void	Reset( void )
	{
		pthread_mutex_lock( &m_hQLock );
		m_iHeadIndex 	= 0;
		m_iTailIndex 	= 0;
		m_iSampleCount 	= 0;
		pthread_mutex_unlock( &m_hQLock );
	}

	virtual	int32_t	GetSampleCount( void )
	{
		int32_t nSampleCount = 0;
		pthread_mutex_lock( &m_hQLock );
		nSampleCount = m_iSampleCount;
		pthread_mutex_unlock( &m_hQLock );
		return nSampleCount;
	}

	virtual	void	SetQueueDepth( int32_t depth )
	{
		pthread_mutex_lock( &m_hQLock );
		m_iQueueDepth = depth;
		pthread_mutex_unlock( &m_hQLock );
	}
	
	virtual int32_t GetMaxSampleCount( void )
	{
		int32_t depth;
		pthread_mutex_lock( &m_hQLock );
		depth = m_iQueueDepth;
		pthread_mutex_unlock( &m_hQLock );
		return depth;
	}

protected:
	enum { MAX_QUEUE_COUNT = 128 };
	void *m_pBuffer[MAX_QUEUE_COUNT];
	int32_t m_BufferSize[MAX_QUEUE_COUNT];
	int32_t m_Flags[MAX_QUEUE_COUNT];
	int32_t	m_iHeadIndex, m_iTailIndex, m_iSampleCount;
	int32_t m_iQueueDepth;
	pthread_mutex_t m_hQLock;
private:
	CNX_BufferQueue (const CNX_BufferQueue &Ref);
	CNX_BufferQueue &operator=(const CNX_BufferQueue &Ref);
};	


class CNX_AutoLock{
public:
    CNX_AutoLock( pthread_mutex_t *pLock ) :m_pLock(pLock)
	{
        pthread_mutex_lock( m_pLock );
    }
    ~CNX_AutoLock()
	{
        pthread_mutex_unlock(m_pLock);
    }
protected:
	pthread_mutex_t	*m_pLock;
private:
    CNX_AutoLock (const CNX_AutoLock &Ref);
    CNX_AutoLock &operator=(CNX_AutoLock &Ref);
};

#if(1)
// typedef struct tag_FILTER_STATISTICS {
// 	uint64_t	frameCount;
// 	double		frameRate;
// 	double		bitrate;
// 	double		frequency;
// 	uint64_t	bufAverage;
// 	uint64_t	bufCurrent;
// 	uint64_t	bufMax;
// 	uint64_t	bufLimit;
// } FILTER_STATISTICS;

class CNX_Statistics
{
public:
	CNX_Statistics()
		: m_nFpsPrvTime( 0 )
		, m_nFpsCount( 0 )
		, m_nFps( 0. )
		, m_nFpsSum( 0. )
		, m_nFpsSumCount( 0 )
		, m_nBpsPrvTime( 0 )
		, m_nBps( 0. )
		, m_nBpsSum( 0. )
		, m_nBpsSumCount( 0 )
	{
		pthread_mutex_init( &m_hLock, NULL );
	}
	virtual ~CNX_Statistics()
	{
		pthread_mutex_destroy( &m_hLock );
	}
	virtual void ResetFps( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nFpsPrvTime	= 0;
		m_nFpsCount 	= 0;
		m_nFps 			= 0;
		m_nFpsSum 		= 0;
		m_nFpsSumCount 	= 0;
		pthread_mutex_unlock( &m_hLock );
	}
	virtual void CalculateFps( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nFpsCount++;
		if( !(m_nFpsCount % FPS_SAMPLING_NUMBER) ) {
			uint64_t curTime = NX_GetTickCount();
			if( m_nFpsSumCount ) {
				if( curTime - m_nFpsPrvTime ) {
					m_nFps = (double)(FPS_SAMPLING_NUMBER * 1000.) / (double)(curTime - m_nFpsPrvTime);
					m_nFpsSum += m_nFps;
				}
			}
			m_nFpsPrvTime = curTime;
			m_nFpsSumCount++;
		}
		pthread_mutex_unlock( &m_hLock );
	}
	virtual double GetFpsCurrent( void )
	{	
		double nFps = 0;
		pthread_mutex_lock( &m_hLock );
		nFps = m_nFps;
		pthread_mutex_unlock( &m_hLock );
		return nFps;
	}
	virtual double GetFpsAverage( void )
	{
		double nFps = 0;
		pthread_mutex_lock( &m_hLock );
		if( m_nFpsSumCount > 1 )
			nFps = m_nFpsSum / ((double)m_nFpsSumCount - 1.);
		pthread_mutex_unlock( &m_hLock );
		return nFps;
	}
	virtual void ResetBps( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nBpsPrvTime	= 0;
		m_nBps 			= 0;
		m_nBpsSum 		= 0;
		m_nBpsSumCount 	= 0;
		pthread_mutex_unlock( &m_hLock );
	}	
	virtual void CalculateBpsStart( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nBpsPrvTime = NX_GetTickCount();
		pthread_mutex_unlock( &m_hLock );
	}
	virtual void CalculateBpsEnd( uint32_t nWriteSize )
	{
		uint64_t curTime = NX_GetTickCount();
		if( !nWriteSize ) return;

		pthread_mutex_lock( &m_hLock );
		// printf("nWriteSize = %d, curTime = %lld, m_nBpsPrvTime = %lld\n",
		// 	nWriteSize, curTime, m_nBpsPrvTime);

		if( curTime - m_nBpsPrvTime ) {
			m_nBps = (double)nWriteSize * 8. / (double)(curTime - m_nBpsPrvTime);
			m_nBpsSum += m_nBps;
			m_nBpsSumCount++;
		}
		pthread_mutex_unlock( &m_hLock );
	}
	virtual double GetBpsCurrent( void )
	{	
		double nBps = 0;
		pthread_mutex_lock( &m_hLock );
		nBps = m_nBps;
		pthread_mutex_unlock( &m_hLock );
		return nBps;
	}
	virtual double GetBpsAverage( void )
	{
		double nBps = 0;
		pthread_mutex_lock( &m_hLock );
		if( m_nBpsSumCount > 0 )
			nBps = m_nBpsSum / (double)m_nBpsSumCount;
		pthread_mutex_unlock( &m_hLock );
		return nBps;
	}
	virtual void ResetBufNumber( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nBufNumberMin = 1024;
		m_nBufNumberMax = 0;
		m_nBufNumberAverage = 0;
		m_nBufNumberSum = 0;
		m_nBufNumberCount = 0;
		pthread_mutex_unlock( &m_hLock );
	}
	virtual void CalculateBufNumber( int32_t nBufNumber )
	{
		pthread_mutex_lock( &m_hLock );
		if( m_nBufNumberMin > nBufNumber) m_nBufNumberMin = nBufNumber;
		if( m_nBufNumberMax < nBufNumber) m_nBufNumberMax = nBufNumber;
		m_nBufNumberCur = nBufNumber;
		m_nBufNumberSum += nBufNumber;
		m_nBufNumberCount++;
		pthread_mutex_unlock( &m_hLock );
	}
	virtual int32_t GetBufNumberCur( void )
	{
		int32_t nBufNumber = 0;
		pthread_mutex_lock( &m_hLock );
		nBufNumber = m_nBufNumberCur;
		pthread_mutex_unlock( &m_hLock );
		return nBufNumber;
	}
	virtual int32_t GetBufNumberMin( void )
	{
		int32_t nBufNumber = 0;
		pthread_mutex_lock( &m_hLock );
		nBufNumber = m_nBufNumberMin;
		pthread_mutex_unlock( &m_hLock );
		return nBufNumber;
	}
	virtual int32_t GetBufNumberMax( void )
	{
		int32_t nBufNumber = 0;
		pthread_mutex_lock( &m_hLock );
		nBufNumber = m_nBufNumberMax;
		pthread_mutex_unlock( &m_hLock );
		return nBufNumber;
	}
	virtual int32_t GetBufNumberAverage( void )
	{
		int32_t nBufNumber = 0;
		pthread_mutex_lock( &m_hLock );
		nBufNumber = (int32_t)((double)m_nBufNumberSum / (double)m_nBufNumberCount);
		pthread_mutex_unlock( &m_hLock );
		return nBufNumber;
	}
	virtual void CalculateIntervalStart( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nIntervalTime = NX_GetTickCount();
		pthread_mutex_unlock( &m_hLock );
	}
	virtual void CalculateIntervalEnd( void )
	{
		pthread_mutex_lock( &m_hLock );
		m_nInterval = NX_GetTickCount() - m_nIntervalTime;
		pthread_mutex_unlock( &m_hLock );
	}
	virtual uint64_t GetIntervalCur( void )
	{
		uint64_t nInterval = 0;
		pthread_mutex_lock( &m_hLock );
		nInterval = m_nInterval;
		pthread_mutex_unlock( &m_hLock );
		return nInterval;
	}
protected:
	pthread_mutex_t m_hLock;
	
	enum 		{ FPS_SAMPLING_NUMBER = 30 };
	uint64_t	m_nDisplayTime;
	uint64_t 	m_nDisplayPrvTime;

	uint64_t	m_nFpsPrvTime;
	uint64_t	m_nFpsCount;
	double		m_nFps;
	double		m_nFpsSum;
	uint64_t	m_nFpsSumCount;

	uint64_t	m_nBpsPrvTime;
	double		m_nBps;
	double		m_nBpsSum;
	uint64_t	m_nBpsSumCount;

	int32_t		m_nBufNumberCur;
	int32_t		m_nBufNumberMin;
	int32_t		m_nBufNumberMax;
	int32_t		m_nBufNumberAverage;
	uint64_t	m_nBufNumberSum;
	uint64_t	m_nBufNumberCount;

	uint64_t	m_nIntervalTime;
	uint64_t	m_nInterval;
	uint64_t 	m_nIntervalCur;
	uint64_t	m_nIntervalMin;
	uint64_t	m_nIntervalMax;
	uint64_t	m_nIntervalSum;
	uint64_t	m_nIntervalCount;

private:
	CNX_Statistics (const CNX_Statistics &Ref);
	CNX_Statistics &operator=(const CNX_Statistics &Ref);
};
#endif

#endif //	__cplusplus

#endif // __CNX_BASEFILTER_H__

