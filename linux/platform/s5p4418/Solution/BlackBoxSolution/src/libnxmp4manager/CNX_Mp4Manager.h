//------------------------------------------------------------------------------
//
//	Copyright (C) 2014 Nexell Co. All Rights Reserved
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

#ifndef __CNX_MP4MANAGER_H__
#define __CNX_MP4MANAGER_H__

#include <CNX_RefClock.h>
#include <NX_FilterConfigTypes.h>

#include <CNX_VIPFilter.h>
#include <CNX_VRFilter.h>
#include <CNX_H264Encoder.h>
#include <CNX_AudCaptureFilter.h>
#include <CNX_AacEncoder.h>
#include <CNX_InterleaverFilter.h>
#include <CNX_Mp4MuxerFilter.h>

#include "CNX_Mp4Notify.h"
#include "INX_Mp4Manager.h"

enum {
	MP4_CODEC_TYPE_MPEG4	= 0x20,
	MP4_CODEC_TYPE_H264		= 0x21,
	MP4_CODEC_TYPE_AAC		= 0x40,
	MP4_CODEC_TYPE_MP3		= 0x6B,
};

enum {
	FRAME_SIZE_AAC			= 1024,
	FRAME_SIZE_MP3			= 1152,
};

class CNX_Mp4Manager
	: public INX_Mp4Manager
{
public:
	CNX_Mp4Manager();
	~CNX_Mp4Manager();

public:
	virtual int32_t	SetConfig( Mp4ManagerConfig *pConfig );
	
	virtual int32_t Init( void );
	virtual int32_t Deinit( void );
	
	virtual int32_t Start( char *pFileName );
	virtual int32_t Stop( void );

	virtual int32_t Capture( char *pFileName );
	virtual int32_t EnableRender( int32_t enable );
	
	virtual int32_t RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t *, uint32_t) );

private:
	int32_t BuildFilter( void );
	void	SetNotifier( void );

private:
	CNX_RefClock 			*m_pRefClock;
	CNX_Mp4Notify			*m_pNotifier;

	CNX_VIPFilter			*m_pVipFilter;
	CNX_VRFilter			*m_pVrFilter;
	CNX_H264Encoder			*m_pAvcEncFilter;

	CNX_AudCaptureFilter	*m_pAudCapFilter;
	CNX_AacEncoder			*m_pAacEncFilter;

	CNX_Mp4MuxerFilter		*m_pMp4MuxerFilter;

	// Configuration
	NX_VIP_CONFIG			m_VipConfig;
	NX_VIDRENDER_CONFIG		m_VidRenderConfig;
	NX_VIDENC_CONFIG		m_VidEncConfig;

	NX_AUDCAPTURE_CONFIG	m_AudCapConfig;
	NX_AUDENC_CONFIG		m_AudEncConfig;

	NX_MP4MUXER_CONFIG		m_Mp4MuxerConfig;

	Mp4ManagerConfig		m_ManagerConfig;

private:
	int32_t					m_bInit;
	int32_t					m_bRun;
	
	char 					m_FileName[1024];

	pthread_mutex_t			m_hLock;
};

INX_Mp4Manager *GetMp4ManagerHandle( void )
{
	return (INX_Mp4Manager *)new CNX_Mp4Manager();
}

void ReleaseMp4ManagerHandle( INX_Mp4Manager *pMp4Manager )
{
	if( pMp4Manager ) delete (CNX_Mp4Manager*)pMp4Manager;
}

#endif