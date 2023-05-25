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

#ifndef __CNX_VIDEODECODER_H__
#define __CNX_VIDEODECODER_H__

#include <nx_video_api.h>
#include <nx_alloc_mem.h>
#include <nx_fourcc.h>

#include "CNX_BaseFilter.h"
#include "media_reader.h"
#include "NX_FilterConfigTypes.h"

#ifdef __cplusplus

class CNX_VideoDecoder
	: public CNX_BaseFilter
{
public:
	CNX_VideoDecoder();
	virtual ~CNX_VideoDecoder();

public:
	//------------------------------------------------------------------------
	virtual void		Init( void );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );
	//------------------------------------------------------------------------

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *pArg );

public:
			int32_t		SetFileName( uint8_t *pFileName );
			int32_t 	GetVideoResolution( int32_t *width, int32_t *height );
			int32_t 	GetVideoFramerate( int32_t *fpsNum, int32_t *fpsDen );
			int32_t 	SetFrameDown( int32_t targetFps );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t					m_bInit;
	int32_t					m_bRun;
	CNX_Semaphore			*m_pSemOut;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t					m_bThreadExit;
	pthread_t				m_hThread;
	//------------------------------------------------------------------------
	//	Decoder
	//------------------------------------------------------------------------
	NX_VID_DEC_HANDLE 		m_hDec;
	CMediaReader			*m_pMediaReader;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 4, MAX_SEQ_BUF_SIZE = 4 * 1024, MAX_STREAM_BUF_SIZE = 4 * 1024 * 1024};	

	int32_t 				m_SkipFrame;
};

#endif //	__cplusplus

#endif	// __CNX_VIDEODECODER_H__