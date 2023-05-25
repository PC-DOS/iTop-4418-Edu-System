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
#include <string.h>

#include "CNX_TextOverlayFilter.h"
#include "NX_Font8x16.h"

#define	NX_DTAG	"[CNX_TextOverlayFilter] "
#include "NX_DbgMsg.h"
#include "NX_FilterSysApi.h"

#define	DUMP_Y			0
#define DUMP_PATH		""

#if( 1 )
#define NxDbgColorMsg(A, B) do {										\
								if( gNxFilterDebugLevel>=A ) {			\
									printf("\033[1;37;41m%s", NX_DTAG);	\
									DEBUG_PRINT B;						\
									printf("\033[0m\r\n");				\
								}										\
							} while(0)
#else
#define NxDbgColorMsg(A, B) do {} while(0)
#endif

//------------------------------------------------------------------------------
CNX_TextOverlayFilter::CNX_TextOverlayFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bEnable( false )
	, m_TextOverlayBufSize( MAX_TEXTOVERLAY_SIZE )
	, m_StartX( 0 )
	, m_StartY( 0 )
	, m_MaxInterval( 0 )

{
	memset( m_TextOverlayBuf, 0x00, sizeof(m_TextOverlayBuf) );
	memset( m_FontBuf, 0x00, sizeof(m_FontBuf) );
	pthread_mutex_init( &m_hEnableLock, NULL );
}

//------------------------------------------------------------------------------
CNX_TextOverlayFilter::~CNX_TextOverlayFilter()
{
	pthread_mutex_destroy( &m_hEnableLock );
}

//------------------------------------------------------------------------------
void	CNX_TextOverlayFilter::Init( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( false == m_bInit );

	if( false == m_bInit ) {
		m_bInit = true;
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_TextOverlayFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit ) {
		if( m_bRun )	Stop();
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_TextOverlayFilter::Receive( CNX_Sample *pSample )
{
	CNX_VideoSample *pVideoSample = (CNX_VideoSample *)pSample;
	NX_ASSERT( NULL != pVideoSample );

	pVideoSample->Lock();

	pthread_mutex_lock( &m_hEnableLock );
	int32_t bEnable = m_bEnable;
	pthread_mutex_unlock( &m_hEnableLock );

	if( bEnable ) {
		// uint64_t interval = NX_GetTickCount();
		GetTextOverlayFromCallback( pVideoSample->GetVideoMemory() );
		// interval = NX_GetTickCount() - interval;

		// if( interval > m_MaxInterval ) {
		// 	m_MaxInterval = interval;
		// 	printf("\033[1;37;41mTextOverlay Max Running Time = %lld\033[0m\r\n", m_MaxInterval );
		// }
	}
	else {
		NxDbgColorMsg( NX_DBG_WARN, (TEXT("Textoverlay filter is disable.")) );
	}

	Deliver( pSample );
	pVideoSample->Unlock();

	return true;

}

//------------------------------------------------------------------------------
int32_t CNX_TextOverlayFilter::ReleaseSample(CNX_Sample *pSample)
{
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_TextOverlayFilter::Run(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if (m_bRun == false) {
		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_TextOverlayFilter::Stop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if (true == m_bRun) {
		m_bRun = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_TextOverlayFilter::GetTextOverlayFromCallback( NX_VID_MEMORY_INFO *pVideoMemory )
{
	NX_ASSERT( pVideoMemory );

	if( TextOverlayCallbackFunc ) {
		TextOverlayCallbackFunc( m_TextOverlayBuf, &m_TextOverlayBufSize, &m_StartX, &m_StartY );
		TextOverlay( pVideoMemory );
	}

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_TextOverlayFilter::TextOverlay( NX_VID_MEMORY_INFO *pVideoMemory )
{
	NX_ASSERT( pVideoMemory );

	uint32_t i, j, k;
	int32_t overRange = m_StartX + m_TextOverlayBufSize * BITMAP_FONT_WIDTH - pVideoMemory->imgWidth;

	for( i = 0; i < m_TextOverlayBufSize; i++) {
		for( j = 0; j < BITMAP_FONT_HEIGHT; j++ ) {
			for( k = 0;  k < BITMAP_FONT_WIDTH; k++ ) {
				m_FontBuf[j][k + i * BITMAP_FONT_WIDTH] = (font8x16[m_TextOverlayBuf[i]][j] >> (BITMAP_FONT_WIDTH - k) & 0x01) ? YUV_COLOR_FONT : YUV_COLOR_BACKGROUND;
			}
		}
	}

	for( i = 0; i < BITMAP_FONT_HEIGHT; i++ )
	{
		if( m_StartY + i < (uint32_t)pVideoMemory->imgHeight ) {
			memcpy((uint8_t*)(pVideoMemory->luVirAddr + pVideoMemory->luStride * (m_StartY + i) + m_StartX), m_FontBuf[i], overRange <= 0 ? BITMAP_FONT_WIDTH * m_TextOverlayBufSize : BITMAP_FONT_WIDTH * m_TextOverlayBufSize - overRange );
		}
	}

#if( DUMP_Y )
	FILE *outFp = NULL;
	char dump_file_name[512];

	time_t eTime;
	struct tm *eTm;
	
	time( &eTime );
	eTm = localtime( &eTime );
	sprintf(dump_file_name, "%s/textoverlay_%dx%d_%04d%02d%02d%02d%02d%02d.yuv", 
		DUMP_PATH, 
		pVideoMemory->imgWidth, pVideoMemory->imgHeight,
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec );

	outFp = fopen(dump_file_name, "wb+");

	if( outFp ) {
		fwrite( (void*)pVideoMemory->luVirAddr, 1, pVideoMemory->imgWidth * pVideoMemory->imgHeight, outFp );
	}

	if( outFp ) {
		fclose(outFp);
		outFp = NULL;
	}
	NxDbgMsg( NX_DBG_WARN, (TEXT("Dump File.( %s, %d byte)\n"), dump_file_name, pVideoMemory->imgWidth * pVideoMemory->imgHeight) );
#endif

	return true;
}

//------------------------------------------------------------------------------
void	CNX_TextOverlayFilter::RegTextOverlayCallback( int32_t(*cbFunc)( uint8_t *, uint32_t *, uint32_t *, uint32_t *) )
{
	NX_ASSERT( cbFunc );
	if( cbFunc ) {
		TextOverlayCallbackFunc = cbFunc;
	}
}

//------------------------------------------------------------------------------
int32_t	CNX_TextOverlayFilter::EnableTextOverlay( uint32_t enable )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s++\n"), __func__) );
	pthread_mutex_lock( &m_hEnableLock );
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnable)?"Enable":"Disable", (enable)?"Enable":"Disable") );
	m_bEnable = enable;
	pthread_mutex_unlock( &m_hEnableLock );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	
	return true;
}