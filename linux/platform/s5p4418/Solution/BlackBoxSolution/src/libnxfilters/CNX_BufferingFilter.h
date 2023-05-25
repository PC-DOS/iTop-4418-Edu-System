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

#ifndef __CNX_EVENTBUFFERINGFILTER_H__
#define __CNX_EVENTBUFFERINGFILTER_H__

#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>

#ifdef __cplusplus

enum {
	BUFFERING_MODE_BOTH,
	BUFFERING_MODE_EVENT_ONLY,
};

enum {
	STATUS_NORMAL_WRITING,
	STATUS_EVENT_WRITING,
};

class	CNX_BufferingFilter
	: public CNX_BaseFilter
{
public:
	CNX_BufferingFilter();
	virtual ~CNX_BufferingFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_BUFFERING_CONFIG *pConfig );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------	
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );
	//------------------------------------------------------------------------

protected:
	virtual int32_t		AllocateMemory();
	virtual int32_t		FreeMemory(void);
	
	virtual int32_t		GetSample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static	void*		ThreadMain( void *arg );

private:
			int32_t		DeliverFlagSample( int32_t flags );
			int32_t		PushStream( CNX_MuxerSample *pSample );
			int32_t		PopStream( void );
			int32_t		PopStreamAll( void );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t 	ChangeBufferingMode( int32_t mode );
			int32_t		StartFile( void );
			int32_t		StopFile( void );
			int32_t		ChangeFile( void );
			int32_t		PopBufferdData( int32_t bPopBufferdData );
			uint64_t 	GetTimeStamp( void );
			int32_t 	GetStatistics( NX_FILTER_STATISTICS *pStatistics );
protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	CNX_Semaphore		*m_pSemIn;

	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFERD_TIME = 20 };

	CNX_SampleQueue		m_StreamQueue;		// Temp Queue
	CNX_SampleQueue		m_EventQueue[MAX_BUFFERD_TIME];
	CNX_SampleQueue		m_SampleInQueue;
	
	int32_t				m_EventBufferedTime;
	int32_t				m_EventCount, m_EventHead, m_EventTail;
	int64_t				m_PrvTimeStamp, m_CurTimeStamp;
	int32_t				m_bPopBufferdData;
	int32_t				m_PrevStatus;

	pthread_mutex_t		m_hBufferedLock;
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	CNX_Statistics		*m_pInStatistics;
	int32_t				m_Flags;
	int32_t				m_BufferingMode;
	int32_t				m_Status;
	int32_t				m_bStartFile;
	int32_t				m_bStopFile;
	int32_t				m_bChangeFile;
};

#endif //	__cplusplus

#endif //	__CNX_EVENTBUFFERINGFILTER_H__

