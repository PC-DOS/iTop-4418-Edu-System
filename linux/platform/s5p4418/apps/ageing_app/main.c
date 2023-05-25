#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	//	getopt & optarg
#include <pthread.h>
#include <signal.h>

#include <NX_MoviePlay.h>
#include <NX_TypeFind.h>

struct AppData{
	MP_HANDLE hPlayer;
};


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

//-------------------------------------------------------------------------
//ageing

typedef struct NX_MovPlayList {
	char		*uri[1000];
	int			num_uri_list;
	int			cur_idx;
}NX_MovPlayList;
NX_MovPlayList gst_play_list;

typedef struct NX_CmdList {
	int		cmd_num;
	char		*cmd;
	int		percent;
}NX_CmdList;

typedef struct PlayCmd {
	int		open;
	int		play;
	int		stop;
	int		close;
	int		pause;
}PlayCmd;


int		gst_player_eos = 0;
int		gst_player_init = 0;
int		gst_ageing_cmd = 0;

NX_CmdList gst_cmd_list[] = 
{  
// {4, "open 0 0 0 ", 0}, {1,"play " ,10}, {2, "seek 15 ", 20},  {1, "pause ", 5}, {1, "stop ",0},
 {4, "open 0 0 0 ", 0}, {1, "play ", 20}, {2, "vol 30 ", 80}, {1, "close ", 0},
 {4, "open 0 0 0 ", 0}, {1, "play ", 30}, {7, "pos 0 0 30 30 720 480 ", 50}, {1, "info ", 70}, {1, "status ", 80}, {7, "pos 0 0 0 0 1280 720 ", 100},
 {1, "NULL", 0}
};

enum{
	AGEING_CMD_LONG,
	AGEING_CMD_NORMAL,
	AGEING_CMD_HEAVY_OPEN,
	AGEING_CMD_HEAVY_SEEK_NORMAL,
	AGEING_CMD_HEAVY_SEEK_HEAVY,
};

//
//	Description :
//		Make media file list & save to out_file.
//
int scan_media_file( const char *dir, const char *out_file  )
{
	char line_buf[1204];
	char cmd[512];
	FILE *fd;
	FILE *mfd = NULL;
	FILE *ofd = NULL;
	int len;
	int num_list = 0;

	

	sprintf( cmd, " find %s", dir );

	fd = popen( cmd, "r" );
	ofd = fopen( out_file, "w+" );

	//	reset all play lists
	memset( &gst_play_list, 0, sizeof(gst_play_list) );

	if( fd )
	{
		while( fgets(line_buf, sizeof(line_buf), fd ) != NULL )
		{
			len = strlen( line_buf );
			if( line_buf[len-1] == '\n' )
				line_buf[len-1] = 0;

			if( len < 4 )
				continue;

			if(		(0 == strncasecmp( &line_buf[len-5], ".mkv", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".avi", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".flv", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-4], ".ts", 3 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".vob", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-6], ".rmvb", 5 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".wmv", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".mp4", 4 )) 
				//audio
				||	(0 == strncasecmp( &line_buf[len-6], ".flac", 5 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".wma", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".aac", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".wav", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".3gp", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".m4a", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".mp3", 4 )) 
				||	(0 == strncasecmp( &line_buf[len-5], ".ogg", 4 )) 
				)

			{
				//	Check Media File
				mfd = fopen( line_buf, "r" );
				if( mfd != NULL )
				{
					//	check file name
					printf("[ScanMediaFile] file name = %s\n", line_buf );
					gst_play_list.uri[num_list] = (char*)malloc(len+1);
					strcpy( gst_play_list.uri[num_list], line_buf );
					num_list ++;
					fclose( mfd );
					if(ofd) fprintf( ofd, "%s\n", line_buf );
				}
			}
		}
		gst_play_list.num_uri_list = num_list;
	}
	if( ofd )
		fclose( ofd );
}
//-------------------------------------------------------------------------


typedef struct AppData AppData;

static void callback( void *privateDesc, unsigned int message, unsigned int param1, unsigned int param2 )
{
	if( message == CALLBACK_MSG_EOS )
	{
		printf("App : callback(privateDesc = %p)\n", privateDesc);
		if( privateDesc )
		{
			AppData *appData = (AppData*)privateDesc;
			if( appData->hPlayer )
			{
				printf("NX_MPStop ++\n");
				NX_MPStop(appData->hPlayer);
				printf("NX_MPStop --\n");
				gst_player_eos = 1;
				gst_player_init = 0;
			}
		}
		printf("CALLBACK_MSG_EOS\n");
	}
	else if( message == CALLBACK_MSG_PLAY_ERR )
	{
		printf("Cannot Play Contents\n");

	}
}

#define	SHELL_MAX_ARG	32
#define	SHELL_MAX_STR	1024
static int GetArgument( char *pSrc, char arg[][SHELL_MAX_STR] )
{
	int	i, j;

	// Reset all arguments
	for( i=0 ; i<SHELL_MAX_ARG ; i++ )
	{
		arg[i][0] = 0;
	}

	for( i=0 ; i<SHELL_MAX_ARG ; i++ )
	{
		// Remove space char
		while( *pSrc == ' ' )
			pSrc++;
		// check end of string.
		if( *pSrc == 0 || *pSrc == '\n' )
			break;

		j=0;
		while( (*pSrc != ' ') && (*pSrc != 0) && *pSrc != '\n' )
		{
			arg[i][j] = *pSrc++;
			j++;
			if( j > (SHELL_MAX_STR-1) )
				j = SHELL_MAX_STR-1;
		}
		arg[i][j] = 0;
	}
	return i;
}

static void typefind_debug(TYMEDIA_INFO *ty_handle)
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

static void shell_help( void )
{
	//               1         2         3         4         5         6         7
	//      12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n");
	printf("===============================================================================\n");
	printf("                       Play Control Commands\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("    open( or o) [Moudle] [Port] [filename] : open player for specific file\n");
	printf("    close                                  : close player\n");
	printf("    info                                   : show file information\n");
	printf("    play                                   : play\n");
	printf("    pause                                  : pause playing\n");
	printf("    stop                                   : stop playing\n");
	printf("    seek (or s) [milli seconds]            : seek\n");
	printf("    status (or st)                         : display current player status\n");
	printf("    p                                      : toggle play & pause\n");
	printf("    typefind                                 : file typefind\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("                        Other Control Commands\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("    pos [module] [port] [x] [y] [w] [h]    : change play position\n");
	printf("    vol [gain(%%), 0~100]                  : change volume(0:mute)\n");
	printf("===============================================================================\n\n");
}


static void ageing_test_help( void )
{
	//               1         2         3         4         5         6         7
	//      12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n");
	printf("===============================================================================\n");
	printf("                       Ageing Test Commands\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("    long						: Long-term Test\n");
	printf("    normal						: Normal Test\n");
	printf("    heavy						: Heavy Test\n");
	printf("===============================================================================\n\n");
}


static int shell_main(const char *scan_path, const char *out_file)
{
	static	char cmdstring[SHELL_MAX_ARG * SHELL_MAX_STR];
	static 	char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	int idx = 0;
	int volume = 50;
	int cmdCnt;
	int dspPort = DISPLAY_PORT_LCD;						//DISPLAY_PORT_LCD:(0) ,DISPLAY_PORT_HDMI:(1) 
	int dspModule = DISPLAY_MODULE_MLC0;				//DISPLAY_MODULE_MLC0:(0) ,DISPLAY_MODULE_MLC1:(1) 
	int audio_request_track_num = 0;					//multi track, default 0
	int video_request_track_num = 0;					//multi track, default 0
	int program_no_track_num = 0;
	MP_RESULT mpResult = ERROR_NONE;
	int ret = 0;
	TYMEDIA_INFO input_media_info;
	char uri[2048];
	int position, duration;
	int priority = 0;

	//	Player Specific Parameters

	MP_HANDLE hPlayer = NULL;
	MP_MEDIA_INFO media_info;
	AppData appData;
	int isPaused = 0;
	memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );
	

	strcpy(uri, scan_path);

	scan_media_file( scan_path, out_file );

	//shell_help();
#if 1
	ageing_test_help();
	while(1)
	{
		printf("Ageing: Input Cmd> ");
		fgets( cmdstring, sizeof(cmdstring), stdin );
		cmdCnt = GetArgument( cmdstring, cmd );
		printf("cmd[0] = %s\n",cmd[0]);
		if( (0 == strcasecmp( cmd[0], "long" )) ){
			gst_ageing_cmd = AGEING_CMD_LONG;
			break;
		}
		else if( (0 == strcasecmp( cmd[0], "normal" )) ){
			gst_ageing_cmd = AGEING_CMD_NORMAL;\
			break;
		}
		else if( (0 == strcasecmp( cmd[0], "heavy" )) )
		{
			//               1         2         3         4         5         6         7
			//      12345678901234567890123456789012345678901234567890123456789012345678901234567890
			printf("\n");
			printf("===============================================================================\n");
			printf("                       Heavy Test Commands\n");
			printf("-------------------------------------------------------------------------------\n");
			printf("    open							: open->close->open->close Test\n");
			printf("    seek-normal							: seek random Test\n");
			printf("    seek-heavy							: seek heavy Test\n");
			printf("===============================================================================\n\n");
			fgets( cmdstring, sizeof(cmdstring), stdin );
			cmdCnt = GetArgument( cmdstring, cmd );
			printf("cmd[0] = %s\n",cmd[0]);
			if( (0 == strcasecmp( cmd[0], "open" )) ){
				gst_ageing_cmd = AGEING_CMD_HEAVY_OPEN;
				break;
			}
			else if( (0 == strcasecmp( cmd[0], "seek-normal" )) ){
				gst_ageing_cmd = AGEING_CMD_HEAVY_SEEK_NORMAL;
				break;
			}
			else if( (0 == strcasecmp( cmd[0], "seek-heavy" )) ){
				gst_ageing_cmd = AGEING_CMD_HEAVY_SEEK_HEAVY;
				break;
			}
		}
	}
#else
		gst_ageing_cmd = AGEING_CMD_LONG;
#endif		

	//------------------------------------------------------------------------------------------- long
	if(gst_ageing_cmd == AGEING_CMD_LONG)
	{
		int total_count = 1;
		while(1)
		{
			if(gst_player_init == 0)
			{				
				memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );
				volume = 100;
				dspModule = 0;
				dspPort = 0;
				audio_request_track_num = 0;
				video_request_track_num = 0;
				memset(&appData, 0, sizeof(appData));

				if( gst_play_list.cur_idx >= gst_play_list.num_uri_list || gst_play_list.cur_idx < 0 )
				{
					//			printf("END index : num_uri_list = %d, cur_idex (0~%d)\n", gst_play_list.num_uri_list, gst_play_list.cur_idx);
					//printf("Invalid index : idex (0~%d)\n", gst_play_list.num_uri_list-1);
					//gst_play_list.cur_idx = 0;
					printf("\n");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("SUCCESS:: num_uri_list = %d,  cur_idx = %d\n", gst_play_list.num_uri_list,  gst_play_list.cur_idx);
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("\n");
					gst_play_list.cur_idx = 0;
					total_count++;
//					exit(1);
				}


				if( hPlayer )
				{
					NX_MPClose( hPlayer );
					hPlayer = NULL;
				}

				if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, gst_play_list.uri[gst_play_list.cur_idx], volume, dspModule, dspPort, audio_request_track_num, video_request_track_num, (char *)&input_media_info, 0, priority, &callback, &appData )) )
				{
					printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", gst_play_list.uri[gst_play_list.cur_idx], mpResult);
					//		continue;
				}

				if( hPlayer )
				{
					if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
						//			continue;
					}
					isPaused = 0;
				}
				else
				{
					printf("Error : not Opened!!!");
				}

				gst_player_init = 1;
				gst_play_list.cur_idx++;
				printf("END index : num_uri_list = (0~%d), cur_idex (%d)\n", gst_play_list.num_uri_list-1, gst_play_list.cur_idx);
			}

			usleep(300000);
			duration = 0;
			position = 0;
			NX_MPGetCurDuration(hPlayer, &duration);
			NX_MPGetCurPosition(hPlayer, &position);

			printf("Postion : %d / %d msec | num_uri_list = %d,  cur_idx = %d, total_count = %d\n", position, duration, gst_play_list.num_uri_list,  gst_play_list.cur_idx, total_count);
//			printf("Postion : %d / %d msec \n", position, duration);

		} //while

		//
		//	gst_play_list.cur_idx = index;
	}
	else if(gst_ageing_cmd == AGEING_CMD_HEAVY_OPEN)
	{


	}
	//-------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------- stress seek
	else if( (gst_ageing_cmd == AGEING_CMD_HEAVY_SEEK_NORMAL) || (gst_ageing_cmd == AGEING_CMD_HEAVY_SEEK_HEAVY) )
	{
		int percent = 0;
		int seek_up = 0;
		int seek_down = 0;
		int secend = 0;
		int total_count = 1;
		while(1)
		{
			int seek_value = 0;
			
			if(gst_player_init == 0)
			{
				secend = 0;
				percent = 5;
				seek_up = 1;
				seek_down = 0;
				memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );
				volume = 100;
				dspModule = 0;
				dspPort = 0;
				audio_request_track_num = 0;
				video_request_track_num = 0;
				memset(&appData, 0, sizeof(appData));

				if( gst_play_list.cur_idx >= gst_play_list.num_uri_list || gst_play_list.cur_idx < 0 )
				{
					//			printf("END index : num_uri_list = %d, cur_idex (0~%d)\n", gst_play_list.num_uri_list, gst_play_list.cur_idx);
					//printf("Invalid index : idex (0~%d)\n", gst_play_list.num_uri_list-1);
					//gst_play_list.cur_idx = 0;
					printf("\n");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("SUCCESS:: num_uri_list = %d,  cur_idx = %d\n", gst_play_list.num_uri_list,  gst_play_list.cur_idx);
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("\n");
					//exit(1);
					gst_play_list.cur_idx = 0;
					total_count++;
				}

				if( hPlayer )
				{
					NX_MPClose( hPlayer );
					hPlayer = NULL;
					printf("!!!!!!!!!! NX_MPClose( hPlayer) !!!!!!!!!!!\n");
				}
				printf("!!!!!!!!!! uri = %s, cur_idx = %d !!!!!!!!!!!\n",gst_play_list.uri[gst_play_list.cur_idx], gst_play_list.cur_idx);

				if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, gst_play_list.uri[gst_play_list.cur_idx], volume, dspModule, dspPort, audio_request_track_num, video_request_track_num, (char *)&input_media_info, 0, priority, &callback, &appData )) )
				{
					printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", gst_play_list.uri[gst_play_list.cur_idx], mpResult);
					//		continue;
				}

				if( hPlayer )
				{
					if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
						//			continue;
					}
					isPaused = 0;
				}
				else
				{
					printf("Error : not Opened!!!");
				}

				gst_player_init = 1;
				gst_play_list.cur_idx++;
				printf("END index : num_uri_list = (0~%d), cur_idex (%d)\n", gst_play_list.num_uri_list-1, gst_play_list.cur_idx);
			}	//if

			//		usleep(300000);

			NX_MPGetCurDuration(hPlayer, &duration);
			NX_MPGetCurPosition(hPlayer, &position);

			printf("Postion : %d / %d msec | num_uri_list = %d,  cur_idx = %d, total_count = %d\n", position, duration, gst_play_list.num_uri_list,  gst_play_list.cur_idx, total_count);

			//seek
			//fast seek up~~~~~down
			if( (gst_ageing_cmd == AGEING_CMD_HEAVY_SEEK_HEAVY) )
			{
				usleep(30000);
				seek_value = duration * 0.01 * (float)percent;
				printf("seek_value : %d msec\n", seek_value);
				NX_MPSeek( hPlayer, seek_value );

				if(seek_up)
					percent = percent + 5;
				else	//seek_down
					percent = percent - 5;

				if(percent >= 90){
					seek_up = 0;
				}
				else if(percent < 10){
					seek_up = 1;
				}
			}
			//random seek
			else if( (gst_ageing_cmd == AGEING_CMD_HEAVY_SEEK_NORMAL)  )
			{
				usleep(300000);
				if( secend < (300*3*60*10)){	//10 min
//				if( secend < (300*10)){	//10 min
					seek_value = duration * 0.01 * (float)percent;
					//printf("seek_value : %d msec\n", seek_value);
					NX_MPSeek( hPlayer, seek_value );
					percent = random()%100;
					if(percent >= 95){
						percent = 95;
					}
					secend = secend + 300;
				}
			}			
		} //while

		//
		//	gst_play_list.cur_idx = index;
	}
	//-------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------- normal 
	else if( (gst_ageing_cmd == AGEING_CMD_NORMAL)  )
	{
		//gst_player_eos
		int percent = 0;
		int count = 0;
		int cmd_count = 0;
		int cmd_num= 0;
		int cmd_end = 0;
		int cmd_read = 1;
		char cmd_list_tmp[50];
		char cmd_tmp_1[10];
		char *p_list_tmp, *p_cmd;
		p_list_tmp = cmd_list_tmp;
		PlayCmd play_cmd;
		memset(&play_cmd, 0, sizeof(PlayCmd));
		//	p_cmd = cmd_tmp;
		int pause_sec = 0;
		int total_count = 1;
		while(1)
		{
			if(gst_player_init == 0)
			{
				if( gst_play_list.cur_idx >= gst_play_list.num_uri_list || gst_play_list.cur_idx < 0 )
				{
					//			printf("END index : num_uri_list = %d, cur_idex (0~%d)\n", gst_play_list.num_uri_list, gst_play_list.cur_idx);
					//printf("Invalid index : idex (0~%d)\n", gst_play_list.num_uri_list-1);
					//gst_play_list.cur_idx = 0;
					printf("\n");
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("SUCCESS:: num_uri_list = %d,  cur_idx = %d\n", gst_play_list.num_uri_list,  gst_play_list.cur_idx);
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					printf("\n");
					//exit(1);
					gst_play_list.cur_idx = 0;
					total_count++;
				}
				//memset(&play_cmd, 0, sizeof(PlayCmd));
				cmd_count = 0;
				pause_sec = 0;

				percent = 0;
				count = 0;
				cmd_num= 0;
				cmd_end = 0;
				cmd_read = 1;

				gst_player_init = 1;
				gst_play_list.cur_idx++;

				printf("============= play_cmd.play =%d\n",play_cmd.play);

			}
			//{
				usleep(300000);
				//		memset(cmd_tmp,0,10);
				memset(cmd, 0, sizeof(cmd));
				count = 0;

				strcpy(cmd_list_tmp, gst_cmd_list[cmd_count].cmd);
				cmd_num = gst_cmd_list[cmd_count].cmd_num;
				p_list_tmp = cmd_list_tmp;
				p_cmd = &cmd[count][0];

				if( (cmd_read == 1) && (cmd_end == 0) && (play_cmd.pause == 0))
				{
					while(1)
					{
						char temp;
						if(cmd_num <= 0)
							break;
						temp = *p_list_tmp++;
						if(temp == 0x20)	//0x20=>space
						{
							*p_cmd++ = NULL;
							count++;
							p_cmd = &cmd[count][0];
							cmd_num--;
							cmd_read = 0;

						}
						*p_cmd++ = temp;
						//printf("=+++++++++++++++++\n");
						//			count++;
					}
				}
				//--------------------------------------------------------------------------------------
				//printf("Ageing: File Player> ");
				printf("===cmd[0] = %s \n",cmd[0]);
				printf("===cmd[1] = %s \n",cmd[1]);
				printf("===cmd[2] = %s \n",cmd[2]);
				printf("===cmd[3] = %s \n",cmd[3]);

				if( (0 == strcasecmp( cmd[0], "NULL" )) ){
					cmd_end = 1;
					printf("==NULL !!!!!!!!!!!!!!\n");			
				}

				if( (0 == strcasecmp( cmd[0], "open" )) ){
					memset(&play_cmd, 0, sizeof(PlayCmd));
//printf("111111111111111111111111333\n");
					strcpy(cmd[3], gst_play_list.uri[gst_play_list.cur_idx-1]);
//printf("111111111111111111111111444\n");
					cmdCnt = gst_cmd_list[cmd_count].cmd_num;
					//cmdCnt++;
					cmd_read = 1;
					play_cmd.open = 1;
//printf("111111111111111111111111555\n");
				}
				else if( (0 == strcasecmp( cmd[0], "seek" )) ){
					int seek_value = 0;
					seek_value = atoi( cmd[1] );
					printf("==seek_value = %d\n",seek_value);
					NX_MPGetCurDuration(hPlayer, &duration);
					seek_value = duration * 0.01 * (float)seek_value;
					printf("==seek_value.... = %d\n",seek_value);
					sprintf(cmd[1], "%d", seek_value); 
					printf("==cmd[1] = %s.... = %d\n",cmd[1]);
				}
				else if( (0 == strcasecmp( cmd[0], "pause" )) ){
					pause_sec = gst_cmd_list[cmd_count].percent;
					pause_sec = pause_sec * 1000;	//ms
					printf("====pause_sec = %d\n",pause_sec);
					play_cmd.pause = 1;
				}
				cmdCnt = gst_cmd_list[cmd_count].cmd_num;

				if(play_cmd.pause == 1){
					pause_sec = pause_sec - 300;	//ms
					printf("====pause_sec -- = %d\n",pause_sec);
					if(pause_sec <= 0){
						play_cmd.pause = 0;
						cmd[0][0] = 'p';
						cmd[0][1] = 'l';
						cmd[0][2] = 'a';
						cmd[0][3] = 'y';
						printf("====cmd -- = %s\n",cmd[0]);
					}
				}
//printf("1111111111111111111111117777\n");
				if(play_cmd.pause == 0){
					if(cmd_end == 0){
						if(gst_cmd_list[cmd_count].percent == 0) {
							cmd_count++;
							cmd_read = 1;
						}
						else{
							int tmp = 0;
							if(play_cmd.play == 1){
								NX_MPGetCurDuration(hPlayer, &duration);
								tmp = duration * 0.01 * (float)gst_cmd_list[cmd_count].percent;
								NX_MPGetCurPosition(hPlayer, &position);
								if(tmp < position) {
									cmd_count++;
									cmd_read = 1;
								}
							}
						}
					}
				}

//printf("11111111111111111111111166666 play_cmd.play =%d\n",play_cmd.play);

				if(play_cmd.play == 1){
					NX_MPGetCurDuration(hPlayer, &duration);
					NX_MPGetCurPosition(hPlayer, &position);
					printf("Postion : %d / %d msec | num_uri_list = %d,  cur_idx = %d : normal\n", position, duration, gst_play_list.num_uri_list,  gst_play_list.cur_idx);
				}
				
//printf("111111111111111111111111\n");

				//
				//	Play Control Commands
				//
				if( (0 == strcasecmp( cmd[0], "typefind" )) )
				{
					//typefind
					memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );

					TYMEDIA_INFO *ty_handle = NULL;
					ret  = NX_TypeFind_Open( &ty_handle );
					if(ret != ERROR_NONE)
					{
						printf("Error : NX_TypeFind_Open!!!\n");
						continue;
					}

					ret = NX_TypeFind( ty_handle, uri );

					typefind_debug( ty_handle);

					memcpy(&input_media_info, ty_handle, sizeof(TYMEDIA_INFO) );

					NX_TypeFind_Close(ty_handle);

					if(ret != ERROR_NONE)
					{
						printf("Error : Not Support Contents!!!\n");
						continue;
					}

					if(ty_handle->DemuxType == DEMUX_TYPE_MPEGTSDEMUX) 
					{
						if(ty_handle->program_tot_no > 1){
							printf("Input program Number!!!\n");
							scanf("%d",&program_no_track_num);
							program_no_track_num--;
							audio_request_track_num = program_no_track_num;
							video_request_track_num = program_no_track_num;
						}
					}
					else
					{
						if(ty_handle->AudioTrackTotNum > 1){
							printf("Input Audio Request Track Number!!!\n");
							scanf("%d",&audio_request_track_num);
							while(1){
								if(audio_request_track_num < 1 || audio_request_track_num > ty_handle->AudioTrackTotNum){
									printf("Error : Input Audio Request Track Number!!!\n");
									scanf("%d",&audio_request_track_num);
								}
								else{
									audio_request_track_num--;
									break;
								}
							}
						}
						if(ty_handle->VideoTrackTotNum > 1){
							printf("Input Video Request Track Number!!!\n");
							scanf("%d",&video_request_track_num);
							while(1){
								if(video_request_track_num < 1 || video_request_track_num > ty_handle->VideoTrackTotNum){
									printf("Error : Input Video Request Track Number!!!\n");
									scanf("%d",&video_request_track_num);
								}
								else{
									video_request_track_num--;
									break;
								}
							}
						}
					}
					//typefind end

				}
				else if( (0 == strcasecmp( cmd[0], "open" )) | (0 == strcasecmp( cmd[0], "o" )) )
				{
					const char *openName;
					if( cmdCnt > 3)
					{
						dspModule = atoi(cmd[1]);
						dspPort = atoi(cmd[2]);
						openName = cmd[3];
					}
					else if( cmdCnt > 2 )
					{
						dspModule = atoi(cmd[1]);
						dspPort = atoi(cmd[2]);
						if( uri )
							openName = uri;
					}
					else if( cmdCnt > 1 )
					{
						dspModule = atoi(cmd[1]);
						if( uri )
							openName = uri;
					}
					else
					{
						if( uri == NULL )
						{
							printf("Error : Invalid argument !!!, Usage : open [filename]\n");
							continue;
						}
						else
						{
							openName = uri;
						}
					}
					if( hPlayer )
					{
						NX_MPClose( hPlayer );
						hPlayer = NULL;
					}
//printf("2222222222222222222222\n");
					memset(&appData, 0, sizeof(appData));
					if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, openName, volume, dspModule, dspPort, audio_request_track_num, video_request_track_num, (char *)&input_media_info, 0, priority, &callback, &appData )) )
					{
						printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", openName, mpResult);
						continue;
					}
					isPaused = 0;
					appData.hPlayer = hPlayer;

					if( dspModule == 1 && dspPort == 1 )
					{
						if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( hPlayer, dspModule, dspPort, 0, 0, 1920, 1080 ) ) )
						{
							printf("Error : NX_MPSetDspPosition Failed!!!(%d)\n", mpResult);
						}
					}

				}
				else if( 0 == strcasecmp( cmd[0], "close" ) )
				{
					if( hPlayer )
					{
						NX_MPClose( hPlayer );
						isPaused = 0;
						hPlayer = NULL;
					}
					else
					{
						printf("Already closed or not Opened!!!");
					}
				}
				else if( 0 == strcasecmp( cmd[0], "info" ) )
				{
					//	N/A
					if( hPlayer )
					{
						NX_MPGetMediaInfo( hPlayer, 0 , &media_info);
					}
				}
				else if( 0 == strcasecmp( cmd[0], "play" ) )
				{
					if( hPlayer )
					{
						if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
						{
							printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
							continue;
						}
						isPaused = 0;
						play_cmd.play = 1;
					}
					else
					{
						printf("Error : not Opened!!!");
					}
				}
				else if( 0 == strcasecmp( cmd[0], "pause" ) )
				{
					if( hPlayer )
					{
						if( ERROR_NONE != (mpResult = NX_MPPause( hPlayer )) )
						{
							printf("Error : NX_MPPause Failed!!!(%d)\n", mpResult);
							continue;
						}
						isPaused = 1;
					}
					else
					{
						printf("Error : not Opened!!!");
					}
				}
				else if( 0 == strcasecmp( cmd[0], "p" ) )
				{
					if( hPlayer )
					{
						if( isPaused )
						{
							if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
							{
								printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
								continue;
							}
							isPaused = 0;
							printf("Play~~~\n");
						}
						else
						{
							if( ERROR_NONE != (mpResult = NX_MPPause( hPlayer )) )
							{
								printf("Error : NX_MPPause Failed!!!(%d)\n", mpResult);
								continue;
							}
							isPaused = 1;
							printf("Paused~~~\n");
						}
					}
				}
				else if( 0 == strcasecmp( cmd[0], "stop" ) )
				{
					if( hPlayer )
					{
						if( ERROR_NONE != (mpResult = NX_MPStop( hPlayer )) )
						{
							printf("Error : NX_MPStop Failed!!!(%d)\n", mpResult);
						}
					}
					else
					{
						printf("Error : not Opened!!!");
					}
				}
				else if( (0 == strcasecmp( cmd[0], "seek" )) || (0 == strcasecmp( cmd[0], "s" )) )
				{
					int seekTime;
					if(cmdCnt<2)
					{
						printf("Error : Invalid argument !!!, Usage : seek (or s) [milli seconds]\n");
						continue;
					}

					seekTime = atoi( cmd[1] );

					if( hPlayer )
					{
						if( ERROR_NONE != (mpResult = NX_MPSeek( hPlayer, seekTime )) )
						{
							printf("Error : NX_MPSeek Failed!!!(%d)\n", mpResult);
						}
					}
					else
					{
						printf("Error : not Opened!!!");
					}
				}
				else if( (0 == strcasecmp( cmd[0], "status" )) || (0 == strcasecmp( cmd[0], "st" )) )
				{
					if( hPlayer )
					{
						unsigned int duration, position;
						NX_MPGetCurDuration(hPlayer, &duration);
						NX_MPGetCurPosition(hPlayer, &position);
						printf("Postion : %d / %d msec\n", position, duration);
					}
					else
					{
						printf("\nPlayer does not initialized!!\n");
					}
				}

				//
				//	Other Control Commands
				//
				else if( 0 == strcasecmp( cmd[0], "pos" ) )
				{
					int x, y, w, h;

					if( cmdCnt < 7)
					{
						printf("\nError : Invalid arguments, Usage : pos [module] [port] [x] [y] [w] [h]\n");
						continue;
					}
					dspModule 	= atoi(cmd[1]);
					dspPort 	= atoi(cmd[2]);
					x 			= atoi(cmd[3]);
					y 			= atoi(cmd[4]);
					w 			= atoi(cmd[5]);
					h 			= atoi(cmd[6]);

					if( hPlayer )
					{
						if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( hPlayer, dspModule, dspPort, x, y, w, h ) ) )
						{
							printf("Error : NX_MPSetDspPosition Failed!!!(%d)\n", mpResult);
						}
					}
				}
				else if( 0 == strcasecmp( cmd[0], "vol" ) )
				{
					int vol;
					if(cmdCnt<2)
					{
						printf("Error : Invalid argument !!!, Usage : vol [volume]\n");
						continue;
					}

					vol = atoi( cmd[1] ) % 101;

					if( vol > 100 )
					{
						vol = 100;
					}
					else if( vol < 0 )
					{
						vol = 0;
					}

					if( hPlayer )
					{
						if( ERROR_NONE != (mpResult = NX_MPSetVolume( hPlayer, vol ) ) )
						{
							printf("Error : NX_MPSeek Failed!!!(%d)\n", mpResult);
						}
						//	Save to Application Volume
						volume = vol;
					}
				}

				//--------------------------------------------------------------------------------------
				//gst_player_init = 1;
				//gst_play_list.cur_idx++;
				
			} //while
		//} // gst_player_init end

		//
		//	gst_play_list.cur_idx = index;
	}
	//-------------------------------------------------------------------------------------

	{
		int i;
		for( i = 0 ; i<gst_play_list.num_uri_list ; i++ )
		{
			if( gst_play_list.uri[i] )
			{
				free(gst_play_list.uri[i]);
				gst_play_list.uri[i] = NULL;
			}
		}
	}



	//
	while(1)
	{
		printf("File Player> ");

		memset(cmd, 0, sizeof(cmd));
		fgets( cmdstring, sizeof(cmdstring), stdin );
		cmdCnt = GetArgument( cmdstring, cmd );

		//
		//	Play Control Commands
		//
		if( (0 == strcasecmp( cmd[0], "typefind" )) )
		{
			//typefind
			memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );

			TYMEDIA_INFO *ty_handle = NULL;
			ret  = NX_TypeFind_Open( &ty_handle );
			if(ret != ERROR_NONE)
			{
				printf("Error : NX_TypeFind_Open!!!\n");
				continue;
			}

			ret = NX_TypeFind( ty_handle, uri );

			typefind_debug( ty_handle);

			memcpy(&input_media_info, ty_handle, sizeof(TYMEDIA_INFO) );

			NX_TypeFind_Close(ty_handle);

			if(ret != ERROR_NONE)
			{
				printf("Error : Not Support Contents!!!\n");
				continue;
			}

			if(ty_handle->DemuxType == DEMUX_TYPE_MPEGTSDEMUX) 
			{
				if(ty_handle->program_tot_no > 1){
					printf("Input program Number!!!\n");
					scanf("%d",&program_no_track_num);
					program_no_track_num--;
					audio_request_track_num = program_no_track_num;
					video_request_track_num = program_no_track_num;
				}
			}
			else
			{
				if(ty_handle->AudioTrackTotNum > 1){
					printf("Input Audio Request Track Number!!!\n");
					scanf("%d",&audio_request_track_num);
					while(1){
						if(audio_request_track_num < 1 || audio_request_track_num > ty_handle->AudioTrackTotNum){
							printf("Error : Input Audio Request Track Number!!!\n");
							scanf("%d",&audio_request_track_num);
						}
						else{
							audio_request_track_num--;
							break;
						}
					}
				}
				if(ty_handle->VideoTrackTotNum > 1){
					printf("Input Video Request Track Number!!!\n");
					scanf("%d",&video_request_track_num);
					while(1){
						if(video_request_track_num < 1 || video_request_track_num > ty_handle->VideoTrackTotNum){
							printf("Error : Input Video Request Track Number!!!\n");
							scanf("%d",&video_request_track_num);
						}
						else{
							video_request_track_num--;
							break;
						}
					}
				}
			}
			//typefind end

		}
		else if( (0 == strcasecmp( cmd[0], "open" )) | (0 == strcasecmp( cmd[0], "o" )) )
		{
			const char *openName;
			if( cmdCnt > 3)
			{
				dspModule = atoi(cmd[1]);
				dspPort = atoi(cmd[2]);
				openName = cmd[3];
			}
			else if( cmdCnt > 2 )
			{
				dspModule = atoi(cmd[1]);
				dspPort = atoi(cmd[2]);
				if( uri )
					openName = uri;
			}
			else if( cmdCnt > 1 )
			{
				dspModule = atoi(cmd[1]);
				if( uri )
					openName = uri;
			}
			else
			{
				if( uri == NULL )
				{
					printf("Error : Invalid argument !!!, Usage : open [filename]\n");
					continue;
				}
				else
				{
					openName = uri;
				}
			}
			if( hPlayer )
			{
				NX_MPClose( hPlayer );
				hPlayer = NULL;
			}

			memset(&appData, 0, sizeof(appData));
			if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, openName, volume, dspModule, dspPort, audio_request_track_num, video_request_track_num, (char *)&input_media_info, 0,  priority, &callback, &appData )) )
			{
				printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", openName, mpResult);
				continue;
			}
			isPaused = 0;
			appData.hPlayer = hPlayer;

			if( dspModule == 1 && dspPort == 1 )
			{
				if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( hPlayer, dspModule, dspPort, 0, 0, 1920, 1080 ) ) )
				{
					printf("Error : NX_MPSetDspPosition Failed!!!(%d)\n", mpResult);
				}
			}

		}
		else if( 0 == strcasecmp( cmd[0], "close" ) )
		{
			if( hPlayer )
			{
				NX_MPClose( hPlayer );
				isPaused = 0;
				hPlayer = NULL;
			}
			else
			{
				printf("Already closed or not Opened!!!");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "info" ) )
		{
			//	N/A
			if( hPlayer )
			{
				NX_MPGetMediaInfo( hPlayer, 0 , &media_info);
			}
		}
		else if( 0 == strcasecmp( cmd[0], "play" ) )
		{
			if( hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
				{
					printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
					continue;
				}
				isPaused = 0;
			}
			else
			{
				printf("Error : not Opened!!!");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "pause" ) )
		{
			if( hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPPause( hPlayer )) )
				{
					printf("Error : NX_MPPause Failed!!!(%d)\n", mpResult);
					continue;
				}
				isPaused = 1;
			}
			else
			{
				printf("Error : not Opened!!!");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "p" ) )
		{
			if( hPlayer )
			{
				if( isPaused )
				{
					if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
						continue;
					}
					isPaused = 0;
					printf("Play~~~\n");
				}
				else
				{
				if( ERROR_NONE != (mpResult = NX_MPPause( hPlayer )) )
					{
						printf("Error : NX_MPPause Failed!!!(%d)\n", mpResult);
						continue;
					}
					isPaused = 1;
					printf("Paused~~~\n");
				}
			}
		}
		else if( 0 == strcasecmp( cmd[0], "stop" ) )
		{
			if( hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPStop( hPlayer )) )
				{
					printf("Error : NX_MPStop Failed!!!(%d)\n", mpResult);
				}
			}
			else
			{
				printf("Error : not Opened!!!");
			}
		}
		else if( (0 == strcasecmp( cmd[0], "seek" )) || (0 == strcasecmp( cmd[0], "s" )) )
		{
			int seekTime;
			if(cmdCnt<2)
			{
				printf("Error : Invalid argument !!!, Usage : seek (or s) [milli seconds]\n");
				continue;
			}

			seekTime = atoi( cmd[1] );

			if( hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPSeek( hPlayer, seekTime )) )
				{
					printf("Error : NX_MPSeek Failed!!!(%d)\n", mpResult);
				}
			}
			else
			{
				printf("Error : not Opened!!!");
			}
		}
		else if( (0 == strcasecmp( cmd[0], "status" )) || (0 == strcasecmp( cmd[0], "st" )) )
		{
			if( hPlayer )
			{
				unsigned int duration, position;
				NX_MPGetCurDuration(hPlayer, &duration);
				NX_MPGetCurPosition(hPlayer, &position);
				printf("Postion : %d / %d msec\n", position, duration);
			}
			else
			{
				printf("\nPlayer does not initialized!!\n");
			}
		}

		//
		//	Other Control Commands
		//
		else if( 0 == strcasecmp( cmd[0], "pos" ) )
		{
			int x, y, w, h;

			if( cmdCnt < 7)
			{
				printf("\nError : Invalid arguments, Usage : pos [module] [port] [x] [y] [w] [h]\n");
				continue;
			}
			dspModule 	= atoi(cmd[1]);
			dspPort 	= atoi(cmd[2]);
			x 			= atoi(cmd[3]);
			y 			= atoi(cmd[4]);
			w 			= atoi(cmd[5]);
			h 			= atoi(cmd[6]);

			if( hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( hPlayer, dspModule, dspPort, x, y, w, h ) ) )
				{
					printf("Error : NX_MPSetDspPosition Failed!!!(%d)\n", mpResult);
				}
			}
		}
		else if( 0 == strcasecmp( cmd[0], "vol" ) )
		{
			int vol;
			if(cmdCnt<2)
			{
				printf("Error : Invalid argument !!!, Usage : vol [volume]\n");
				continue;
			}

			vol = atoi( cmd[1] ) % 101;

			if( vol > 100 )
			{
				vol = 100;
			}
			else if( vol < 0 )
			{
				vol = 0;
			}

			if( hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPSetVolume( hPlayer, vol ) ) )
				{
					printf("Error : NX_MPSeek Failed!!!(%d)\n", mpResult);
				}
				//	Save to Application Volume
				volume = vol;
			}
		}

		//
		//	Help & Exit
		//
		else if( (0 == strcasecmp( cmd[0], "exit" )) || (0 == strcasecmp( cmd[0], "q" )) )
		{
			printf("Exit ByeBye ~~~\n");
			break;
		}
		else if( (0 == strcasecmp( cmd[0], "help" )) || (0 == strcasecmp( cmd[0], "h" )) || (0 == strcasecmp( cmd[0], "?" )) )
		{
			shell_help();
		}
		else
		{
			if( cmdCnt != 0 )
				printf("Unknown command : %s, cmdCnt=%d, %d\n", cmd[0], cmdCnt, strlen(cmd[0]) );
		}
	}

	if( hPlayer )
	{
		NX_MPClose( hPlayer );
	}

	return 0;
}

void print_usage(const char *appName)
{
	printf( "usage: %s [options]\n", appName );
	printf( "    -f [file name]     : file name\n" 
			"    -d [scan path]     : scan path\n"
			"    -o [scan out name] : scan out file name\n", appName);

//	printf( "    -s                 : shell command mode\n" );
}


int main( int argc, char *argv[] )
{	
	int opt = 0, count = 0;
	unsigned int duration = 0, position = 0;
	int level = 1, shell_mode = 0;
	static char uri[2048];
	MP_HANDLE handle = NULL;
	int volume = 1;
	int dspPort = DISPLAY_PORT_LCD;						//DISPLAY_PORT_LCD:(0) ,DISPLAY_PORT_HDMI:(1) 
	int dspModule = DISPLAY_MODULE_MLC0;				//DISPLAY_MODULE_MLC0:(0) ,DISPLAY_MODULE_MLC1:(1) 

	int audio_request_track_num = 0;						//multi track, default 0
	int video_request_track_num = 0;						//multi track, default 0
	TYMEDIA_INFO input_media_info;

	static char	*scan_path=NULL;
	static char	*out_file=NULL;
	int priority = 0;


	if( 2>argc )
	{
		print_usage( argv[0] );
		return 0;
	}

	while( -1 != (opt=getopt(argc, argv, "hsf:d:o:")))
	{
		switch( opt ){
			case 'h':	print_usage(argv[0]);		return 0;
			case 'f':	strcpy(uri, optarg);		break;
			case 's':	shell_mode = 1;				break;
			case 'd':	scan_path = strdup(optarg);	break;
			case 'o':	out_file = strdup(optarg);	break;
			default:								break;
		}
	}

	if( shell_mode )
	{
		return shell_main( scan_path, out_file );
	}

	memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );
	NX_MPOpen( &handle, uri, 100, 0, 0, audio_request_track_num, video_request_track_num, (char *)&input_media_info, 0,  priority, &callback, NULL);
	printf("handle_s1 = %p\n",handle);

	if( handle )
	{

		printf("Init Play Done\n");

		if( NX_MPPlay( handle, 1 ) != 0 )
		{
			printf("NX_MPPlay failed\n");
		}
		printf("Start Play Done\n");
	}
	count = 0;

	while(1)
	{
		usleep(300000);

		NX_MPGetCurDuration(handle, &duration);
		NX_MPGetCurPosition(handle, &position);

		printf("Postion : %d / %d msec\n", position, duration);

		count ++;	
		

		if( count == 50 )
		{
//			NX_MPSeek( handle, 90000 );
//			NX_MPPause( handle);
//			NX_MPSetVolume(handle, 0);
		}
	}
}