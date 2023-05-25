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

#include <stdio.h>

#include <CNX_RefClock.h>
#include <CNX_VIPFilter.h>
#include <CNX_MotionDetectFilter.h>
#include <CNX_ImageEffectFilter.h>
#include <CNX_TextOverlayFilter.h>
#include <CNX_VRFilter.h>
#include <CNX_H264Encoder.h>
#include <CNX_AudCaptureFilter.h>
#include <CNX_AacEncoder.h>
#include <CNX_Mp3Encoder.h>
#include <CNX_UserDataFilter.h>
#include <CNX_InterleaverFilter.h>
#include <CNX_BufferingFilter.h>
#include <CNX_Mp4MuxerFilter.h>
#include <CNX_TsMuxerFilter.h>
#include <CNX_FileWriter.h>
#include <CNX_SimpleFileWriter.h>
#include <CNX_HLSFilter.h>
#include <CNX_RTPFilter.h>

#include "CNX_DvrNotify.h"
#include "CNX_DvrManager.h"

#include "NX_DvrManagerBuildInfo.h"

#define NX_DTAG		"[CNX_DvrManager] "
#include <NX_DbgMsg.h>

#define HLS_SUPPORT

#ifndef SAFE_CONNECT_FILTER
#define SAFE_CONNECT_FILTER(A, B)	if(A){A->Connect(B);}
#endif
#ifndef	SAFE_DELETE_FILTER
#define	SAFE_DELETE_FILTER(A)		if(A){delete A;A = NULL;}
#endif
#ifndef	SAFE_START_FILTER
#define	SAFE_START_FILTER(A)		if(A){A->Run();}
#endif
#ifndef	SAFE_STOP_FILTER
#define	SAFE_STOP_FILTER(A)			if(A){A->Stop();}
#endif
#ifndef	SAFE_DEINIT_FILTER
#define	SAFE_DEINIT_FILTER(A)		if(A){A->Deinit();}
#endif
#ifndef SET_EVENT_NOTIFIER
#define SET_EVENT_NOTIFIER(A, B)	if(A){(A)->SetNotifier((INX_EventNotify *)B);}
#endif

enum {
	NX_WRITING_MODE_NONE,
	NX_WRITING_MODE_NORMAL,
	NX_WRITING_MODE_EVENT,
	NX_WRITING_MODE_EVENT_WAIT,	
};

//------------------------------------------------------------------------------
CNX_DvrManager::CNX_DvrManager()
	: m_pRefClock( NULL )
	, m_pNotifier( NULL )
	, m_pInterleaverFilter( NULL )
	, m_pBufferingFilter( NULL )
	, m_pMp4MuxerFilter( NULL )
	, m_pTsMuxerFilter( NULL )
	, m_pFileWriter( NULL )
	, m_pHlsFilter( NULL )
	, m_pRtpFilter( NULL )
	, m_bInit( false )
	, m_bRun( false )
	, m_nMode( DVR_ENCODE_NORMAL )
	, m_bChageMode( false )
	, m_bEvent( false )
	, m_VideoNum( 0 )
	, m_AudioNum( 0 )
	, m_TextNum( 0 )
	, m_VideoCodec( DVR_CODEC_H264 )
	, m_AudioCodec( DVR_CODEC_AAC )
	, m_Container( DVR_CONTAINER_MP4 )
	, m_DisplayChannel( 0 )
	, m_NormalDuration( 0 )
	, m_EventDuration( 0 )
	, m_EventBufferDuration( 0 )
	, m_bThreadExit( false )
	, NormalFileNameCallbackFunc( NULL )
	, EventFileNameCallbackFunc( NULL )
	, ParkingFileNameCallbackFunc( NULL )
{
	for( int i = 0; i < MAX_VID_NUM; i++ )
	{
		m_pVipFilter[i]     = NULL;
		m_pMdFilter[i]		= NULL;
		m_pOverlayFilter[i] = NULL;
		m_pVrFilter[i]		= NULL;
		m_pAvcEncFilter[i]  = NULL;
		m_pEffectFilter[i]	= NULL;
	}
	
	for( int i = 0 ; i<MAX_AUD_NUM ; i++ )
	{
		m_pAudCapFilter[i] = NULL;
		m_pAacEncFilter[i] = NULL;
		m_pMp3EncFilter[i] = NULL;
	}
	
	for( int i=0 ; i<MAX_AUD_NUM ; i++ )
	{
		m_pUserDataFilter[i] = NULL;
	}
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_DvrManager::~CNX_DvrManager()
{
	if( m_bInit ) Deinit();
	pthread_mutex_destroy( &m_hLock );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private Function
//

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::BuildFilter( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	// Create Filter
	m_pRefClock					= new CNX_RefClock();
	m_pNotifier					= new CNX_DvrNotify();
	
	for( int32_t i = 0; i < m_VideoNum; i++ )
	{
		m_pVipFilter[i]			= new CNX_VIPFilter();
		m_pOverlayFilter[i]		= new CNX_TextOverlayFilter();
		m_pVrFilter[i]			= new CNX_VRFilter();
		
		if( m_VideoCodec == DVR_CODEC_H264) {
			m_pAvcEncFilter[i]	= new CNX_H264Encoder();
		}
		else {
			//m_pMp4EncFilter[i]	= new CNX_Mp4Encoder();
		}
		
		if( m_MdEnable[i] ) 
			m_pMdFilter[i]		= new CNX_MotionDetectFilter();

		if( m_ExternProc[i] )
			m_pEffectFilter[i]	= new CNX_ImageEffectFilter();
	}
	
	for( int32_t i = 0; i < m_AudioNum; i++ )
	{
		m_pAudCapFilter[i]		= new CNX_AudCaptureFilter();
		if( m_AudioCodec == DVR_CODEC_AAC ) {
			m_pAacEncFilter[i]	= new CNX_AacEncoder();
		}
		else if( m_AudioCodec == DVR_CODEC_MP3 ) {
			m_pMp3EncFilter[i]	= new CNX_Mp3Encoder();
		}
	}

	for( int32_t i = 0; i < m_TextNum; i++ )
	{
		m_pUserDataFilter[i]	= new CNX_UserDataFilter();
	}
	
	m_pInterleaverFilter		= new CNX_InterleaverFilter();
	m_pBufferingFilter			= new CNX_BufferingFilter();
	
	if( m_Container == DVR_CONTAINER_MP4 ) {
		m_pMp4MuxerFilter		= new CNX_Mp4MuxerFilter();
		
		if( m_RtpEnable )
			m_pRtpFilter		= new CNX_RTPFilter();
	}
	else if( m_Container == DVR_CONTAINER_TS ) {
		m_pTsMuxerFilter		= new CNX_TsMuxerFilter();
#ifdef SIMPLE_WRITER
		m_pFileWriter			= new CNX_SimpleFileWriter();
#else
		m_pFileWriter			= new CNX_FileWriter();
#endif		
		
		if( m_HlsEnable )
			m_pHlsFilter		= new CNX_HLSFilter();

		if( m_RtpEnable )
			m_pRtpFilter		= new CNX_RTPFilter();
	}

	// Filter Connect
	for( int32_t i = 0; i < m_VideoNum; i++ ) 
	{
		if( m_ExternProc[i] && m_MdEnable[i] ) {
			SAFE_CONNECT_FILTER( m_pVipFilter[i],		m_pMdFilter[i] );
			SAFE_CONNECT_FILTER( m_pMdFilter[i], 		m_pEffectFilter[i] );
			SAFE_CONNECT_FILTER( m_pEffectFilter[i],	m_pOverlayFilter[i] );
			SAFE_CONNECT_FILTER( m_pOverlayFilter[i],	m_pVrFilter[i] );
		}
		else if( m_ExternProc[i] && !m_MdEnable[i] ) {
			SAFE_CONNECT_FILTER( m_pVipFilter[i],		m_pEffectFilter[i] );
			SAFE_CONNECT_FILTER( m_pEffectFilter[i],	m_pOverlayFilter[i] );
			SAFE_CONNECT_FILTER( m_pOverlayFilter[i],	m_pVrFilter[i] );
		}
		else if( !m_ExternProc[i] && m_MdEnable[i] ) {
			SAFE_CONNECT_FILTER( m_pVipFilter[i],		m_pMdFilter[i] );
			SAFE_CONNECT_FILTER( m_pMdFilter[i],		m_pOverlayFilter[i] );
			SAFE_CONNECT_FILTER( m_pOverlayFilter[i],	m_pVrFilter[i] );
		}
		else if( !m_ExternProc[i] && !m_MdEnable[i] ) {
			SAFE_CONNECT_FILTER( m_pVipFilter[i],		m_pOverlayFilter[i] );
			SAFE_CONNECT_FILTER( m_pOverlayFilter[i],	m_pVrFilter[i] );
		}

		if( m_VideoCodec == DVR_CODEC_H264 ) {
			SAFE_CONNECT_FILTER( m_pVrFilter[i],		m_pAvcEncFilter[i] );
			SAFE_CONNECT_FILTER( m_pAvcEncFilter[i],	m_pInterleaverFilter );
		}
		else if( m_VideoCodec == DVR_CODEC_MPEG4 ) {
			// NOT IMPLEMENTATION
			//SAFE_CONNECT_FILTER( m_pVrFilter[i],		m_pMp4EncFilter[i] );
			//SAFE_CONNECT_FILTER( m_pMp4EncFilter[i],	m_pInterleaverFilter );
		}
	}

	for( int32_t i = 0; i < m_AudioNum; i++ ) 
	{
		if( m_AudioCodec == DVR_CODEC_AAC ) {
			SAFE_CONNECT_FILTER( m_pAudCapFilter[i],	m_pAacEncFilter[i] );
			SAFE_CONNECT_FILTER( m_pAacEncFilter[i],	m_pInterleaverFilter );
		}
		else if( m_AudioCodec == DVR_CODEC_MP3 ) {
			SAFE_CONNECT_FILTER( m_pAudCapFilter[i],	m_pMp3EncFilter[i] );
			SAFE_CONNECT_FILTER( m_pMp3EncFilter[i], 	m_pInterleaverFilter );
		}
	}

	for( int32_t i = 0; i < m_TextNum; i++ )
	{
		SAFE_CONNECT_FILTER( m_pUserDataFilter[i],	m_pInterleaverFilter );
	}

	if( m_Container == DVR_CONTAINER_MP4 )
	{
		if( !m_RtpEnable ) {
			SAFE_CONNECT_FILTER( m_pInterleaverFilter,	m_pBufferingFilter );
			SAFE_CONNECT_FILTER( m_pBufferingFilter,	m_pMp4MuxerFilter );
		}
		else {
			SAFE_CONNECT_FILTER( m_pInterleaverFilter, 	m_pRtpFilter );
			SAFE_CONNECT_FILTER( m_pRtpFilter,			m_pBufferingFilter );
			SAFE_CONNECT_FILTER( m_pBufferingFilter,	m_pMp4MuxerFilter );	
		}
	}
	else if (m_Container == DVR_CONTAINER_TS )
	{
		if( !m_HlsEnable && !m_RtpEnable ) {
			SAFE_CONNECT_FILTER( m_pInterleaverFilter,	m_pTsMuxerFilter );
			SAFE_CONNECT_FILTER( m_pTsMuxerFilter,		m_pBufferingFilter );
			SAFE_CONNECT_FILTER( m_pBufferingFilter,	m_pFileWriter );
		}

		if( m_HlsEnable ) {
			SAFE_CONNECT_FILTER( m_pInterleaverFilter,	m_pTsMuxerFilter );
			SAFE_CONNECT_FILTER( m_pTsMuxerFilter,		m_pHlsFilter );
			SAFE_CONNECT_FILTER( m_pHlsFilter,			m_pBufferingFilter );
			SAFE_CONNECT_FILTER( m_pBufferingFilter,	m_pFileWriter );
		}

		if( m_RtpEnable ) {
			SAFE_CONNECT_FILTER( m_pInterleaverFilter,	m_pRtpFilter );
			SAFE_CONNECT_FILTER( m_pRtpFilter,			m_pTsMuxerFilter );
			SAFE_CONNECT_FILTER( m_pTsMuxerFilter,		m_pBufferingFilter );
			SAFE_CONNECT_FILTER( m_pBufferingFilter,	m_pFileWriter );
		}
	}
	
	// Filter Initialize
	uint8_t dsiInfo[24] = { 0x00, };
	int32_t dsiSize = 0;

	for( int32_t i = 0; i < m_VideoNum; i++ )
	{
		if( m_pVipFilter[i] )		m_pVipFilter[i]->Init( &m_VipConfig[i] );
		if( m_pMdFilter[i] )		m_pMdFilter[i]->Init( &m_MdConfig[i] );
		if( m_pEffectFilter[i] )	m_pEffectFilter[i]->Init( &m_EffectConfig[i] );
		if( m_pOverlayFilter[i] )	m_pOverlayFilter[i]->Init();
		if( m_pVrFilter[i] )		m_pVrFilter[i]->Init( &m_VidRenderConfig[i] );
		if( m_pAvcEncFilter[i] )	m_pAvcEncFilter[i]->Init( &m_VidEncConfig[i] );
		//if( m_pMp4EncFilter[i] )	m_pMp4EncFilter[i]->Init( &m_VidEncConfig[i] );


		if( m_pAvcEncFilter[i] )	m_pAvcEncFilter[i]->SetPacketID(i);
		if( m_pAvcEncFilter[i] )	m_pAvcEncFilter[i]->GetDsiInfo( dsiInfo, &dsiSize );
		//if( m_pMp4EncFilter[i] )	m_pMp4EncFilter[i]->SetPacketID(i);
		//if( m_pMp4EncFilter[i] )	m_pMp4EncFilter[i]->GetDsiInfo( dsiInfo, &dsiSize );

		if( m_pMp4MuxerFilter )		m_pMp4MuxerFilter->SetDsiInfo( i, dsiInfo, dsiSize );
	}

	for( int32_t i = 0; i < m_AudioNum; i++ )
	{
		if( m_pAudCapFilter[i] )	m_pAudCapFilter[i]->Init( &m_AudCapConfig[i] );

		// AAC
		if( m_pAacEncFilter[i] )	m_pAacEncFilter[i]->Init( &m_AudEncConfig[i] );
		if( m_pAacEncFilter[i] )	m_pAacEncFilter[i]->SetPacketID( i + m_VideoNum );
		if( m_pAacEncFilter[i] )	m_pAacEncFilter[i]->GetDsiInfo( dsiInfo, &dsiSize );

		// MP3
		if( m_pMp3EncFilter[i] )	m_pMp3EncFilter[i]->Init( &m_AudEncConfig[i] );
		if( m_pMp3EncFilter[i] )	m_pMp3EncFilter[i]->SetPacketID( i + m_VideoNum );

		if( m_pAacEncFilter[i] )
		{
			if( m_pMp4MuxerFilter )	m_pMp4MuxerFilter->SetDsiInfo( i + m_VideoNum, dsiInfo, dsiSize );
		}
	}
	for( int32_t i = 0; i < m_TextNum; i++ )
	{
		if( m_pUserDataFilter[i] )	m_pUserDataFilter[i]->Init( &m_UserDataConfig[i] );
		if( m_pUserDataFilter[i] )	m_pUserDataFilter[i]->SetPacketID(i + m_VideoNum + m_AudioNum);
	}

	{
		if( m_pInterleaverFilter )	m_pInterleaverFilter->Init( &m_InterleaverConfig );
		if( m_pBufferingFilter )	m_pBufferingFilter->Init( &m_BufferingConfig );
		if( m_pMp4MuxerFilter )		m_pMp4MuxerFilter->Init( &m_Mp4MuxerConfig );

		if( m_pTsMuxerFilter )		m_pTsMuxerFilter->Init( &m_TsMuxerConfig );
		if( m_pFileWriter )			m_pFileWriter->Init();
		if( m_pHlsFilter )			m_pHlsFilter->Init( &m_HlsConfig );
		if( m_pRtpFilter )			m_pRtpFilter->Init( &m_RtpConfig );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;	
}

//------------------------------------------------------------------------------
int32_t	CNX_DvrManager::SetConfig( NX_DVR_MEDIA_CONFIG *pMediaConfig, NX_DVR_RECORD_CONFIG *pRecordConfig, NX_DVR_DISPLAY_CONFIG *pDisplayConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	m_VideoNum 		= pMediaConfig->nVideoChannel;
	m_AudioNum		= pMediaConfig->bEnableAudio	? 1 : 0;
	m_TextNum		= pMediaConfig->bEnableUserData	? 1 : 0;
	m_VideoCodec	= pMediaConfig->videoConfig[0].nCodec;
	m_AudioCodec	= pMediaConfig->audioConfig.nCodec;
	m_Container		= pMediaConfig->nContainer;
	m_HlsEnable		= pRecordConfig->networkType == DVR_NETWORK_HLS ? 1 : 0;
	m_RtpEnable		= pRecordConfig->networkType == DVR_NETWORK_RTP ? 1 : 0;
	m_DisplayEnable	= pDisplayConfig->bEnable;
	m_DisplayChannel = pDisplayConfig->nChannel;

	for( int32_t i = 0; i < m_VideoNum; i++ )
	{
		m_VipConfig[i].port				= pMediaConfig->videoConfig[i].nPort;
		m_VipConfig[i].width			= pMediaConfig->videoConfig[i].nSrcWidth;
		m_VipConfig[i].height			= pMediaConfig->videoConfig[i].nSrcHeight;
		m_VipConfig[i].fps				= pMediaConfig->videoConfig[i].nFps;
	
		m_MdEnable[i]					= pRecordConfig->bMdEnable[i];
		m_MdConfig[i].samplingWidth		= 16;
		m_MdConfig[i].samplingHeight	= 16;
		m_MdConfig[i].threshold			= pRecordConfig->mdConfig[i].nMdThreshold;
		m_MdConfig[i].sensitivity		= pRecordConfig->mdConfig[i].nMdSensitivity;
		m_MdConfig[i].sampingFrame		= pRecordConfig->mdConfig[i].nMdSampingFrame;
	
		m_EffectConfig[i].width			= pMediaConfig->videoConfig[i].nDstWidth;
		m_EffectConfig[i].height		= pMediaConfig->videoConfig[i].nDstHeight;

		m_VidRenderConfig[i].port 		= pDisplayConfig->nModule;
		m_VidRenderConfig[i].width		= pMediaConfig->videoConfig[i].nDstWidth;
		m_VidRenderConfig[i].height		= pMediaConfig->videoConfig[i].nDstHeight;
		m_VidRenderConfig[i].cropLeft	= pDisplayConfig->cropRect.nLeft;
		m_VidRenderConfig[i].cropTop	= pDisplayConfig->cropRect.nTop;
		m_VidRenderConfig[i].cropRight	= pDisplayConfig->cropRect.nRight;
		m_VidRenderConfig[i].cropBottom	= pDisplayConfig->cropRect.nBottom;
		m_VidRenderConfig[i].dspLeft	= pDisplayConfig->dspRect.nLeft;
		m_VidRenderConfig[i].dspTop		= pDisplayConfig->dspRect.nTop;
		m_VidRenderConfig[i].dspRight	= pDisplayConfig->dspRect.nRight;
		m_VidRenderConfig[i].dspBottom	= pDisplayConfig->dspRect.nBottom;

		m_VidEncConfig[i].width			= pMediaConfig->videoConfig[i].nDstWidth;
		m_VidEncConfig[i].height		= pMediaConfig->videoConfig[i].nDstHeight;
		m_VidEncConfig[i].fps			= pMediaConfig->videoConfig[i].nFps;
		m_VidEncConfig[i].bitrate		= pMediaConfig->videoConfig[i].nBitrate;
		m_VidEncConfig[i].codec			= pMediaConfig->videoConfig[i].nCodec;

		m_ExternProc[i]					= pMediaConfig->videoConfig[i].bExternProc;
	}

	for( int32_t i = 0; i < m_AudioNum; i++ )
	{
		m_AudCapConfig[i].channels		= pMediaConfig->audioConfig.nChannel;
		m_AudCapConfig[i].frequency		= pMediaConfig->audioConfig.nFrequency;
		m_AudCapConfig[i].samples		= (pMediaConfig->audioConfig.nCodec == DVR_CODEC_AAC) ? FRAME_SIZE_AAC : FRAME_SIZE_MP3;	// AAC(1024), MP3(1152), AC3(1536)

		m_AudEncConfig[i].channels		= pMediaConfig->audioConfig.nChannel;
		m_AudEncConfig[i].frequency		= pMediaConfig->audioConfig.nFrequency;
		m_AudEncConfig[i].bitrate		= pMediaConfig->audioConfig.nBitrate;
		m_AudEncConfig[i].codec			= pMediaConfig->audioConfig.nCodec;
		m_AudEncConfig[i].adts			= (pMediaConfig->nContainer == DVR_CONTAINER_MP4) ? 0 : 1;
	}

	for( int32_t i = 0; i < m_TextNum; i++ )
	{
		//m_UserDataConfig[i].bitrate 	= pMediaConfig->textConfig.nBitrate;	// N/A
		m_UserDataConfig[i].interval	= pMediaConfig->textConfig.nInterval;	
		m_UserDataConfig[i].bufSize		= 256;	// User Data Size Fixed ( 256 )
	}

	if( pMediaConfig->nContainer == DVR_CONTAINER_MP4) {
		// Mp4 Muxer Filter Config
		m_Mp4MuxerConfig.videoTrack		= m_VideoNum;
		m_Mp4MuxerConfig.audioTrack		= m_AudioNum;
		m_Mp4MuxerConfig.textTrack		= m_TextNum;

		for( int32_t i = 0; i < m_VideoNum + m_AudioNum + m_TextNum; i++ )
		{
			if( i < m_VideoNum ) {
				m_Mp4MuxerConfig.trackConfig[i].width		= pMediaConfig->videoConfig[i].nDstWidth;
				m_Mp4MuxerConfig.trackConfig[i].height		= pMediaConfig->videoConfig[i].nDstHeight;
				m_Mp4MuxerConfig.trackConfig[i].frameRate	= pMediaConfig->videoConfig[i].nFps;
				m_Mp4MuxerConfig.trackConfig[i].bitrate		= pMediaConfig->videoConfig[i].nBitrate;
				m_Mp4MuxerConfig.trackConfig[i].codecType	= (pMediaConfig->videoConfig[i].nCodec == DVR_CODEC_MPEG4) ? MP4_CODEC_TYPE_MPEG4 : MP4_CODEC_TYPE_H264;
			}
			else if( i < m_VideoNum + m_AudioNum ) {
				m_Mp4MuxerConfig.trackConfig[i].channel		= pMediaConfig->audioConfig.nChannel;
				m_Mp4MuxerConfig.trackConfig[i].frequency	= pMediaConfig->audioConfig.nFrequency;
				m_Mp4MuxerConfig.trackConfig[i].bitrate		= pMediaConfig->audioConfig.nBitrate;
				m_Mp4MuxerConfig.trackConfig[i].codecType	= (pMediaConfig->audioConfig.nCodec == DVR_CODEC_MP3) ? MP4_CODEC_TYPE_MP3 : MP4_CODEC_TYPE_AAC;
			}
			else if( i < m_VideoNum + m_AudioNum + m_TextNum ) {
				m_Mp4MuxerConfig.trackConfig[i].bitrate = pMediaConfig->textConfig.nBitrate;
			}
		}
	}
	else if( pMediaConfig->nContainer == DVR_CONTAINER_TS ) {
		// Ts Muxer Filter / Ts FileWriter Filter Config
		m_TsMuxerConfig.videoTrack		= m_VideoNum;
		m_TsMuxerConfig.audioTrack		= m_AudioNum;
		m_TsMuxerConfig.textTrack		= m_TextNum;

		for( int32_t i = 0; i < m_VideoNum + m_AudioNum + m_TextNum; i++ )
		{
			if( i < m_VideoNum ) {
				m_TsMuxerConfig.codecType[i]	= (pMediaConfig->videoConfig[i].nCodec == DVR_CODEC_MPEG4) ? TS_CODEC_TYPE_MPEG4 : TS_CODEC_TYPE_H264;
			}
			else if( i < m_VideoNum + m_AudioNum ) {
				m_TsMuxerConfig.codecType[i]	= (pMediaConfig->audioConfig.nCodec == DVR_CODEC_MP3) ? TS_CODEC_TYPE_MP3 : TS_CODEC_TYPE_AAC;
			}
			else if( i < m_VideoNum + m_AudioNum + m_TextNum ) {
				//N/A
			}
		}
	}
	
	m_InterleaverConfig.channel	= m_VideoNum + m_AudioNum + m_TextNum;

	m_BufferingConfig.bufferedTime = (int32_t)((float)pRecordConfig->nEventBufferDuration / (float)1000);

	// Record Config
	m_NormalDuration		= pRecordConfig->nNormalDuration;
	m_EventDuration			= pRecordConfig->nEventDuration;
	m_EventBufferDuration	= pRecordConfig->nEventBufferDuration;

	if( m_HlsEnable ) {
		strcpy( (char*)m_HlsConfig.MetaFileName,	(char*)pRecordConfig->hlsConfig.MetaFileName );
		strcpy( (char*)m_HlsConfig.SegmentName,		(char*)pRecordConfig->hlsConfig.SegmentFileName );
		strcpy( (char*)m_HlsConfig.SegmentRoot,		(char*)pRecordConfig->hlsConfig.SegmentRootDir );
		m_HlsConfig.SegmentDuration = pRecordConfig->hlsConfig.nSegmentDuration;
		m_HlsConfig.SegmentNumber	= pRecordConfig->hlsConfig.nSegmentNumber;
	}

	if( m_RtpEnable ) {
		m_RtpConfig.port 		= pRecordConfig->rtpConfig.nPort;
		m_RtpConfig.sessionNum 	= pRecordConfig->rtpConfig.nSessionNum;
		m_RtpConfig.connectNum 	= pRecordConfig->rtpConfig.nConnectNum;

		for( uint32_t i = 0; i < m_RtpConfig.sessionNum; i++ ) {
			strcpy( (char*)m_RtpConfig.sessionName[i],	(char*)pRecordConfig->rtpConfig.sessionName[i] );	
		}
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
void CNX_DvrManager::SetNotifier( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	if( m_pNotifier ) {
		for( int32_t i = 0; i < m_VideoNum; i++ ) 
		{
			SET_EVENT_NOTIFIER( m_pVipFilter[i],		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pMdFilter[i], 		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pEffectFilter[i], 	m_pNotifier );
			SET_EVENT_NOTIFIER( m_pOverlayFilter[i],	m_pNotifier );
			SET_EVENT_NOTIFIER( m_pVrFilter[i],			m_pNotifier );
			SET_EVENT_NOTIFIER( m_pAvcEncFilter[i],		m_pNotifier );
			//SET_EVENT_NOTIFIER( m_pMp4EncFilter[i],		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pEffectFilter[i], 	m_pNotifier );
		}
		for( int32_t i = 0; i < m_AudioNum; i++ )
		{
			SET_EVENT_NOTIFIER( m_pAudCapFilter[i],		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pAacEncFilter[i],		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pMp3EncFilter[i], 	m_pNotifier );
		}
		for( int32_t i = 0; i < m_TextNum; i++ )
		{
			SET_EVENT_NOTIFIER( m_pUserDataFilter[i],	m_pNotifier );
		}
		{
			SET_EVENT_NOTIFIER( m_pInterleaverFilter,	m_pNotifier );
			SET_EVENT_NOTIFIER( m_pBufferingFilter,		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pMp4MuxerFilter,		m_pNotifier );
			SET_EVENT_NOTIFIER( m_pFileWriter,			m_pNotifier );
		}
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::StartManager( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	m_bThreadExit = false;
	if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) ) {
		NxDbgMsg( NX_DBG_ERR, ("%s[%s] Fail, Create Thread\n", NX_DTAG, __func__) );
		return -1;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::StopManager( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	m_bThreadExit = true;
	pthread_join( m_hThread, NULL );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
void CNX_DvrManager::StartNormalWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	for( int32_t i = 0; i < m_VideoNum; i++) {
		if( m_pMdFilter[i] )	m_pMdFilter[i]->EnableMotionDetect( false );
	}
	if( m_pBufferingFilter ) 	m_pBufferingFilter->ChangeBufferingMode( BUFFERING_MODE_BOTH );
	if( m_pFileWriter ) 		m_pFileWriter->RegFileNameCallback( NormalFileNameCallbackFunc );
	if( m_pMp4MuxerFilter )		m_pMp4MuxerFilter->RegFileNameCallback( NormalFileNameCallbackFunc );
	if( m_pBufferingFilter )	m_pBufferingFilter->StartFile();
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::StartMotionWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	if( m_pBufferingFilter )	m_pBufferingFilter->ChangeBufferingMode( BUFFERING_MODE_EVENT_ONLY );
	if( m_pFileWriter ) 		m_pFileWriter->RegFileNameCallback( EventFileNameCallbackFunc );
	if( m_pMp4MuxerFilter )		m_pMp4MuxerFilter->RegFileNameCallback( EventFileNameCallbackFunc );
	for( int32_t i = 0; i < m_VideoNum; i++) {
		if( m_MdEnable[i] )		m_pMdFilter[i]->EnableMotionDetect( m_MdEnable[i] );
	}
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::StopWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_pBufferingFilter )	m_pBufferingFilter->StopFile();
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::ChangeNormalWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	if( m_pFileWriter )			m_pFileWriter->RegFileNameCallback( NormalFileNameCallbackFunc );
	if( m_pMp4MuxerFilter )		m_pMp4MuxerFilter->RegFileNameCallback( NormalFileNameCallbackFunc );
	if( m_pBufferingFilter )	m_pBufferingFilter->ChangeFile();
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::ChangeEventWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	if( m_pFileWriter )			m_pFileWriter->RegFileNameCallback( EventFileNameCallbackFunc );
	if( m_pMp4MuxerFilter )		m_pMp4MuxerFilter->RegFileNameCallback( EventFileNameCallbackFunc );
	if( m_pBufferingFilter )	m_pBufferingFilter->PopBufferdData( true );
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::ThreadLoop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	DVR_STATUS curStatus;
	uint64_t curTime, prvTime;

	// Initlize current status
	pthread_mutex_lock( &m_hLock );
	if( m_nMode == DVR_ENCODE_NORMAL ) curStatus = DVR_STATUS_NORMAL_START;
	else curStatus = DVR_STATUS_MOTION_START;
	curTime =
	prvTime = 0;
	pthread_mutex_unlock( &m_hLock );

	while( !m_bThreadExit )
	{
		// a. Get timestamp of filter. ( for file split )
		if( prvTime == 0 ) {
			if( m_pFileWriter ) 	curTime = prvTime = m_pBufferingFilter->GetTimeStamp();
			if( m_pMp4MuxerFilter ) curTime = prvTime = m_pBufferingFilter->GetTimeStamp();
			usleep(10000);
			continue;
		}
		else {
			if( m_pFileWriter ) 	curTime = m_pBufferingFilter->GetTimeStamp();
			if( m_pMp4MuxerFilter ) curTime = m_pBufferingFilter->GetTimeStamp();
		}

		// b. Change State machine at Specific case.
		//    Mode Change.( Normal <-> Motion ) / Event Occur.
		pthread_mutex_lock( &m_hLock );
		if( m_bChageMode ) {
			m_bChageMode = false;

			if( m_nMode == DVR_ENCODE_NORMAL ) {
				StopWriting();
				curStatus = DVR_STATUS_NORMAL_START;
			}
			else {
				StopWriting();
				curStatus = DVR_STATUS_MOTION_START;
			}
		}

		if( m_bEvent ) {
			m_bEvent = false;
			if( curStatus == DVR_STATUS_NORMAL_WAIT )	{
				curStatus = DVR_STATUS_NORMAL_EVENT;
			}
			if( curStatus == DVR_STATUS_MOTION_WAIT )	{
				curStatus = DVR_STATUS_MOTION_EVENT;

			}
		}
		pthread_mutex_unlock( &m_hLock );

		// c. State machine.
		switch( curStatus )
		{
		case DVR_STATUS_NORMAL_START:
			StartNormalWriting();
			prvTime = curTime;
			curStatus = DVR_STATUS_NORMAL_WAIT;
			break;
		case DVR_STATUS_NORMAL_WAIT:
			if( curTime > prvTime + m_NormalDuration ) {
				ChangeNormalWriting();
				prvTime = curTime;
			}
			break;
		case DVR_STATUS_NORMAL_EVENT:
			ChangeEventWriting();
			curStatus = DVR_STATUS_NORMAL_EVENT_WAIT;
			prvTime = curTime;
			break;
		case DVR_STATUS_NORMAL_EVENT_WAIT:
			if( curTime > prvTime + m_EventDuration ) {
				ChangeNormalWriting();
				curStatus = DVR_STATUS_NORMAL_WAIT;
				prvTime = curTime;
			}
			break;
		case DVR_STATUS_MOTION_START:
			StartMotionWriting();
			curStatus = DVR_STATUS_MOTION_WAIT;
			prvTime = curStatus;
			break;
		case DVR_STATUS_MOTION_WAIT:
			break;
		case DVR_STATUS_MOTION_EVENT:
			ChangeEventWriting();
			curStatus = DVR_STATUS_MOTION_EVENT_WAIT;
			prvTime = curTime;
			break;
		case DVR_STATUS_MOTION_EVENT_WAIT:
			if( curTime > prvTime + m_EventDuration ) {
				StopWriting();
				curStatus = DVR_STATUS_MOTION_WAIT;
				prvTime = curTime;
			}
			break;
		}

		usleep(10000);
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void* CNX_DvrManager::ThreadMain( void* arg )
{
	CNX_DvrManager *pClass = (CNX_DvrManager *)arg;

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::Init( NX_DVR_MEDIA_CONFIG *pMediaConfig, NX_DVR_RECORD_CONFIG *pRecordConfig, NX_DVR_DISPLAY_CONFIG *pDisplayConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	SetConfig( pMediaConfig, pRecordConfig, pDisplayConfig );
	int32_t ret = BuildFilter();
	
	if( !ret ) {
		m_bInit = true;	
		SetNotifier();
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit ) {
		// Filter Deinitalize
		for( int32_t i = 0; i < m_VideoNum; i++ ) 
		{
			SAFE_DEINIT_FILTER( m_pVipFilter[i] );
			SAFE_DEINIT_FILTER( m_pMdFilter[i] );
			SAFE_DEINIT_FILTER( m_pEffectFilter[i] );
			SAFE_DEINIT_FILTER( m_pOverlayFilter[i] );
			SAFE_DEINIT_FILTER( m_pVrFilter[i] );
			SAFE_DEINIT_FILTER( m_pAvcEncFilter[i] );
			//SAFE_DEINIT_FILTER( m_pMp4EncFilter[i] );
		}
		for( int32_t i = 0; i < m_AudioNum; i++ )
		{
			SAFE_DEINIT_FILTER( m_pAudCapFilter[i] );
			SAFE_DEINIT_FILTER( m_pAacEncFilter[i] );
			SAFE_DEINIT_FILTER( m_pMp3EncFilter[i] );
		}
		for( int32_t i = 0; i < m_TextNum; i++ )
		{
			SAFE_DEINIT_FILTER( m_pUserDataFilter[i] );
		}
		{
			SAFE_DEINIT_FILTER( m_pInterleaverFilter );
			SAFE_DEINIT_FILTER( m_pBufferingFilter );
			SAFE_DEINIT_FILTER( m_pMp4MuxerFilter );
			SAFE_DEINIT_FILTER( m_pTsMuxerFilter );
			SAFE_DEINIT_FILTER( m_pFileWriter );
			SAFE_DEINIT_FILTER( m_pHlsFilter );
			SAFE_DEINIT_FILTER( m_pRtpFilter );
		}

		// Filter Delete
		for( int32_t i = 0; i < m_VideoNum; i++ ) 
		{
			SAFE_DELETE_FILTER( m_pVipFilter[i] );
			SAFE_DELETE_FILTER( m_pMdFilter[i] );
			SAFE_DELETE_FILTER( m_pEffectFilter[i] );
			SAFE_DELETE_FILTER( m_pOverlayFilter[i] );
			SAFE_DELETE_FILTER( m_pVrFilter[i] );
			SAFE_DELETE_FILTER( m_pAvcEncFilter[i] );
			//SAFE_DELETE_FILTER( m_pMp4EncFilter[i] );
		}
		for( int32_t i = 0; i < m_AudioNum; i++ )
		{
			SAFE_DELETE_FILTER( m_pAudCapFilter[i] );
			SAFE_DELETE_FILTER( m_pAacEncFilter[i] );
			SAFE_DELETE_FILTER( m_pMp3EncFilter[i] );
		}
		for( int32_t i = 0; i < m_TextNum; i++ )
		{
			SAFE_DELETE_FILTER( m_pUserDataFilter[i] );
		}
		{
			SAFE_DELETE_FILTER( m_pInterleaverFilter );
			SAFE_DELETE_FILTER( m_pBufferingFilter );
			SAFE_DELETE_FILTER( m_pMp4MuxerFilter );
			SAFE_DELETE_FILTER( m_pTsMuxerFilter );
			SAFE_DELETE_FILTER( m_pFileWriter );
			SAFE_DELETE_FILTER( m_pHlsFilter );
			SAFE_DELETE_FILTER( m_pRtpFilter );
		}
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::Start( NX_DVR_ENCODE_TYPE encodeType )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit ) {
		m_nMode = encodeType;

		if( m_DisplayEnable ) {
			if( m_pVrFilter[m_DisplayChannel] )	m_pVrFilter[m_DisplayChannel]->EnableRender( true );
		}

		for( int32_t i = 0; i < m_VideoNum; i++ ) 
		{
			if( encodeType == DVR_ENCODE_NORMAL ) {
				if( m_MdEnable[i] )	m_pMdFilter[i]->EnableMotionDetect( false );
			}
			else  {
				if( m_MdEnable[i] )	m_pMdFilter[i]->EnableMotionDetect( m_MdEnable[i] );
			}

			if( m_pOverlayFilter[i] ) m_pOverlayFilter[i]->EnableTextOverlay( true );
		}

		{
			SAFE_START_FILTER( m_pNotifier );
			SAFE_START_FILTER( m_pFileWriter );
			SAFE_START_FILTER( m_pMp4MuxerFilter );
			SAFE_START_FILTER( m_pBufferingFilter );
			SAFE_START_FILTER( m_pTsMuxerFilter );
			SAFE_START_FILTER( m_pHlsFilter );
			SAFE_START_FILTER( m_pRtpFilter );
			SAFE_START_FILTER( m_pInterleaverFilter );
		}

		for( int32_t i = 0; i < m_VideoNum; i++ ) 
		{
			SAFE_START_FILTER( m_pAvcEncFilter[i] );
			//SAFE_START_FILTER( m_pMp4EncFilter[i] );
			SAFE_START_FILTER( m_pOverlayFilter[i] );
			SAFE_START_FILTER( m_pEffectFilter[i] );
			SAFE_START_FILTER( m_pVrFilter[i] );
			SAFE_START_FILTER( m_pMdFilter[i] );
			SAFE_START_FILTER( m_pVipFilter[i] );
		}

		for( int32_t i = 0; i < m_AudioNum; i++ )
		{
			SAFE_START_FILTER( m_pAacEncFilter[i] );
			SAFE_START_FILTER( m_pMp3EncFilter[i] );
			SAFE_START_FILTER( m_pAudCapFilter[i] );
		}

		for( int32_t i = 0; i < m_TextNum; i++ )
		{
			SAFE_START_FILTER( m_pUserDataFilter[i] );
		}

		StartManager();
		m_bRun = true;
	}
	else {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Fail, Filter is not running!\n")) );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( m_bRun ) {
		StopManager();

		for( int32_t i = 0; i < m_VideoNum; i++ ) 
		{
			if( m_MdEnable[i] )	m_pMdFilter[i]->EnableMotionDetect( false );

			SAFE_STOP_FILTER( m_pVipFilter[i] );
			SAFE_STOP_FILTER( m_pMdFilter[i] );
			SAFE_STOP_FILTER( m_pVrFilter[i] );
			SAFE_STOP_FILTER( m_pEffectFilter[i] );
			SAFE_STOP_FILTER( m_pOverlayFilter[i] );
			SAFE_STOP_FILTER( m_pAvcEncFilter[i] );
			//SFAE_STOP_FILTER( m_pMp4EncFilter[i] );
		}

		for( int32_t i = 0; i < m_AudioNum; i++ )
		{
			SAFE_STOP_FILTER( m_pAudCapFilter[i] );
			SAFE_STOP_FILTER( m_pAacEncFilter[i] );
			SAFE_STOP_FILTER( m_pMp3EncFilter[i] );
		}

		for( int32_t i = 0; i < m_TextNum; i++ )
		{
			SAFE_STOP_FILTER( m_pUserDataFilter[i] );
		}

		{
			SAFE_STOP_FILTER( m_pInterleaverFilter );
			SAFE_STOP_FILTER( m_pHlsFilter );
			SAFE_STOP_FILTER( m_pRtpFilter );
			SAFE_STOP_FILTER( m_pTsMuxerFilter );
			SAFE_STOP_FILTER( m_pBufferingFilter );
			SAFE_STOP_FILTER( m_pFileWriter );
			SAFE_STOP_FILTER( m_pMp4MuxerFilter );
			SAFE_STOP_FILTER( m_pNotifier );
		}

		m_bRun = false;
	}
	else {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Fail, Filter is not running!\n")) );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::SetEvent( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );
	
	m_bEvent = true;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::ChangeMode( NX_DVR_ENCODE_TYPE mode )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );
	
	m_nMode = mode;
	m_bChageMode = true;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::SetCapture( int32_t channel )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	// a. Check capture channel
	if( channel >= m_VideoNum ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Overrange capture number.\n")) );
		return -1;
	}

	// b. capture channel
	if( m_pVipFilter[channel] )
		m_pVipFilter[channel]->EnableCapture( true );
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
#include <nx_dsp.h>

int32_t CNX_DvrManager::SetDisplay( NX_DVR_DISPLAY_CONFIG *pDisplayConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );
	
	if( m_DisplayEnable != pDisplayConfig->bEnable ) {
		m_DisplayEnable = pDisplayConfig->bEnable;
		
		if( !m_DisplayEnable ) {
			if( m_pVrFilter[m_DisplayChannel] ) m_pVrFilter[m_DisplayChannel]->EnableRender( false );	
			if( m_pVrFilter[m_DisplayChannel] ) m_pVrFilter[m_DisplayChannel]->EnableHdmiRender( false );
		}
		else {
			if( m_pVrFilter[m_DisplayChannel] ) m_pVrFilter[m_DisplayChannel]->EnableRender( true );
		}
		goto END;
	}

	DSP_IMG_RECT cropRect, dspRect;

	cropRect.left	= pDisplayConfig->cropRect.nLeft;
	cropRect.top	= pDisplayConfig->cropRect.nTop;
	cropRect.right	= pDisplayConfig->cropRect.nRight;
	cropRect.bottom	= pDisplayConfig->cropRect.nBottom;
		
	dspRect.left	= pDisplayConfig->dspRect.nLeft;
	dspRect.top		= pDisplayConfig->dspRect.nTop;
	dspRect.right	= pDisplayConfig->dspRect.nRight;
	dspRect.bottom	= pDisplayConfig->dspRect.nBottom;

	if( !m_pVrFilter[m_DisplayChannel]->SetRenderCrop( &cropRect ) )
	{
		m_VidRenderConfig[m_DisplayChannel].cropLeft	= pDisplayConfig->cropRect.nLeft;
		m_VidRenderConfig[m_DisplayChannel].cropTop		= pDisplayConfig->cropRect.nTop;
		m_VidRenderConfig[m_DisplayChannel].cropRight	= pDisplayConfig->cropRect.nRight;
		m_VidRenderConfig[m_DisplayChannel].cropBottom	= pDisplayConfig->cropRect.nBottom;		
	}
	
	if( !m_pVrFilter[m_DisplayChannel]->SetRenderPosition( &dspRect ) )
	{
		m_VidRenderConfig[m_DisplayChannel].dspLeft		= pDisplayConfig->dspRect.nLeft;
		m_VidRenderConfig[m_DisplayChannel].dspTop		= pDisplayConfig->dspRect.nTop;
		m_VidRenderConfig[m_DisplayChannel].dspRight	= pDisplayConfig->dspRect.nRight;
		m_VidRenderConfig[m_DisplayChannel].dspBottom	= pDisplayConfig->dspRect.nBottom;
	}

END:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}


//------------------------------------------------------------------------------
int32_t CNX_DvrManager::SetPreview( int32_t channel )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( !m_DisplayEnable ) {
		goto END;
	}

	// a. Check rendering channel
	if( channel >= m_VideoNum ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Overrange rendering number.\n")) );
		goto END;
	}

	// b. Disable rendering
	for( int32_t i = 0; i < m_VideoNum; i++ )
	{
		if( m_pVrFilter[i] ) m_pVrFilter[i]->EnableRender( false );	
		if( m_pVrFilter[i] ) m_pVrFilter[i]->EnableHdmiRender( false );
	}

	// c. Enable rendering
	if( m_pVrFilter[channel] ) m_pVrFilter[channel]->EnableRender( true );	

	// d. preview channel update
	m_DisplayChannel = channel;

END:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::SetPreviewHdmi( int32_t channel )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( !m_DisplayEnable ) {
		goto END;
	}

	// a. Check rendering channel
	if( channel >= m_VideoNum ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Overrange rendering number.\n")) );
		goto END;
	}

	// b. Disable rendering
	for( int32_t i = 0; i < m_VideoNum; i++ )
	{
		if( m_pVrFilter[i] ) m_pVrFilter[i]->EnableRender( false );	
		if( m_pVrFilter[i] ) m_pVrFilter[i]->EnableHdmiRender( false );
	}

	// c. Enable HDMI rendering
	if( m_pVrFilter[channel] ) m_pVrFilter[channel]->EnableHdmiRender( true );	

	// d. preview channel update
	m_DisplayChannel = channel;

END:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callback Function
//

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::RegisterGetFileNameCallback( 
	int32_t (*cbNormalFileName)		(uint8_t*, uint32_t),
	int32_t (*cbEventFileName)		(uint8_t*, uint32_t),
	int32_t (*cbParkingFileName)	(uint8_t*, uint32_t),
	int32_t (*cbJpegFileName)		(uint8_t*, uint32_t)
)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );
	
	if( cbNormalFileName )
		NormalFileNameCallbackFunc	= cbNormalFileName;	
	
	if( cbEventFileName )
		EventFileNameCallbackFunc	= cbEventFileName;	
	
	if( cbParkingFileName )
		ParkingFileNameCallbackFunc	= cbParkingFileName;	
	
	if( cbJpegFileName ) {
		for( int i = 0; i < m_VideoNum; i++ )
		{
			if( m_pVipFilter[i] )	m_pVipFilter[i]->RegJpegFileNameCallback( cbJpegFileName );
		}
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t	CNX_DvrManager::RegisterUserDataCallback(
	int32_t (*cbUserData)			(uint8_t*, uint32_t)
)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( cbUserData && m_pUserDataFilter[0] )
		m_pUserDataFilter[0]->RegUserDataCallback( cbUserData );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t	CNX_DvrManager::RegisterTextOverlayCallback(
	int32_t (*cbFrontTextOverlay)	(uint8_t*, uint32_t*, uint32_t*, uint32_t*),
	int32_t (*cbRearTextOverlay) 	(uint8_t*, uint32_t*, uint32_t*, uint32_t*)
)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( cbFrontTextOverlay && m_pOverlayFilter[0] )
		m_pOverlayFilter[0]->RegTextOverlayCallback( cbFrontTextOverlay );
	
	if( cbRearTextOverlay  && m_pOverlayFilter[1] )
		m_pOverlayFilter[1]->RegTextOverlayCallback( cbRearTextOverlay );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t	CNX_DvrManager::RegisterNotifyCallback(
	int32_t (*cbNotify)				(uint32_t, uint8_t*, uint32_t)
)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( cbNotify && m_pNotifier )
		m_pNotifier->RegisterNotifyCallback( cbNotify );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::RegisterImageEffectCallback( 
	int32_t(*cbFrontImageEffect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *),
	int32_t(*cbRearImageEffect)		(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *)
)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( cbFrontImageEffect && m_pEffectFilter[0] )
		m_pEffectFilter[0]->RegImageEffectCallback( cbFrontImageEffect );

	if( cbRearImageEffect  && m_pEffectFilter[1] )
		m_pEffectFilter[1]->RegImageEffectCallback( cbRearImageEffect );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_DvrManager::RegisterMotionDetectCallback(
	int32_t (*cbFrontMotionDetect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *),
	int32_t (*cbRearMotionDetect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *)
)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( cbFrontMotionDetect && m_pMdFilter[0] )
		m_pMdFilter[0]->RegMotionDetectCallback( cbFrontMotionDetect );

	if( cbRearMotionDetect  && m_pMdFilter[1] )
		m_pMdFilter[1]->RegMotionDetectCallback( cbRearMotionDetect );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
void CNX_DvrManager::GetAPIVersion( int32_t *pMajor, int32_t *pMinor, int32_t *pRevision, uint8_t *pBuildDate, uint8_t *pBuildTime, uint8_t *pBuildAuthor )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	*pMajor			= DVR_MANAGER_MAJOR;
	*pMinor			= DVR_MANAGER_MINOR;
	*pRevision		= DVR_MANAGER_REVISION;

	if( pBuildDate ) 	strcpy( (char*)pBuildDate, DVR_MANAGER_DATE );
	if( pBuildTime ) 	strcpy( (char*)pBuildTime, DVR_MANAGER_TIME );
	if( pBuildAuthor )	strcpy( (char*)pBuildAuthor, DVR_MANAGER_USER );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::ChangeDebugLevel( int32_t dbgLevel )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	NxChgFilterDebugLevel( dbgLevel );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_DvrManager::GetStatistics( FILTER_STATISTICS *pFilterStatistics )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}