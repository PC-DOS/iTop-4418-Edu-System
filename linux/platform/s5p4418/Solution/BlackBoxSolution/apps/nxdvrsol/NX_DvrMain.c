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

#include <stdio.h>
#include <stdbool.h>

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/mount.h>

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>

#include <NX_DvrControl.h>
#include <nx_fourcc.h>

#include "NX_DvrCmdQueue.h"

#include "NX_DvrGpsManager.h"
#include "NX_DvrGsensorManager.h"
#include "NX_DvrPowerManager.h"
#include "NX_DvrFileManager.h"
#include "NX_DvrLedCtrl.h"
#include "NX_DvrTools.h"

#include <nx_alloc_mem.h>
#include <nx_graphictools.h>
#include <nx_audio.h>
#include <nx_gpio.h>
#include <nx_dsp.h>

// #define VEHICLE_TEST
// #define DEMO

#define BOARD_TYPE_LYNX
//#define CAMERA_TYPE_FHD

// #define DISABLE_AUDIO

#define DISPLAY_TICK

#define AUDIO_TYPE_AAC
#define EXTERNAL_MOTION_DETECTION

#define MMCBLOCK				"mmcblk0"
#define DIRECTORY_TOP			"/mnt/mmc"

static int32_t DvrInputEventThreadStart( void );
static int32_t DvrInputEventThreadStop( void );

static int32_t DvrShellThreadStart( void );
static int32_t DvrShellThreadStop( void );

static int32_t DvrSDCheckerThreadStart( void );
static int32_t DvrSDCheckerThreadStop( void );

enum {
	CONTAINER_TYPE_TS,
	CONTAINER_TYPE_MP4,
};

// Handle
static NX_DVR_HANDLE			g_hDvr = NULL;
static CMD_QUEUE_HANDLE			g_hCmd = NULL;
static GPS_MANAGER_HANDLE		g_hGpsManager = NULL;
static GSENSOR_MANAGER_HANDLE	g_hGsensorManager = NULL;
static POWER_MANAGER_HANDLE		g_hPowerManager = NULL;
static FILE_MANAGER_HANDLE		g_hNormalFileManager = NULL;
static FILE_MANAGER_HANDLE		g_hEventFileManager = NULL;
static FILE_MANAGER_HANDLE		g_hCaptureFileManager = NULL;
static NX_GT_SCALER_HANDLE		g_hFrontEffect = NULL;
static NX_GT_SCALER_HANDLE		g_hRearEffect = NULL;
static NX_AUDIO_HANDLE			g_hAudio = NULL;

static uint8_t dir_top[256]	 	= {0x00, };
static uint8_t dir_normal[256]	= {0x00, };
static uint8_t dir_event[256]	= {0x00, };
static uint8_t dir_capture[256]	= {0x00, };

static pthread_mutex_t	gstModeLock; 
static int32_t			gDvrMode 	= DVR_MODE_NORMAL;	// DVR_MODE_NORMAL / DVR_MODE_MOTION

static uint8_t gstSdNode[64];

static int32_t gstMsg = false;
static int32_t gstNoConsole = false;
static int32_t gstVideoChannel = 1;
static int32_t gstAudioEnable = false;
static int32_t gstUserDataEnable = false;
static int32_t gstNetworkType = false;

#ifdef DEMO
static int32_t gstMotionEnable = true;
#else
static int32_t gstMotionEnable = false;
#endif

static int32_t gstApiMajor = 0, gstApiMinor = 0, gstApiRevision = 0;
static int32_t gstContainer = CONTAINER_TYPE_TS;		// CONTAINER_TYPE_MP4 / CONTAINER_TYPE_TS
static int32_t gstTestEvent	= false;

static int32_t gstPreviewChannel = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Test Thread
//
static int32_t bTestEventTask		= false;
static int32_t bEventTaskThreadRun 	= false;
static pthread_t hEventTaskThread	= 0;

void *DvrEventTaskThread( void *arg )
{
	int32_t i = 0;
	CMD_MESSAGE	cmd;

	memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
	cmd.cmdType = CMD_TYPE_EVENT;

	while( bEventTaskThreadRun )
	{
		for( i = 0; i < 100; i++)	// 100sec
		{
			usleep(1000000);	// 1sec
		}
		DvrCmdQueuePush( g_hCmd, &cmd );
	}

	return (void*)0xDEADDEAD;
}

static int32_t DvrEventTaskStart( void )
{
	if( bEventTaskThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	bEventTaskThreadRun = true;
	if( 0 > pthread_create( &hEventTaskThread, NULL, &DvrEventTaskThread, NULL) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}

	printf("%s(): Activation Test Event Task.\n", __func__);
	
	return 0;
}

static int32_t DvrEventTaskStop( void )
{
	if( !bTestEventTask )
		return 0;

	if( !bEventTaskThreadRun) {
		printf("%s(): Fail, Already stopping.\n", __func__);
		return -1;
	}
	
	bEventTaskThreadRun = false;
	pthread_join( hEventTaskThread, NULL );
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Signal Handler
//
static void MicomKillSignal( void )
{
	NX_GPIO_HANDLE hGPIOA18 = NX_GpioInit( GPIOA18 );
	if( hGPIOA18 ) NX_GpioDirection( hGPIOA18, GPIO_DIRECTION_OUT );
	if( hGPIOA18 ) NX_GpioSetValue( hGPIOA18, false );
	if( hGPIOA18 ) NX_GpioDeinit( hGPIOA18 );
}

static void ExitApp( void )
{
	DvrInputEventThreadStop();

	if( gstTestEvent ) DvrEventTaskStop();
	if( !gstNoConsole ) DvrShellThreadStop();

	DvrSDCheckerThreadStop();

	if( g_hDvr )	NX_DvrStop( g_hDvr );
	if( g_hDvr )	NX_DvrDeinit( g_hDvr );

	if( g_hFrontEffect )	NX_GTSclClose( g_hFrontEffect );
	if( g_hRearEffect )		NX_GTSclClose( g_hRearEffect );

	if( g_hGpsManager ) 	DvrGpsManagerStop( g_hGpsManager );
	if( g_hGpsManager ) 	DvrGpsManagerDeinit( g_hGpsManager );

	if( g_hGsensorManager ) DvrGsensorManagerStop( g_hGsensorManager );
	if( g_hGsensorManager ) DvrGsensorManagerDeinit( g_hGsensorManager );

	if( g_hPowerManager )	DvrPowerManagerStop( g_hPowerManager );
	if( g_hPowerManager )	DvrPowerManagerDeinit( g_hPowerManager );

	if( g_hNormalFileManager )	DvrFileManagerStop( g_hNormalFileManager );
	if( g_hEventFileManager )	DvrFileManagerStop( g_hEventFileManager );
	if( g_hCaptureFileManager )	DvrFileManagerStop( g_hCaptureFileManager );

	if( g_hNormalFileManager )	DvrFileManagerDeinit( g_hNormalFileManager );
	if( g_hEventFileManager )	DvrFileManagerDeinit( g_hEventFileManager );
	if( g_hCaptureFileManager )	DvrFileManagerDeinit( g_hCaptureFileManager );

	gstMsg = false;
	DvrCmdQueueDeinit( g_hCmd );
	pthread_mutex_destroy( &gstModeLock );
	
	if( g_hAudio )	NX_AudioStop( g_hAudio, true );
	if( g_hAudio )	NX_AudioDeinit( g_hAudio );

	printf("%s(): umount mmc.\n", __func__);
	int32_t pendCnt = 0;
	while( 1 )
	{
		pendCnt++;
		sync();
		usleep(100000);
		if( !umount("/mnt/mmc") ) break;
		if( pendCnt > 10) {
			printf("Fail, umount..\n");
			break;
		}
		printf("[%d] Retry umount..\n", pendCnt);
	}
}

static void signal_handler(int sig)
{
	printf("Aborted by signal %s (%d)...\n", (char*)strsignal(sig), sig);

	switch(sig)
	{   
		case SIGINT :
			printf("SIGINT..\n");   break;
		case SIGTERM :
			printf("SIGTERM..\n");  break;
		case SIGABRT :
			printf("SIGABRT..\n");  break;
		default :
			break;
	}   

	if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/stop.wav" );
	usleep(2000000);
	ExitApp();

	exit(EXIT_FAILURE);
}

static void register_signal(void)
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

////////////////////////////////////////////////////////////////////////////////
//
// Callback function
//
int32_t cbGetNormalFileName( uint8_t *buf, uint32_t bufSize )
{
	time_t eTime;
	struct tm *eTm;
	
	time( &eTime);
	eTm = localtime( &eTime );
	if( gstContainer == CONTAINER_TYPE_TS ) {
		sprintf((char*)buf, "%s/normal_%04d%02d%02d_%02d%02d%02d.ts", dir_normal,
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);
	}
	else {
		sprintf((char*)buf, "%s/normal_%04d%02d%02d_%02d%02d%02d.mp4", dir_normal,
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);
	}

	return 0;
}

int32_t cbGetEventFileName( uint8_t *buf, uint32_t bufSize )
{
	time_t eTime;
	struct tm *eTm;
	
	time( &eTime);
	eTm = localtime( &eTime );
	if( gstContainer == CONTAINER_TYPE_TS ) {
		sprintf((char*)buf, "%s/event_%04d%02d%02d_%02d%02d%02d.ts", dir_event,
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);
	}
	else {
		sprintf((char*)buf, "%s/event_%04d%02d%02d_%02d%02d%02d.mp4", dir_event,
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);
	}

	return 0;
}

int32_t cbGetCaptureFileName( uint8_t *buf, uint32_t bufSize )
{
	time_t eTime;
	struct tm *eTm;
	
	time( &eTime);
	eTm = localtime( &eTime );

	sprintf((char*)buf, "%s/capture_%04d%02d%02d_%02d%02d%02d.jpeg", dir_capture,
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);

	return 0;
}

int32_t cbGetFrontTextOverlay( uint8_t *buf, uint32_t *bufSize, uint32_t *x, uint32_t *y)
{
	struct nmea_gprmc	gpsData;
	GSENSOR_VALUE		gsensorData;
	int32_t				powerData = 0, tempData = 0;
	static uint64_t		frameCounter = 0;
	
	memset( &gpsData, 0x00, sizeof( struct nmea_gprmc ) );
	memset( &gsensorData, 0x00, sizeof( GSENSOR_VALUE) );

	if( g_hGpsManager )		DvrGpsGetData( g_hGpsManager, &gpsData );
	if( g_hGsensorManager )	DvrGsensorGetData( g_hGsensorManager, &gsensorData );
	if( g_hPowerManager )	DvrPowerGetData( g_hPowerManager, &powerData, &tempData );

	pthread_mutex_lock( &gstModeLock );
#ifndef DISPLAY_TICK
	time_t eTime;
	struct tm *eTm;
	time( &eTime);
	eTm = localtime( &eTime );

	sprintf((char*)buf, "[CAM#0 %s] [%08lld] [%4d-%02d-%02d %02d:%02d:%02d] [lat = %+010.06f, long = %+011.06f, speed = %03dKm, x = %+05dmg, y = %+05dmg, z = %+05dmg, pwr = %+05.02fV]",
		(gDvrMode == DVR_MODE_NORMAL) ? "NOR" : (gDvrMode == DVR_MODE_EVENT) ? "EVT" : "MOT",
		++frameCounter,
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec,
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z,
		(double)(powerData / 1000.)
	);
#else
	static uint64_t		prvTick = 0;
	if( !prvTick ) GetSystemTickCount();
	uint64_t curTick = GetSystemTickCount();
	
	sprintf((char*)buf, "[CAM#0 %s] [%08lld] [%08lld/%04lld] [lat = %+010.06f, long = %+011.06f, speed = %03dKm, x = %+05dmg, y = %+05dmg, z = %+05dmg, pwr = %+05.02fV]",
		(gDvrMode == DVR_MODE_NORMAL) ? "NOR" : (gDvrMode == DVR_MODE_EVENT) ? "EVT" : "MOT",
		++frameCounter, curTick, curTick - prvTick,
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z,
		(double)(powerData / 1000.)
	);
	prvTick = curTick;
#endif
	pthread_mutex_unlock( &gstModeLock );
	*bufSize = strlen((char*)buf);
	*x = *y = 1;

	return 0;
}

int32_t cbGetRearTextOverlay( uint8_t *buf, uint32_t *bufSize, uint32_t *x, uint32_t *y)
{
	struct nmea_gprmc	gpsData;
	GSENSOR_VALUE		gsensorData;
	int32_t				powerData = 0, tempData = 0;
	static uint64_t		frameCounter = 0;

	memset( &gpsData, 0x00, sizeof( struct nmea_gprmc ) );
	memset( &gsensorData, 0x00, sizeof( GSENSOR_VALUE) );

	if( g_hGpsManager ) 	DvrGpsGetData( g_hGpsManager, &gpsData );
	if( g_hGsensorManager )	DvrGsensorGetData( g_hGsensorManager, &gsensorData);
	if( g_hPowerManager )	DvrPowerGetData( g_hPowerManager, &powerData, &tempData );

	pthread_mutex_lock( &gstModeLock );
#ifndef DISPLAY_TICK	
	time_t eTime;
	struct tm *eTm;
	time( &eTime);
	eTm = localtime( &eTime );

	sprintf((char*)buf, "[CAM#1 %s] [%08lld] [%4d-%02d-%02d %02d:%02d:%02d] [lat = %+010.06f, long = %+011.06f, speed = %03dKm, x = %+05dmg, y = %+05dmg, z = %+05dmg, pwr = %+05.02fV]",
		(gDvrMode == DVR_MODE_NORMAL) ? "NOR" : (gDvrMode == DVR_MODE_EVENT) ? "EVT" : "MOT",
		++frameCounter,
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec,
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z,
		(double)(powerData / 1000.)
	);
#else
	static uint64_t		prvTick = 0;
	if( !prvTick ) GetSystemTickCount();
	uint64_t curTick = GetSystemTickCount();
	
	sprintf((char*)buf, "[CAM#1 %s] [%08lld] [%08lld/%04lld] [lat = %+010.06f, long = %+011.06f, speed = %03dKm, x = %+05dmg, y = %+05dmg, z = %+05dmg, pwr = %+05.02fV]",
		(gDvrMode == DVR_MODE_NORMAL) ? "NOR" : (gDvrMode == DVR_MODE_EVENT) ? "EVT" : "MOT",
		++frameCounter, curTick, curTick - prvTick,
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z,
		(double)(powerData / 1000.)
	);
	prvTick = curTick;
#endif	
	pthread_mutex_unlock( &gstModeLock );
	*bufSize = strlen((char*)buf);
	*x = *y = 1;

	return 0;
}

int32_t cbUserData( uint8_t *buf, uint32_t bufSize)
{
	struct nmea_gprmc gpsData;
	GSENSOR_VALUE gsensorData;

	memset( &gpsData, 0x00, sizeof( struct nmea_gprmc ) );
	memset( &gsensorData, 0x00, sizeof( GSENSOR_VALUE) );

	if( g_hGpsManager )		DvrGpsGetData( g_hGpsManager, &gpsData );
	if( g_hGsensorManager )	DvrGsensorGetData( g_hGsensorManager, &gsensorData );

	sprintf((char*)buf, "%+010.06f %+011.06f %03d %+05d %+05d %+05d",
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z
	);
	
	return strlen((char*)buf);
}

int32_t cbNotifier( uint32_t eventCode, uint8_t *pEventData, uint32_t dataSize )
{
	switch( eventCode )
	{
	case DVR_NOTIFY_NORALWIRTHING_DONE :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : Normal file writing done. ( %s )\n", __func__, pEventData);
			DvrFileManagerPush( g_hNormalFileManager, (char*)pEventData );
		}
		break;
	case DVR_NOTIFY_EVENTWRITING_DONE :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : Event file writing done. ( %s )\n", __func__, pEventData);
			pthread_mutex_lock( &gstModeLock );
			if( gDvrMode != DVR_MODE_MOTION ) {
				gDvrMode = DVR_MODE_NORMAL;
				pthread_mutex_unlock( &gstModeLock );
				DvrLedEventStop();
				if( g_hGsensorManager ) DvrGsensorManagerSetStatus( g_hGsensorManager, DVR_MODE_NORMAL );
			}
			else {
				pthread_mutex_unlock( &gstModeLock );
				DvrLedEventStop();
			}
			DvrFileManagerPush( g_hEventFileManager, (char*)pEventData );
		}
		break;
	case DVR_NOTIFY_JPEGWRITING_DONE :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : Jpeg file writing done. ( %s )\n", __func__, pEventData);
			DvrFileManagerPush( g_hCaptureFileManager, (char*)pEventData );
		}
		break;
	case DVR_NOTIFY_MOTION :
		DvrLedEventStart();
		if( g_hDvr )	NX_DvrEvent( g_hDvr );
		break;
	case DVR_NOTIFY_ERR_VIDEO_INPUT :
		break;
	case DVR_NOTIFY_ERR_VIDEO_ENCODE :
		break;
	case DVR_NOTIFY_ERR_OPEN_FAIL :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : File open failed. ( %s )\n", __func__, pEventData);
		}
		break;
	case DVR_NOTIFY_ERR_WRITE :
		if( pEventData && dataSize > 0 ) {	
			printf("[%s] : File writing failed. ( %s )\n", __func__, pEventData);
		}
		break;
	}
	
	return 0;
}

int32_t cbFrontImageEffect( NX_VID_MEMORY_INFO *pInBuf, NX_VID_MEMORY_INFO *pOutBuf )
{
	if( g_hFrontEffect ) NX_GTSclDoScale( g_hFrontEffect, pInBuf, pOutBuf);
	return 0;
}

int32_t cbRearImageEffect( NX_VID_MEMORY_INFO *pInBuf, NX_VID_MEMORY_INFO *pOutBuf )
{
	if( g_hRearEffect ) NX_GTSclDoScale( g_hRearEffect, pInBuf, pOutBuf);
	return 0;
}

#ifdef EXTERNAL_MOTION_DETECTION
int32_t cbMotionDetect( NX_VID_MEMORY_INFO *pPrevMemory, NX_VID_MEMORY_INFO *pCurMemory )
{
	int32_t i, j;

	unsigned char *pPrevAddr	= (unsigned char*)pPrevMemory->luVirAddr;
	unsigned char *pCurAddr		= (unsigned char*)pCurMemory->luVirAddr;

	unsigned char prevValue	= 0;
	unsigned char curValue	= 0;

	unsigned char diffValue	= 0;
	
	int32_t		diffCnt		= 0;

	for( i = 0; i < pPrevMemory->imgHeight; i += 16 )
	{
		for( j = 0; j < pPrevMemory->imgWidth; j += 16 )
		{
			prevValue = *(pPrevAddr + j + (pPrevMemory->luStride * i));
			curValue  = *(pCurAddr  + j + (pCurMemory->luStride  * i));
			
			if(prevValue > curValue )
				diffValue = prevValue - curValue;
			else
				diffValue = curValue - prevValue;

			if( diffValue > 100 ) diffCnt++;
		}
	}

	if( diffCnt > 100 )	{
		return true;
	}
	
	return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// debug shell
//
#define	SHELL_MAX_ARG	32
#define	SHELL_MAX_STR	1024

static int32_t bDvrShellThreadRun = false;
static pthread_t hDvrShellThread = 0;

static int32_t GetArgument( char *pSrc, char arg[][SHELL_MAX_STR] )
{
	int32_t i, j;

	// Reset all arguments
	for( i = 0; i < SHELL_MAX_ARG; i++ )
	{
		arg[i][0] = 0x00;
	}

	for( i = 0; i < SHELL_MAX_ARG; i++ )
	{
		// Remove space char
		while( *pSrc == ' ' )
			pSrc++;
		
		// check end of string.
		if( *pSrc == 0 || *pSrc == '\n' )
			break;

		j = 0;
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

void shell_usage( void )
{
	printf("\n");
	printf("+----------------------------------------------------------------+\n");
	printf("|                 Nexell DVR Test Application                    |\n");
	printf("+----------------------------------------------------------------+\n");
	printf("| help                        : help                             |\n");
	printf("| event                       : occur event                      |\n");
	printf("| capture [channel]           : jpeg capture                     |\n");
	printf("| dsp [enable]                : display enable                   |\n");
	printf("| crop [L] [T] [L] [B]        : source image display crop        |\n");
	printf("| pos [L] [T] [L] [B]         : change display position          |\n");
	printf("| lcd [channel]               : lcd preview channel              |\n");
	printf("| hdmi [channel]              : hdmi preview channel             |\n");
	printf("| dbg [debug_level]           : change debug level( 0 - 5)       |\n");
	printf("| exit                        : exit application                 |\n");
	printf("+----------------------------------------------------------------+\n");
}

void *DvrShellThread( void *arg )
{
	static char cmdString[SHELL_MAX_ARG * SHELL_MAX_STR];
	static char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	int32_t cmdCnt;

    fd_set readfds;
    int32_t state;
	struct timeval tv;
	
	CMD_MESSAGE cmdMsg; 

	shell_usage();

	printf(" > ");

	while( bDvrShellThreadRun )
	{
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		state = select( STDIN_FILENO + 1, &readfds, NULL, NULL, &tv );

		if( state < 0 )
		{
			printf("%s(): Select Error!!!\n", __func__);
			break;
		}
		else if( state == 0 )
		{
		}
		else
		{
			printf(" > ");

			memset( cmdString, 0x00, sizeof( cmdString ) );
			fgets(cmdString, sizeof(cmdString), stdin);
			if( strlen(cmdString) != 0 ) cmdString[strlen(cmdString) - 1] = 0x00;

			cmdCnt = GetArgument( cmdString, cmd );

			if( cmdCnt == 0 )
				continue;

			if( !strcasecmp( cmd[0], "exit") ) {
				printf("Exit.\n");
				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/stop.wav" );
				usleep(2000000);
				ExitApp();
				break;
			}
			else if( !strcasecmp( cmd[0], "help" ) ) {
				shell_usage();
			}
			else if( !strcasecmp( cmd[0], "event") ) {
				printf("Event.\n");
				memset( &cmdMsg, 0x00, sizeof(CMD_MESSAGE) );
				cmdMsg.cmdType = CMD_TYPE_EVENT;
				DvrCmdQueuePush( g_hCmd, &cmdMsg );
			}
			else if( !strcasecmp( cmd[0], "capture") ) {
				if( !strcasecmp( cmd[1], "0" ) || !strcasecmp( cmd[1], "1" ) ) {
					int32_t ch = atoi(cmd[1]);
					printf("Capture. (ch = %d)\n", ch );
					cmdMsg.cmdType = CMD_TYPE_CAPTURE;
					cmdMsg.cmdData = ch;
					DvrCmdQueuePush( g_hCmd, &cmdMsg );
				}
				else {
					printf("unknown argument. ( 0 or 1 )\n");
				}
			}
			else if( !strcasecmp( cmd[0], "dsp") ) {
				if( !strcasecmp( cmd[1], "0" ) || !strcasecmp( cmd[1], "1" ) ) {
					NX_DVR_DISPLAY_CONFIG dspConf;
					memset( &dspConf, 0x00, sizeof(dspConf) );
					dspConf.bEnable	= atoi(cmd[1]);

					NX_DvrSetDisplay( g_hDvr, &dspConf );
				}
				else {
					printf("unknown argument. ( 0 or 1 )\n");	
				}
			}
			else if( !strcasecmp( cmd[0], "crop") ) {
				if( cmdCnt == 5 ) {
					NX_DVR_DISPLAY_CONFIG dspConf;
					memset( &dspConf, 0x00, sizeof(dspConf) );
					dspConf.bEnable				= true;
					dspConf.cropRect.nLeft 		= atoi(cmd[1]);
					dspConf.cropRect.nTop		= atoi(cmd[2]);
					dspConf.cropRect.nRight 	= atoi(cmd[3]);
					dspConf.cropRect.nBottom	= atoi(cmd[4]);

					NX_DvrSetDisplay( g_hDvr, &dspConf );
				}
				else {
					printf("unknwon argument.\n");
				}
			}
			else if( !strcasecmp( cmd[0], "pos") ) {
				if( cmdCnt == 5 ) {
					NX_DVR_DISPLAY_CONFIG dspConf;
					memset( &dspConf, 0x00, sizeof(dspConf) );
					dspConf.bEnable				= true;
					dspConf.dspRect.nLeft 		= atoi(cmd[1]);
					dspConf.dspRect.nTop		= atoi(cmd[2]);
					dspConf.dspRect.nRight 		= atoi(cmd[3]);
					dspConf.dspRect.nBottom		= atoi(cmd[4]);

					NX_DvrSetDisplay( g_hDvr, &dspConf );
				}
				else {
					printf("unknwon argument.\n");
				}
			}
			else if( !strcasecmp( cmd[0], "lcd") ) {
				if( !strcasecmp( cmd[1], "0" ) || !strcasecmp( cmd[1], "1" ) ) {
					int32_t ch = atoi(cmd[1]);
					printf("LCD Preview. (ch = %d)\n", ch );
					cmdMsg.cmdType = CMD_TYPE_CHG_LCD;
					cmdMsg.cmdData = ch;
					DvrCmdQueuePush( g_hCmd, &cmdMsg );
				}
				else {
					printf("unknown argument. ( 0 or 1 )\n");
				}				
			}
			else if( !strcasecmp( cmd[0], "hdmi") ) {
				if( !strcasecmp( cmd[1], "0" ) || !strcasecmp( cmd[1], "1" ) ) {
					int32_t ch = atoi(cmd[1]);
					printf("HDMI Preview. (ch = %d)\n", ch );
					cmdMsg.cmdType = CMD_TYPE_CHG_HDMI;
					cmdMsg.cmdData = ch;
					DvrCmdQueuePush( g_hCmd, &cmdMsg );
				}
				else {
					printf("unknown argument. ( 0 or 1 )\n");
				}				
			}
			else if( !strcasecmp( cmd[0], "dbg") ) {
				if( !strcasecmp( cmd[1], "0" ) || 
					!strcasecmp( cmd[1], "1" ) ||
					!strcasecmp( cmd[1], "2" ) ||
					!strcasecmp( cmd[1], "3" ) ||
					!strcasecmp( cmd[1], "4" ) ||
					!strcasecmp( cmd[1], "5" ) ) {
					int32_t level = atoi(cmd[1]);

					printf("Change Debug Level. ( %s )\n", 
						level == 0 ? "NX_DBG_DISABLE" :
						level == 1 ? "NX_DBG_ERR" :
						level == 2 ? "NX_DBG_WARN" :
						level == 3 ? "NX_DBG_INFO" :
						level == 4 ? "NX_DBG_DEBUG" : "NX_DBG_VBS");
					
					NX_DvrChgDebugLevel( g_hDvr, level );
				}
				else {
					printf("unknown argument. (0 - 5)\n");
				}
			}			
			else {
				printf("unknown command. (%s)\n", cmdString );
			}
		}
	}

	FD_CLR( STDIN_FILENO, &readfds );
	return (void*)0xDEADDEAD;
}

static int32_t DvrShellThreadStart( void )
{
	bDvrShellThreadRun = true;
	if( 0 > pthread_create( &hDvrShellThread, NULL, &DvrShellThread, NULL) )
	{
		printf("%s(: Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

static int32_t DvrShellThreadStop( void )
{
	bDvrShellThreadRun = false;
	fputs("exit", stdin);
	pthread_join( hDvrShellThread, NULL );

	return 0;
}

static void Usage( void )
{
	printf(" Nexell Blackbox Encoding Test Application.\n\n");
	printf(" Usage :\n");
	printf("   -h                     : Show usage.\n");
	printf("   -b                     : No command line.\n");
	printf("   -a                     : Audio enable.\n");
	printf("   -e                     : Test event task. ( 100sec cycle )\n");
	printf("   -u                     : Userdata enable.\n");
	printf("   -m                     : Motion detection enable.\n");
	printf("   -n [network type]      : Network streaming enable.( 1:HLS or 2:RTP )\n");
	printf("   -t [stoage positon]    : Storage Position.\n");
	printf("   -c [encoding channel]  : Encoding Channel.( 1 or 2 )\n");

	printf("\n");
	printf(" Default Setting :\n");
	printf("   stoage positon         : /mnt/mmc\n");
	printf("   video channel          : 1 channel.\n");
	printf("   audio status           : disable.\n");
	printf("   userdata status        : disable.\n");
	printf("   motion detection       : disable.\n");
	printf("   network streaming      : disable.\n");


	printf("\n");
}

////////////////////////////////////////////////////////////
//
// SD Card Checker
//
static int32_t bSDCheckerThreadRun	= false;
static pthread_t hSDCheckerThread	= 0;

void *DvrSDCheckerThread( void *arg )
{
	CMD_MESSAGE	cmd;

	char buf[4096], sdNodekeyword[64];
	int32_t len, socketFd = 0;
	struct sockaddr_nl sockAddr;
	struct iovec	iov	= { buf, sizeof(buf) };
	struct msghdr	msg	= { &sockAddr, sizeof(sockAddr), &iov, 1, NULL, 0, 0 };

    fd_set readfds;
    int32_t state;
	struct timeval tv;

	memset( &sockAddr, 0, sizeof(sockAddr) );
	sockAddr.nl_family = AF_NETLINK;
	sockAddr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

	socketFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);

	if( socketFd < 0 ) {
		printf("%s(): socket error.\n", __func__);
		goto ERROR;
	}
	if( bind(socketFd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) ) {
		printf("%s(): bind error.\n", __func__);
		close( socketFd );
		goto ERROR;
	}

	// first node checker
	sprintf( sdNodekeyword, "%sp1", MMCBLOCK );
	sprintf( (char*)gstSdNode, "/dev/%s", sdNodekeyword );
	if( !access( (char*)gstSdNode, F_OK) )
	{
		printf("%s(): Insert SD Card. ( node : %s)\n", __func__, gstSdNode);
		memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
		cmd.cmdType = CMD_TYPE_SD_INSERT;
		if( g_hCmd )	DvrCmdQueuePush( g_hCmd, &cmd );
	}

	while( bSDCheckerThreadRun )
	{
		FD_ZERO(&readfds);
		FD_SET(socketFd, &readfds);
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		state = select( socketFd+1, &readfds, NULL, NULL, &tv );

		if( state < 0 )
		{
			printf("%s(): Select Error!!!\n", __func__);
			break;
		}
		else if( state == 0 )
		{
			//printf("%s(): System Message Wait Timeout\n", __func__);
		}
		else
		{
			memset( buf, 0x00, sizeof(buf) );
			len = recvmsg( socketFd, &msg, 0 );
			if( len < 0 ) {
				printf("%s(): recvmsg error.\n", __func__);
				break;
			}

			if( strstr(buf, "add@") && strstr(buf, sdNodekeyword) )
			{
				printf("%s(): Insert SD Card. ( node : %s)\n", __func__, gstSdNode);
				memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
				cmd.cmdType = CMD_TYPE_SD_INSERT;
				if( g_hCmd )	DvrCmdQueuePush( g_hCmd, &cmd );
			}

			if( strstr(buf, "remove@") && strstr(buf, "mmcblk0p1") )
			{
				printf("%s(): Remove SD Card. ( node : %s)\n", __func__, gstSdNode);
				memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
				cmd.cmdType = CMD_TYPE_SD_REMOVE;
				if( g_hCmd )	DvrCmdQueuePush( g_hCmd, &cmd );
			}
		}
	}
	
	FD_CLR(socketFd, &readfds);
	close( socketFd );

ERROR:
	bSDCheckerThreadRun = false;
	return (void*)0xDEADDEAD;
}

static int32_t DvrSDCheckerThreadStart( void )
{
	if( bSDCheckerThreadRun )
	{
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	bSDCheckerThreadRun = true;
	if( 0 > pthread_create( &hSDCheckerThread, NULL, &DvrSDCheckerThread, NULL) )
	{
		printf("%s(: Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

static int32_t DvrSDCheckerThreadStop( void )
{
	if( !bSDCheckerThreadRun )
	{
		printf("%s(): Fail, Already stopppng.\n", __func__);
		return -1;
	}

	bSDCheckerThreadRun = false;

	pthread_join( hSDCheckerThread, NULL );
	return 0;
}

////////////////////////////////////////////////////////////
//
// Input Event
//
#include <linux/input.h>

#define EVENT_BUF_NUM 	1
#define	EVENT_DEV_NAME	"/dev/input/event0"

static int32_t bInputEventThreadRun	= false;
static pthread_t hInputEventThread	= 0;

#define KEY_CAPTURE		234

static int parse_ev_key(struct input_event * evt)
{
	uint16_t code = evt->code;
	uint32_t val  = evt->value;

	CMD_MESSAGE cmd;
	//printf("KEY  	=%03d, val=%s\n", code, val?"PRESS":"RELEASE");

	if( (code == KEY_CAPTURE) && !val)
	{
		memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
		cmd.cmdType = CMD_TYPE_CAPTURE;
		DvrCmdQueuePush( g_hCmd, &cmd );
	}

	return 0;
}

void *DvrInputEventThread( void* arg )
{
	int    evt_fd;
    char   evt_dev[256] = EVENT_DEV_NAME;

    struct input_event evt_buf;
    size_t length;

    fd_set fdset;
	struct timeval zero;

	/* event wait time */
   	zero.tv_sec  = 1000;
   	zero.tv_usec = 0;

	evt_fd = open(evt_dev, O_RDONLY);
    if (0 > evt_fd) {
		printf("%s: device open failed!\n", __func__);
        goto ERROR;
    }

    while( bInputEventThreadRun )
    {
		FD_ZERO(&fdset);
   		FD_SET(evt_fd, &fdset);

   		zero.tv_sec  = 2;
        if (select(evt_fd + 1, &fdset, NULL, NULL, &zero) > 0) {

            if (FD_ISSET(evt_fd, &fdset)) {

                length = read(evt_fd, &evt_buf, sizeof(struct input_event) );
                if (0 > (int)length) {
                    printf("%s(): read failed.\n", __func__);
                    close(evt_fd);
                    goto ERROR;
                }

				parse_ev_key( &evt_buf );
            }
        }
		else
		{
		}
    }

   	FD_CLR(evt_fd, &fdset);

    close(evt_fd);

ERROR:
    return (void*)0xDEADDEAD;
}

static int32_t DvrInputEventThreadStart( void )
{
	if( bInputEventThreadRun )
	{
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}

	bInputEventThreadRun = true;
	if( 0 > pthread_create( &hInputEventThread, NULL, &DvrInputEventThread, NULL) )
	{
		printf("%s(: Fail, Create Thread.\n", __func__);
		return -1;
	}

	return 0;
}

static int32_t DvrInputEventThreadStop( void )
{
	if( !bInputEventThreadRun )
	{
		printf("%s(): Fail, Already stopppng.\n", __func__);
		return -1;
	}

	bInputEventThreadRun = false;

	pthread_join( hInputEventThread, NULL );
	return 0;
}

void DvrPrintConfig( NX_DVR_MEDIA_CONFIG *pMediaConfig )
{
	//      0         1         2         3         4         5         6         7         8       
	//      0123456789012345678901234567890123456789012345678901234567890123456789012345678901
	printf("##################################################################################\n");
	printf(" Confiugration value\n");
	printf("   API Version   : %d.%d.%d\n", gstApiMajor, gstApiMinor, gstApiRevision);
	printf("   Storage       : %s\n", dir_top );
	printf("   Video Channel : %d channel\n", gstVideoChannel);
	printf("   Audio         : %s\n", gstAudioEnable ? "enable" : "disable");
	printf("   UserData      : %s\n", gstUserDataEnable ? "enable" : "disable" );
	printf("   Network       : %s\n", (gstNetworkType == DVR_NETWORK_HLS) ? "HLS enable" : (gstNetworkType == DVR_NETWORK_RTP) ? "RTP enable" : "disable" );
	printf("   Motion Detect : %s\n", gstMotionEnable ? "enable" : "disable" );
	
	{
#if(0)		
		printf("   Cam0 Resol    : %d x %d, %dfps, %dbps\n",
			pMediaConfig->videoConfig[0].nSrcWidth,
			pMediaConfig->videoConfig[0].nSrcHeight,
			pMediaConfig->videoConfig[0].nFps,
			pMediaConfig->videoConfig[0].nBitrate );
#else
		printf("   \033[1;37;41mCam0 Resol    : %d x %d, %dfps, %dbps\033[0m\n",
			pMediaConfig->videoConfig[0].nSrcWidth,
			pMediaConfig->videoConfig[0].nSrcHeight,
			pMediaConfig->videoConfig[0].nFps,
			pMediaConfig->videoConfig[0].nBitrate );
#endif		
	}
	
	if( pMediaConfig->nVideoChannel == 2 ) {
#if(0)		
		printf("   Cam1 Resol    : %d x %d, %dfps, %dbps\n",
			pMediaConfig->videoConfig[1].nSrcWidth,
			pMediaConfig->videoConfig[1].nSrcHeight,
			pMediaConfig->videoConfig[1].nFps,
			pMediaConfig->videoConfig[1].nBitrate );
#else
		printf("   \033[1;37;41mCam1 Resol    : %d x %d, %dfps, %dbps\033[0m\n",
			pMediaConfig->videoConfig[1].nSrcWidth,
			pMediaConfig->videoConfig[1].nSrcHeight,
			pMediaConfig->videoConfig[1].nFps,
			pMediaConfig->videoConfig[1].nBitrate );
#endif		
	}

	printf("##################################################################################\n");
}

int main( int32_t argc, char *argv[] )
{
	int32_t opt;
	uint64_t interval;

	NX_DVR_MEDIA_CONFIG		mediaConfig;
	NX_DVR_RECORD_CONFIG	recordConfig;
	NX_DVR_DISPLAY_CONFIG	displayConfig;

	CMD_MESSAGE cmd; 

	NX_DspVideoSetPriority( DISPLAY_MODULE_MLC0, 0 );

	// Core Dump Debug
	//echo "1" > /proc/sys/kernel/core_uses_pid;echo "/mnt/mmc/core.%e" > /proc/sys/kernel/core_pattern;ulimit -c 99999999
	system("echo \"1\" > /proc/sys/kernel/core_uses_pid");
	system("echo \"/mnt/mmc/core.%e\" > /proc/sys/kernel/core_pattern");
	system("ulimit -c 99999999");

	pthread_mutex_init( &gstModeLock, NULL );
	
	memset( dir_top, 0x00, sizeof(dir_top) );
	sprintf( (char*)dir_top, "%s", DIRECTORY_TOP );

	while( -1 != (opt = getopt(argc, argv, "umhn:beat:c:"))  )
	{
		switch(opt)
		{
		case 'h':
			Usage();
			goto END;
		case 'b':
			gstNoConsole = true;
			break;
		case 'a':
			gstAudioEnable = true;
			break;
		case 'n':
			if( atoi( optarg ) == 1 )
				gstNetworkType = DVR_NETWORK_HLS;
			else if( atoi( optarg ) == 2 )
				gstNetworkType = DVR_NETWORK_RTP;
			else {
				printf("Incorrect argument. (1 or 2)\n");
				goto END;
			}
			break;
		case 'e':
			gstTestEvent = true;
			break;
		case 'u':
			gstUserDataEnable = true;
			break;
		case 't':
			memset( dir_top, 0x00, sizeof(dir_top) );
			sprintf( (char*)dir_top, "%s", optarg );
			break;
		case 'm':
			gstMotionEnable = true;
			break;
		case 'c':
			gstVideoChannel = atoi( optarg );
			if( gstVideoChannel != 1 && gstVideoChannel != 2 )
			{
				printf("Incorrect argument. (1 or 2)\n");
				goto END;
			}	
		}
	}

	mediaConfig.nVideoChannel	= gstVideoChannel;
	mediaConfig.bEnableAudio	= gstAudioEnable;
	mediaConfig.bEnableUserData = gstUserDataEnable;
	if( gstContainer == CONTAINER_TYPE_TS ) {
		mediaConfig.nContainer		= DVR_CONTAINER_TS;	
	}
	else {
		mediaConfig.nContainer		= DVR_CONTAINER_MP4;
	}


#ifndef BOARD_TYPE_LYNX
	mediaConfig.videoConfig[0].nPort 		= DVR_CAMERA_VIP1;
	mediaConfig.videoConfig[0].nSrcWidth	= 1280;
	mediaConfig.videoConfig[0].nSrcHeight	= 720;
	mediaConfig.videoConfig[0].nFps			= 30;
#else
#ifdef CAMERA_TYPE_FHD
	mediaConfig.videoConfig[0].nPort 		= DVR_CAMERA_MIPI;
	mediaConfig.videoConfig[0].nSrcWidth	= 1920;
	mediaConfig.videoConfig[0].nSrcHeight	= 1080;
	mediaConfig.videoConfig[0].nFps			= 30;
#else
	mediaConfig.videoConfig[0].nPort 		= DVR_CAMERA_MIPI;
	mediaConfig.videoConfig[0].nSrcWidth	= 1024;
	mediaConfig.videoConfig[0].nSrcHeight	= 768;
	mediaConfig.videoConfig[0].nFps			= 15;
#endif	
#endif
	mediaConfig.videoConfig[0].bExternProc	= false;
	mediaConfig.videoConfig[0].nDstWidth	= !mediaConfig.videoConfig[0].bExternProc ? mediaConfig.videoConfig[0].nSrcWidth : 1920;
	mediaConfig.videoConfig[0].nDstHeight	= !mediaConfig.videoConfig[0].bExternProc ? mediaConfig.videoConfig[0].nSrcHeight : 1080;

#ifdef CAMERA_TYPE_FHD
	mediaConfig.videoConfig[0].nBitrate		= 10000000;
#else
	mediaConfig.videoConfig[0].nBitrate		= 5000000;	// 3M( ), 7M(middle), 12M(MAX)
#endif

	mediaConfig.videoConfig[0].nCodec		= DVR_CODEC_H264;
	
#ifndef BOARD_TYPE_LYNX
	mediaConfig.videoConfig[1].nPort		= DVR_CAMERA_VIP0;
	mediaConfig.videoConfig[1].nSrcWidth	= 1280;
	mediaConfig.videoConfig[1].nSrcHeight	= 720;
	mediaConfig.videoConfig[1].nFps			= 30;
#else
	mediaConfig.videoConfig[1].nPort		= DVR_CAMERA_VIP0;
	mediaConfig.videoConfig[1].nSrcWidth	= 640;
	mediaConfig.videoConfig[1].nSrcHeight	= 480;
	mediaConfig.videoConfig[1].nFps			= 15;
#endif	
	mediaConfig.videoConfig[1].bExternProc	= false;
	mediaConfig.videoConfig[1].nDstWidth	= !mediaConfig.videoConfig[1].bExternProc ? mediaConfig.videoConfig[1].nSrcWidth : 720;
	mediaConfig.videoConfig[1].nDstHeight	= !mediaConfig.videoConfig[1].bExternProc ? mediaConfig.videoConfig[1].nSrcHeight : 480;
	mediaConfig.videoConfig[1].nBitrate		= 5000000;
	mediaConfig.videoConfig[1].nCodec		= DVR_CODEC_H264;
	
	mediaConfig.textConfig.nBitrate			= 3000000;
	mediaConfig.textConfig.nInterval		= 200;

	mediaConfig.audioConfig.nChannel		= 2;
	mediaConfig.audioConfig.nFrequency		= 48000;
	mediaConfig.audioConfig.nBitrate		= 128000;
#ifdef AUDIO_TYPE_AAC
	mediaConfig.audioConfig.nCodec			= DVR_CODEC_AAC;
#else
	mediaConfig.audioConfig.nCodec			= DVR_CODEC_MP3;
#endif

	recordConfig.nNormalDuration			= 20000;
	recordConfig.nEventDuration				= 10000;
	recordConfig.nEventBufferDuration		= 10000;

	recordConfig.networkType				= gstNetworkType;

	// HLS Configuration
	recordConfig.hlsConfig.nSegmentDuration = 10;
	recordConfig.hlsConfig.nSegmentNumber	= 3;
	sprintf( (char*)recordConfig.hlsConfig.MetaFileName,	"test.m3u8" );
	sprintf( (char*)recordConfig.hlsConfig.SegmentFileName,	"segment" );
	sprintf( (char*)recordConfig.hlsConfig.SegmentRootDir,	"/www" );

	// RTP Configuration
	recordConfig.rtpConfig.nPort			= 554;
	recordConfig.rtpConfig.nSessionNum		= gstVideoChannel;
	recordConfig.rtpConfig.nConnectNum		= 2;
	sprintf( (char*)recordConfig.rtpConfig.sessionName[0],	"video0" );
	sprintf( (char*)recordConfig.rtpConfig.sessionName[1],	"video1" );

	recordConfig.bMdEnable[0]				= gstMotionEnable;
	recordConfig.mdConfig[0].nMdThreshold	= 100;
	recordConfig.mdConfig[0].nMdSensitivity	= 100;
	recordConfig.mdConfig[0].nMdSampingFrame= 1;

	recordConfig.bMdEnable[1]				= false;
	recordConfig.mdConfig[1].nMdThreshold	= 100;
	recordConfig.mdConfig[1].nMdSensitivity	= 100;
	recordConfig.mdConfig[1].nMdSampingFrame= 1;

#ifndef BOARD_TYPE_LYNX
	displayConfig.bEnable			= false;
	displayConfig.nChannel			= 0;
	displayConfig.nModule			= 0;
	displayConfig.cropRect.nLeft	= 0;
	displayConfig.cropRect.nTop		= 0;
	displayConfig.cropRect.nRight	= 640;
	displayConfig.cropRect.nBottom	= 480;
	displayConfig.dspRect.nLeft		= 0;
	displayConfig.dspRect.nTop		= 0;
	displayConfig.dspRect.nRight	= 640;
	displayConfig.dspRect.nBottom	= 480;
#else
	displayConfig.bEnable			= true;
	displayConfig.nChannel			= gstPreviewChannel;
	displayConfig.nModule			= 0;
	displayConfig.cropRect.nLeft	= 0;
	displayConfig.cropRect.nTop		= 0;
	displayConfig.cropRect.nRight	= 640;
	displayConfig.cropRect.nBottom	= 480;
	displayConfig.dspRect.nLeft		= 0;
	displayConfig.dspRect.nTop		= 0;
	displayConfig.dspRect.nRight	= 640;
	displayConfig.dspRect.nBottom	= 480;
#endif

#ifndef DISABLE_AUDIO
	g_hAudio	= NX_AudioInit();	
#endif
	g_hCmd 		= DvrCmdQueueInit();

	printf("############################## STARTING APPLICATION ##############################\n");
#ifdef DEMO
	printf("\033[1;37;41mBlackbox Demo Mode\033[0m\n");
#endif
	printf(" Build Infomation\n");
	printf("   Build Time : %s, %s\n", __TIME__, __DATE__);
	printf("   Author     : Sung-won Jo\n");
	printf("   Mail       : doriya@nexell.co.kr\n");
	printf("   COPYRIGHT@2013 NEXELL CO. ALL RIGHT RESERVED.\n");
	printf("##################################################################################\n");

	register_signal( );

	DvrSDCheckerThreadStart();

	gstMsg = true;
	while( gstMsg )
	{
		memset( &cmd, 0x00, sizeof(CMD_MESSAGE) );
		if( 0 > DvrCmdQueuePop( g_hCmd, &cmd) )
			continue;

		switch( cmd.cmdType )
		{
			case CMD_TYPE_SD_INSERT :
				usleep(100000);
				
				if( !mount( (char*)gstSdNode, DIRECTORY_TOP, "vfat", 0, "") ) {
					printf("%s(): mount mmc. (mount pos: %s)\n", __func__, DIRECTORY_TOP);
				}
				else {
					printf("%s(): mount failed.", __func__);
				}

#ifdef DEMO
				if( !access("/mnt/mmc/network.txt", F_OK) ) {
					gstVideoChannel		= 1;
					gstAudioEnable		= true;
					gstUserDataEnable	= false;
					gstNetworkType		= DVR_NETWORK_HLS;

					mediaConfig.nVideoChannel	= gstVideoChannel;
					mediaConfig.bEnableAudio	= gstAudioEnable;
					mediaConfig.bEnableUserData = gstUserDataEnable;
					recordConfig.networkType	= gstNetworkType;

					mediaConfig.videoConfig[0].nBitrate	= 4000000;
					mediaConfig.videoConfig[1].nBitrate	= 4000000;
				}
				else {
					gstVideoChannel		= 2;
					gstAudioEnable		= true;
					gstUserDataEnable	= true;
					gstNetworkType		= false;

					mediaConfig.nVideoChannel	= gstVideoChannel;
					mediaConfig.bEnableAudio	= gstAudioEnable;
					mediaConfig.bEnableUserData = gstUserDataEnable;
					recordConfig.networkType	= gstNetworkType;					

					mediaConfig.videoConfig[0].nBitrate	= 7000000;
					mediaConfig.videoConfig[1].nBitrate	= 7000000;
				}
#endif

				g_hDvr = NX_DvrInit( &mediaConfig, &recordConfig, &displayConfig );
				if( g_hDvr )	NX_DvrGetAPIVersion( g_hDvr, &gstApiMajor, &gstApiMinor, &gstApiRevision );
				
				DvrPrintConfig( &mediaConfig );

				memset( dir_normal, 0x00, sizeof(dir_normal) );
				memset( dir_event, 0x00, sizeof(dir_event) );
				memset( dir_capture, 0x00, sizeof(dir_capture) );

				sprintf( (char*)dir_normal,	"%s/normal", dir_top);
				sprintf( (char*)dir_event, "%s/event", dir_top);
				sprintf( (char*)dir_capture, "%s/capture", dir_top);

				g_hNormalFileManager	= DvrFileManagerInit( (const char*)dir_normal, 50, gstContainer == CONTAINER_TYPE_TS ? "ts" : "mp4" );
				g_hEventFileManager		= DvrFileManagerInit( (const char*)dir_event, 20, gstContainer == CONTAINER_TYPE_TS ? "ts" : "mp4" );
				g_hCaptureFileManager	= DvrFileManagerInit( (const char*)dir_capture, 1, "jpeg" );

				g_hGpsManager			= DvrGpsManagerInit();
#ifndef BOARD_TYPE_LYNX
				g_hGsensorManager 		= DvrGsensorManagerInit();
				g_hPowerManager			= DvrPowerManagerInit();
#endif

				if( g_hPowerManager )	DvrPowerManagerRegCmd( g_hPowerManager, g_hCmd );
				if( g_hGsensorManager )	DvrGsensorManagerRegCmd( g_hGsensorManager, g_hCmd );
				if( g_hGsensorManager )	DvrGsensorManagerMotionEnable( g_hGsensorManager, gstMotionEnable );

				if( g_hNormalFileManager )	DvrFileManagerStart( g_hNormalFileManager );
				if( g_hEventFileManager )	DvrFileManagerStart( g_hEventFileManager );
				if( g_hCaptureFileManager )	DvrFileManagerStart( g_hCaptureFileManager );
	
				if( mediaConfig.videoConfig[0].bExternProc )
					g_hFrontEffect		= NX_GTSclOpen( mediaConfig.videoConfig[0].nSrcWidth, mediaConfig.videoConfig[0].nSrcHeight, mediaConfig.videoConfig[0].nDstWidth, mediaConfig.videoConfig[0].nDstHeight, 12 );
				if( mediaConfig.videoConfig[1].bExternProc )
					g_hRearEffect		= NX_GTSclOpen( mediaConfig.videoConfig[1].nSrcWidth, mediaConfig.videoConfig[1].nSrcHeight, mediaConfig.videoConfig[1].nDstWidth, mediaConfig.videoConfig[1].nDstHeight, 12 );

				if( g_hDvr )	NX_DvrRegisterFileNameCallback( g_hDvr, cbGetNormalFileName, cbGetEventFileName, NULL , cbGetCaptureFileName );
				if( g_hDvr )	NX_DvrRegisterTextOverlayCallback( g_hDvr, cbGetFrontTextOverlay, cbGetRearTextOverlay );
				if( g_hDvr )	NX_DvrRegisterUserDataCallback( g_hDvr, cbUserData );
				if( g_hDvr )	NX_DvrRegisterNotifyCallback( g_hDvr, cbNotifier );
				if( g_hDvr )	NX_DvrRegisterImageEffectCallback( g_hDvr, cbFrontImageEffect, cbRearImageEffect );
#ifdef EXTERNAL_MOTION_DETECTION
				if( g_hDvr )	NX_DvrRegisterMotionDetectCallback( g_hDvr, cbMotionDetect, cbMotionDetect );
#endif
				NX_DvrStart( g_hDvr, (gDvrMode == DVR_MODE_NORMAL) ? DVR_ENCODE_NORMAL : DVR_ENCODE_MOTION );
				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/start.wav");

				if( g_hGpsManager )		DvrGpsManagerStart( g_hGpsManager );
				if( g_hGsensorManager )	DvrGsensorManagerStart( g_hGsensorManager );
				if( g_hPowerManager )	DvrPowerManagerStart( g_hPowerManager );

				if( !gstNoConsole )	DvrShellThreadStart();
				if( gstTestEvent)	DvrEventTaskStart();

				DvrInputEventThreadStart();
				break;

			case CMD_TYPE_SD_REMOVE :
				DvrInputEventThreadStop();

				if( gstTestEvent) DvrEventTaskStop();
				if( !gstNoConsole ) DvrShellThreadStop();

				if( g_hDvr ) NX_DvrStop( g_hDvr );
				if( g_hDvr ) NX_DvrDeinit( g_hDvr );

				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/stop.wav" );
				usleep(2000000);
				if( g_hAudio )	NX_AudioStop( g_hAudio, true );

				if( g_hGpsManager ) 	DvrGpsManagerStop( g_hGpsManager );
				if( g_hGpsManager ) 	DvrGpsManagerDeinit( g_hGpsManager );

				if( g_hGsensorManager ) DvrGsensorManagerStop( g_hGsensorManager );
				if( g_hGsensorManager ) DvrGsensorManagerDeinit( g_hGsensorManager );

				if( g_hPowerManager )	DvrPowerManagerStop( g_hPowerManager );
				if( g_hPowerManager )	DvrPowerManagerDeinit( g_hPowerManager );

				if( g_hNormalFileManager )	DvrFileManagerStop( g_hNormalFileManager );
				if( g_hNormalFileManager )	DvrFileManagerDeinit( g_hNormalFileManager );
				if( g_hEventFileManager )	DvrFileManagerStop( g_hEventFileManager );
				if( g_hEventFileManager )	DvrFileManagerDeinit( g_hEventFileManager );
				if( g_hCaptureFileManager )	DvrFileManagerStop( g_hCaptureFileManager );
				if( g_hCaptureFileManager )	DvrFileManagerDeinit( g_hCaptureFileManager );				

				if( g_hGpsManager ) 		g_hGpsManager = NULL;
				if( g_hGsensorManager ) 	g_hGsensorManager = NULL;
				if( g_hPowerManager )		g_hPowerManager = NULL;
				if( g_hNormalFileManager )	g_hNormalFileManager = NULL;
				if( g_hEventFileManager )	g_hEventFileManager = NULL;
				if( g_hCaptureFileManager )	g_hCaptureFileManager = NULL;

				printf("%s(): umount mmc.\n", __func__);
				int32_t pendCnt = 0;
				while( 1 )
				{
					pendCnt++;
					sync();
					usleep(100000);
					if( !umount("/mnt/mmc") ) break;
					if( pendCnt < 10) {
						printf("Fail, umount..\n");
						break;
					}
					printf("[%d] Retry umount..\n", pendCnt);
				}
				break;

			case CMD_TYPE_EVENT :
				pthread_mutex_lock( &gstModeLock );
				if( gDvrMode != DVR_MODE_EVENT ) {
					gDvrMode = DVR_MODE_EVENT;
					pthread_mutex_unlock( &gstModeLock );	
					if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/event.wav" );
					DvrLedEventStart();
					if( g_hDvr )	NX_DvrEvent( g_hDvr );
				}
				else {
					pthread_mutex_unlock( &gstModeLock );	
				}
				break;

			case CMD_TYPE_CAPTURE :
				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/capture.wav" );
				if( g_hDvr )	NX_DvrCapture( g_hDvr, cmd.cmdData );
				break;

			case CMD_TYPE_CHG_LCD :
				if( g_hDvr )	NX_DvrSetPreview( g_hDvr, cmd.cmdData );
				break;

			case CMD_TYPE_CHG_HDMI :
				if( g_hDvr )	NX_DvrSetPreviewHdmi( g_hDvr, cmd.cmdData );
				break;

			case CMD_TYPE_CHG_MODE :
				pthread_mutex_lock( &gstModeLock );
				if( gDvrMode != DVR_MODE_MOTION ) {
					gDvrMode = DVR_MODE_MOTION;
					DvrLedMotionStart();
				}
				else {
					gDvrMode = DVR_MODE_NORMAL;
					DvrLedMotionStop();
				}
				pthread_mutex_unlock( &gstModeLock );

				if( g_hDvr )	NX_DvrChangeMode( g_hDvr, cmd.cmdData );
				break;

			case CMD_TYPE_LOW_VOLTAGE :
				printf("Detect Low voltage.\n");
				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/poweroff.wav" );
				interval = GetSystemTickCount();
				ExitApp();
				interval = GetSystemTickCount() - interval;
				printf("Stop Application. ( %lld mSec )\n", interval );

				MicomKillSignal();
				gstMsg = false;
				break;

			case CMD_TYPE_HIGH_TEMP :
				printf("Detect High Temperature.\n");
				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/poweroff.wav" );
				interval = GetSystemTickCount();
				ExitApp();
				interval = GetSystemTickCount() - interval;
				printf("Stop Application. ( %lld mSec )\n", interval );

				MicomKillSignal();
				gstMsg = false;
				break;

			case CMD_TYPE_MICOM_INT :
				printf("Micom Interrput.\n");
				if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/poweroff.wav" );
				interval = GetSystemTickCount();
				ExitApp();
				interval = GetSystemTickCount() - interval;
				printf("Stop Application. ( %lld mSec )\n", interval );

				MicomKillSignal();
				gstMsg = false;
				break;
				
			default :
				printf("Unknown Command!\n");
				break;
		}
	}

END :
	return 0;
}
