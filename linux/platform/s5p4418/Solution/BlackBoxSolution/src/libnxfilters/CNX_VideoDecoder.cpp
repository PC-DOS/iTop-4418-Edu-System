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

#include <stdlib.h>
#include <CNX_VideoDecoder.h>
#include <math.h>

#include "codec_info.h"

#define NX_DTAG "[CNX_VideoDecoder] "
#include <NX_DbgMsg.h>

// #define DISPLAY_TIMESTAMP

//------------------------------------------------------------------------------
CNX_VideoDecoder::CNX_VideoDecoder()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0x00 )
	, m_hDec( NULL )
	, m_pMediaReader( NULL )
	, m_SkipFrame( 0 )
{
	m_pSemOut = new CNX_Semaphore(MAX_BUFFER, 0);
	NX_ASSERT( m_pSemOut );
}

//------------------------------------------------------------------------------
CNX_VideoDecoder::~CNX_VideoDecoder()
{
	if( true == m_bInit )
		Deinit();

	delete m_pSemOut;
}

//------------------------------------------------------------------------------
void CNX_VideoDecoder::Init()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );

	if( false == m_bInit )
	{
		m_pMediaReader = new CMediaReader();
		m_bInit = true;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_VideoDecoder::Deinit()
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );
	if( true == m_bInit )
	{
		if( m_bRun )	Stop();
		
		if( m_hDec ) {
			NX_VidDecClose( m_hDec );
			m_hDec = NULL;
		}
		
		if( m_pMediaReader ) {
			m_pMediaReader->CloseFile();
			delete m_pMediaReader;
			m_pMediaReader = NULL;
		}

		m_bInit = false;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::Receive( CNX_Sample *pSample )
{
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::ReleaseSample( CNX_Sample *pSample )
{
	if( pSample ) {
		NX_VidDecClrDspFlag( m_hDec, 
			((CNX_VideoSample*)pSample)->GetVideoMemory(), 
			((CNX_VideoSample*)pSample)->GetVideoMemoryIndex()
		);
		delete pSample;

		m_pSemOut->Post();
	}

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bThreadExit 	= false;
		NX_ASSERT( !m_hThread );

		m_pSemOut->Init();
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
int32_t CNX_VideoDecoder::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemOut->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_VideoDecoder::ThreadLoop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	uint8_t streamBuffer[MAX_STREAM_BUF_SIZE];
	int32_t seqSize = 0;
	int32_t size, key = 0;
	int32_t bInit = false;
	long long timeStamp = -1;

	int32_t skipCount = 0;

#ifdef DISPLAY_TIMESTAMP
	int64_t prvTimeStamp = 0;
#endif

	VID_ERROR_E 		vidRet;
	NX_VID_SEQ_IN 		seqIn;
	NX_VID_SEQ_OUT 		seqOut;
	NX_VID_DEC_IN 		decIn;
	NX_VID_DEC_OUT 		decOut;

	seqSize = m_pMediaReader->GetVideoSeqInfo( streamBuffer );
	
	for( int32_t i = 0; i < MAX_BUFFER; i++ ) {
		m_pSemOut->Post();
	}
	
	while( !m_bThreadExit )
	{
		m_pSemOut->Pend();

		if( 0 != m_pMediaReader->ReadStream( CMediaReader::MEDIA_TYPE_VIDEO, streamBuffer+seqSize, &size, &key, &timeStamp ) ) {
			break;
		}

		if( !bInit && !key ) continue;

		if( !bInit ) {
			memset( &seqIn, 0, sizeof(seqIn) );
			seqIn.addNumBuffers		= 4;
			seqIn.enablePostFilter	= 0;
			seqIn.seqInfo			= streamBuffer;
			seqIn.seqSize			= seqSize + size;
			seqIn.enableUserData	= 0;
			seqIn.disableOutReorder = 0;

			vidRet = NX_VidDecInit( m_hDec, &seqIn, &seqOut );
			if( vidRet == VID_ERR_NEED_MORE_BUF ) {
				NxDbgMsg( NX_DBG_ERR, (TEXT("VPU Initialize Failed!!!\n")) );
				break;
			}

			seqSize = 0;
			bInit = 1;
			size = 0;
			//continue;
		}
		memset(&decIn, 0x00, sizeof(decIn));
		decIn.strmBuf 	= streamBuffer;
		decIn.strmSize 	= size;
		decIn.timeStamp = timeStamp;
		decIn.eos = 0;

#ifdef DISPLAY_TIMESTAMP
		printf("In TimeStamp ( %lld ), In StreamSize ( %d )\n", decIn.timeStamp, decIn.strmSize );
#endif
		vidRet = NX_VidDecDecodeFrame( m_hDec, &decIn, &decOut );

		if( vidRet == VID_ERR_NEED_MORE_BUF ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("VID_ERR_NEED_MORE_BUF NX_VidDecDecodeFrame.\n")) );
			m_pSemOut->Post();
			continue;
		}
		
		if( vidRet < 0 ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("Decoding Error!!!\n")) );
			exit(-2);
		}
		
		if(decOut.outImgIdx >= 0 ) {
#ifdef DISPLAY_TIMESTAMP
			printf("Out TimeStamp ( %lld ), Gap ( %lld )\n", decOut.timeStamp, decOut.timeStamp - prvTimeStamp );
			prvTimeStamp = decOut.timeStamp;
#endif

			if( !m_SkipFrame ) {
				CNX_VideoSample *pVideoSample = new CNX_VideoSample();
				
				pVideoSample->Lock();
				pVideoSample->SetOwner(this);
				pVideoSample->SetTimeStamp( decOut.timeStamp );
				pVideoSample->SetVideoMemory( &decOut.outImg );
				pVideoSample->SetVideoMemoryIndex( decOut.outImgIdx );
				Deliver( pVideoSample );
				pVideoSample->Unlock();
			}
			else {
				if( (skipCount % m_SkipFrame) == 0) {
					skipCount = 0;
					CNX_VideoSample *pVideoSample = new CNX_VideoSample();
				
					pVideoSample->Lock();
					pVideoSample->SetOwner(this);
					pVideoSample->SetTimeStamp( decOut.timeStamp );
					pVideoSample->SetVideoMemory( &decOut.outImg );
					pVideoSample->SetVideoMemoryIndex( decOut.outImgIdx );
					Deliver( pVideoSample );
					pVideoSample->Unlock();					
				}
				else {
					NX_VidDecClrDspFlag( m_hDec, &decOut.outImg, decOut.outImgIdx );
					m_pSemOut->Post();
				}
				skipCount++;
			}
		}
		else {
			m_pSemOut->Post();
		}
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void* CNX_VideoDecoder::ThreadMain( void *pArg)
{
	CNX_VideoDecoder *pClass = (CNX_VideoDecoder *)pArg;

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::SetFileName( uint8_t *pFileName )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_pMediaReader ) {
		if( !m_pMediaReader->OpenFile( (char*)pFileName ) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Open failed.\n"), __func__) );
			goto ErrorExit;
		}

		int32_t vpuCodecType;
		int32_t mp4Class = 0;
		int32_t codecTag = -1, codecId = -1;

		m_pMediaReader->GetCodecTagId( AVMEDIA_TYPE_VIDEO, &codecTag, &codecId );
		vpuCodecType = CodecIdToVpuType( codecId, codecTag );
		mp4Class = fourCCToMp4Class( codecTag );

		if( mp4Class == -1 )
			mp4Class = codecIdToMp4Class( codecId );
		
		mp4Class = 0;
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): vpuCodecType = %d, mp4Class = %d\n"), __func__, vpuCodecType, mp4Class) );

		if( NULL == (m_hDec = NX_VidDecOpen((VID_TYPE_E)vpuCodecType, mp4Class, 0, NULL)) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): NX_VidDecOpen(%d) failed!!!\n"), __func__, vpuCodecType) );
			goto ErrorExit;
		}		
	}
	else {
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Media Reader is not initialized.\n"), __func__) );
		goto ErrorExit;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;

ErrorExit:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::GetVideoResolution( int32_t *width, int32_t *height )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	*width = *height = 0;

	if( m_pMediaReader ) {
		m_pMediaReader->GetVideoResolution( width, height );
	}
	else {
		goto ErrorExit;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;

ErrorExit:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::GetVideoFramerate( int32_t *fpsNum, int32_t *fpsDen )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	double fps = 0.;
	*fpsNum = *fpsDen = 0;

	if( m_pMediaReader ) {
		m_pMediaReader->GetVideoFramerate( fpsNum, fpsDen );
	}
	else {
		goto ErrorExit;
	}

	fps = floor( ((double)*fpsNum / (double)*fpsDen) + .5 );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return (int32_t)fps;

ErrorExit:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_VideoDecoder::SetFrameDown( int32_t targetFps )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	int32_t fpsNum = 0, fpsDen = 0, fps = 0;
	fps = GetVideoFramerate( &fpsNum, &fpsDen );

	if( targetFps >= fps ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Error ( Source Frame(%d fps), Target Frame(%dfps) )\n"), __func__, (int32_t)fps, targetFps) );
		goto ErrorExit;
	}

	m_SkipFrame = fps / targetFps;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;

ErrorExit:
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return false;
}