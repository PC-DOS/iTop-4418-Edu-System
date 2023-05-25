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

//#define DEBUG_WRITING

#define	NX_DTAG	"[CNX_FileWriiter] "
#include <NX_DbgMsg.h>

#include "CNX_FileWriter.h"

#define MAX_BUFFER_QCOUNT		64

//------------------------------------------------------------------------------
CNX_FileWriter::CNX_FileWriter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0 )
	, m_OutFd( -1 )
	, m_CurWriteBuffer( NULL )
	, m_CurWritePos( 0 )
	, m_WritingMode( WRITING_MODE_NORMAL )
{
	m_pSemWriter	= new CNX_Semaphore( MAX_BUFFER_QCOUNT, 0 );
	m_pStatistics	= new CNX_Statistics();
	m_pStreamBuf	= new CNX_Statistics();
	m_pWriterBuf	= new CNX_Statistics();

	memset( m_FileName, 0x00, sizeof(m_FileName) );

	for( int32_t i = 0; i < MAX_NUM_MEDIA_SAMPLES; i++ )
	{
		m_pStreamBuffer[i] = NULL;
	}
}

//------------------------------------------------------------------------------
CNX_FileWriter::~CNX_FileWriter()
{
	if( true == m_bInit )
		Deinit();

	delete m_pSemWriter;
	delete m_pStatistics;
	delete m_pStreamBuf;
	delete m_pWriterBuf;
}

//------------------------------------------------------------------------------
void	CNX_FileWriter::Init( void )
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
void	CNX_FileWriter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bInit ) {
		if( m_bRun ) {
			Stop();
		}

		FreeBuffer();
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_FileWriter::Receive( CNX_Sample *pSample )
{
#if(1)
	CNX_MediaSample *pInSample = (CNX_MediaSample*)pSample;
	NX_ASSERT( NULL != pInSample );

	pSample->Lock();

	uint8_t	*pSrcBuf = NULL;
	int32_t srcSize, writable, dataSize;
	int32_t flags = pInSample->GetFlags();

	// a.check start dummy sample.
	if( flags == FLAGS_WRITING_NORMAL_START || flags == FLAGS_WRITING_EVENT_START ) {
		// a-1. Get file name & remove duplicate filename.
		GetFileNameFromCallback();
		unlink( (char*)m_FileName );
		
		// a-2. Open file
		if( 0 > (m_OutFd = open( (char*)m_FileName, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0777 ))) {
			if( m_pNotify )
				m_pNotify->EventNotify( 0xF003, m_FileName, strlen((char*)m_FileName) + 1 );
		}
		
		if( flags == FLAGS_WRITING_NORMAL_START ) {
			NxDbgMsg( NX_DBG_INFO, (TEXT("Create Normal File( %d ) : %s\n"), m_OutFd, m_FileName) );
			m_WritingMode = WRITING_MODE_NORMAL;
		}
		if( flags == FLAGS_WRITING_EVENT_START ) {
			NxDbgMsg( NX_DBG_INFO, (TEXT("Create Event File( %d ) : %s\n"), m_OutFd, m_FileName) );
			m_WritingMode = WRITING_MODE_EVENT;
		}

		pSample->Unlock();
		return true;
	}
	// b.check stop dummy sample.
	else if( flags == FLAGS_WRITING_NORMAL_STOP || flags == FLAGS_WRITING_EVENT_STOP ) {
		// b-1. check writing buffer
		while( m_WriterQueue.IsReady() );

		// b-2.finalize last remind buffer.
		if( m_CurWritePos ) {
			m_pStatistics->CalculateBpsStart();
			int32_t ret = write( m_OutFd, m_CurWriteBuffer, m_CurWritePos );
			m_pStatistics->CalculateBpsEnd(m_CurWritePos);

			if( 0 > ret ) {
				if( m_pNotify )
					m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
			}
#ifdef DEBUG_WRITING
			NxDbgMsg( NX_DBG_DEBUG, (TEXT("[%d/%d] [%d/%d] Write( %p, %d ), Bitrate( %.03f Kbps / %.03f MB/s )\n"), 
				m_pStreamBuf->GetBufNumberCur(), m_pStreamBuf->GetBufNumberMax(), 
				m_pWriterBuf->GetBufNumberCur(), m_pWriterBuf->GetBufNumberMax(), 
				m_CurWriteBuffer, m_CurWritePos, m_pStatistics->GetBpsCurrent(), m_pStatistics->GetBpsCurrent() / 8 / 1024) );
#endif			
			m_CurWritePos = 0;
		}
	
		// b-3. close file
		if( m_OutFd >= 0 )  {
			close( m_OutFd );
			m_OutFd = -1;

			// b-4. notifier
			if( flags == FLAGS_WRITING_NORMAL_STOP ) {
				if( m_pNotify ) {
					m_pNotify->EventNotify( 0x1001, m_FileName, strlen((char*)m_FileName) + 1 );
				}
			}
			if( flags == FLAGS_WRITING_EVENT_STOP ) {
				if( m_pNotify ) {
					m_pNotify->EventNotify( 0x1002, m_FileName, strlen((char*)m_FileName) + 1 );
				}
			}
		}

		pSample->Unlock();
		return true;
	}

	// c. push data file to write buffer.
	pInSample->GetBuffer( &pSrcBuf, &srcSize);
	dataSize = pInSample->GetActualDataLength();
	pSample->Unlock();

	m_pStreamBuf->CalculateBufNumber( NUM_WRITE_UNIT - m_StreamQueue.GetSampleCount() );
	m_pWriterBuf->CalculateBufNumber( m_WriterQueue.GetSampleCount() );

	do {
		if( m_CurWritePos + dataSize > SIZE_WRITE_UNIT ) {
			writable = SIZE_WRITE_UNIT - m_CurWritePos;
			memcpy( m_CurWriteBuffer + m_CurWritePos, pSrcBuf, writable );
			dataSize -= writable;
			pSrcBuf += writable;

			if( !m_StreamQueue.IsReady() ){
				NxDbgMsg( NX_DBG_ERR, (TEXT("Have No Writable Buffer.\n")) );
				return false;
			}

			m_WriterQueue.Push( m_CurWriteBuffer, SIZE_WRITE_UNIT );
			m_pSemWriter->Post();
			m_StreamQueue.Pop( (void**)&m_CurWriteBuffer, &m_CurWritePos );
			m_CurWritePos = 0;
		}
		else {
			memcpy( m_CurWriteBuffer + m_CurWritePos, pSrcBuf, dataSize );
			m_CurWritePos += dataSize;
			break;
		}
	} while(dataSize > 0);
#else
	CNX_MediaSample *pInSample = (CNX_MediaSample*)pSample;
	NX_ASSERT( NULL != pInSample );

	pSample->Lock();

	int32_t flags = pInSample->GetFlags();
	sprintf( (char*)m_FileName, "NOTHING" );

	// a.check start dummy sample.
	if( flags == FLAGS_WRITING_NORMAL_START || flags == FLAGS_WRITING_EVENT_START ) {
		if( flags == FLAGS_WRITING_NORMAL_START ) {
			NxDbgMsg( NX_DBG_INFO, (TEXT("Create Normal File : %s\n"), m_FileName) );
			m_WritingMode = WRITING_MODE_NORMAL;
		}
		if( flags == FLAGS_WRITING_EVENT_START ) {
			NxDbgMsg( NX_DBG_INFO, (TEXT("Create Event File : %s\n"), m_FileName) );
			m_WritingMode = WRITING_MODE_EVENT;
		}
	}
	// b.check stop dummy sample.
	else if( flags == FLAGS_WRITING_NORMAL_STOP || flags == FLAGS_WRITING_EVENT_STOP ) {
		if( flags == FLAGS_WRITING_NORMAL_STOP ) {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1001, m_FileName, strlen((char*)m_FileName) + 1 );
		}
		if( flags == FLAGS_WRITING_EVENT_STOP ) {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1002, m_FileName, strlen((char*)m_FileName) + 1 );
		}
	}
	pSample->Unlock();
#endif

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_FileWriter::ReleaseSample( CNX_Sample *pSample )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_FileWriter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bThreadExit = false;
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
int32_t	CNX_FileWriter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemWriter->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;		
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_FileWriter::AllocateBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	
	m_StreamQueue.Reset();
	for( int32_t i=0; i< NUM_WRITE_UNIT ; i++ )
	{
		m_pStreamBuffer[i] = new uint8_t[SIZE_WRITE_UNIT];
		NX_ASSERT(m_pStreamBuffer[i]);
		m_StreamQueue.Push(m_pStreamBuffer[i], 0);
	}
	
	m_WriterQueue.Reset();

	//	PopBuffer
	m_StreamQueue.Pop( (void**)&m_CurWriteBuffer, &m_CurWritePos );
	m_CurWritePos = 0;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_FileWriter::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	for( int32_t i=0; i< NUM_WRITE_UNIT ; i++ )
	{
		delete []m_pStreamBuffer[i];
	}	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_FileWriter::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	int32_t ret;

	uint8_t *pBuffer = NULL;
	int32_t size;

	while( !m_bThreadExit )
	{
		m_pSemWriter->Pend();

		if( m_WriterQueue.IsReady() ) {
			m_WriterQueue.Pop( (void **)&pBuffer, &size );

			if( 0 <= m_OutFd ) {
				m_pStatistics->CalculateBpsStart();
				ret = write( m_OutFd, pBuffer, size );
				m_pStatistics->CalculateBpsEnd( size );

				if( 0 > ret ) {
					if( m_pNotify )
						m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
				}
#ifdef DEBUG_WRITING
				NxDbgMsg( NX_DBG_DEBUG, (TEXT("[%d/%d] [%d/%d] Write( %p, %d ), Bitrate( %.03f Kbps / %.03f MB/s )\n"), 
					m_pStreamBuf->GetBufNumberCur(), m_pStreamBuf->GetBufNumberMax(), 
					m_pWriterBuf->GetBufNumberCur(), m_pWriterBuf->GetBufNumberMax(), 
					pBuffer, size, m_pStatistics->GetBpsCurrent(), m_pStatistics->GetBpsCurrent() / 8 / 1024) );
#endif
			} else {
				NxDbgMsg( NX_DBG_ERR, (TEXT("Have No writing handle.\n")) );
			}

			m_StreamQueue.Push( pBuffer, 0 );
		}
	}

	// Remind Buffer Flush
	while( m_WriterQueue.IsReady() )
	{
		m_WriterQueue.Pop( (void **)&pBuffer, &size );

		if( 0 <= m_OutFd ) {
			m_pStatistics->CalculateBpsStart();
			ret = write( m_OutFd, pBuffer, size );
			m_pStatistics->CalculateBpsEnd( size );

			if( 0 > ret ) {
				if( m_pNotify )
					m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
			}
#ifdef DEBUG_WRITING
			NxDbgMsg( NX_DBG_DEBUG, (TEXT("[%d/%d] [%d/%d] Write( %p, %d ), Bitrate( %.03f Kbps / %.03f MB/s )\n"), 
				m_pStreamBuf->GetBufNumberCur(), m_pStreamBuf->GetBufNumberMax(), 
				m_pWriterBuf->GetBufNumberCur(), m_pWriterBuf->GetBufNumberMax(), 
				pBuffer, size, m_pStatistics->GetBpsCurrent(), m_pStatistics->GetBpsCurrent() / 8 / 1024) );
#endif
		}
		m_StreamQueue.Push( pBuffer, 0 );
	}

	if( m_CurWritePos > 0 ) {
		if( 0 <= m_OutFd ) {
			ret = write( m_OutFd, m_CurWriteBuffer, m_CurWritePos );
			if( 0 > ret ) {
				if( m_pNotify )
					m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
			}
#ifdef DEBUG_WRITING
			NxDbgMsg( NX_DBG_DEBUG, (TEXT("[%d/%d] [%d/%d] Write( %p, %d ), Bitrate( %.03f Kbps / %.03f MB/s )\n"), 
				m_pStreamBuf->GetBufNumberCur(), m_pStreamBuf->GetBufNumberMax(), 
				m_pWriterBuf->GetBufNumberCur(), m_pWriterBuf->GetBufNumberMax(), 
				pBuffer, m_CurWritePos, m_pStatistics->GetBpsCurrent(), m_pStatistics->GetBpsCurrent() / 8 / 1024) );
#endif
		}
		m_CurWritePos = 0;
	}
	if( m_OutFd >= 0 ) {
		close( m_OutFd );
		if( m_WritingMode == WRITING_MODE_NORMAL) {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1001, m_FileName, strlen((char*)m_FileName) + 1 );
		}
		if( m_WritingMode == WRITING_MODE_EVENT) {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1002, m_FileName, strlen((char*)m_FileName) + 1 );
		}
		m_OutFd = -1;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_FileWriter::ThreadMain(void*arg)
{
	CNX_FileWriter *pClass = (CNX_FileWriter *)arg;

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
int32_t CNX_FileWriter::GetFileNameFromCallback( void )
{
	// user define filename.
	if( FileNameCallbackFunc ) {		
		uint32_t bufSize = 0;
		FileNameCallbackFunc( m_FileName, bufSize );
	}
	// default filename.
	else {
		time_t eTime;
		struct tm *eTm;

		time( &eTime );
		eTm = localtime( &eTime );

		sprintf((char*)m_FileName, "clip_%04d%02d%02d_%02d%02d%02d.ts",
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec );
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_FileWriter::RegFileNameCallback( int32_t (*cbFunc)(uint8_t *, uint32_t) )
{
	NX_ASSERT( cbFunc );
	if( cbFunc ) {
		FileNameCallbackFunc = cbFunc;
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_FileWriter::GetStatistics( NX_FILTER_STATISTICS *pStatistics )
{
	return true;
}