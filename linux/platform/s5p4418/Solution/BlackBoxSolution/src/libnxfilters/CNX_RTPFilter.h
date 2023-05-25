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

#ifndef __CNX_RTPFILTER_H__
#define __CNX_RTPFILTER_H__

#include <CNX_BaseFilter.h>
#include "CNX_DynamicRTSPServer.h"

#include "NX_FilterConfigTypes.h"

#include <queue>

#ifdef __cplusplus

class CNX_LiveSource;

class CNX_RTPFilter
	: public CNX_BaseFilter
{
public:
	CNX_RTPFilter();
	virtual ~CNX_RTPFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_RTP_CONFIG *pConfig );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );
	
protected:
	//------------------------------------------------------------------------
	virtual void		AllocateMemory( void );
	virtual void		FreeMemory( void );

	virtual int32_t		GetSample( CNX_Sample **ppSample );
	virtual int32_t		GetDeliverySample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

private:
			int32_t		CreateSample( CNX_Sample *pSrcSample, CNX_Sample *pDstSample );
			int32_t		DestorySample( CNX_Sample *pSample );
			
public:
			int32_t		SetSourceInstance( CNX_LiveSource *pLiveSource, int32_t type );
			int32_t		ClearSourceInstance( CNX_LiveSource *pLiveSource );
			int32_t		ConnectIsReady( void );

private:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t					m_bInit;
	int32_t					m_bRun;

	pthread_mutex_t			m_hLock;
	
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t					m_bThreadExit;
	pthread_t				m_hThread;

	//------------------------------------------------------------------------
	//	RTP / RTSP
	//------------------------------------------------------------------------
	NX_RTP_CONFIG			m_RtpConfig;

	TaskScheduler			*m_pScheduler;
	UsageEnvironment		*m_pEnv;
	RTSPServer				*m_pRtspServer;
	portNumBits 			m_iRtspPortNum;

	char	 				m_iWatchFlag;

	CNX_LiveSource			*m_pLiveSource[MAX_SESSION_NUM];
	uint32_t 				m_CurStreamType;
	int32_t					m_nCurConnectNum;
	int32_t					m_nMaxConnectNum;

	int32_t					m_nMaxSessionNum;
		
};

#endif //	__cplusplus

#endif	// __CNX_RTPFILTER_H__