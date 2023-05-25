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
#ifndef __NX_TYPEFIND_H__
#define __NX_TYPEFIND_H__

#include <stdint.h>

//---------------------------------------
typedef struct _AUDIO_INFO {
	int32_t		AudioTrackNum;
	int32_t		ACodecID;
	int32_t		samplerate;
	int32_t		channels;
} AUDIO_INFO;

typedef struct _VIDEO_INFO {
	int32_t		VideoTrackNum;
	int32_t		VCodecID;
	int32_t		Width;
	int32_t		Height;
} VIDEO_INFO;

#define MEDIA_MAX		5
typedef struct _MEDIA_INFO{
	int32_t		AudioTrackTotNum;
	AUDIO_INFO	AudioInfo[MEDIA_MAX];
	int32_t		VideoTrackTotNum;
	VIDEO_INFO	VideoInfo[MEDIA_MAX];
	int32_t		DataTrackTotNum;
	int64_t		Duration;
} MEDIA_INFO;


//---------------------------------------



#endif	//	__NX_TYPEFIND_H__
