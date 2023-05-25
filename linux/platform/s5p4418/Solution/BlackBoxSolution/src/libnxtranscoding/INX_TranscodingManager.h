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

#ifndef __INX_TRANSCODINGMANAGER_H__
#define __INX_TRANSCODINGMANAGER_H__

#include <stdint.h>

typedef struct tagNX_TRANSCODING_MGR_CONFIG {
	uint8_t		*pInFileName;
	uint8_t		*pOutFileName;

	uint32_t 	nEncFps;
	uint32_t	nEncBitrate;
} NX_TRANSCODING_MGR_CONFIG;

class INX_TranscodingManager
{
public:
	virtual ~INX_TranscodingManager(){}

public:
	virtual int32_t	Init( NX_TRANSCODING_MGR_CONFIG *pConfig ) = 0;
	virtual int32_t	Deinit( void ) = 0;
	virtual int32_t	Start( void ) = 0;
	virtual int32_t	Stop( void ) = 0;
	virtual int32_t	RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) ) = 0;
};

extern INX_TranscodingManager *GetTranscodingHandle( void );
extern void ReleaseTranscodingHandle(INX_TranscodingManager *iHlsMgr);

#endif	// __INX_TRANSCODINGMANAGER_H__
