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

#ifndef __NX_FILTERCONFIGTYPES_H__
#define __NX_FILTERCONFIGTYPES_H__
#include <nx_alloc_mem.h>

//----------------------------------------------------------------------------
//	Video Config
//----------------------------------------------------------------------------
typedef struct tag_NX_VIP_CONFIG {
	int32_t		port;
	int32_t		width;
	int32_t		height;
	int32_t		fps;
} NX_VIP_CONFIG;

//----------------------------------------------------------------------------
//	Video Renderer Config
//----------------------------------------------------------------------------
typedef struct tag_NX_VIDRENDER_CONFIG
{
	int32_t		port;		// display port
	
	// Source Image
	int32_t		width;		// source image width
	int32_t		height;		// source image height

	// Crop Region
	int32_t		cropLeft;	// crop region left
	int32_t		cropTop;	// crop region top
	int32_t		cropRight;	// crop region right
	int32_t		cropBottom;	// crop region bottom

	// Display Region
	int32_t		dspLeft;	// display region left
	int32_t		dspTop;		// display region top
	int32_t		dspRight;	// display region right
	int32_t		dspBottom;	// display region bottom
} NX_VIDRENDER_CONFIG;

typedef struct tag_NX_VIDENC_CONFIG
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	fps;
	uint32_t	bitrate;
	uint32_t	codec;
} NX_VIDENC_CONFIG;

//----------------------------------------------------------------------------
//	Audio Config
//----------------------------------------------------------------------------
typedef struct tag_NX_AUDCAPTURE_CONFIG
{
	uint32_t	channels;
	uint32_t	frequency;		//	sampling frequency
	uint32_t	samples;		//  num of samples : MP3( 1152 ) / AAC( 1024 ) :: outBuffer = channels * NumOfSamples * 2 ( Bytes per sample )
} NX_AUDCAPTURE_CONFIG;

typedef struct tag_NX_AUDENC_CONFIG
{
	uint32_t	channels;
	uint32_t	frequency;
	uint32_t	bitrate;
	uint32_t	codec;
	uint32_t	adts;
} NX_AUDENC_CONFIG;

//----------------------------------------------------------------------------
//	User Data Config
//----------------------------------------------------------------------------
typedef struct tag_NX_USERDATA_CONFIG
{
	uint32_t	interval;
	uint32_t	bufSize;
} NX_USERDATA_CONFIG;

//----------------------------------------------------------------------------
//	File Container Config
//----------------------------------------------------------------------------
#define MAX_VID_NUM			4
#define MAX_AUD_NUM			1
#define MAX_TXT_NUM			1

typedef struct tag_NX_MP4MUXER_TRACK_CONFIG {
	uint32_t	width;		// video
	uint32_t	height;		// video
	uint32_t	frameRate;	// video
	uint32_t	channel;	// audio
	uint32_t	frequency;	// audio
	uint32_t	codecType;	// video / audio
	uint32_t	bitrate;	// video / audio / text
} NX_MP4MUXER_TRACK_CONFIG;

typedef struct tag_NX_MP4MUXER_CONFIG
{
	uint32_t					videoTrack;		// max 4EA
	uint32_t					audioTrack;		// max 1EA
	uint32_t					textTrack;		// max 1EA
	NX_MP4MUXER_TRACK_CONFIG	trackConfig[MAX_VID_NUM + MAX_AUD_NUM + MAX_TXT_NUM];
} NX_MP4MUXER_CONFIG;

typedef struct tag_NX_TSMUXER_TRACK_CONFIG
{
	uint32_t	codecType;	// video / audio
	uint32_t	bitrate;	// video / audio / text (N/A)
} NX_TSMUXER_TRACK_CONFIG;

typedef struct tag_NX_TSMUXER_CONFIG
{
	uint32_t		videoTrack;		// max 4EA
	uint32_t		audioTrack;		// max 1EA
	uint32_t		textTrack;		// max 1EA
	uint32_t		codecType[MAX_VID_NUM + MAX_AUD_NUM + MAX_TXT_NUM];
} NX_TSMUXER_CONFIG;

typedef struct tag_NX_HLS_CONFIG
{
	uint8_t			MetaFileName[1024];		// meta file name
	uint8_t			SegmentRoot[1024];		// segment root directory
	uint8_t			SegmentName[1024];		// name of segment file
	uint8_t			SegmentDuration;		// max duration of segment file
	uint8_t			SegmentNumber;			// number of segment file
} NX_HLS_CONFIG;

typedef struct tag_NX_RTP_CONFIG
{
	uint32_t		port;
	uint32_t 		sessionNum;				// videoNumber
	uint32_t		connectNum;
	uint8_t 		sessionName[MAX_VID_NUM][255];
} NX_RTP_CONFIG;

//----------------------------------------------------------------------------
//	ETC Config
//----------------------------------------------------------------------------
typedef struct tag_NX_INTERLEAVER_CONFIG
{
	int32_t		channel;
} NX_INTERLEAVER_CONFIG;

typedef struct tag_NX_BUFFERING_CONFIG
{
	int32_t		bufferedTime;	
} NX_BUFFERING_CONFIG;

typedef struct tag_NX_IMAGE_EFFECT_CONFIG {
	int32_t		width;
	int32_t		height;
} NX_IMAGE_EFFECT_CONFIG;

typedef struct tag_NX_MOTION_DETECT_CONFIG {
	int32_t		samplingWidth;
	int32_t		samplingHeight;
	int32_t		threshold;
	int32_t		sensitivity;
	int32_t		sampingFrame;
} NX_MOTION_DETECT_CONFIG;

//----------------------------------------------------------------------------
//	Statistics Information
//----------------------------------------------------------------------------
#define MAX_BUFFER_SIZE		10

typedef struct tag_NX_FILTER_BUFFER {
	int32_t		cur;
	int32_t		max;
	int32_t		min;
	int32_t		average;		// N/A
	int32_t		limit;	
} NX_FILTER_BUFFER;

typedef struct tag_NX_FILTER_STATISTICS {
	uint64_t			inFrameCount;
	uint64_t			outFrameCount;

	double				inFps;
	double				outFps;
	double				bitrate;
	double				frequency;

	NX_FILTER_BUFFER 	inBuf[MAX_BUFFER_SIZE];
	NX_FILTER_BUFFER 	outBuf[MAX_BUFFER_SIZE];
} NX_FILTER_STATISTICS;

#endif	//	__NX_FILTERCONFIGTYPES_H__

