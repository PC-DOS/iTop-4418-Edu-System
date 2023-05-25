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

#include "CNX_MotionDetectFilter.h"

#define NX_DTAG "[CNX_MotionDetectFilter] "
#include "NX_DbgMsg.h"

//------------------------------------------------------------------------------
CNX_MotionDetectFilter::CNX_MotionDetectFilter()
	: MotionDetectCallbackFunc( NULL )
	, m_bInit( false )
	, m_bRun( false )
	, m_bEnabled( false )
	, m_pSemIn( NULL )
	, m_bThreadExit( true )
	, m_hThread( 0 )
	, m_SamplingWidth( 16 )
	, m_SamplingHeight( 16 )
	, m_Threshold( 10 )
	, m_Sensitivity( 10 )
	, m_SamplingFrame( 1 )
	, m_SamplingFrameCnt( 0 )
	, m_pPrevVideoSample( NULL )
{
	m_pSemIn	= new CNX_Semaphore( MAX_BUFFER, 0 );
	NX_ASSERT( m_pSemIn );
}

//------------------------------------------------------------------------------
CNX_MotionDetectFilter::~CNX_MotionDetectFilter()
{
	if( true == m_bInit )
		Deinit();
	
	delete m_pSemIn;
}

//------------------------------------------------------------------------------
void CNX_MotionDetectFilter::Init( NX_MOTION_DETECT_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );

	if( false == m_bInit )
	{
		m_SamplingWidth		= pConfig->samplingWidth;	// 16
		m_SamplingHeight 	= pConfig->samplingHeight;	// 16
		m_Threshold 		= pConfig->threshold;		// 50
		m_Sensitivity 		= pConfig->sensitivity;		// 1000
		m_SamplingFrame 	= pConfig->sampingFrame;	// 1

		AllocateBuffer();
		m_bInit = true;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_MotionDetectFilter::Deinit( void )
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
int32_t CNX_MotionDetectFilter::Receive( CNX_Sample *pSample )
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
int32_t	CNX_MotionDetectFilter::ReleaseSample( CNX_Sample *pSample )
{
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_MotionDetectFilter::Run( void )
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
int32_t	CNX_MotionDetectFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemIn->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_MotionDetectFilter::AllocateBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_MotionDetectFilter::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_pSemIn->Post();
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_MotionDetectFilter::GetSample( CNX_Sample **ppSample )
{
	m_pSemIn->Pend();
	if( true == m_SampleInQueue.IsReady() ){
		m_SampleInQueue.PopSample( ppSample );
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
int32_t	CNX_MotionDetectFilter::GetDeliverySample( CNX_Sample **ppSample )
{
	return false;
}

//------------------------------------------------------------------------------
void CNX_MotionDetectFilter::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	CNX_VideoSample		*pSample = NULL;

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

		Deliver( pSample );
		
		if( m_bEnabled )
		{
			if( !(m_SamplingFrameCnt % m_SamplingFrame) )
			{
				m_SamplingFrameCnt = 0;

				if(NULL != m_pPrevVideoSample)
				{
					int32_t ret = false;

					if( MotionDetectCallbackFunc )
						ret = UserMotionDetect( m_pPrevVideoSample->GetVideoMemory(), pSample->GetVideoMemory() );
					else
						ret = MotionDetect( m_pPrevVideoSample->GetVideoMemory(), pSample->GetVideoMemory() );
					
					if( ret )
					{
						if( m_pNotify )
							m_pNotify->EventNotify( 0x1004, NULL, 0 );
					}

					m_pPrevVideoSample->Unlock();
				}
				m_pPrevVideoSample = pSample;
			}
			else
			{
				pSample->Unlock();
			}
			m_SamplingFrameCnt++;
		}
		else
		{
			if( m_SamplingFrameCnt != 0)
			{
				m_SamplingFrameCnt = 0;
			}

			if( m_pPrevVideoSample )
			{
				m_pPrevVideoSample->Unlock();
				m_pPrevVideoSample = NULL;
			}
			
			pSample->Unlock();
		}
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
void* CNX_MotionDetectFilter::ThreadMain(void*arg)
{
	CNX_MotionDetectFilter *pClass = (CNX_MotionDetectFilter *)arg;
	NX_ASSERT( NULL != pClass );

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
int32_t CNX_MotionDetectFilter::MotionDetect( NX_VID_MEMORY_INFO *pPrevMemory, NX_VID_MEMORY_INFO *pCurMemory )
{
	unsigned char *pPrevAddr	= (unsigned char*)pPrevMemory->luVirAddr;
	unsigned char *pCurAddr		= (unsigned char*)pCurMemory->luVirAddr;

	unsigned char prevValue	= 0;
	unsigned char curValue	= 0;

	unsigned char diffValue	= 0;
	
	double		diffAverage	= 0.0;
	int32_t		diffCnt		= 0;

	for( int32_t i = 0; i < pPrevMemory->imgHeight; i += m_SamplingWidth )
	{
		for( int32_t j = 0; j < pPrevMemory->imgWidth; j += m_SamplingHeight )
		{
			prevValue = *(pPrevAddr + j + (pPrevMemory->luStride * i));
			curValue  = *(pCurAddr  + j + (pCurMemory->luStride  * i));
			
			if(prevValue > curValue )
				diffValue = prevValue - curValue;
			else
				diffValue = curValue - prevValue;

			diffAverage += diffValue;
			
			if( diffValue > m_Threshold ) diffCnt++;
		}
	}

	diffAverage /= (double)(m_SamplingWidth * m_SamplingHeight);
	//NxDbgMsg( NX_DBG_VBS, (TEXT("%s(): diffCnt = %03d, diffAverage %03.03f\n"), __func__, diffCnt, diffAverage) );

	if( diffCnt > m_Sensitivity )
		return true;
	else
		return false;
}

//------------------------------------------------------------------------------
int32_t CNX_MotionDetectFilter::UserMotionDetect( NX_VID_MEMORY_INFO *pPrevMemory, NX_VID_MEMORY_INFO *pCurMemory )
{
	NX_ASSERT( MotionDetectCallbackFunc );

	int32_t ret = false;
	if( MotionDetectCallbackFunc )
	{
		ret = MotionDetectCallbackFunc( pPrevMemory, pCurMemory );	
	}
	return ret;
}

//------------------------------------------------------------------------------
void CNX_MotionDetectFilter::RegMotionDetectCallback( int32_t (*cbFunc)( NX_VID_MEMORY_INFO*, NX_VID_MEMORY_INFO*) )
{
	NX_ASSERT( cbFunc );
	if( cbFunc )
	{
		MotionDetectCallbackFunc = cbFunc;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_MotionDetectFilter::EnableMotionDetect( int32_t enable )
{
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnabled)?"Enable":"Disable", (enable)?"Enable":"Disable") );
	m_bEnabled = enable;
	return true;
}