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

#ifndef __INX_JPEGCAPTURE_H__
#define __INX_JPEGCAPTURE_H__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <stdint.h>
#include <nx_video_api.h>

#include "CNX_BaseFilter.h"

#ifdef __cplusplus

#define MAX_JPG_HEADER_SIZE		(4 * 1024)
#define MAX_JPG_BUFFER			4

typedef NX_VID_ENC_INIT_PARAM		ENC_INFO;

class INX_JpegCapture
{
public:
	INX_JpegCapture()
		: m_bThreadExit( true )
		, m_hThread( 0x00 )
		, m_JpegQuality( 100 )
		, m_pNotify( NULL )
	{
		m_pSemIn	= new CNX_Semaphore(MAX_JPG_BUFFER, 0);

		m_bThreadExit 	= false;
		pthread_create( &this->m_hThread, NULL, this->ThreadMain, this );
	}
	~INX_JpegCapture()
	{
		m_bThreadExit = true;
		m_pSemIn->Post();
		pthread_join( m_hThread, NULL );

		delete m_pSemIn;
	}

public:
	int32_t SetFileName( char *fileName )
	{
		sprintf( m_JpegFileName, "%s", fileName );
		fileName[0] = 0x00;
		return 0;
	}
	int32_t SetQuality( uint32_t quality )
	{
		m_JpegQuality = quality;
		return 0;
	}	
	int32_t Encode( CNX_Sample *pSample )
	{
		pSample->Lock();
		m_SampleInQueue.PushSample( pSample );
		m_pSemIn->Post();

		return true;
	}
	void SetNotifier( INX_EventNotify *pNotify )
	{ 
		if( pNotify ) m_pNotify = pNotify;
	}

protected: 
	int32_t GetSample( CNX_Sample **ppSample )
	{
		m_pSemIn->Pend();
		if( true == m_SampleInQueue.IsReady() ) {
			m_SampleInQueue.PopSample( ppSample );
			return true;
		}
		return false;
	}
	void ThreadLoop( void )
	{
		CNX_VideoSample *pSample = NULL;

		while( !m_bThreadExit )
		{
			if( false == GetSample( (CNX_Sample **)&pSample) ) {
				continue;
			}
			if( NULL == pSample ) {
				continue;
			}

			JpegEncode( pSample->GetVideoMemory() );
			pSample->Unlock();
		}
	}
	static void* ThreadMain( void *arg )
	{
		INX_JpegCapture *pClass = (INX_JpegCapture *)arg;
		pClass->ThreadLoop();
		return (void*)0xDEADDEAD;
	}	
	int32_t JpegEncode( NX_VID_MEMORY_INFO *pVideoMemory )
	{
		int32_t fd = 0, writeSize = 0;

		NX_VID_ENC_HANDLE	hJpegEnc = NULL;
		ENC_INFO			jpegEncInfo;
		NX_VID_ENC_OUT		jpegEncOut;
		int32_t 			jpegHeaderSize = 0;
		uint8_t				jpegHeader[MAX_JPG_HEADER_SIZE];

		memset( &jpegEncInfo, 0x00, sizeof(jpegEncInfo) );
		jpegEncInfo.width			= pVideoMemory->imgWidth;
		jpegEncInfo.height			= pVideoMemory->imgHeight;
		jpegEncInfo.rotAngle		= 0;
		jpegEncInfo.mirDirection	= 0;
		jpegEncInfo.jpgQuality		= m_JpegQuality;
		
		hJpegEnc = NX_VidEncOpen( NX_JPEG_ENC, NULL );
		NX_VidEncInit( hJpegEnc, &jpegEncInfo );
		
		memset( jpegHeader, 0x00, sizeof(jpegHeader) );
		memset( &jpegEncOut, 0x00, sizeof(jpegEncOut) );
		NX_VidEncJpegGetHeader( hJpegEnc, jpegHeader, &jpegHeaderSize );
		NX_VidEncJpegRunFrame( hJpegEnc, pVideoMemory, &jpegEncOut );

		fd = open( (char*)m_JpegFileName, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0777 );

		if( fd >= 0 && jpegHeaderSize > 0 && jpegEncOut.bufSize > 0 ) {
			// jpeg writing
			writeSize += write( fd, jpegHeader, jpegHeaderSize );
			writeSize += write( fd, jpegEncOut.outBuf, jpegEncOut.bufSize );
			if( fd >= 0) close( fd );

			if( writeSize == jpegHeaderSize + jpegEncOut.bufSize ) {
				// Jpeg encoding & file writing sucess.
				if( m_pNotify )
					m_pNotify->EventNotify( 0x1003, m_JpegFileName, strlen((char*)m_JpegFileName) + 1 );

				if( hJpegEnc )	NX_VidEncClose( hJpegEnc );
				return writeSize;		
			}
		}

		// Jpeg encoding Error. ( WriteSize != RealdataSize, fd is invalid )
		if( m_pNotify )
			m_pNotify->EventNotify( 0xF003, m_JpegFileName, strlen((char*)m_JpegFileName) + 1 );

		if( fd >= 0) close(fd);
		if( hJpegEnc )	NX_VidEncClose( hJpegEnc );
		
		return -1;
	}

protected:
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;

	CNX_Semaphore		*m_pSemIn;
	CNX_SampleQueue		m_SampleInQueue;
	
	char				m_JpegFileName[1024];
	uint32_t			m_JpegQuality;

	INX_EventNotify 	*m_pNotify;
};

#endif	// __cplusplus

#endif	// __INX_JPEGCAPTURE_H__
