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
#include <stdlib.h>
#include <nx_fourcc.h>

#include "CNX_ImageEffectFilter.h"
#define	NX_DTAG	"[CNX_ImageEffectFilter] "
#include "NX_DbgMsg.h"

//------------------------------------------------------------------------------
CNX_ImageEffectFilter::CNX_ImageEffectFilter()
	: ImageEffectCallbackFunc( NULL )
	, m_bInit( false )
	, m_bRun( false )
	, m_pSemIn( NULL )
	, m_pSemOut( NULL )
	, m_bThreadExit( true )
	, m_hThread( 0 )
	, m_FourCC( FOURCC_MVS0 )
	, m_iNumOfBuffer(MAX_BUFFER)
{
	memset( &m_ImageEffectConfig, 0x00, sizeof(m_ImageEffectConfig) );
	
	for( int32_t i = 0; i < MAX_BUFFER; i++ )
	{
		m_VideoMemory[i] = NULL;
	}

	m_pSemIn	= new CNX_Semaphore( MAX_BUFFER, 0 );
	m_pSemOut	= new CNX_Semaphore( MAX_BUFFER, 0 );
	
	NX_ASSERT( m_pSemIn );
	NX_ASSERT( m_pSemOut );
}

//------------------------------------------------------------------------------
CNX_ImageEffectFilter::~CNX_ImageEffectFilter()
{
	if( true == m_bInit )
		Deinit();
	
	delete m_pSemIn;
	delete m_pSemOut;
}

//------------------------------------------------------------------------------
void	CNX_ImageEffectFilter::Init( NX_IMAGE_EFFECT_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );
	if( false == m_bInit )
	{
		m_ImageEffectConfig.width 	= pConfig->width;
		m_ImageEffectConfig.height 	= pConfig->height;
		
		AllocateBuffer( m_ImageEffectConfig.width, m_ImageEffectConfig.height, 256, 256, NUM_ALLOC_BUFFER, m_FourCC);
		m_bInit = true;
	}


	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//----------------------------------------------------------------------------
void	CNX_ImageEffectFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit )
	{
		if( m_bRun )	Stop();
		FreeBuffer();
		m_bInit = false;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_ImageEffectFilter::Receive( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != pSample );

	if( !m_pOutFilter )
		return true;

	pSample->Lock();
	m_SampleInQueue.PushSample(pSample);
	m_pSemIn->Post();
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_ImageEffectFilter::ReleaseSample(CNX_Sample *pSample)
{
	m_SampleOutQueue.PushSample( pSample );
	m_pSemOut->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_ImageEffectFilter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bThreadExit 	= false;
		NX_ASSERT( !m_hThread );

		if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) )
		{
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Create Thread\n"), __func__) );
			return false;
		}

		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_ImageEffectFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemIn->Post();
		m_pSemOut->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void	CNX_ImageEffectFilter::AllocateBuffer( int32_t width, int32_t height, int32_t alignx, int32_t aligny, int32_t numOfBuffer, uint32_t dwFourCC )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( numOfBuffer <= MAX_BUFFER );

	m_SampleOutQueue.Reset();
	m_SampleOutQueue.SetQueueDepth( numOfBuffer );

	m_pSemOut->Init();
	for( int32_t i = 0; i < numOfBuffer; i++) {
		NX_ASSERT( NULL == m_VideoMemory[i] );
		m_VideoMemory[i] = NX_VideoAllocateMemory( 4096, width, height, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );
		NX_ASSERT( NULL != m_VideoMemory[i] );

		m_VideoSample[i].SetOwner( this );
		m_VideoSample[i].SetVideoMemory( m_VideoMemory[i] );

		m_SampleOutQueue.PushSample( &m_VideoSample[i] );
		m_pSemOut->Post();
	}
	m_iNumOfBuffer = numOfBuffer;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()-- (m_iNumOfBuffer=%d)\n"), __func__, m_iNumOfBuffer) );
}

//------------------------------------------------------------------------------
void	CNX_ImageEffectFilter::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_SampleOutQueue.Reset();
	for( int32_t i = 0; i < m_iNumOfBuffer; i++ )
	{
		NX_ASSERT(NULL != m_VideoMemory[i]);
		if( m_VideoMemory[i] )
		{
			NX_FreeVideoMemory( m_VideoMemory[i] );
		}
	}
	m_pSemIn->Post();		//	Send Dummy
	m_pSemOut->Post();		//	Send Dummy
	m_iNumOfBuffer = 0;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_ImageEffectFilter::GetSample( CNX_Sample **ppSample )
{
	m_pSemIn->Pend();
	if( true == m_SampleInQueue.IsReady() ){
		m_SampleInQueue.PopSample( ppSample );
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_ImageEffectFilter::GetDeliverySample( CNX_Sample **ppSample )
{
	m_pSemOut->Pend();
	if( true == m_SampleOutQueue.IsReady() ) {
		m_SampleOutQueue.PopSample( ppSample );
		(*ppSample)->Lock();
		return true;
	}
	NxDbgMsg( NX_DBG_WARN, (TEXT("Sample is not ready\n")) );
	return false;
}

//------------------------------------------------------------------------------
void	CNX_ImageEffectFilter::ThreadLoop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	CNX_VideoSample		*pSample = NULL;
	CNX_VideoSample		*pOutSample = NULL;

	while( !m_bThreadExit )
	{
		if( false == GetSample((CNX_Sample **)&pSample) )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetSample() Failed\n")) );
			continue;
		}
		if( NULL == pSample )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("Sample is NULL\n")) );
			continue;
		}
		if( false == GetDeliverySample( (CNX_Sample **)&pOutSample) )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetDeliverySample() Failed\n")) );
			pSample->Unlock();
			continue;
		}
		if( NULL == pOutSample )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("ImageEffect buffer is NULL\n")) );
			pSample->Unlock();
			continue;
		}

		if( !GetImageEffectFromCallback( pSample->GetVideoMemory(), pOutSample->GetVideoMemory() ) )
		{
			pOutSample->SetTimeStamp( pSample->GetTimeStamp() );
			Deliver( pOutSample );
 			pOutSample->Unlock();
 		}
 		else
 		{
 			Deliver( pSample );
			pOutSample->Unlock();
		}

		pSample->Unlock();
	}

	while( m_SampleInQueue.IsReady() )
	{
		m_SampleInQueue.PopSample((CNX_Sample**)&pSample);
		if( pSample )
			pSample->Unlock();
	}	

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_ImageEffectFilter::ThreadMain( void* arg )
{
	CNX_ImageEffectFilter *pClass = (CNX_ImageEffectFilter *)arg;

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
int32_t	CNX_ImageEffectFilter::GetImageEffectFromCallback( NX_VID_MEMORY_INFO *pSrcMemory, NX_VID_MEMORY_INFO *pDstMemory )
{
	NX_ASSERT( pSrcMemory );
	NX_ASSERT( pDstMemory );
	
	int32_t ret = -1;

	if( ImageEffectCallbackFunc )
	{
		ret = ImageEffectCallbackFunc( pSrcMemory, pDstMemory );
	}

	return ret;
}

//------------------------------------------------------------------------------
void	CNX_ImageEffectFilter::RegImageEffectCallback( int32_t(*cbFunc)( NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *) )
{
	NX_ASSERT( cbFunc );
	if( cbFunc )
	{
		ImageEffectCallbackFunc = cbFunc;
	}
}

//------------------------------------------------------------------------------
int32_t  CNX_ImageEffectFilter::GetStatistics( NX_FILTER_STATISTICS *pStatistics )
{
	return true;
}
