#ifndef __NX_MoviePlay_FFMPEG_H__
#define __NX_MoviePlay_FFMPEG_H__

#include "NX_TypeDefind.h"

typedef struct MOVIE_TYPE	*MP_HANDLE;
#define  MP_RESULT           int32_t

//	uri type
enum{
	URI_TYPE_FILE,
	URI_TYPE_URL
};

//	disply port
#if 1
enum{
	DISPLAY_LCD,
	DISPLAY_HDMI,
	DISPLAY_DUAL
};
#endif

//	disply module
enum{
	DISPLAY_MLC0,
	DISPLAY_MLC1
};

enum{
	CALLBACK_MSG_EOS			= 0x1000,
	CALLBACK_MSG_PLAY_ERR		= 0x8001,
};


//ErrorCode
enum{
	ERROR_NONE 				                    = 0,
	ERROR										= -1,
	ERROR_HANDLE		                 		= -2,
	ERROR_NOT_SUPPORT_CONTENTS          		= -3
}; 


#ifdef __cplusplus
extern "C" {
#endif	//	__cplusplus

MP_RESULT NX_MPCreate(MP_HANDLE *phandle, const char *uri, MEDIA_INFO *media_info,
		void(*cb)(void *owner, unsigned int EventType, unsigned int EventDatam, unsigned int parm), void *cbPrivate);
MP_RESULT NX_MPOpen(MP_HANDLE handle, int volumem, int dspModule, int dspPort, int audio_track_num, int video_track_num, int display);

void NX_MPClose( MP_HANDLE handle );
MP_RESULT NX_MPGetMediaInfo(MP_HANDLE handle, int index, MEDIA_INFO *pInfo);
MP_RESULT NX_MPPlay( MP_HANDLE handle, float speed );
MP_RESULT NX_MPPause(MP_HANDLE hande);
MP_RESULT NX_MPStop(MP_HANDLE hande);
MP_RESULT NX_MPSeek(MP_HANDLE hande, unsigned int seekTime);				//seekTime : msec
MP_RESULT NX_MPGetCurDuration(MP_HANDLE handle, unsigned int *duration);	//duration : msec
MP_RESULT NX_MPGetCurPosition(MP_HANDLE handle, unsigned int *position);	//position : msec	
MP_RESULT NX_MPSetDspPosition(MP_HANDLE handle,	int dspModule, int dsPport, int x, int y, int width, int height );
MP_RESULT NX_MPSetVolume(MP_HANDLE handle, int volume);						//volume range : 0 ~ 100

#ifdef __cplusplus
}
#endif

#endif // __NX_MoviePlay_H__
