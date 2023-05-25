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

#include <string.h>
#include <CNX_VRFilter.h>

#define	NX_DTAG	"[CNX_VRFilter] "
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
CNX_VRFilter::CNX_VRFilter( void )
	: m_bInit( false )
	, m_bRun( false )
	, m_bEnable( false )
	, m_bEnableHdmi( false )
	, m_hDsp( NULL )
	, m_pPrevVideoSample( NULL )
{
	memset( &m_DisplayInfo, 0x00, sizeof(DISPLAY_INFO) );
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_VRFilter::~CNX_VRFilter( void )
{
	if( true == m_bInit )
		Deinit();

	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
void	CNX_VRFilter::Init( NX_VIDRENDER_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );

	if( false == m_bInit )
	{
		if( pConfig->port == 0) {
			m_DisplayInfo.port		= pConfig->port;
			m_DisplayInfo.module	= DISPLAY_MODULE_MLC0;
		}
		else {
			m_DisplayInfo.port		= pConfig->port;
			m_DisplayInfo.module	= DISPLAY_MODULE_MLC1;
		}
		
		m_DisplayInfo.width			= pConfig->width;
		m_DisplayInfo.height		= pConfig->height;

		m_DisplayInfo.numPlane		= 1;

		m_DisplayInfo.dspSrcRect.left	= pConfig->cropLeft;
		m_DisplayInfo.dspSrcRect.top	= pConfig->cropTop;
		m_DisplayInfo.dspSrcRect.right	= pConfig->cropRight;
		m_DisplayInfo.dspSrcRect.bottom	= pConfig->cropBottom;

		m_DisplayInfo.dspDstRect.left	= pConfig->dspLeft;
		m_DisplayInfo.dspDstRect.top	= pConfig->dspTop;
		m_DisplayInfo.dspDstRect.right	= pConfig->dspRight;
		m_DisplayInfo.dspDstRect.bottom	= pConfig->dspBottom;

		m_bInit = true;
	}
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_VRFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit )
	{
		if( m_bRun )	Stop();
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Receive( CNX_Sample *pSample )
{
	CNX_AutoLock lock( &m_hLock );

	CNX_VideoSample *pVideoSample = (CNX_VideoSample *)pSample;
	NX_ASSERT( NULL != pVideoSample );

	if( m_bEnable || m_bEnableHdmi ) {
		pVideoSample->Lock();
		if( m_hDsp ) NX_DspQueueBuffer( m_hDsp, pVideoSample->GetVideoMemory() );

		Deliver( pSample );		

		if( m_pPrevVideoSample ) {
			if( m_hDsp ) NX_DspDequeueBuffer( m_hDsp );
			m_pPrevVideoSample->Unlock();
		}

		m_pPrevVideoSample = pVideoSample;
	}
	else {
		pVideoSample->Lock();
		Deliver( pSample );		
		pVideoSample->Unlock();
	}

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::ReleaseSample( CNX_Sample *pSample )
{
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );
	
	if( false == m_bRun ) {
		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( true == m_bRun ) {
		m_bRun = false;
		
		if( m_hDsp ) {
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;			
		}

		if( m_pPrevVideoSample ) {
			m_pPrevVideoSample->Unlock();
			m_pPrevVideoSample = NULL;
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::EnableRender( uint32_t enable )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnable)?"Enable":"Disable", (enable)?"Enable":"Disable") );

	if( enable ) {
		if( !m_hDsp ) {
			m_hDsp = NX_DspInit( &m_DisplayInfo );
		}
	}
	else {
		if( m_hDsp ) {
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;
		}
		
		if( m_pPrevVideoSample ) {
			m_pPrevVideoSample->Unlock();
			m_pPrevVideoSample = NULL;
		}		
	}
	m_bEnable = enable;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;	
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::EnableHdmiRender( uint32_t enable )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnableHdmi)?"Enable":"Disable", (enable)?"Enable":"Disable") );

	DISPLAY_INFO dspInfo;
	memset( &dspInfo, 0x00, sizeof(dspInfo) );

	dspInfo.port 		= 1;
	dspInfo.module		= DISPLAY_MODULE_MLC1;
	dspInfo.width		= m_DisplayInfo.width;
	dspInfo.height		= m_DisplayInfo.height;

	dspInfo.numPlane	= 1;

	dspInfo.dspSrcRect.left		= m_DisplayInfo.dspSrcRect.left;
	dspInfo.dspSrcRect.top		= m_DisplayInfo.dspSrcRect.top;
	dspInfo.dspSrcRect.right	= m_DisplayInfo.dspSrcRect.right;
	dspInfo.dspSrcRect.bottom	= m_DisplayInfo.dspSrcRect.bottom;

	dspInfo.dspDstRect.left		= 0;
	dspInfo.dspDstRect.top		= 0;
	dspInfo.dspDstRect.right	= 1920;
	dspInfo.dspDstRect.bottom	= 1080;

	if( enable ) {
		if( !m_hDsp ) {
			m_hDsp = NX_DspInit( &dspInfo );
		}
	}
	else {
		if( m_hDsp ) {
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;
		}

		if( m_pPrevVideoSample ) {
			m_pPrevVideoSample->Unlock();
			m_pPrevVideoSample = NULL;
		}		
	}
	m_bEnableHdmi = enable;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;	
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::SetRenderCrop( DSP_IMG_RECT *pCropRect )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( pCropRect->left 	== 0 &&
		pCropRect->top		== 0 &&
		pCropRect->right	== 0 &&
		pCropRect->bottom	== 0 ) {
		NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
		return -1;
	}

	if( m_DisplayInfo.dspSrcRect.left 	!= pCropRect->left	||
		m_DisplayInfo.dspSrcRect.top	!= pCropRect->top 	||
		m_DisplayInfo.dspSrcRect.right	!= pCropRect->right	||
		m_DisplayInfo.dspSrcRect.bottom	!= pCropRect->bottom ) {
		
		m_DisplayInfo.dspSrcRect = *pCropRect;
		NX_DspVideoSetSourceCrop( m_hDsp, pCropRect );

		NxDbgMsg( NX_DBG_INFO, (TEXT("%s(): crop(%d, %d, %d, %d)\n"), __func__, 
			pCropRect->left, pCropRect->top, pCropRect->right, pCropRect->bottom) );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::SetRenderPosition( DSP_IMG_RECT *pDspRect )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_AutoLock lock( &m_hLock );

	if( pDspRect->left 		== 0 &&
		pDspRect->top		== 0 &&
		pDspRect->right		== 0 &&
		pDspRect->bottom	== 0 ) {
		NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
		return -1;
	}

	if( m_DisplayInfo.dspDstRect.left	!= pDspRect->left	||
		m_DisplayInfo.dspDstRect.top	!= pDspRect->top	||
		m_DisplayInfo.dspDstRect.right	!= pDspRect->right	||
		m_DisplayInfo.dspDstRect.bottom	!= pDspRect->bottom ) {

		m_DisplayInfo.dspDstRect = *pDspRect;

		NX_DspVideoSetPosition( m_hDsp, pDspRect );

		NxDbgMsg( NX_DBG_INFO, (TEXT("%s(): position(%d, %d, %d, %d)\n"), __func__, 
			pDspRect->left, pDspRect->top, pDspRect->right, pDspRect->bottom) );
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return 0;
}

