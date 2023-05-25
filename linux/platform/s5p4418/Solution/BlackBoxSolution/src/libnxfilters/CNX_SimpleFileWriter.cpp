//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co. All Rights Reserved
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
//	Author		: Ray
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <NX_FilterSysApi.h>

#define	NX_DTAG	"[CNX_FileWriiter] "
#include <NX_DbgMsg.h>

#include "CNX_SimpleFileWriter.h"

#define MAX_BUFFER_QCOUNT		64

//------------------------------------------------------------------------------
CNX_SimpleFileWriter::CNX_SimpleFileWriter()
	: m_bInit( false )
	, m_bRun( false )
	, m_OutFd( 0 )
	, m_nWritingMode( WRITING_MODE_NORMAL )
	, m_bEnableWriting( false )
{
	m_pStatistics	= new CNX_Statistics();

	pthread_mutex_init( &m_hWriteLock, NULL );
	pthread_mutex_init( &m_hTimeLock, NULL );
}

//------------------------------------------------------------------------------
CNX_SimpleFileWriter::~CNX_SimpleFileWriter()
{
	if( true == m_bInit )
		Deinit();

	pthread_mutex_destroy( &m_hWriteLock );
	pthread_mutex_destroy( &m_hTimeLock );

	delete m_pStatistics;
}

//------------------------------------------------------------------------------
void	CNX_SimpleFileWriter::Init( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );

	if( false == m_bInit ) {
		AllocateBuffer();
		m_bInit = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_SimpleFileWriter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bInit )
	{
		if( m_bRun ) {
			Stop();
		}

		FreeBuffer();
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_SimpleFileWriter::Receive( CNX_Sample *pSample )
{
	CNX_AutoLock lock ( &m_hWriteLock );
	NX_ASSERT( NULL != pSample );

	uint8_t	*pSrcBuffer;
	int32_t srcSize;
	
	pthread_mutex_lock( &m_hTimeLock );
	m_CurTimeStamp = pSample->GetTimeStamp();
	pthread_mutex_unlock( &m_hTimeLock );
	
	((CNX_MediaSample*)pSample)->GetBuffer( &pSrcBuffer, &srcSize);

	if( m_OutFd ) {
		m_pStatistics->CalculateBpsStart();
		int32_t ret = write( m_OutFd, pSrcBuffer, srcSize );
		printf("Push stream. (buf = %p, size = %d)\n", pSrcBuffer, srcSize);
		m_pStatistics->CalculateBpsEnd( srcSize );

		// static int32_t samplingCount = 0;
		// samplingCount++;
		// if( !(samplingCount % 4) ) {
		// 	samplingCount = 0;
		// 	NxDbgMsg( NX_DBG_DEBUG, (TEXT("Write( %p, %d ), Bitrate( %.03f Kbps )\n"), 
		// 		pSrcBuffer, srcSize, m_pStatistics->GetBpsCurrent()) );
		// }

		if( 0 > ret ) {
			if( m_pNotify )
				m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
		}
	}

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_SimpleFileWriter::ReleaseSample( CNX_Sample *pSample )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_SimpleFileWriter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_SimpleFileWriter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bRun = false;
		if( m_bEnableWriting ) {
			StopWriting();
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_SimpleFileWriter::AllocateBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_SimpleFileWriter::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_SimpleFileWriter::StartWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	GetFileNameFromCallback();
	unlink( (char*)m_FileName );

	m_OutFd = open( (char*)m_FileName, O_WRONLY | O_CREAT, 0777 | O_TRUNC | O_SYNC );
	printf("Create File( %d ) : %s\n", m_OutFd, m_FileName);
	if( m_OutFd < 0)
	{
		if( m_pNotify )
			m_pNotify->EventNotify( 0xF003, m_FileName, strlen((char*)m_FileName) + 1 );
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_SimpleFileWriter::StopWriting( void )
{
	NxDbgMsg( NX_DBG_VBS, ("%s IN\n", __func__) );
	if( m_OutFd > 0){
		close( m_OutFd );
		m_OutFd = 0;

		if( m_nWritingMode == WRITING_MODE_NORMAL ) {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1001, m_FileName, strlen((char*)m_FileName) + 1 );
		}
		else {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1002, m_FileName, strlen((char*)m_FileName) + 1 );
		}
	}
	NxDbgMsg( NX_DBG_VBS, ("%s OUT\n", __func__) );

	return true;
}

//------------------------------------------------------------------------------
void CNX_SimpleFileWriter::GetFileNameFromCallback( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( FileNameCallbackFunc ) {		// user define filename
		uint32_t bufSize = 0;
		if( !FileNameCallbackFunc(m_FileName, bufSize) ) {
			// Error!
		}
	} else {							// default filename
		time_t eTime;
		struct tm *eTm;

		time( &eTime );
		//eTm = gmtime( &eTime );
		eTm = localtime( &eTime );

		sprintf((char*)m_FileName, "aud_%04d%02d%02d_%02d%02d%02d.pcm",
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec );
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_SimpleFileWriter::RegFileNameCallback( int32_t (*cbFunc)(uint8_t *, uint32_t) )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( cbFunc );
	if( cbFunc )
	{
		FileNameCallbackFunc = cbFunc;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()-\n"), __func__) );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_SimpleFileWriter::EnableWriting( bool enable )
{
	CNX_AutoLock lock ( &m_hWriteLock );
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnableWriting)?"Enable":"Disable", (enable)?"Enable":"Disable") );
	if( !m_bRun ) {
		m_bEnableWriting = enable;
		return true;
	}

	if( enable ) {
		//	Diable --> Enable
		if( !m_bEnableWriting ){
			m_bEnableWriting = enable;
			StartWriting();
		}
	} 
	else {
		//	Enable --> Disable
		if( m_bEnableWriting ){
			m_bEnableWriting = enable;
			StopWriting();
		}
	}
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_SimpleFileWriter::SetWritingMode( int32_t mode )
{
	m_nWritingMode = mode;

	return 0;
}

uint64_t CNX_SimpleFileWriter::GetTimeStamp( void )
{
	uint64_t timeStamp;

	pthread_mutex_lock( &m_hTimeLock );
	timeStamp = m_CurTimeStamp;
	pthread_mutex_unlock( &m_hTimeLock );

	return timeStamp;
}