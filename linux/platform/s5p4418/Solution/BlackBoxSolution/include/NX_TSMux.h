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
//	Module		: Mpeg2 ts muxer
//	File		:
//	Description	:
//	Author		: Kye Lim, Ray Park
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __NX_TSMUX_H__
#define __NX_TSMUX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef NX_HANDLE
#define NX_HANDLE		void*
#endif
#ifndef NX_RESULT
#define NX_RESULT		int32_t
#endif

// 13818-1 : Table 2-29
#define STREAM_TYPE_H264		0x1B		// H264 video
#define STREAM_TYPE_MPEG4		0x10		// MPEG4 video
#define STREAM_TYPE_AAC			0x0F		// AAC audio (MPEG2)
//#define STREAM_TYPE_AAC			0x1C		// AAC audio (MPEG4)
#define STREAM_TYPE_MP3			0x04		// MP3 audio

enum {
	NX_ES_VIDEO	= 0,
	NX_ES_AUDIO,
	NX_ES_USER
};

enum {
	ERROR_ENVALID_VALUE = -8,
	ERROR_MUX_RUNNING,
	ERROR_MUX_AVAILABLE_STATUS,
	ERROR_MUX_INVALID_ARG,
	ERROR_MUX_INVALID_TAG,
	ERROR_AES_ENCRYPT_FAIL,
	ERROR_LACK_MEMORY,
	ERROR_DUPLICATE_PID,
	ERROR_DUPLICATE_PROGRAM,
	ERROR_INDEX_ERROR,
	ERROR_MUX_NONE = 0,
	NEED_MORE_PACKET = 1
};

typedef struct tag_MUX_DATA {
	uint32_t	pid;
	uint32_t	tag;
	uint32_t	keyFrame;
	uint32_t	pts;
	uint8_t		*pBuf;
	uint32_t	size;
} MUX_DATA;

////////////////////////////////////////////////////////////////////////////////////////////////////
// NX_TSMuxOpen			: TSMux Library Open Function
// 
// pesBufSize			: Muxer Internal Buffer Size 
//
NX_HANDLE	NX_TSMuxOpen( uint32_t pesBufSize );

////////////////////////////////////////////////////////////////////////////////////////////////////
// NX_TSMuxClose		: TSMux Library Open Function
// 
NX_RESULT	NX_TSMuxClose( NX_HANDLE handle );

////////////////////////////////////////////////////////////////////////////////////////////////////
// NX_TSMuxAddPID		: TSMux Library Add PID Function
// 
// pid					: PID Number
// tag					: NX_ES_VIDEO, NX_ES_AUDIO, NX_ES_USER
// streamType			: STREAM_TYPE_H264, STREAM_TYPE_MPEG4, STREAM_TYPE_AAC_MP3
//
NX_RESULT	NX_TSMuxAddPID( NX_HANDLE handle, uint32_t pid, uint32_t tag, uint32_t streamType);

////////////////////////////////////////////////////////////////////////////////////////////////////
// NX_TSMuxBuildProgram	: TSMux Library Build Program Function
// 
// programNumber		: Program Number ( must be not '0x00' )
// pmtPID				: PMT PID
// pcrPID				: PCR PID
// pidList				: PID List
// pidListNum			: PID List Number
//
NX_RESULT	NX_TSMuxBuildProgram( NX_HANDLE handle, uint32_t programNumber, uint32_t pmtPID, uint32_t pcrPID, uint32_t *pidList, uint32_t pidListNum);

////////////////////////////////////////////////////////////////////////////////////////////////////
// NX_TSMuxPacket		: TSMux Library Make Packet Function
// 
// inData				: pid, tag, keyFrame, pts, pBuf, bufSize
// outData				: pBuf, bufSize
//
NX_RESULT	NX_TSMuxPacket( NX_HANDLE handle, MUX_DATA *inData, MUX_DATA *outData );

#ifdef __cplusplus
};
#endif

#endif // __NX_TSMUX_H__
