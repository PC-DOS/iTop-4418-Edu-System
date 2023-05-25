#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	//	getopt & optarg
#include <pthread.h>
#include <signal.h>

#include <NX_TypeFind.h>

#define DEMUX_TYPE_NUM	20
static const char *DemuxTypeString[DEMUX_TYPE_NUM] =
{
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

#define AUDIO_TYPE_NUM	15
static const char *AudioTypeString[AUDIO_TYPE_NUM] =
{
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

#define VIDEO_TYPE_NUM	12
static const char *VideoTypeString[VIDEO_TYPE_NUM] =
{
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


void print_usage(const char *appName)
{
	printf( "usage: %s [options]\n", appName );
	printf( "    -f [file name]     : file name\n" );
}

static void typefind_message(TYMEDIA_INFO *ty_handle)
{
	int i = 0;

	printf("\n");
	printf("===============================================================================\n");
	printf("    DemuxType : %d, %s \n", ty_handle->DemuxType, DemuxTypeString[ty_handle->DemuxType]);
	if(ty_handle->VideoTrackTotNum > 0)
	{
		printf("-------------------------------------------------------------------------------\n");
		printf("                       Video Information \n");
		printf("-------------------------------------------------------------------------------\n");
		printf("    VideoTrackTotNum: %d  \n",  (int)ty_handle->VideoTrackTotNum);
		printf("\n");
		for(i = 0; i < (int)ty_handle->VideoTrackTotNum; i++)
		{
			printf("    VideoTrackNum	: %d  \n",  (int)ty_handle->VideoInfo[i].VideoTrackNum);
			printf("    VCodecType		: %d, %s \n", (int)ty_handle->VideoInfo[i].VCodecType, VideoTypeString[ty_handle->VideoInfo[i].VCodecType] );
			printf("    Width		: %d  \n",  (int)ty_handle->VideoInfo[i].Width);
			printf("    Height		: %d  \n",(int)ty_handle->VideoInfo[i].Height);
			if(ty_handle->VideoInfo[i].Framerate.value_numerator == 0 || ty_handle->VideoInfo[i].Framerate.value_denominator == 0)
				printf("    Framerate		: %f  \n", 0); 
			else
				printf("    Framerate		: %f  \n", (float)ty_handle->VideoInfo[i].Framerate.value_numerator/ty_handle->VideoInfo[i].Framerate.value_denominator); 
			printf("\n");
			printf("\n");
		}
	}
	if(ty_handle->AudioTrackTotNum > 0)
	{
		printf("-------------------------------------------------------------------------------\n");
		printf("                       Audio Information \n");
		printf("-------------------------------------------------------------------------------\n");
		printf("    AudioTrackTotNum: %d  \n",  (int)ty_handle->AudioTrackTotNum);
		printf("\n");
		for(i = 0; i < (int)ty_handle->AudioTrackTotNum; i++)
		{
			printf("    AudioTrackNum	: %d  \n",  (int)ty_handle->AudioInfo[i].AudioTrackNum);
			printf("    ACodecType		: %d, %s \n", (int)ty_handle->AudioInfo[i].ACodecType, AudioTypeString[ty_handle->AudioInfo[i].ACodecType] );
			printf("    samplerate		: %d  \n",  (int)ty_handle->AudioInfo[i].samplerate);
			printf("    channels		: %d  \n",(int)ty_handle->AudioInfo[i].channels);
			printf("\n");
			printf("\n");
		}
	}
	printf("===============================================================================\n\n");
}

int main( int argc, char *argv[] )
{	
	int opt = 0;
	static char uri[2048];
	int ret = 0;
	TYMEDIA_INFO *ty_handle = NULL;

	if( 2>argc )
	{
		print_usage( argv[0] );
		return 0;
	}

	while( -1 != (opt=getopt(argc, argv, "hf:")))
	{
		switch( opt ){
			case 'h':	print_usage(argv[0]);		return 0;
			case 'f':	strcpy(uri, optarg);		break;
			default:								break;
		}
	}

	ret  = NX_TypeFind_Open( &ty_handle );

	ret = NX_TypeFind( ty_handle, uri );

	if(ret != ERROR_NONE)
	{
		NX_TypeFind_Close(ty_handle);
		printf("    ERROR Not Support Contents \n");
		return 0;
	}

	typefind_message(ty_handle);


	NX_TypeFind_Close(ty_handle);

	return 0;

}