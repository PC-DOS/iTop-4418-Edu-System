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

#include "CNX_TranscodingNotify.h"
#include "CNX_TranscodingManager.h"

#define NX_DTAG		"[CNX_TranscodingManager]"
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
CNX_TranscodingManager::CNX_TranscodingManager()
	: m_pRefClock( NULL )
	, m_pNotifier( NULL )
	, m_pDecoder( NULL )
	, m_pVidRender( NULL )
	, m_pEncoder( NULL )
	, m_pMp4Muxer( NULL )
	, m_pInFileName( NULL )
	, m_pOutFileName( NULL )
{
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_TranscodingManager::~CNX_TranscodingManager()
{
	if( m_bInit ) Deinit();
	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::BuildFilter( void )
{
	m_pRefClock		= new CNX_RefClock();
	m_pNotifier 	= new CNX_TranscodingNotify();

	m_pDecoder		= new CNX_VideoDecoder();
	m_pVidRender 	= new CNX_VRFilter();
	m_pEncoder		= new CNX_H264Encoder();
	m_pMp4Muxer		= new CNX_Mp4MuxerFilter();

	if( m_pDecoder )	m_pDecoder->Connect( m_pVidRender );
	if( m_pVidRender )	m_pVidRender->Connect( m_pEncoder );
	if( m_pEncoder )	m_pEncoder->Connect( m_pMp4Muxer );

	int32_t width = 0, height = 0;
	int32_t fpsNum = 0, fpsDen = 0, fps = 0;

	if( m_pDecoder ) {
		m_pDecoder->Init();
		m_pDecoder->SetFileName( m_pInFileName );
		m_pDecoder->GetVideoResolution( &width, &height );
		fps = m_pDecoder->GetVideoFramerate( &fpsNum, &fpsDen );
		
		if( fps > (int32_t)m_VidEncConfig.fps && 0 < (int32_t)m_VidEncConfig.fps ) {
			m_pDecoder->SetFrameDown( m_VidEncConfig.fps );
		}

		m_VidRenderConfig.width		= width;
		m_VidRenderConfig.height 	= height;

		m_VidEncConfig.width		= width;
		m_VidEncConfig.height		= height;

		m_Mp4MuxerConfig.trackConfig[0].width	= width;
		m_Mp4MuxerConfig.trackConfig[0].height	= height;
	}		

	if( m_pVidRender ) {
		if( width && height ) {
			m_VidRenderConfig.cropRight		= width;
			m_VidRenderConfig.cropBottom	= height;
		}
		m_pVidRender->Init( &m_VidRenderConfig );
	}

	if( m_pEncoder ) {
		if( !m_VidEncConfig.fps )
			m_VidEncConfig.fps = fps;
		
		m_pEncoder->Init( &m_VidEncConfig );
	}	
	
	if( m_pMp4Muxer ) {
		if( !m_Mp4MuxerConfig.trackConfig[0].frameRate ) 
			m_Mp4MuxerConfig.trackConfig[0].frameRate = fps;
		
		m_pMp4Muxer->Init( &m_Mp4MuxerConfig );
		if( m_pOutFileName ) 
			m_pMp4Muxer->SetFileName( (char*)m_pOutFileName );
	}
	
	uint8_t dsiInfo[1024] = { 0x00, };
	int32_t dsiSize = 0;
	if( m_pEncoder )	m_pEncoder->SetPacketID( 0 );
	if( m_pEncoder )	m_pEncoder->GetDsiInfo( dsiInfo, &dsiSize );
	if( m_pMp4Muxer )	m_pMp4Muxer->SetDsiInfo( 0, dsiInfo, dsiSize );

	return 0;
}

//------------------------------------------------------------------------------
#ifndef SET_EVENT_NOTIFIER
#define SET_EVENT_NOTIFIER(A, B)	if(A){(A)->SetNotifier((INX_EventNotify *)B);};
#endif
void CNX_TranscodingManager::SetNotifier( void )
{
	if( m_pNotifier )
	{
		SET_EVENT_NOTIFIER( m_pDecoder,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pVidRender,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pEncoder,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pMp4Muxer,	m_pNotifier );
	}
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::SetConfig( NX_TRANSCODING_MGR_CONFIG *pConfig )
{
	if( pConfig->pInFileName ) m_pInFileName = pConfig->pInFileName;
	if( pConfig->pOutFileName ) m_pOutFileName = pConfig->pOutFileName; 

	memset( &m_VidRenderConfig, 0x00, sizeof(m_VidRenderConfig) );
	m_VidRenderConfig.port 			= 0;
	m_VidRenderConfig.cropLeft		= 0;
	m_VidRenderConfig.cropTop		= 0;
	m_VidRenderConfig.cropRight		= 1024;		// Resetting BuildFilter()
	m_VidRenderConfig.cropBottom	= 768;		// Resetting BuildFilter()
	m_VidRenderConfig.dspLeft		= 0;
	m_VidRenderConfig.dspTop		= 0;
	m_VidRenderConfig.dspRight		= 1024;
	m_VidRenderConfig.dspBottom		= 768;

	memset( &m_VidEncConfig, 0x00, sizeof(m_VidEncConfig) );
	m_VidEncConfig.fps				= pConfig->nEncFps;
	m_VidEncConfig.bitrate			= pConfig->nEncBitrate;
	m_VidEncConfig.codec			= 0x21;

	memset( &m_Mp4MuxerConfig, 0x00, sizeof(m_Mp4MuxerConfig) );
	m_Mp4MuxerConfig.videoTrack		= 1;
	m_Mp4MuxerConfig.audioTrack		= 0;
	m_Mp4MuxerConfig.textTrack		= 0;

	m_Mp4MuxerConfig.trackConfig[0].frameRate	= pConfig->nEncFps;
	m_Mp4MuxerConfig.trackConfig[0].bitrate		= pConfig->nEncBitrate;
	m_Mp4MuxerConfig.trackConfig[0].codecType	= 0x21;

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::Init( NX_TRANSCODING_MGR_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );
	int32_t ret = 0;

	SetConfig( pConfig );

	if( !(ret = BuildFilter()) )
	{
		SetNotifier();
		m_bInit = true;
	}

	return ret;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit )
	{
		SAFE_DEINIT_FILTER( m_pDecoder );
		SAFE_DEINIT_FILTER( m_pVidRender );
		SAFE_DEINIT_FILTER( m_pEncoder );
		SAFE_DEINIT_FILTER( m_pMp4Muxer );

		SAFE_DELETE_FILTER( m_pDecoder );
		SAFE_DELETE_FILTER( m_pVidRender );
		SAFE_DELETE_FILTER( m_pEncoder );
		SAFE_DELETE_FILTER( m_pMp4Muxer );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;	
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::Start()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit )
	{
		SAFE_START_FILTER( m_pDecoder );
		SAFE_START_FILTER( m_pVidRender );
		SAFE_START_FILTER( m_pEncoder );
		SAFE_START_FILTER( m_pMp4Muxer );

		if( m_pVidRender )	m_pVidRender->EnableRender( true );
		if( m_pMp4Muxer )	m_pMp4Muxer->EnableMp4Muxing( true );

		m_bRun = true;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( m_bRun )
	{
		SAFE_STOP_FILTER( m_pDecoder );
		SAFE_STOP_FILTER( m_pVidRender );
		SAFE_STOP_FILTER( m_pEncoder );
		SAFE_STOP_FILTER( m_pMp4Muxer );

		if( m_pVidRender )	m_pVidRender->EnableRender( false );
		if( m_pMp4Muxer )	m_pMp4Muxer->EnableMp4Muxing( false );

		m_bRun = false;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TranscodingManager::RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) )
{
	CNX_AutoLock lock( &m_hLock );
	
	if( cbNotify )
	{
		m_pNotifier->RegisterNotifyCallback( cbNotify );
	}

	return 0;
}
