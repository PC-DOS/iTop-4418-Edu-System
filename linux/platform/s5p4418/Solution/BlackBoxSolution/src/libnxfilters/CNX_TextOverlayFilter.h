
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

#ifndef __CNX_TEXTOVERLAYFILTER_H__
#define __CNX_TEXTOVERLAYFILTER_H__

#include <CNX_BaseFilter.h>

#define MAX_TEXTOVERLAY_SIZE		256

#define BITMAP_FONT_WIDTH			8
#define BITMAP_FONT_HEIGHT			16

#define YUV_COLOR_FONT				0xFF
#define YUV_COLOR_BACKGROUND		0x5F

class CNX_TextOverlayFilter
	: public CNX_BaseFilter
{
public:
	CNX_TextOverlayFilter();
	~CNX_TextOverlayFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( void );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual int32_t		Run( void );
	virtual int32_t		Stop( void );
	//------------------------------------------------------------------------

protected:
			int32_t		GetTextOverlayFromCallback( NX_VID_MEMORY_INFO *pVideoMemory );
			int32_t		TextOverlay( NX_VID_MEMORY_INFO *pVideoMemory );
			//	Application callback functions for overlay
			int32_t		(*TextOverlayCallbackFunc)( uint8_t *buf, uint32_t *bufSize, uint32_t *x, uint32_t *y);

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			void		RegTextOverlayCallback( int32_t(*cbFunc)( uint8_t *, uint32_t *, uint32_t *, uint32_t *) );
			int32_t		EnableTextOverlay( uint32_t enable );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	int32_t				m_bEnable;
	pthread_mutex_t		m_hEnableLock;
	//------------------------------------------------------------------------
	//	Overlay buffer
	//------------------------------------------------------------------------
	uint8_t				m_TextOverlayBuf[MAX_TEXTOVERLAY_SIZE];
	uint32_t			m_TextOverlayBufSize;
	uint32_t			m_StartX, m_StartY;
	uint8_t				m_FontBuf[BITMAP_FONT_HEIGHT][MAX_TEXTOVERLAY_SIZE * BITMAP_FONT_WIDTH];
	uint64_t			m_MaxInterval;
};

#endif	// __CNX_TEXTOVERLAYFILTER_H__
