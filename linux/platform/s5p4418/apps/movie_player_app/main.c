#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	//	getopt & optarg
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#include <NX_MoviePlay.h>
#include <NX_TypeFind.h>
#include <uevent.h>


typedef struct CommandBuffer{
	int		cmd_cnt;	
	char	cmd[10][10];	
}CommandBuffer;


#define			MAX_COMMAND_QUEUE		1024
pthread_mutex_t	gst_CmdMutex;
pthread_mutex_t	gst_HdmiMutex;
static CommandBuffer		gst_CommandQueue[MAX_COMMAND_QUEUE];
static int		gst_NumCmd = 0;
static int		gst_CmdHead = 0;
static int		gst_CmdTail = 0;
pthread_t		gst_hHdmiThread;
pthread_t		gst_hCmdThread;
static int 		thread_flag = 0;

char uri_tmp[1024];


static int PushCommand( CommandBuffer *cmd );
static int PopCommand(CommandBuffer *cmd);


unsigned int  position_save = 0;

typedef struct Static_player_st{
	int display;
	int	hdmi_detect;
	int	hdmi_detect_init;
	int	volume;
	int	audio_request_track_num;
	int	video_request_track_num;
}Static_player_st;

Static_player_st	static_player;

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
	printf("    typefind                               : file typefind\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("                        Other Control Commands\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("    pos [module] [port] [x] [y] [w] [h]    : change play position\n");
	printf("    vol [gain(%%), 0~100]                  : change volume(0:mute)\n");
	printf("===============================================================================\n\n");
}

#define HDMI_STATE_FILE     "/sys/class/switch/hdmi/state"
void *HdmiDetectThread( void *arg )
{
	int err;	
	int fd;
	int init;
	unsigned int hdmi;
	struct pollfd fds[1];
	unsigned char uevent_desc[2048];
	char val;
	CommandBuffer cmd_st;

	pthread_mutex_init( &gst_HdmiMutex, NULL );

	uevent_init();

    fds[0].fd = uevent_get_fd();
    fds[0].events = POLLIN;

	fd = open(HDMI_STATE_FILE, O_RDONLY);
	if( fd > 0 ){
		if (read(fd, (char *)&val, 1) == 1 && val == '1') {
			printf("=== HDMI ON ==== %d\n", static_player.hdmi_detect);
			if(static_player.hdmi_detect != ON){
				static_player.hdmi_detect = ON;
				memset(&cmd_st, 0, sizeof(CommandBuffer));
				cmd_st.cmd_cnt = 1;
				strcpy(cmd_st.cmd[0], "display");
//				printf("=== display ==== \n");
				PushCommand( &cmd_st );
			}
			
		}else {
			printf("=== HDMI OFF ==== %d\n", static_player.hdmi_detect);
//			static_player.hdmi_detect = OFF;
			if(static_player.hdmi_detect != OFF){
				//			static_player.hdmi_detect_init = 0;
				static_player.hdmi_detect = OFF;
				memset(&cmd_st, 0, sizeof(CommandBuffer));
				cmd_st.cmd_cnt = 1;
				strcpy(cmd_st.cmd[0], "display");
//				printf("=== display ==== \n");
				PushCommand( &cmd_st );
			}
		}
		close(fd);
	}


	while(1) {

		if(thread_flag == 1)
			break;

        err = poll(fds, 1, -1);

        printf("=== display 123456  ==== \n");

		if( (static_player.hdmi_detect == ON) && (static_player.display != DISPLAY_PORT_DUAL))
		{
			static_player.hdmi_detect = ON;
			memset(&cmd_st, 0, sizeof(CommandBuffer));
			cmd_st.cmd_cnt = 1;
			strcpy(cmd_st.cmd[0], "display");
			printf("=== display ==== \n");
			PushCommand( &cmd_st );
		}

		if (err > 0) {
			if (fds[0].revents & POLLIN) {
				int len = uevent_next_event((char *)uevent_desc, sizeof(uevent_desc) - 2);
				int hdmi = !strcmp((const char *)uevent_desc, (const char *)"change@/devices/virtual/switch/hdmi");
				if (hdmi){
					fd = open(HDMI_STATE_FILE, O_RDONLY);
					if (fd < 0) {
						printf("failed to open hdmi state fd: %s", HDMI_STATE_FILE);
					}
					if( fd > 0 )
					{
						if (read(fd, &val, 1) == 1 && val == '1') {
							printf("=== HDMI ON ==== %d, %d\n", static_player.hdmi_detect, static_player.display );
							if(static_player.hdmi_detect != ON){
								static_player.hdmi_detect = ON;
								memset(&cmd_st, 0, sizeof(CommandBuffer));
								cmd_st.cmd_cnt = 1;
								strcpy(cmd_st.cmd[0], "display");
//								printf("=== display ==== \n");
								PushCommand( &cmd_st );
							}						

						}else {
							printf("=== HDMI OFF ==== %d\n", static_player.hdmi_detect);
							if(static_player.hdmi_detect != OFF){
								//			static_player.hdmi_detect_init = 0;
								static_player.hdmi_detect = OFF;
								memset(&cmd_st, 0, sizeof(CommandBuffer));
								cmd_st.cmd_cnt = 1;
								strcpy(cmd_st.cmd[0], "display");
//								printf("=== display ==== \n");
								PushCommand( &cmd_st );
							}
						}
						close(fd);
					}
				}
			}
		} else if (err == -1) {
			printf("error in vsync thread \n");
		}
    }

	pthread_mutex_destroy( &gst_HdmiMutex );

	return (void*)0xdeaddead;
}

static int PopCommand(CommandBuffer *cmd )
{
	//	Command Pop Operation
	pthread_mutex_lock( &gst_CmdMutex );
	*cmd = gst_CommandQueue[ gst_CmdTail++ ];
	if( gst_CmdTail >= MAX_COMMAND_QUEUE )
	{
		gst_CmdTail = 0;
	}
	gst_NumCmd --;
	pthread_mutex_unlock( &gst_CmdMutex );
	return 0;
}

static int PushCommand( CommandBuffer *cmd  )
{
	//	Command Pop Operation
	pthread_mutex_lock( &gst_CmdMutex );
	gst_CommandQueue[ gst_CmdHead++ ] = *cmd;
	if( gst_CmdHead >= MAX_COMMAND_QUEUE )
	{
		gst_CmdHead = 0;
	}
	gst_NumCmd ++;
	pthread_mutex_unlock( &gst_CmdMutex );
	return 0;
}

void *CommandThread( void *arg )
{
	static 	char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	int i = 0;
	int volume = 50;
	int cmdCnt;
	int dspPort = DISPLAY_PORT_LCD;						//DISPLAY_PORT_LCD:(0) ,DISPLAY_PORT_HDMI:(1) 
	int dspModule = DISPLAY_MODULE_MLC0;				//DISPLAY_MODULE_MLC0:(0) ,DISPLAY_MODULE_MLC1:(1) 
	int audio_request_track_num = 0;					//multi track, default 0
	int video_request_track_num = 0;					//multi track, default 0
	int program_no_track_num = 0;
	MP_RESULT mpResult = ERROR_NONE;
	int ret = 0;
	int	display = DISPLAY_PORT_LCD;
	TYMEDIA_INFO input_media_info;
	CommandBuffer cmd_st;
	int status;
	int priority = 0;

	//	Player Specific Parameters

	MP_HANDLE hPlayer = NULL;
	MP_MEDIA_INFO media_info;
	AppData appData;
	int isPaused = 0;

	const char *uri = NULL;

	memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );
	memset(&static_player, 0, sizeof(Static_player_st) );
	uri = uri_tmp;


	pthread_mutex_init( &gst_CmdMutex, NULL );

	while(1)
	{
		if( gst_NumCmd < 1 )
		{
			usleep( 30 );
			continue;
		}

		PopCommand(&cmd_st);

		//
		memset(cmd, 0, sizeof(cmd));
		cmdCnt = cmd_st.cmd_cnt;
		for(i = 0; i < cmdCnt; i++){
			strcpy(cmd[i], cmd_st.cmd[i]);
		}
			
#if 1
	    //
		//	Play Control Commands
		//
		if( (0 == strcasecmp( cmd[0], "display" )) ){
				printf("================== input display========================\n");
			if(static_player.hdmi_detect == OFF)
			{
				//display = DISPLAY_PORT_LCD;
				printf("================== input lcd========================\n");
				if( hPlayer ){
					static_player.volume = volume;
					static_player.audio_request_track_num = audio_request_track_num;
					static_player.video_request_track_num = video_request_track_num;
					NX_MPGetCurPosition(hPlayer, &position_save);

					NX_MPClose( hPlayer );
					isPaused = 0;
					hPlayer = NULL;

					memset(&appData, 0, sizeof(appData));
					display = DISPLAY_PORT_LCD;

					dspModule = 0;
					dspPort = 0;

					static_player.display = DISPLAY_PORT_LCD;
					if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, uri, static_player.volume, dspModule, dspPort, static_player.audio_request_track_num, static_player.video_request_track_num , (char *)&input_media_info, display,  priority, &callback, &appData )) )
					{
						printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", uri, mpResult);
						continue;
					}

					if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
						continue;
					}
					isPaused = 0;


					if( ERROR_NONE != (mpResult = NX_MPSeek( hPlayer, position_save )) )
					{
						printf("Error : NX_MPSeek Failed!!!(%d)\n", mpResult);
					}

					if(static_player.hdmi_detect_init == 1)
						static_player.hdmi_detect_init = 0;
				}
			}
			else if(static_player.hdmi_detect == ON)
			{
					printf("===================== input dual============================\n");
					{
						if( hPlayer ){
							static_player.volume = volume;
							static_player.audio_request_track_num = audio_request_track_num;
							static_player.video_request_track_num = video_request_track_num;
							NX_MPGetCurPosition(hPlayer, &position_save);

							NX_MPClose( hPlayer );
							isPaused = 0;
							hPlayer = NULL;

							memset(&appData, 0, sizeof(appData));

							dspModule = 0;
							dspPort = 0;

							display = DISPLAY_PORT_DUAL;
							static_player.display = DISPLAY_PORT_DUAL;
							if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, uri, static_player.volume, dspModule, dspPort, static_player.audio_request_track_num, static_player.video_request_track_num , (char *)&input_media_info, display,  priority, &callback, &appData )) )
							{
								printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", uri, mpResult);
								continue;
							}

							dspModule = 1;
							dspPort = 1;
							if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( hPlayer, dspModule, dspPort, 0, 0, 1920, 1080 ) ) )
							{
								printf("Error : NX_MPSetDspPosition Failed!!!(%d)\n", mpResult);
							}


							if( ERROR_NONE != (mpResult = NX_MPPlay( hPlayer, 1.0 )) )
							{
								printf("Error : NX_MPPlay Failed!!!(%d)\n", mpResult);
								continue;
							}
							isPaused = 0;


							if( ERROR_NONE != (mpResult = NX_MPSeek( hPlayer, position_save )) )
							{
								printf("Error : NX_MPSeek Failed!!!(%d)\n", mpResult);
							}

							static_player.hdmi_detect_init = 1;
						}
					}
				}
				else {
					printf("Waring : Hdmi Not Connect!!!\n");
				}
		}
		else if( (0 == strcasecmp( cmd[0], "typefind" )) )
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
			if( dspModule == 1 && dspPort == 1 ){
				display = DISPLAY_PORT_HDMI;
				static_player.display = DISPLAY_PORT_DUAL;
			}
			else if( dspModule == 0 && dspPort == 0 ){
				display = DISPLAY_PORT_LCD;
				static_player.display = DISPLAY_PORT_LCD;
			}
			if( ERROR_NONE != (mpResult = NX_MPOpen( &hPlayer, openName, volume, dspModule, dspPort, audio_request_track_num, video_request_track_num, (char *)&input_media_info, display,  priority, &callback, &appData )) )
			{
				printf("Error : NX_MPOpen Failed!!!(uri=%s, %d)\n", openName, mpResult);
				continue;
			}
			isPaused = 0;
			appData.hPlayer = hPlayer;

			if( dspModule == 1 && dspPort == 1 )
			{
				//printf("=== HDMI ===\n");
				//printf("=== dspModule = %d, dspPort = %d ===\n", dspModule, dspPort);
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
				memset(&static_player, 0, sizeof(Static_player_st) );
			}
			else
			{
				printf("Already closed or not Opened!!!");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "info" ) )
		{
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
				if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( hPlayer, dspModule, dspPort, x, y, w, h) ) )
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
			thread_flag = 1;
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
#endif
	}

	if( hPlayer )
	{
		NX_MPClose( hPlayer );
	}

	pthread_mutex_destroy( &gst_CmdMutex );

	return (void*)0xdeaddead;
}

static int shell_main( const char *uri )
{
	static	char cmdstring[SHELL_MAX_ARG * SHELL_MAX_STR];
	static 	char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	int i = 0;
	int cmdCnt;
	CommandBuffer cmd_st;

	//	Player Specific Parameters

	MP_HANDLE hPlayer = NULL;
	memset(&static_player, 0, sizeof(Static_player_st) );

	strcpy(uri_tmp, uri);
	

	if( pthread_create( &gst_hHdmiThread, NULL, HdmiDetectThread, NULL ) != 0 )
	{
		exit( -2 );
	}

	if( pthread_create( &gst_hCmdThread, NULL, CommandThread, NULL ) != 0 )
	{
		exit( -2 );
	}

	shell_help();
	while(1)
	{
		printf("File Player> ");

		memset(cmd, 0, sizeof(cmd));
		fgets( cmdstring, sizeof(cmdstring), stdin );
		cmdCnt = GetArgument( cmdstring, cmd );

		memset(&cmd_st, 0, sizeof(CommandBuffer));
		cmd_st.cmd_cnt = cmdCnt;
		for(i = 0; i < cmdCnt; i++){
			strcpy(cmd_st.cmd[i], cmd[i]);
		}
		PushCommand( &cmd_st );

		if( (0 == strcasecmp( cmd[0], "exit" )) || (0 == strcasecmp( cmd[0], "q" )) )
			break;

		if(thread_flag == 1)
			break;
	}

	pthread_join( gst_hHdmiThread, NULL );
	pthread_join( gst_hCmdThread, NULL );

	return 0;
}

void print_usage(const char *appName)
{
	printf( "usage: %s [options]\n", appName );
	printf( "    -f [file name]     : file name\n" );
	printf( "    -s                 : shell command mode\n" );
}


int main( int argc, char *argv[] )
{	
	int opt = 0, count = 0;
	unsigned int duration = 0, position = 0;
	int shell_mode = 0;
	static char uri[2048];
	MP_HANDLE handle = NULL;

	int audio_request_track_num = 0;						//multi track, default 0
	int video_request_track_num = 0;						//multi track, default 0
	TYMEDIA_INFO input_media_info;
	int display = DISPLAY_PORT_LCD;
	int priority = 0;

	if( 2>argc )
	{
		print_usage( argv[0] );
		return 0;
	}

	while( -1 != (opt=getopt(argc, argv, "hsf:")))
	{
		switch( opt ){
			case 'h':	print_usage(argv[0]);		return 0;
			case 'f':	strcpy(uri, optarg);		break;
			case 's':	shell_mode = 1;				break;
			default:								break;
		}
	}

	if( shell_mode )
	{
		return shell_main( uri );
	}

	memset(&input_media_info, 0, sizeof(TYMEDIA_INFO) );
	NX_MPOpen( &handle, uri, 100, 0, 0, audio_request_track_num, video_request_track_num, (char *)&input_media_info, display, priority, &callback, NULL);
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