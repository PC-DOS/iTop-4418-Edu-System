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

#include "CNX_RtpNotify.h"
#include "CNX_RtpManager.h"

#define NX_DTAG		"[CNX_RtpManager] "
#include <NX_DbgMsg.h>

#ifndef	SAFE_DELETE_FILTER
#define	SAFE_DELETE_FILTER(A)	if(A){delete A;A=NULL;}
#endif
#ifndef	SAFE_START_FILTER
#define	SAFE_START_FILTER(A)    if(A){A->Run();};
#endif
#ifndef	SAFE_STOP_FILTER
#define	SAFE_STOP_FILTER(A)     if(A){A->Stop();};
#endif
#ifndef	SAFE_DEINIT_FILTER
#define	SAFE_DEINIT_FILTER(A)   if(A){A->Deinit();};
#endif

//------------------------------------------------------------------------------
CNX_RtpManager::CNX_RtpManager()
	: m_pRefClock( NULL )
	, m_pNotifier( NULL )
	, m_pVipFilter( NULL )
	, m_pVrFilter( NULL )
	, m_pAvcEncFilter( NULL )
	, m_pRtpFilter( NULL )
	, m_bInit( 0 )
	, m_bRun( 0 )
{
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_RtpManager::~CNX_RtpManager()
{
	if( m_bInit ) Deinit();
	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::BuildFilter( void )
{
	m_pRefClock			= new CNX_RefClock();
	m_pNotifier 		= new CNX_RtpNotify();

	m_pVipFilter		= new CNX_VIPFilter();
	m_pVrFilter			= new CNX_VRFilter();
	m_pAvcEncFilter 	= new CNX_H264Encoder();
	m_pRtpFilter		= new CNX_RTPFilter();

	m_pVipFilter->Connect( m_pVrFilter );
	m_pVrFilter->Connect( m_pAvcEncFilter );
	m_pAvcEncFilter->Connect( m_pRtpFilter );

	m_pVipFilter->Init( &m_VipConfig );
	m_pVrFilter->Init( &m_VidRenderConfig );
	m_pAvcEncFilter->Init( &m_VidEncConfig );
	m_pRtpFilter->Init( &m_RtpConfig );

	// etc configuration
	m_pVrFilter->EnableRender( true );
	m_pAvcEncFilter->SetPacketID( 0 );

	return 0;
}

//------------------------------------------------------------------------------
#ifndef SET_EVENT_NOTIFIER
#define SET_EVENT_NOTIFIER(A, B)	if(A){(A)->SetNotifier((INX_EventNotify *)B);};
#endif
void CNX_RtpManager::SetNotifier( void )
{
	if( m_pNotifier )
	{
		SET_EVENT_NOTIFIER( m_pVipFilter,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pVrFilter,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAvcEncFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pRtpFilter,		m_pNotifier );
	}
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::SetConfig( NX_RTP_MGR_CONFIG *pConfig )
{
	memset( &m_VipConfig, 0x00, sizeof(m_VipConfig) );
	m_VipConfig.port				= pConfig->nPort;
	m_VipConfig.width				= pConfig->nWidth;
	m_VipConfig.height				= pConfig->nHeight;
	m_VipConfig.fps					= pConfig->nFps;

	memset( &m_VidRenderConfig, 0x00, sizeof(m_VidRenderConfig) );
	m_VidRenderConfig.port 			= 0;
	m_VidRenderConfig.width			= pConfig->nWidth;
	m_VidRenderConfig.height 		= pConfig->nHeight;
	m_VidRenderConfig.cropLeft		= 0;
	m_VidRenderConfig.cropTop		= 0;
	m_VidRenderConfig.cropRight		= pConfig->nWidth;
	m_VidRenderConfig.cropBottom	= pConfig->nHeight;
	m_VidRenderConfig.dspLeft		= 0;
	m_VidRenderConfig.dspTop		= 0;
	m_VidRenderConfig.dspRight		= pConfig->nDspWidth;
	m_VidRenderConfig.dspBottom		= pConfig->nDspHeight;

	memset( &m_VidEncConfig, 0x00, sizeof(m_VidEncConfig) );
	m_VidEncConfig.width			= pConfig->nWidth;
	m_VidEncConfig.height			= pConfig->nHeight;
	m_VidEncConfig.fps				= pConfig->nFps;
	m_VidEncConfig.bitrate			= pConfig->nBitrate;
	m_VidEncConfig.codec			= 0x21;

	memset( &m_RtpConfig, 0x00, sizeof(m_RtpConfig) );
	m_RtpConfig.port				= 554;
	m_RtpConfig.sessionNum			= 1;
	m_RtpConfig.connectNum			= 2;
	sprintf((char*)m_RtpConfig.sessionName[0], "video0");
	sprintf((char*)m_RtpConfig.sessionName[1], "video1");
	
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::Init( NX_RTP_MGR_CONFIG *pConfig )
{
	CNX_AutoLock lock( &m_hLock );
	int32_t ret = 0;

	SetConfig( pConfig );

	if( !(ret = BuildFilter()) )
	{
		SetNotifier();
		m_bInit = true;
	}

	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::Deinit( void )
{
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit )
	{
		SAFE_DEINIT_FILTER( m_pVipFilter );
		SAFE_DEINIT_FILTER( m_pVrFilter );
		SAFE_DEINIT_FILTER( m_pAvcEncFilter );
		SAFE_DEINIT_FILTER( m_pRtpFilter );

		SAFE_DELETE_FILTER( m_pVipFilter );
		SAFE_DELETE_FILTER( m_pVrFilter );
		SAFE_DELETE_FILTER( m_pAvcEncFilter );
		SAFE_DELETE_FILTER( m_pRtpFilter );
	}

	return 0;	
}

int32_t CNX_RtpManager::Start( void )
{
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit )
	{
		SAFE_START_FILTER( m_pVipFilter );
		SAFE_START_FILTER( m_pVrFilter );
		SAFE_START_FILTER( m_pAvcEncFilter );
		SAFE_START_FILTER( m_pRtpFilter );

		m_bRun = true;
	}

	return 0;
}

int32_t CNX_RtpManager::Stop( void )
{
	
	if( m_bRun )
	{
		SAFE_STOP_FILTER( m_pVipFilter );
		SAFE_STOP_FILTER( m_pVrFilter );
		SAFE_STOP_FILTER( m_pAvcEncFilter );
		SAFE_STOP_FILTER( m_pRtpFilter );

		m_bRun = false;
	}

	return 0;
}

int32_t CNX_RtpManager::RegisterNotifyCallback(uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t)
)
{
	CNX_AutoLock lock( &m_hLock );
	
	if( cbNotify )
	{
		m_pNotifier->RegisterNotifyCallback( cbNotify );
	}

	return 0;
}
