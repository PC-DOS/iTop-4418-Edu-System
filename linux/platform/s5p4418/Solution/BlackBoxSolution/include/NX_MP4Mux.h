//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		: Mpeg4 Muxer
//	File		:
//	Description	:
//	Author		: Kyle Lim, Ray Park
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __MP4_MUX_h__
#define __MP4_MUX_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "NX_MediaType.h"

enum {
	ERROR_PUT_VIDEO_BUF = -6,
	ERROR_PUT_AUDIO_BUF,
	ERROR_SET_INFO,
	ERROR_RESET,
	ERROR_INIT,
	ERROR_NO_STREAM,
	ERROR_NONE = 0,
	BIG_SIZE_FILE = 1
};

enum {
	TRACK_TYPE_VIDEO,
	TRACK_TYPE_AUDIO,
	TRACK_TYPE_TEXT,		//	Streaming Text (3gp)
};

typedef struct tagMP4MUX_TRACK_INFO
{
	U32		object_type;
	U32		fcc_type;
	U32		time_scale;
	U32		bit_rate;
	U32		duration;
	U8		trak_id[4];
	U32		dsi_size;
	U8		dsi[128];
	//	Video Only
	U16		width;
	U16		height;
	U32		frame_rate;
	//	Audio Only
	U32		sampling_rate;
	U32		channel_num;
}MP4MUX_TRACK_INFO;


typedef struct tagMP4_STREAM_INFO
{
	U8		*pData;				//	Previous
	S32		size;				//
	S32		flag;				//
	S64		time;				//
	S64		totalDuration;		//
}MP4_STREAM_INFO;

typedef void (*cbMp4WriteBuffer)(void *pObj, U8 *pBuf, S32 bufSize);
typedef S32  (*cbMp4GetBuffer)(void *pObj, U8 **pBuf, S32 *bufSize);


//
// Initalize MP4 Muxer
// Input  : fnWriteBuf (Buffer Write function pointer)
//			fnGetBuf (Buffer Supply function pointer)
//			pObj (Function pointer's owner object or Handle)
// Output :	Handle (Mp4 Muxer's Main Handle)
//
NX_HANDLE	NxMP4MuxInit(cbMp4WriteBuffer fnWriteBuf, cbMp4GetBuffer fnGetBuf, void *pObj);

//
// Reset MP4 Muxer
// Input  :	Handle (Mp4 Muxer's Main Handle)
// Output : ErrorCode (Define NX_MP4Box.h)
//
NX_RESULT	NxMP4MuxReset(NX_HANDLE handle);

//
// Close MP4 Muxer
// Input  :	Handle (Mp4 Muxer's Main Handle)
// Output : ErrorCode (Define NX_MP4Box.h)
//
NX_RESULT	NxMP4MuxClose(NX_HANDLE handle);


//
//	Add Track
//	Input  : Handle (Mp4 Muxer's Main Handle)
//			 MP4MUX_TRACK_INFO
//			 TrackType ( Defined in NX_MP4Mux.h )
//	Output : Track Index ( TrackIndex >= 0 )
//
S32			NxMP4MuxAddTrack(NX_HANDLE handle, MP4MUX_TRACK_INFO *pInfo, S32 TrackType);

//
// Update Information for MP4 Muxer
// Input  :	Handle (Mp4 Muxer's Main Handle)
//		  :	FpPos (return MDAT File Position)
//		  :	Size (return MDAT File Size)
// Output : ErrorCode (Define NX_MP4Box.h)
//
NX_RESULT	NxMP4MuxUpdateInfo(NX_HANDLE handle, U32 *FpPos, U32 *Size);

//
// Put Audio & Video Frame to MP4 Muxer
// Input  :	Handle (Mp4 Muxer's Main Handle)
//		  :	pBuf (Video or Audio Frame Buffer)
//		  :	Size (Video or Audio Frame Size)
//		  :	TimeStamp (Video or Audio TimeStamp)
//		  :	Tag (Video or Audio Tag)
//		  :	Key	(Video : keyframe, Audio : not use)
// Output : ErrorCode (Define NX_MP4Box.h)
//
NX_RESULT	NxMP4MuxPutData(NX_HANDLE handle, U8 *pBuf, S32 Size, U64 TimeStamp, S32 TrackIndex, U32 Key);


//
//	Set Audio & Video Decode Specific Information
//	Input	: Handle (Mp4 Muxer's Main Handle)
//			: pBuf (Video or Audio DSI Buffer)
//			: Size (DSI buffer payload size)
//			: TrackIndex
//
//	Output	: Error Code( 0 or -1 )
//
NX_RESULT	NxMp4MuxSetDsiInfo( NX_HANDLE handle, U8 *pBuf, U32 Size, S32 TrackIndex );


#ifdef __cplusplus
};
#endif

#endif // __MP4_MUX_h__

