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

#ifndef __INX_HLSMANAGER_H__
#define __INX_HLSMANAGER_H__

#include <stdint.h>

typedef struct tagNX_HLS_MGR_CONFIG {
	uint32_t	nPort;
	uint32_t	nWidth;
	uint32_t	nHeight;
	uint32_t	nFps;

	uint8_t		nSegmentDuration;
	uint8_t		nSegmentNumber;
	uint8_t		pMetaFileName[256];
	uint8_t		pSegmentFileName[256];
	uint8_t		pSegmentRoot[256];
} NX_HLS_MGR_CONFIG;

class INX_HlsManager
{
public:
	virtual ~INX_HlsManager(){}

public:
	virtual int32_t	Init( NX_HLS_MGR_CONFIG *pConfig ) = 0;
	virtual int32_t	Deinit( void ) = 0;
	virtual int32_t	Start( void ) = 0;
	virtual int32_t	Stop( void ) = 0;
	virtual int32_t	RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) ) = 0;
};

extern INX_HlsManager *GetHlsHandle( void );
extern void ReleaseHlsHandle(INX_HlsManager *iHlsMgr);

#endif	// __INX_HLSMANAGER_H__
