#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NX_MoviePlay.h"
#include "NX_CManager.h"

typedef struct MOVIE_TYPE
{
	
	void (*callback)(void*, int, int, int );
	void *owner;
	char *uri;
	int uri_type;
	NX_CManager *hManager;

}MOVIE_TYPE;

typedef struct MOVIE_TYPE	*MP_HANDLE;

typedef struct AppData{
	MP_HANDLE hPlayer;
}AppData;


MP_RESULT NX_MPCreate(MP_HANDLE *phandle, const char *uri, MEDIA_INFO *media_info,
	void(*cb)(void *owner, unsigned int EventType, unsigned int EventDatam, unsigned int parm), void *cbPrivate)
{
	MP_HANDLE handle = NULL;
	int32_t uri_len = 0;

	handle = (MP_HANDLE)malloc(sizeof(MOVIE_TYPE));
	if (NULL == handle)
		return ERROR_HANDLE;

	memset(handle, 0, sizeof(MOVIE_TYPE));

	if ((strncmp(uri, "http://", 7) == 0) ||
		(strncmp(uri, "https://", 8) == 0))
	{
		handle->uri_type = URI_TYPE_URL;
	}
	else
	{
		handle->uri_type = URI_TYPE_FILE;
	}

	uri_len = strlen(uri);

	if (uri_len <= 4)
		goto ERROR_EXIT;
	//	surfix = &uri[ uri_len-4 ];

	handle->uri = strdup(uri);

	if (cbPrivate != NULL)
		handle->owner = cbPrivate;

	handle->hManager = new NX_CManager(cb, uri);
	if (handle->hManager == NULL)
		goto ERROR_EXIT;
	handle->hManager->GetMediaInfo(media_info);

	*phandle = handle;
	return ERROR_NONE;

ERROR_EXIT:

	if (handle){
		free(handle);
	}

	return ERROR;
}

MP_RESULT NX_MPOpen(MP_HANDLE handle, int volumem, int dspModule, int dspPort, int audio_track_num, int video_track_num, int display)
{
	handle->hManager->FilterCreate(audio_track_num, video_track_num);

	return ERROR_NONE;
}

void NX_MPClose( MP_HANDLE handle )
{

	if( !handle )
		return;

	if (handle->hManager)
		delete handle->hManager;

	if(handle->uri)
		free( handle->uri );
	free( handle );

}

MP_RESULT NX_MPGetMediaInfo(MP_HANDLE handle, int idx, MEDIA_INFO *pInfo)
{

	return ERROR_NONE;
}

MP_RESULT NX_MPPlay( MP_HANDLE handle, float speed )
{

	if (NULL == handle->hManager)
		return ERROR;

	handle->hManager->Run();

	return	ERROR_NONE;	

}


MP_RESULT NX_MPPause(MP_HANDLE handle)
{
	if (NULL == handle->hManager)
		return ERROR;

	handle->hManager->Pause();

	return	ERROR_NONE;	
}

MP_RESULT NX_MPStop(MP_HANDLE handle)
{

	if (NULL == handle->hManager)
		return ERROR;

	handle->hManager->Stop();

	return	ERROR_NONE;	

}

MP_RESULT NX_MPSeek(MP_HANDLE handle, unsigned int seekTime)
{
	
	if (NULL == handle->hManager)
		return ERROR;

	handle->hManager->Seek(seekTime);

	return	ERROR_NONE;
}


MP_RESULT NX_MPGetCurDuration(MP_HANDLE handle, unsigned int *duration)
{
	if (NULL == handle)
		return ERROR;

	if (NULL == handle->hManager)
		return ERROR;

	*duration = handle->hManager->GetDuration();

	return	ERROR_NONE;
}


MP_RESULT NX_MPGetCurPosition(MP_HANDLE handle, unsigned int *position)
{
	if (NULL == handle)
		return ERROR;

	if (NULL == handle->hManager)
		return ERROR;

	*position = handle->hManager->GetPosition();

	return ERROR_NONE;
}


MP_RESULT NX_MPSetDspPosition(MP_HANDLE handle,	int dspModule, int dsPport, int x, int y, int width, int height )
{

	if (NULL == handle->hManager)
		return ERROR;

	handle->hManager->SetVideoPosition(dspModule, dsPport, x, y, width, height);

	return ERROR_NONE;
}


MP_RESULT NX_MPSetVolume(MP_HANDLE handle, int volume)
{

	if (NULL == handle->hManager)
		return ERROR;

	handle->hManager->SetVolume(volume);

	return ERROR_NONE;
}

