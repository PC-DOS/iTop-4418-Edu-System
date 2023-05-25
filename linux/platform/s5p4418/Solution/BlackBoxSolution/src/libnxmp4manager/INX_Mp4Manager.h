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

#ifndef __INX_MP4MANAGER_H__
#define __INX_MP4MANAGER_H__

typedef struct tagMp4ManagerConfig {
	int32_t			port;
	int32_t 		width;
	int32_t			height;
	int32_t			fps;
	int32_t			bitrate;

	int32_t			bAudEnable;

	int32_t			dspLeft;
	int32_t			dspTop;
	int32_t			dspRight;
	int32_t			dspBottom;
} Mp4ManagerConfig;

typedef enum {
	NX_NOTIFY_FILEWRITING_DONE	= 0x1001,
	NX_NOTIFY_JPEGWRITING_DONE	= 0x1003,
	NX_NOTIFY_ERR_VIDEO_INPUT	= 0xF001,
	NX_NOTIFY_ERR_VIDEO_ENCODE	= 0xF002,
	NX_NOTIFY_ERR_OPEN_FAIL		= 0xF003,
	NX_NOTIFY_ERR_WRITE			= 0xF004,
} NX_NOTIFY_TYPE;

#include <stdint.h>

class INX_Mp4Manager
{
public:
	virtual ~INX_Mp4Manager(){}

public:
	virtual int32_t	SetConfig( Mp4ManagerConfig *pConfig ) = 0;
	
	virtual int32_t	Init( void ) = 0;
	virtual int32_t	Deinit( void ) = 0;
	
	virtual int32_t	Start( char *pFileName ) = 0;
	virtual int32_t	Stop( void ) = 0;
	
	virtual int32_t Capture( char *pFileName ) = 0;
	virtual int32_t EnableRender( int32_t enable ) = 0;

	virtual int32_t	RegisterNotifyCallback( uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t) ) = 0;
};

extern INX_Mp4Manager *GetMp4ManagerHandle( void );
extern void ReleaseMp4ManagerHandle( INX_Mp4Manager *pMp4Manager );

#endif	// __INX_MP4MANAGER_H__
