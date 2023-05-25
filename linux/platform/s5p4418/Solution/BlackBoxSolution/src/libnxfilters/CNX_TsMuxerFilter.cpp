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

#include <NX_FilterSysApi.h>
#include <CNX_TsMuxerFilter.h>

#define NX_DTAG "[CNX_TsMuxerFilter] "
#include <NX_DbgMsg.h>

#define MAX_BUFFER			16
#define MAX_PES_BUF_SIZE	(1024 * 1024)

//------------------------------------------------------------------------------
CNX_TsMuxerFilter::CNX_TsMuxerFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_hTsMuxer( NULL )
{
	memset( &m_MuxConfig, 0x00, sizeof(m_MuxConfig) );
	pthread_mutex_init( &m_hEncodeLock, NULL );
}

//------------------------------------------------------------------------------
CNX_TsMuxerFilter::~CNX_TsMuxerFilter()
{
	if( true == m_bInit )
		Deinit();
	pthread_mutex_destroy( &m_hEncodeLock );
}

//------------------------------------------------------------------------------
void CNX_TsMuxerFilter::Init( NX_TSMUXER_CONFIG *pConfig )
{
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );

	if( false == m_bInit )
	{
		m_bInit = true;
		memcpy( &m_MuxConfig, pConfig, sizeof(m_MuxConfig) );
		
		m_hTsMuxer = NX_TSMuxOpen( MAX_PES_BUF_SIZE );

		// Muxer Add PID / Build Program
		for( uint32_t i = 0; i < m_MuxConfig.videoTrack; i++ )
		{
			NX_TSMuxAddPID( m_hTsMuxer, VIDEO_PID_BASE + i, NX_ES_VIDEO, pConfig->codecType[i] );
		}		
		for( uint32_t i = 0; i < m_MuxConfig.audioTrack; i++ )
		{
			NX_TSMuxAddPID( m_hTsMuxer, AUDIO_PID_BASE + i, NX_ES_AUDIO, pConfig->codecType[i + m_MuxConfig.videoTrack]);
		}
		for( uint32_t i = 0; i < m_MuxConfig.textTrack; i++ )
		{
			// not yet..
			NX_TSMuxAddPID( m_hTsMuxer, USER_PID_BASE + i, NX_ES_USER, 0x00 );
		}

		//	Program Add
		for( uint32_t i = 0; i < m_MuxConfig.videoTrack; i++ )
		{
			int32_t pidListCnt = 0;

			m_ProgramPidList[i][0] = VIDEO_PID_BASE + i;
			pidListCnt++;
			if( m_MuxConfig.audioTrack ) 
			{
				m_ProgramPidList[i][1] = AUDIO_PID_BASE;
				pidListCnt++;
			}
			if( m_MuxConfig.textTrack )
			{
				m_ProgramPidList[i][2] = USER_PID_BASE;
				pidListCnt++;
			}
			NX_TSMuxBuildProgram( m_hTsMuxer, PRGORAM_NUMBER_BASE + i, PMT_PID_BASE + i, VIDEO_PID_BASE, m_ProgramPidList[i], pidListCnt);
		}
	}
}

//------------------------------------------------------------------------------
void CNX_TsMuxerFilter::Deinit( void )
{
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit )
	{
		if( m_bRun )	Stop();

		NX_TSMuxClose( m_hTsMuxer );
		m_hTsMuxer = NULL;
		m_bInit = false;
	}
}
	
//------------------------------------------------------------------------------
int32_t	CNX_TsMuxerFilter::Receive( CNX_Sample *pSample )
{
	CNX_AutoLock lock ( &m_hEncodeLock );
	int32_t ret;

	NX_ASSERT( NULL != pSample );
	
	pSample->Lock();
	ret = MuxEncodedSample( (CNX_MuxerSample*)pSample );
	
	return ret;
}

//------------------------------------------------------------------------------
int32_t	CNX_TsMuxerFilter::ReleaseSample( CNX_Sample *pSample )
{
	uint8_t *pBuf = NULL;
	int32_t bufSize = 0;

	((CNX_MuxerSample*)pSample)->GetBuffer( &pBuf, &bufSize);
	if( pBuf ) {
		free( pBuf );
	}

	delete ((CNX_MuxerSample*)pSample);

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_TsMuxerFilter::Run( void )
{
	if( m_bRun == false ) {
		m_bRun = true;
	}
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_TsMuxerFilter::Stop( void )
{
	if( true == m_bRun ) {
		m_bRun = false;
	}
	return true;
}

//------------------------------------------------------------------------------
uint64_t CNX_TsMuxerFilter::GetMuxerTime( uint64_t tickCounter )
{
	return tickCounter * 90;
}

//------------------------------------------------------------------------------
int32_t	CNX_TsMuxerFilter::MuxEncodedSample( CNX_MuxerSample *pSample )
{
	uint32_t trackID = pSample->GetDataType();
	NX_ASSERT( NULL != m_hTsMuxer );

	if( NULL == m_hTsMuxer ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("TS Muxer handle is NULL.\n")) );
		return false;
	}

	if( pSample ) {
		NX_RESULT ret = 0;
		MUX_DATA inData, outData;

		memset( &inData, 0x00, sizeof(MUX_DATA) );
		memset( &outData, 0x00, sizeof(MUX_DATA) );
		
		pSample->GetBuffer( &inData.pBuf, (int32_t*)&inData.size );

		inData.size		= pSample->GetActualDataLength();
		inData.pts		= GetMuxerTime( pSample->GetTimeStamp() );
		inData.keyFrame	= pSample->GetSyncPoint();

		if( trackID < m_MuxConfig.videoTrack ) {
			inData.pid = VIDEO_PID_BASE + trackID;
			inData.tag = NX_ES_VIDEO;
		}
		else if( trackID < m_MuxConfig.videoTrack + m_MuxConfig.audioTrack ) {
			inData.pid = AUDIO_PID_BASE + trackID - m_MuxConfig.videoTrack;
			inData.tag = NX_ES_AUDIO;
		}
		else if( trackID < m_MuxConfig.videoTrack + m_MuxConfig.audioTrack + m_MuxConfig.textTrack ) {
			inData.pid = USER_PID_BASE + trackID -  m_MuxConfig.videoTrack - m_MuxConfig.audioTrack;
			inData.tag = NX_ES_USER;
		}

		// if( inData.tag == NX_ES_USER ) {
		// 	//NxDbgMsg( NX_DBG_INFO, (("Muxer Input Info : tag=%d, size = %d, timeStamp = %lld\n"), inData.tag, inData.size, (int64_t)inData.pts) );
		// 	//NxDbgMsg( NX_DBG_INFO, (("Muxer data(%d) : %s\n"), inData.size, inData.pBuf) );
		// }
		// if( inData.tag == NX_ES_AUDIO ) {
		// 	//NxDbgMsg( NX_DBG_INFO, (("Muxer Input Info : tag=%d, size = %d, timeStamp = %lld\n"), inData.tag, inData.size, (int64_t)inData.pts) );
		// }

		int32_t outDataSize = ((int32_t)((double)inData.size / 184.) + 10 ) * 188;
		outData.pBuf = (uint8_t*)malloc( sizeof(uint8_t) * outDataSize );
		//outData.pBuf = (uint8_t*)malloc( sizeof(uint8_t) * MAX_PES_BUF_SIZE );

		if( !outData.pBuf ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("malloc failed!!\n")) );
			pSample->Unlock();
			return false;
		}

		ret = NX_TSMuxPacket( m_hTsMuxer, &inData, &outData );

		if( 0 > ret ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("NX_TSMuxPacket() failed!!(%d).\n"), ret) );
			if( outData.pBuf ) free( outData.pBuf );
			pSample->Unlock();
			return false;
		}

		CNX_MuxerSample *pOutSample = new CNX_MuxerSample();
		if( !pOutSample ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("OutSample is NULL.\n")) );
			pSample->Unlock();
			return false;
		}

		pOutSample->Lock();
		pOutSample->SetOwner( this );
		pOutSample->SetDataType( pSample->GetDataType() );
		pOutSample->SetTimeStamp( pSample->GetTimeStamp() );
		pOutSample->SetSyncPoint( pSample->GetSyncPoint() );
		pOutSample->SetBuffer( outData.pBuf, outData.size );
		pOutSample->SetActualDataLength( outData.size );
		
		// printf("[%d] inData.size = %d, outData.size = %d ( %d )\n", trackID, inData.size, outData.size, outData.size - inData.size );
		Deliver( (CNX_Sample*)pOutSample );
		
		if( pSample ) pSample->Unlock();
		if( pOutSample ) pOutSample->Unlock();
	}
	return true;
}


