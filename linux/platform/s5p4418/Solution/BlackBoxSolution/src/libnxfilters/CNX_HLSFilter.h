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

#ifndef __CNX_HLSFILTER_H__
#define __CNX_HLSFILTER_H__

#include "CNX_BaseFilter.h"
#include "NX_FilterConfigTypes.h"

#ifdef __cplusplus

#define MAX_QUEUE_SIZE			32
#define MAX_FILENAME_SIZE		256
#define ERASE_SEGMENT_MARGIN	2

class CNX_HLSFilter
	: public CNX_BaseFilter
{
public:
	CNX_HLSFilter();
	virtual ~CNX_HLSFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_HLS_CONFIG *pConfig );
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
	virtual void		AllocateBuffer( void );
	virtual void		FreeBuffer( void );

	virtual int32_t		GetSample( CNX_Sample **ppSample );
	virtual int32_t		GetDeliverySample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

private:
			int32_t		GetIPAddress( char *addr );
			void		ChangeFileName( void );
			int32_t		MakeMetaInfo( void );

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
	//	HTTP Live Streaming
	//------------------------------------------------------------------------
	enum { MAX_SEGMENT_NUM = 10 };

	//NX_HLS_CONFIG		m_HLSConfig;

	char				m_M3U8Root[256];
	char				m_M3U8Name[256];
	//char				m_M3U8FileName[256];
	int32_t				m_MaxDuration;
	int32_t				m_Duration;
	int64_t				m_MediaSequence;
	
	char				m_SegmentRoot[256];
	char				m_SegmentName[256];
	char				m_SegmentFileName[256];
	char				m_URLPrefix[256];

	int32_t				m_SegmentFileNum;

	int32_t				m_OutFd;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 32 };

	CNX_SampleQueue		m_SampleInQueue;
	int64_t				m_CurSampleTime;
	int64_t				m_LastSampleTime;
};

#endif	// __cplusplus

#endif	// __CNX_HLSFILTER_H__
