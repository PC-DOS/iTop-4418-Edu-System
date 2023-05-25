#ifndef __NX_ParserUserData_h__
#define __NX_ParserUserData_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "NX_MediaType.h"

#define VIDEO		0
#define AUDIO		1
#define UTEXT		2
#define TRAK_NUM	6

#define NXHandle	U32
#define NXRet		S32 

	
#if 0 
enum {
	ERROR_STSZ_FAIL = -6,
	ERROR_NOT_SUPPORT_MULTI,	
	ERROR_NO_AUTHORIZE,
	ERROR_READ_DATA_FAIL,
	ERRPR_SEEK,
	ERROR_FILE_ACCESS,
	ERROR_DEMUX_NONE	= 0,
	DEMUX_LAST_FRAME
};
#endif


typedef struct PARSER_INFO
{
	U32			TrackNum;
	U32			AudioTrackNum;
	U32			VideoTrackNum;
	U32			TextTrackNum;
} PARSER_INFO;


//--------PASER USER DATA TRACK------------------------------------
// Initalize Init
// Input  :	pFilName (parser_handle Main Handle)
// Input  :	MaxBufSize (MaxBufSize)
// Output :	Handle (parser_handle Main Handle)
//
NXHandle NXParserUserDataInit(const char *pFilName, U32 MaxBufSize);

//
// Set Wrapping Function
// Input  :	Handle (parser_handle Main Handle)
//		  :	ReadFunc 
//		  : GetLenFunc 
// Output : ErrorCode 
//
NXRet NXParserUserDataWrapFunc(NXHandle handle, int (*ReadFunc)(__int64 Pos, long Len, unsigned char *pBuf), int (*GetLenFunc)(LONGLONG *Total, LONGLONG *Avail)) ;

//
// Get Audio & Video & Text Info to MP4 Demuxer
// Input  :	Handle (parser_handle Main Handle)
//		  :	ParserInfo (Paser Inforamtion)
// Output : ErrorCode 
//
NXRet NXParserUserDataGetInfo(NXHandle handle, PARSER_INFO *ParserInfo);

//
// Get Information 
// Input  :	Handle (parser_handle Main Handle)
//		  :	Size : Text Frame Size
//		  :	TimeStamp (Text TimeStamp)
// Output : ErrorCode 
//
NXRet NXParserGetIndex(NXHandle handle, U32 *Size, U64 *TimeStamp);

//
// Get Text Frame to MP4 Demuxer
// Input  :	Handle (parser_handle Main Handle)
//		  :	pBuf (Text Frame Buffer)
//		  :	Size (Text Frame Size)
//		  :	TimeStamp (Text TimeStamp)
// Output : ErrorCode 
//
NXRet NXParserUserDataGetFrame(NXHandle handle, U8 **pBuf, U32 *Size, U64 *TimeStamp);


//
// Close Close
// Input  :	Handle (parser_handle Main Handle)
// Output : ErrorCode 
//
NXRet NXParserUserDataClose(NXHandle handle);
//-----------------------------------------------------------------

#ifdef __cplusplus
};
#endif

#endif //__NX_ParserUserData_h__

