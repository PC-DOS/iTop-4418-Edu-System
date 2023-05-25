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

#ifndef __CNX_HLSMANAGER_H__
#define __CNX_HLSMANAGER_H__

#include <CNX_RefClock.h>
#include <CNX_VIPFilter.h>
#include <CNX_VRFilter.h>
#include <CNX_H264Encoder.h>
#include <CNX_AudCaptureFilter.h>
#include <CNX_AacEncoder.h>
#include <CNX_TsMuxerFilter.h>
#include <CNX_HLSFilter.h>
#include <NX_FilterConfigTypes.h>

#include "INX_HlsManager.h"

class CNX_HlsManager
	: public INX_HlsManager
{
public:
	CNX_HlsManager();
	~CNX_HlsManager();

public:
	virtual int32_t	Init( NX_HLS_MGR_CONFIG *pConfig );
	virtual int32_t	Deinit( void );

	virtual int32_t	Start( void );
	virtual int32_t	Stop( void );

	virtual int32_t RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) );

private:
	int32_t	SetConfig( NX_HLS_MGR_CONFIG *pConfig );
	int32_t	BuildFilter( void );
	void	SetNotifier( void );

private:
	CNX_RefClock 			*m_pRefClock;
	CNX_HlsNotify			*m_pNotifier;

	CNX_VIPFilter			*m_pVipFilter;
	CNX_VRFilter			*m_pVrFilter;
	CNX_H264Encoder			*m_pAvcEncFilter;

	CNX_AudCaptureFilter	*m_pAudCapFilter;
	CNX_AacEncoder			*m_pAacEncFilter;

	CNX_TsMuxerFilter 		*m_pTsMuxerFilter;
	CNX_HLSFilter 			*m_pHlsFilter;

	// Configuration
	NX_VIP_CONFIG			m_VipConfig;
	NX_VIDRENDER_CONFIG		m_VidRenderConfig;
	NX_VIDENC_CONFIG		m_VidEncConfig;

	NX_AUDCAPTURE_CONFIG	m_AudCapConfig;
	NX_AUDENC_CONFIG		m_AudEncConfig;

	NX_TSMUXER_CONFIG		m_TsMuxerConfig;
	NX_HLS_CONFIG			m_HlsConfig;

private:
	int32_t					m_bInit;
	int32_t					m_bRun;

	pthread_mutex_t			m_hLock;	
};

INX_HlsManager *GetHlsHandle( void )
{
	return (INX_HlsManager *)new CNX_HlsManager();
}

void ReleaseHlsHandle(INX_HlsManager *iHlsMgr)
{
	if( iHlsMgr ) delete (CNX_HlsManager*)iHlsMgr;
}

#endif	// __CNX_HLSMANAGER_H__
