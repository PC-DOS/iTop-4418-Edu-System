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

#include "CNX_RTPFilter.h"

#define	NX_DTAG	"[CNX_RTPFilter] "
#include "NX_DbgMsg.h"

//------------------------------------------------------------------------------
CNX_RTPFilter::CNX_RTPFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0x00 )
	, m_pEnv( NULL )
	, m_iRtspPortNum( 554 )
	, m_iWatchFlag( 0 )
	, m_CurStreamType( RTSP_VIDEO_DATA0 )
	, m_nCurConnectNum( 0 )
	, m_nMaxConnectNum( 2 )
	, m_nMaxSessionNum( MAX_SESSION_NUM )
{
	for( int32_t i = 0; i < m_nMaxConnectNum; i++ ) {
		m_pLiveSource[i] = NULL;
	}

	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_RTPFilter::~CNX_RTPFilter()
{
	if( true == m_bInit )
		Deinit();

	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::Init( NX_RTP_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( false == m_bInit ) {
		memset( &m_RtpConfig, 0x00, sizeof(m_RtpConfig) );
		m_RtpConfig.port		= pConfig->port;
		m_RtpConfig.sessionNum	= pConfig->sessionNum;
		m_RtpConfig.connectNum	= pConfig->connectNum;
		
		for(int32_t i = 0; i < (int32_t)m_RtpConfig.sessionNum; i++)
		{
			sprintf( (char*)m_RtpConfig.sessionName[i], "%s", (char*)pConfig->sessionName[i]);
		}

		m_iRtspPortNum 		= m_RtpConfig.port;
		m_nMaxSessionNum	= m_RtpConfig.sessionNum;
		m_nMaxConnectNum 	= m_RtpConfig.connectNum;

		m_bInit = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bInit ) {
		if( m_bRun ) {
			Stop();
		}
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_RTPFilter::Receive( CNX_Sample *pSample )
{
	CNX_AutoLock lock ( &m_hLock );
	CNX_MuxerSample *pInSample = NULL;

	NX_ASSERT( NULL != pSample );
	pSample->Lock();
	Deliver( pSample );

	// a. Create Sample.
	if( m_nCurConnectNum && (((CNX_MuxerSample*)pSample)->GetDataType() == m_CurStreamType) ) {
		pInSample = new CNX_MuxerSample();
		CreateSample( pSample, (CNX_Sample*)pInSample );
	}
	
	// b. Push Samples
	for( int32_t i = 0; i < m_nMaxConnectNum; i++ )
	{
		if( m_pLiveSource[i] && (((CNX_MuxerSample*)pSample)->GetDataType() == m_CurStreamType) ) {
			m_pLiveSource[i]->PushSample( (CNX_Sample*)pInSample );
			m_pLiveSource[i]->Post();
		}
	}
	pSample->Unlock();
	
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::ReleaseSample( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != pSample );
	DestorySample( pSample );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_RTPFilter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bThreadExit 	= false;
		NX_ASSERT( !m_hThread );

		if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Create Thread\n"), __func__) );
			return false;
		}

		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_RTPFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;

		for( int32_t i = 0; i < m_nMaxConnectNum; i++ ) {
			if( m_pLiveSource[i] ) m_pLiveSource[i]->Post();
		}

		m_iWatchFlag = 1;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::AllocateMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::FreeMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::GetSample( CNX_Sample **ppSample)
{
	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::GetDeliverySample( CNX_Sample **ppSample )
{
	return false;
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	m_pScheduler 	= BasicTaskScheduler::createNew();
	m_pEnv 			= BasicUsageEnvironment::createNew( *m_pScheduler );
	
	if( NULL == (m_pRtspServer = CNX_DynamicRTSPServer::createNew( *m_pEnv, m_iRtspPortNum, NULL )) ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Create RTSP Server.\n"), __func__) );
		NX_ASSERT( m_pRtspServer != NULL );	
		return ;
	}

	((CNX_DynamicRTSPServer*)m_pRtspServer)->SetOwnerFilter( this );
	((CNX_DynamicRTSPServer*)m_pRtspServer)->SetMaxSessionNum( m_nMaxSessionNum );

	for(int32_t i = 0; i < m_nMaxSessionNum; i++) {
		((CNX_DynamicRTSPServer*)m_pRtspServer)->SetSessionName( i, m_RtpConfig.sessionName[i] );
	}

	char *urlPrefix = m_pRtspServer->rtspURLPrefix();
	NxDbgMsg( NX_DBG_INFO, (TEXT("Nexell RTSP Media Server on LIVE555.\n")) );
	for(int32_t i = 0; i < m_nMaxSessionNum; i++ ) {
		NxDbgMsg( NX_DBG_INFO, (TEXT("URL #%d : %s%s\n"), i, urlPrefix, m_RtpConfig.sessionName[i]) );	
	}

	if( m_pRtspServer->setUpTunnelingOverHTTP(80) || m_pRtspServer->setUpTunnelingOverHTTP(8000) || m_pRtspServer->setUpTunnelingOverHTTP(8080) ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("RTSP-over-HTTP tunneling. ( port : %d )\n"), m_pRtspServer->httpServerPortNum()) );
	}
	else {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Not available RTSP-over-HTTP tunneling.\n")) );
	}

	m_pEnv->taskScheduler().doEventLoop( &m_iWatchFlag );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );	
}

//------------------------------------------------------------------------------
void* CNX_RTPFilter::ThreadMain(void*arg)
{
	CNX_RTPFilter *pClass = (CNX_RTPFilter *)arg;
	
	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::CreateSample( CNX_Sample *pSrcSample, CNX_Sample *pDstSample )
{
	uint8_t *pSrcBuf, *pDstBuf;
	int32_t SrcSize, DstSize;

	((CNX_MuxerSample*)pSrcSample)->GetBuffer( (uint8_t**)&pSrcBuf, &SrcSize );

	DstSize = SrcSize;
	pDstBuf = (uint8_t*)malloc( DstSize );

	memset( pDstBuf, 0x00, DstSize );
	memcpy( pDstBuf, pSrcBuf, DstSize );

	((CNX_MuxerSample*)pDstSample)->SetOwner( this );
	((CNX_MuxerSample*)pDstSample)->SetBuffer( pDstBuf, DstSize );
	((CNX_MuxerSample*)pDstSample)->SetSyncPoint( ((CNX_MuxerSample*)pSrcSample)->GetSyncPoint() );
	((CNX_MuxerSample*)pDstSample)->SetTimeStamp( ((CNX_MuxerSample*)pSrcSample)->GetTimeStamp() );

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::DestorySample( CNX_Sample *pSample )
{
	uint8_t *pBuf = NULL;
	int32_t size = 0;

	((CNX_MuxerSample*)pSample)->GetBuffer( &pBuf, &size );

	if( pBuf ) 		free( pBuf );
	else 			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Invalid buffer.\n"), __func__) );
	
	if( pSample )	delete pSample;

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::SetSourceInstance( CNX_LiveSource *pLiveSource, int32_t type )
{
	CNX_AutoLock lock ( &m_hLock );

	for( int32_t i = 0; i < m_nMaxConnectNum; i++ ) {
		if( m_pLiveSource[i] == NULL ) {
			m_pLiveSource[i] 	= pLiveSource;
			m_CurStreamType		= type;
			m_nCurConnectNum++;
			NxDbgMsg( NX_DBG_VBS, (TEXT("%s(): [%d] Set Instance. (LiveSource = 0x%08x, StreamType = %d)\n"), __func__, m_nCurConnectNum, (int32_t)pLiveSource, type) );
			return true;
		}
	}

	NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, is not empty slot.\n"), __func__) );
	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::ClearSourceInstance( CNX_LiveSource *pLiveSource )
{
	CNX_AutoLock lock ( &m_hLock );

	for( int32_t i = 0; i < m_nMaxConnectNum; i++ ) {
		if( m_pLiveSource[i] == pLiveSource ) {
			m_nCurConnectNum--;
			NxDbgMsg( NX_DBG_VBS, (TEXT("%s(): [%d] Clear Instance. (LiveSource = 0x%08x)\n"), __func__, m_nCurConnectNum, (int32_t)pLiveSource) );
			m_pLiveSource[i] = NULL;
			return true;
		}
	}

	NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, not match slot.\n"), __func__) );
	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::ConnectIsReady( void )
{
	CNX_AutoLock lock ( &m_hLock );
	
	if( m_nCurConnectNum < m_nMaxConnectNum )
		return true;

	return false;
}