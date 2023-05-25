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

#ifndef __CNX_TRANSCODINGMANAGER_H__
#define __CNX_TRANSCODINGMANAGER_H__

#include <CNX_RefClock.h>
#include <CNX_VideoDecoder.h>
#include <CNX_H264Encoder.h>
#include <CNX_VRFilter.h>
#include <CNX_Mp4MuxerFilter.h>

#include <NX_FilterConfigTypes.h>

#include "INX_TranscodingManager.h"

class CNX_TranscodingManager
	: public INX_TranscodingManager
{
public:
	CNX_TranscodingManager();
	~CNX_TranscodingManager();

public:
	virtual int32_t	Init( NX_TRANSCODING_MGR_CONFIG *pConfig );
	virtual int32_t	Deinit( void );

	virtual int32_t	Start( void );
	virtual int32_t	Stop( void );

	virtual int32_t RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) );

private:
	int32_t	SetConfig( NX_TRANSCODING_MGR_CONFIG *pConfig );
	int32_t	BuildFilter( void );
	void	SetNotifier( void );

private:
	CNX_RefClock 			*m_pRefClock;
	CNX_TranscodingNotify	*m_pNotifier;

	CNX_VideoDecoder		*m_pDecoder;
	CNX_VRFilter			*m_pVidRender;
	CNX_H264Encoder			*m_pEncoder;
	CNX_Mp4MuxerFilter		*m_pMp4Muxer;

	NX_VIDRENDER_CONFIG		m_VidRenderConfig;
	NX_VIDENC_CONFIG		m_VidEncConfig;
	NX_MP4MUXER_CONFIG		m_Mp4MuxerConfig;

private:
	int32_t					m_bInit;
	int32_t					m_bRun;
	uint8_t					*m_pInFileName;
	uint8_t					*m_pOutFileName;
	pthread_mutex_t			m_hLock;	
};

INX_TranscodingManager *GetTranscodingHandle( void )
{
	return (INX_TranscodingManager *)new CNX_TranscodingManager();
}

void ReleaseTranscodingHandle(INX_TranscodingManager *iTranscodingMgr)
{
	if( iTranscodingMgr ) delete (CNX_TranscodingManager*)iTranscodingMgr;
}

#endif	// __CNX_TRANSCODINGMANAGER_H__
