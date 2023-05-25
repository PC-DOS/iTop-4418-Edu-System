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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "CNX_HLSFilter.h"
#define	NX_DTAG	"[CNX_HLSFilter] "
#include "NX_DbgMsg.h"

#define MAX_HLS_QCOUNT		64

#define HLS_TEMP_DIR 		"/tmp"
#define HLS_TEMP_M3U8		"hls_temp.m3u8"

//#define META_FILE_WITH_URL

#ifdef META_FILE_WITH_URL
#define DEFAULT_ETHERNET_DEV		"wlan0"
#endif

//------------------------------------------------------------------------------
CNX_HLSFilter::CNX_HLSFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0x00 )
	, m_MaxDuration( 10 )
	, m_Duration( 10 )
	, m_MediaSequence( 0 )
	, m_OutFd( 0 )
	, m_CurSampleTime( 0 )
	, m_LastSampleTime( 0 )
{
	m_pSemIn	= new CNX_Semaphore( MAX_HLS_QCOUNT, 0 );
	NX_ASSERT( m_pSemIn );

	memset( m_M3U8Root, 0x00, sizeof(m_M3U8Root) );
	memset( m_M3U8Name, 0x00, sizeof(m_M3U8Name) );
	memset( m_SegmentRoot, 0x00, sizeof(m_SegmentRoot) );
	memset( m_SegmentName, 0x00, sizeof(m_SegmentName) );

	memset( m_URLPrefix, 0x00, sizeof(m_URLPrefix) );
	memset( m_SegmentFileName, 0x00, sizeof(m_SegmentFileName) );

	m_SampleInQueue.Reset();
}

//------------------------------------------------------------------------------
CNX_HLSFilter::~CNX_HLSFilter()
{
	if( true == m_bInit )
		Deinit();

	delete m_pSemIn;
}

//------------------------------------------------------------------------------
void CNX_HLSFilter::Init( NX_HLS_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );

	if( false == m_bInit ) {
		sprintf( m_M3U8Root, "%s", pConfig->SegmentRoot );
		sprintf( m_M3U8Name, "%s", pConfig->MetaFileName );
		sprintf( m_SegmentRoot, "%s", pConfig->SegmentRoot );
		sprintf( m_SegmentName, "%s", pConfig->SegmentName );

		m_SegmentFileNum	= pConfig->SegmentNumber;
		m_MaxDuration		= pConfig->SegmentDuration;
		m_Duration			= pConfig->SegmentDuration;	

#ifdef META_FILE_WITH_URL
		GetIPAddress( m_URLPrefix );
#endif
		m_bInit = true;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_HLSFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );
	
	if( true == m_bInit ) {
		if( m_bRun ) {
			Stop();
		}
		m_bInit = false;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_HLSFilter::Receive( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != pSample );

	pSample->Lock();
	Deliver( pSample );
	m_SampleInQueue.PushSample(pSample);
	m_pSemIn->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_HLSFilter::ReleaseSample( CNX_Sample *pSample )
{
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_HLSFilter::Run( void )
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
int32_t CNX_HLSFilter::Stop( void )
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
void CNX_HLSFilter::AllocateBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_HLSFilter::FreeBuffer( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_pSemIn->Post();		//	Send Dummy
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_HLSFilter::GetSample( CNX_Sample **ppSample)
{
	m_pSemIn->Pend();
	if( true == m_SampleInQueue.IsReady() ) {
		m_SampleInQueue.PopSample( ppSample );
		return true;
	}
	return false;

}

//------------------------------------------------------------------------------
int32_t CNX_HLSFilter::GetDeliverySample( CNX_Sample **ppSample )
{
	return false;
}

//------------------------------------------------------------------------------
void CNX_HLSFilter::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );

	CNX_MuxerSample		*pSample = NULL;
	
	ChangeFileName();

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

		if( (m_CurSampleTime == 0) && (m_LastSampleTime == 0) )
			m_CurSampleTime = m_LastSampleTime = pSample->GetTimeStamp();

		m_CurSampleTime = pSample->GetTimeStamp();
		
		if( (m_LastSampleTime + (m_Duration * 1000)) < m_CurSampleTime ) {
			m_LastSampleTime = m_CurSampleTime;

			if( m_OutFd ) close( m_OutFd );
			
			if( !MakeMetaInfo() ) {
				NxDbgMsg( NX_DBG_ERR, (TEXT("MakeMetaInfo() failed!\n")) );
			}
			ChangeFileName();
		}
		
		if( m_OutFd ) {
			uint8_t *pData;
			int32_t dataSize, writeSize;
			pSample->GetBuffer( &pData, &dataSize );
			if( dataSize > 0 ) {
				writeSize = write( m_OutFd, pData, dataSize);

				if( dataSize != writeSize ) {
					NxDbgMsg( NX_DBG_ERR, (TEXT("write failed.(dataSize = %d, writeSize = %d\n"), dataSize, writeSize) );
				}
			}
		}

		if( pSample )
			pSample->Unlock();
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_HLSFilter::ThreadMain(void*arg)
{
	CNX_HLSFilter *pClass = (CNX_HLSFilter *)arg;

	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
#ifdef META_FILE_WITH_URL
int32_t CNX_HLSFilter::GetIPAddress( char *addr )
{
	int32_t sock;
	struct ifreq		ifr;
	struct sockaddr_in	*sin;

	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if( sock < 0 ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("Socket error!\n")) );
		return false;
	}

	strcpy( ifr.ifr_name, DEFAULT_ETHERNET_DEV );
	if( ioctl(sock, SIOCGIFADDR, &ifr)< 0) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("ioctl() error!\n")) );
		close(sock);
		return false;
	}

	sin = (sockaddr_in*)&ifr.ifr_addr;
	strcpy(addr, inet_ntoa(sin->sin_addr));

	close(sock);
	return true;
}
#endif

//------------------------------------------------------------------------------
void CNX_HLSFilter::ChangeFileName( void )
{
	snprintf(m_SegmentFileName, sizeof(m_SegmentFileName), "%s/%s-%lld.ts", m_SegmentRoot, m_SegmentName, m_MediaSequence++);
	unlink( m_SegmentFileName );
	m_OutFd = open( (char*)m_SegmentFileName, O_WRONLY | O_CREAT | O_TRUNC, 0777 );
	NxDbgMsg( NX_DBG_VBS, (TEXT("Create HLS File( %d ) : %s\n"), m_OutFd, m_SegmentFileName) );
}

//------------------------------------------------------------------------------
int32_t CNX_HLSFilter::MakeMetaInfo( void )
{
	FILE *fp = NULL;
	char buf[1024], buf_temp[1024];
	
	// snprintf( buf_temp, sizeof(buf_temp), "%s/%s", HLS_TEMP_DIR, HLS_TEMP_M3U8 );
	snprintf( buf_temp, sizeof(buf), "%s/%s", m_SegmentRoot, m_M3U8Name);
	
	if( NULL == (fp = fopen( buf_temp, "w") ) ) {
		NxDbgMsg( NX_DBG_ERR, (TEXT("open failed( %s ).\n"), buf_temp) );
		return false;
	}

	if( m_MediaSequence < m_SegmentFileNum ) {
		snprintf( buf, sizeof(buf), "#EXTM3U\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:%d\n", m_MaxDuration, 0);
		fwrite( buf, strlen( buf ), 1, fp );

		for( int32_t i = 0; i < m_MediaSequence; i++ )
		{
#ifdef META_FILE_WITH_URL			
			snprintf( buf, sizeof(buf), "#EXTINF:%d,\nhttp://%s/%s-%d.ts\n", m_MaxDuration, m_URLPrefix, m_SegmentName, i );
#else
			snprintf( buf, sizeof(buf), "#EXTINF:%d,\n%s-%d.ts\n", m_MaxDuration, m_SegmentName, i );
#endif
			fwrite( buf, strlen( buf ), 1, fp );
		}
	}
	else
	{
		snprintf( buf, sizeof(buf), "#EXTM3U\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:%lld\n", m_MaxDuration, m_MediaSequence - m_SegmentFileNum);
		fwrite( buf, strlen( buf ), 1, fp );

		for(int32_t i = 0; i < m_SegmentFileNum; i++)
		{
#ifdef META_FILE_WITH_URL			
			snprintf( buf, sizeof(buf), "#EXTINF:%d,\nhttp://%s/%s-%lld.ts\n", m_MaxDuration, m_URLPrefix, m_SegmentName, m_MediaSequence - m_SegmentFileNum + i );
#else			
			snprintf( buf, sizeof(buf), "#EXTINF:%d,\n%s-%lld.ts\n", m_MaxDuration, m_SegmentName, m_MediaSequence - m_SegmentFileNum + i );
#endif			
			fwrite( buf, strlen( buf ), 1, fp );
		}
	}

	if( m_MediaSequence > m_SegmentFileNum + ERASE_SEGMENT_MARGIN ) {
		snprintf( buf, sizeof(buf), "%s/%s-%lld.ts", m_SegmentRoot, m_SegmentName, m_MediaSequence - m_SegmentFileNum - (ERASE_SEGMENT_MARGIN + 1));
		unlink( buf );
	}

	if( fp ) 
		fclose(fp);

	// snprintf( buf, sizeof(buf), "%s/%s", m_SegmentRoot, m_M3U8Name);
	// return !(rename(buf_temp, buf);
	return true;
}
