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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#include <NX_MoviePlay.h>
#include <NX_TypeFind.h>
#include <uevent.h>

#include <NX_MenuPlayer.h>

#include "NX_PlayerMain.h"

#define DEBUG			0

#if(DEBUG)
#define DEMUX_TYPE_NUM	20
#define VIDEO_TYPE_NUM	12
#define AUDIO_TYPE_NUM	15

static const char *DemuxTypeString[DEMUX_TYPE_NUM] = {
	"video/mpegts",						//mpegtsdemux
	"video/quicktime",					//qtdemux
	"application/ogg",					//oggdemux
	"application/vnd.rn-realmedia",		//rmdemux
	"video/x-msvideo",					//avidemux
	"video/x-ms-asf",					//asfdemux
	"video/x-matroska",					//matroskademux
	"video/x-flv",						//flvdemux
	"video/mpeg",						//mpegpsdemux	
	"application/x-id3",				//audio mp3
	"audio/x-flac",						//audio flac
	"audio/x-m4a",						//audio m4a
	"audio/x-wav",						//audio wav
	"audio/mpeg",						//audio mpeg
	"audio/x-ac3",						//audio ac3
	"audio/x-dts",						//audio dts
	"application/x-3gp",
	"NULL",
	"NULL",
	"NULL",
};

static const char *AudioTypeString[AUDIO_TYPE_NUM] = {
	"audio/mpeg",					//AUDIO_TYPE_MPEG
	"audio/mp3",					//AUDIO_TYPE_MP3
	"audio/aac",					//AUDIO_TYPE_AAC	 mpeg4 lc
	"audio/x-wma",					//AUDIO_TYPE_WMA
	"audio/x-vorbis",				//AUDIO_TYPE_OGG
	"audio/x-ac3",					//AUDIO_TYPE_AC3
	"audio/x-private1-ac3",			//AUDIO_TYPE_AC3_PRI
	"audio/x-flac",					//AUDIO_TYPE_FLAC
	"audio/x-pn-realaudio",			//AUDIO_TYPE_RA		
	"audio/x-dts",
	"audio/x-private1-dts",
	"audio/x-wav",
	"NULL",
	"NULL",
	"NULL",
};

static const char *VideoTypeString[VIDEO_TYPE_NUM] = {
	"video/x-h264",					//VIDEO_TYPE_H264
	"video/x-h263",					//VIDEO_TYPE_H263
	"video/mpeg",					//VIDEO_TYPE_MP4V	mpeg4 video
	"video/mpeg",					//VIDEO_TYPE_MP2V	mpeg2 video
	"video/x-flash-video",			//VIDEO_TYPE_FLV
	"video/x-pn-realvideo",			//VIDEO_TYPE_RV		realvideo
	"video/x-divx",					//VIDEO_TYPE_DIVX
	"video/x-ms-asf",				//VIDEO_TYPE_ASF
	"video/x-wmv",					//VIDEO_TYPE_WMV
	"video/x-theora",				//VIDEO_TYPE_THEORA
	"video/x-xvid",
	"NULL"
};
#endif

static char		stUri[1024];

typedef struct Static_player_st {
	int32_t display;
	int32_t	hdmi_detect;
	int32_t	hdmi_detect_init;
	int32_t	volume;
	int32_t	audio_request_track_num;
	int32_t	video_request_track_num;
} Static_player_st;

typedef struct AppData {
	MP_HANDLE hPlayer;
} AppData;

Static_player_st	static_player;
AppData				appData;

pthread_t	gstPlayerThread;
int32_t		gstPlayerThreadRun = false;

MP_HANDLE	hPlayer = NULL;

int32_t		bPlayerRun = false;

static void cbMessage( void *privateDesc, uint32_t message, uint32_t param1, uint32_t param2 )
{
	if( message == CALLBACK_MSG_EOS ) {
		printf("%s(): end of stream.\n", __func__);
		PushFlags( 3 );
	}
	else if( message == CALLBACK_MSG_PLAY_ERR ) {
		printf("%s(): cannot play contents.\n", __func__);
	}
}

static void typefind_debug(TYMEDIA_INFO *ty_handle)
{
#if(DEBUG)
	printf("\n");
	printf("================================================================================\n");
	printf("    DemuxType : %d, %s\n", ty_handle->DemuxType, DemuxTypeString[ty_handle->DemuxType]);
	
	if( ty_handle->VideoTrackTotNum > 0 ) {
		printf("--------------------------------------------------------------------------------\n");
		printf("                              Video Information                                 \n");
		printf("--------------------------------------------------------------------------------\n");
		printf("    Total Track  : %d\n\n",  (int32_t)ty_handle->VideoTrackTotNum);

		for( int32_t i = 0; i < (int32_t)ty_handle->VideoTrackTotNum; i++ )
		{
			printf("    TrackNumber  : %d\n",		(int32_t)ty_handle->VideoInfo[i].VideoTrackNum);
			printf("    CodecType    : %d, %s\n",	(int32_t)ty_handle->VideoInfo[i].VCodecType, VideoTypeString[ty_handle->VideoInfo[i].VCodecType] );
			printf("    Width        : %d\n",		(int32_t)ty_handle->VideoInfo[i].Width);
			printf("    Height       : %d\n",		(int32_t)ty_handle->VideoInfo[i].Height);

			if( ty_handle->VideoInfo[i].Framerate.value_numerator == 0 || ty_handle->VideoInfo[i].Framerate.value_denominator == 0 )
				printf("    FrameRate	 : %f  \n",	0. ); 	
			else
				printf("    Framerate    : %f  \n",	(float)ty_handle->VideoInfo[i].Framerate.value_numerator/ty_handle->VideoInfo[i].Framerate.value_denominator );
			printf("\n\n");
		}
	}

	if( ty_handle->AudioTrackTotNum > 0 ) {
		printf("--------------------------------------------------------------------------------\n");
		printf("                              Audio Information                                 \n");
		printf("--------------------------------------------------------------------------------\n");
		printf("    Total Track  : %d\n\n",  (int32_t)ty_handle->AudioTrackTotNum);

		for( int32_t i = 0; i < (int32_t)ty_handle->AudioTrackTotNum; i++ )
		{
			printf("    TrackNumber : %d\n",		(int32_t)ty_handle->AudioInfo[i].AudioTrackNum);
			printf("    CodecType   : %d, %s\n",	(int32_t)ty_handle->AudioInfo[i].ACodecType, AudioTypeString[ty_handle->AudioInfo[i].ACodecType] );
			printf("    Samplerate  : %d\n",		(int32_t)ty_handle->AudioInfo[i].samplerate);
			printf("    Channels    : %d\n",		(int32_t)ty_handle->AudioInfo[i].channels);
			printf("\n\n");
		}
	}
	printf("================================================================================\n");
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//
int32_t PlayerStart( const char *filename )
{
	MP_RESULT		mpResult = ERROR_NONE;
	TYMEDIA_INFO	MediaInfo;
	TYMEDIA_INFO 	*pMediaInfo = NULL;

	int32_t audio_request_track_num = 0;	//multi track, default 0
	int32_t video_request_track_num = 0;	//multi track, default 0
	int32_t program_no_track_num = 0;

	int32_t display = DISPLAY_PORT_LCD;

	printf("file name : %s\n", filename);

	if( access(filename, F_OK) || filename[0] == 0x00 ) {
		printf("%s(): File is not exist.\n", __func__);
		return -1;
	}

	sprintf(stUri, "%s", filename);
	
	memset(&MediaInfo, 0x00, sizeof(TYMEDIA_INFO));

	// typefind
	if (ERROR_NONE != NX_TypeFind_Open(&pMediaInfo)) {
		printf("%s(): NX_TypeFind_Open failed!\n", __func__);
	}

	if (ERROR_NONE != NX_TypeFind(pMediaInfo, stUri)) {
		printf("%s(): NX_TypeFind failed! (is not support contents)\n", __func__);
	}
	else {
		typefind_debug(pMediaInfo);
		NX_TypeFind_Close(pMediaInfo);

		if (pMediaInfo->DemuxType == DEMUX_TYPE_MPEGTSDEMUX) {		// TS contents.
			if (pMediaInfo->program_tot_no > 1) {
				//scanf("%d", &program_no_track_num);
				program_no_track_num = 0;
				audio_request_track_num = program_no_track_num;
				video_request_track_num = program_no_track_num;
			}
		}
		else {														// The other contents. ( mp4, .. ) 
			if (pMediaInfo->VideoTrackTotNum > 1){
				//scanf("%d", &video_request_track_num);
				while (1)
				{
					if (video_request_track_num < 1 || video_request_track_num >(signed)pMediaInfo->VideoTrackTotNum) {
						printf("%s(): Error Input video request number.\n", __func__);
						//scanf("%d", &video_request_track_num);
					}
					else {
						video_request_track_num--;
						break;
					}
				}
			}
			if (pMediaInfo->AudioTrackTotNum > 1) {
				//scanf("%d", &audio_request_track_num);
				while (1)
				{
					if (audio_request_track_num < 1 || audio_request_track_num >(signed)pMediaInfo->AudioTrackTotNum) {
						printf("%s(): Error Input audio request number.\n", __func__);
						//scanf("%d", &audio_request_track_num);
					}
					else {
						audio_request_track_num--;
						break;
					}
				}
			}
		}
	}

	mpResult = NX_MPOpen(&hPlayer, stUri, 100, 0, 0, audio_request_track_num, video_request_track_num, (char *)&MediaInfo, display, 0, &cbMessage, NULL);
	if (ERROR_NONE != mpResult) {
		printf("%s(): NX_MPOpen failed!\n", __func__);
	}

	int32_t dspModule = 0, dspPort = 0;
	mpResult = NX_MPSetDspPosition(hPlayer, dspModule, dspPort, 0, 0, 800, 480);
	if (ERROR_NONE != mpResult) {
		printf("%s(): NX_MPSetDspPosition failed!\n", __func__);
	}

	mpResult = NX_MPPlay(hPlayer, 1.);
	if (ERROR_NONE != mpResult) {
		printf("%s(): NX_MPPlay failed!\n", __func__);
	}
	bPlayerRun = true;
	PushFlags( 1 );

	return 0;
}

int32_t PlayerStop( void )
{
	memset( stUri, 0x00, sizeof(stUri) );

	if (hPlayer) NX_MPStop(hPlayer);
	if (hPlayer) NX_MPClose(hPlayer);
	hPlayer = NULL;
	PushFlags( 0 );

	return 0;
}

int32_t PlayerPause( void )
{
	if( bPlayerRun ) {
		if( hPlayer ) NX_MPPause( hPlayer );
	}
	else {
		if( hPlayer ) NX_MPPlay( hPlayer, 1. );
	}
	
	bPlayerRun = !bPlayerRun;

	return 0;
}

int32_t PlayerSeek( uint32_t position )
{
	uint32_t duration = 0;
	if(hPlayer) NX_MPGetCurDuration( hPlayer, &duration );

	uint32_t seekTime = ((float)position / 100.) * duration;
	if(hPlayer) NX_MPSeek( hPlayer, seekTime );

	printf( "%s(): seekTime = %d, duration = %d\n", __func__, seekTime, duration );

	return 0;
}

int32_t PlayerGetPos( uint32_t *position, uint32_t *duration )
{
	if( hPlayer ) NX_MPGetCurDuration( hPlayer, duration );
	if( hPlayer ) NX_MPGetCurPosition( hPlayer, position );

	return 0;
}
