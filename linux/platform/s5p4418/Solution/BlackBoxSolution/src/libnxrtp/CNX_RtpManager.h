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

#ifndef __CNX_RTPMANAGER_H__
#define __CNX_RTPMANAGER_H__

#include <CNX_RefClock.h>
#include <CNX_VIPFilter.h>
#include <CNX_VRFilter.h>
#include <CNX_H264Encoder.h>
#include <CNX_RTPFilter.h>
#include <NX_FilterConfigTypes.h>

#include "INX_RtpManager.h"

class CNX_RtpManager
	: public INX_RtpManager
{
public:
	CNX_RtpManager();
	~CNX_RtpManager();

public:
	virtual int32_t	Init( NX_RTP_MGR_CONFIG *pConfig );
	virtual int32_t	Deinit( void );

	virtual int32_t	Start( void );
	virtual int32_t	Stop( void );

	virtual int32_t RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) );

private:
	int32_t	SetConfig( NX_RTP_MGR_CONFIG *pConfig );
	int32_t	BuildFilter( void );
	void	SetNotifier( void );

private:
	CNX_RefClock 			*m_pRefClock;
	CNX_RtpNotify			*m_pNotifier;

	CNX_VIPFilter			*m_pVipFilter;
	CNX_VRFilter			*m_pVrFilter;
	CNX_H264Encoder			*m_pAvcEncFilter;
	CNX_RTPFilter 			*m_pRtpFilter;

	// Configuration
	NX_VIP_CONFIG			m_VipConfig;
	NX_VIDRENDER_CONFIG		m_VidRenderConfig;
	NX_VIDENC_CONFIG		m_VidEncConfig;
	NX_RTP_CONFIG			m_RtpConfig;

private:
	int32_t					m_bInit;
	int32_t					m_bRun;

	pthread_mutex_t			m_hLock;	
};

INX_RtpManager *GetRtpHandle( void )
{
	return (INX_RtpManager *)new CNX_RtpManager();
}

void ReleaseRtpHandle(INX_RtpManager *iRtpMgr)
{
	if( iRtpMgr ) delete (INX_RtpManager*)iRtpMgr;
}

#endif	// __CNX_RTPMANAGER_H__
