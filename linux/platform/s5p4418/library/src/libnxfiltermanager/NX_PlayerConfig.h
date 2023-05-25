//------------------------------------------------------------------------------
//
//	Copyright (C) 2010 Nexell co., Ltd All Rights Reserved
//	Nexell Proprietary & Confidential
//
//	Module     : 
//	File       : 
//	Description:
//	Author     : RayPark
//	History    :
//------------------------------------------------------------------------------
#ifndef __NX_PlayerConfig_h__
#define __NX_PlayerConfig_h__

#define		DEFAULT_BUFFERING_TIME			(200)			//	500 ms
#define		DEFAULT_MAX_BUFFERING_TIME		(4000)			//	4 Seconds
#define		MEDIA_RESYNC_TIME				(1500)			//	2 seconds : Streaming Client Max Timeout
#define		MAX_PLAYER_BUFFER_TIME			(1200)			//	Player Internal Buffer Time

#define		AUDIO_DUMMY_LIMIT				(150)
#define		RENDERER_MAX_SLEEP_TIME			(DEFAULT_MAX_BUFFERING_TIME)

#define		REFTIME_ADAPTATION_LIMIT		(5)
#define		REFTIME_ADAPTATION_LIMIT_NEG	(-5)

#define		STREAM_READER_MAX_DELAY			(DEFAULT_MAX_BUFFERING_TIME)

#endif	//	__NX_PlayerConfig_h__
