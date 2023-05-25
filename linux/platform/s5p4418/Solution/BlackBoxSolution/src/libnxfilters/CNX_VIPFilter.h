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

#ifndef __CNX_VIPFILTER_H__
#define __CNX_VIPFILTER_H__

#include <nx_vip.h>
#include <nx_alloc_mem.h>
#include <nx_fourcc.h>

#include "CNX_RefClock.h"
#include "CNX_BaseFilter.h"
#include "INX_JpegCapture.h"
#include "NX_FilterConfigTypes.h"

#ifdef __cplusplus

class	CNX_VIPFilter
	: public CNX_BaseFilter
{
public:
	CNX_VIPFilter();
	virtual ~CNX_VIPFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_VIP_CONFIG *pConfig );
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
	virtual void		AllocateBuffer( int32_t width, int32_t height, int32_t alignx, int32_t aligny, int32_t numOfBuffer, uint32_t dwFourCC );
	virtual void		FreeBuffer( void );

	virtual	int32_t		GetSample( CNX_Sample **ppSample );
	virtual	int32_t		GetDeliverySample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

			int32_t		(*JpegFileNameFunc)( uint8_t *buf, uint32_t bufSize );
			void 		JpegEncode( CNX_Sample *pSample );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			void 		SetJpegFileName( uint8_t *pFileName );
			int32_t		EnableCapture( uint32_t enable );
			int32_t		RegJpegFileNameCallback( int32_t(*cbFunc)( uint8_t *, uint32_t ) );
			int32_t  	GetStatistics( NX_FILTER_STATISTICS *pStatistics );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t					m_bInit;
	int32_t					m_bRun;
	CNX_Semaphore			*m_pSemOut;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------	
	int32_t					m_bThreadExit;
	pthread_t				m_hThread;
	//------------------------------------------------------------------------
	//	Video Input
	//------------------------------------------------------------------------
	VIP_HANDLE				m_hVip;
	VIP_INFO				m_VipInfo;	
	uint32_t				m_FourCC;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------	
	enum { MAX_BUFFER = 16, NUM_ALLOC_BUFFER = 12, NUM_PUSHED_BUFFER = 6 };
	// enum { MAX_BUFFER = 16, NUM_ALLOC_BUFFER = 6 };
	int32_t					m_iNumOfBuffer;
	NX_VID_MEMORY_HANDLE	m_VideoMemory[MAX_BUFFER];
	CNX_VideoSample			m_VideoSample[MAX_BUFFER];
	CNX_SampleQueue			m_SampleOutQueue;
	//------------------------------------------------------------------------
	//	For Capture
	//------------------------------------------------------------------------
	INX_JpegCapture			*m_pJpegCapture;
	uint8_t					m_JpegFileName[1024];
	int32_t					m_bCaptured;
	pthread_mutex_t			m_hCaptureLock;
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	CNX_Statistics			*m_pOutStatistics;
};

#endif //	__cplusplus

#endif // __CNX_VIPFILTER_H__

