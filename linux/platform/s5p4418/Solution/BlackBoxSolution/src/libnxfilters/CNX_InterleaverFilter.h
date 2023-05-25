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

#include <stdint.h>
#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>

#ifndef __CNX_INTERLEAVERFIlTER_H__
#define __CNX_INTERLEAVERFIlTER_H__

#ifdef __cplusplus

class	CNX_InterleaverQueue
{
public:
	CNX_InterleaverQueue()
	{
		m_iHeadIndex 	= 0;
		m_iTailIndex 	= 0;
		m_iSampleCount  = 0;
		m_iQueueDepth	= SAMPLE_QUEUE_COUNT;
		for(int32_t i = 0; i < SAMPLE_QUEUE_COUNT; i++)
			m_pSamplePool[i] = NULL;
		pthread_mutex_init( &m_hSampleQLock, NULL );
	}

	virtual ~CNX_InterleaverQueue()
	{
		pthread_mutex_destroy( &m_hSampleQLock );
	};

public:
	virtual void PushSample( CNX_Sample *pSample )
	{
		pthread_mutex_lock( &m_hSampleQLock );

		if( m_iQueueDepth <= m_iSampleCount ) {
			pthread_mutex_unlock( &m_hSampleQLock );
			return ;
		}

		m_pSamplePool[m_iTailIndex] = pSample;
		m_iTailIndex = (m_iTailIndex+1) % SAMPLE_QUEUE_COUNT;
		m_iSampleCount++;

		pthread_mutex_unlock( &m_hSampleQLock );
	}

	virtual void PopSample( CNX_Sample **ppSample )
	{
		pthread_mutex_lock( &m_hSampleQLock );

		if( m_iSampleCount <= 0 ) {
			pthread_mutex_unlock( &m_hSampleQLock );
			return ;
		}

		*ppSample = m_pSamplePool[m_iHeadIndex];
		m_pSamplePool[m_iHeadIndex] = NULL;
		m_iHeadIndex = (m_iHeadIndex+1) % SAMPLE_QUEUE_COUNT;
		m_iSampleCount--;

		pthread_mutex_unlock( &m_hSampleQLock );
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

	virtual void Reset( void )
	{
		pthread_mutex_lock( &m_hSampleQLock );
		m_iHeadIndex 	= 0;
		m_iTailIndex 	= 0;
		m_iSampleCount 	= 0;
		for(int32_t i = 0; i < SAMPLE_QUEUE_COUNT; i++)
			m_pSamplePool[i] = NULL;
		pthread_mutex_unlock( &m_hSampleQLock );
	}

	virtual	int32_t	GetSampleCount( void )
	{
		int32_t sampleCount = 0;
		pthread_mutex_lock( &m_hSampleQLock );
		sampleCount = m_iSampleCount;
		pthread_mutex_unlock( &m_hSampleQLock );	
		return sampleCount;
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
	virtual uint64_t GetSampleTimeStamp( void )
	{
		CNX_Sample *pSample = NULL;
		pthread_mutex_lock( &m_hSampleQLock );
		if( m_iSampleCount == 0 ) {
			pthread_mutex_unlock( &m_hSampleQLock );
			return 0;
		}
		pSample = m_pSamplePool[m_iHeadIndex];
		pthread_mutex_unlock( &m_hSampleQLock );
		
		if( !pSample ) {
			printf("%s(): pSample is NULL\n", __func__);
			return 0;
		}

		return pSample->GetTimeStamp();
	}

	virtual int32_t IsFull( void )
	{
		pthread_mutex_lock( &m_hSampleQLock );
		if( m_iSampleCount == (m_iQueueDepth - 1) )
		{
			pthread_mutex_unlock( &m_hSampleQLock );
			return true;
		}
		pthread_mutex_unlock( &m_hSampleQLock );
		return false;
	}

private:
	enum { SAMPLE_QUEUE_COUNT = 256 };
	CNX_Sample *m_pSamplePool[SAMPLE_QUEUE_COUNT];
	int32_t	m_iHeadIndex, m_iTailIndex, m_iSampleCount;
	int32_t m_iQueueDepth;
	pthread_mutex_t m_hSampleQLock;

private:
	CNX_InterleaverQueue (const CNX_InterleaverQueue &Ref);
	CNX_InterleaverQueue &operator=(const CNX_InterleaverQueue &Ref);
};

class	CNX_InterleaverFilter
		: public CNX_BaseFilter
{
public:
	CNX_InterleaverFilter();
	~CNX_InterleaverFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_INTERLEAVER_CONFIG *pConfig );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );
	//------------------------------------------------------------------------

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t 	GetStatistics( NX_FILTER_STATISTICS *pStatistics );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_CHANNEL = 4 };

	CNX_InterleaverQueue	m_InterleaverQueue[MAX_CHANNEL];
	uint32_t				m_InterleaverChannel;

	pthread_mutex_t			m_hReceiveLock;
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	pthread_mutex_t			m_hStatisticsLock;
	NX_FILTER_STATISTICS	m_FilterStatistics;
};

#endif	// __cplusplus

#endif	// __CNX_INTERLEAVERFIlTER_H__
