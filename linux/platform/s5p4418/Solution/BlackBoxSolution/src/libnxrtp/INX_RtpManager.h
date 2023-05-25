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

#ifndef __INX_RTPMANAGER_H__
#define __INX_RTPMANAGER_H__

#include <stdint.h>

typedef struct tagNX_RTP_MRG_CONFIG {
	uint32_t	nPort;
	uint32_t	nWidth;
	uint32_t	nHeight;
	uint32_t	nFps;
	uint32_t	nBitrate;
	uint32_t	nDspWidth;
	uint32_t	nDspHeight;
} NX_RTP_MGR_CONFIG;

class INX_RtpManager
{
public:
	virtual ~INX_RtpManager(){}

public:
	virtual int32_t	Init( NX_RTP_MGR_CONFIG *pConfig ) = 0;
	virtual int32_t	Deinit( void ) = 0;
	virtual int32_t	Start( void ) = 0;
	virtual int32_t	Stop( void ) = 0;
	virtual int32_t	RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) ) = 0;
};

extern INX_RtpManager *GetRtpHandle( void );
extern void ReleaseRtpHandle(INX_RtpManager *iHlsMgr);

#endif	// __INX_RTPMANAGER_H__
