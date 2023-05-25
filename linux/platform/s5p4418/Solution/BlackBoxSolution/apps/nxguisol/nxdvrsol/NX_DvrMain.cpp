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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <NX_DvrControl.h>
#include <nx_fourcc.h>

#include <NX_DvrCmdQueue.h>
#include <NX_DvrGpsManager.h>
#include <NX_DvrGsensorManager.h>
#include <NX_DvrPowerManager.h>
#include <NX_DvrFileManager.h>
#include <NX_DvrLedCtrl.h>
#include <NX_DvrConfig.h>

#include <nx_audio.h>

static NX_DVR_HANDLE			g_hDvr = NULL;
static CMD_QUEUE_HANDLE			g_hCmd = NULL;
static GPS_MANAGER_HANDLE		g_hGpsManager = NULL;
static GSENSOR_MANAGER_HANDLE	g_hGsensorManager = NULL;
static POWER_MANAGER_HANDLE		g_hPowerManager = NULL;
static FILE_MANAGER_HANDLE		g_hNormalFileManager = NULL;
static FILE_MANAGER_HANDLE		g_hEventFileManager = NULL;
static FILE_MANAGER_HANDLE		g_hCaptureFileManager = NULL;
static NX_AUDIO_HANDLE			g_hAudio = NULL;

static NX_DVR_MEDIA_CONFIG		gstMediaConfig;
static NX_DVR_RECORD_CONFIG		gstRecordConfig;
static NX_DVR_DISPLAY_CONFIG	gstDisplayConfig;

static NX_DVR_ENCODE_TYPE gstMode	= DVR_ENCODE_NORMAL; // DVR_ENCODE_NORMAL, DVR_ENCODE_MOTION
static int32_t gstMotionEnable		= true;
static int32_t gstAudioEnable		= true;
static int32_t gstUserDataEnable	= false;

static pthread_mutex_t	g_hJpegLock;
static int32_t gstJpegDone;

int32_t cbGetNormalFileName( uint8_t *buf, uint32_t bufSize )
{
	time_t eTime;
	struct tm *eTm;
	
	time( &eTime);
	eTm = localtime( &eTime );
	sprintf((char*)buf, "/mnt/mmc/normal/normal_%04d%02d%02d_%02d%02d%02d.ts",
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);

	return 0;
}

int32_t cbGetEventFileName( uint8_t *buf, uint32_t bufSize )
{
	time_t eTime;
	struct tm *eTm;
	
	time( &eTime);
	eTm = localtime( &eTime );
	sprintf((char*)buf, "/mnt/mmc/event/event_%04d%02d%02d_%02d%02d%02d.ts",
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);

	return 0;
}

int32_t cbGetCaptureFileName( uint8_t *buf, uint32_t bufSize )
{
	time_t eTime;
	struct tm *eTm;
	
	time( &eTime);
	eTm = localtime( &eTime );
	sprintf((char*)buf, "/mnt/mmc/capture/capture_%04d%02d%02d_%02d%02d%02d.jpeg",
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec);

	return 0;
}

int32_t cbGetFrontTextOverlay( uint8_t *buf, uint32_t *bufSize, uint32_t *x, uint32_t *y)
{
	time_t eTime;
	struct tm *eTm;
	struct nmea_gprmc	gpsData;
	GSENSOR_VALUE		gsensorData;
	int32_t				powerData = 0;
	static uint64_t		frameCounter = 0;

	memset( &gpsData, 0x00, sizeof( struct nmea_gprmc ) );
	memset( &gsensorData, 0x00, sizeof( GSENSOR_VALUE) );

	if( g_hGpsManager )		DvrGpsGetData( g_hGpsManager, &gpsData );
	if( g_hGsensorManager )	DvrGsensorGetData( g_hGsensorManager, &gsensorData );
	if( g_hPowerManager )	DvrPowerGetData( g_hPowerManager, &powerData );

	time( &eTime);
	eTm = localtime( &eTime );

	sprintf((char*)buf, "[CAM#0 - %08lld] [%4d-%02d-%02d %02d:%02d:%02d] [%+010.06f / %+011.06f / %03d] [%+05d / %+05d / %+05d] [%+05.02f]",
		++frameCounter,
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec,
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z,
		(double)(powerData / 1000.)
	);

	*bufSize = strlen((char*)buf);
	*x = *y = 1;

	return 0;
}

int32_t cbGetRearTextOverlay( uint8_t *buf, uint32_t *bufSize, uint32_t *x, uint32_t *y)
{
	time_t eTime;
	struct tm *eTm;
	struct nmea_gprmc	gpsData;
	GSENSOR_VALUE		gsensorData;
	int32_t				powerData = 0;
	static uint64_t		frameCounter = 0;

	memset( &gpsData, 0x00, sizeof( struct nmea_gprmc ) );
	memset( &gsensorData, 0x00, sizeof( GSENSOR_VALUE) );

	if( g_hGpsManager ) 	DvrGpsGetData( g_hGpsManager, &gpsData );
	if( g_hGsensorManager )	DvrGsensorGetData( g_hGsensorManager, &gsensorData);
	if( g_hPowerManager )	DvrPowerGetData( g_hPowerManager, &powerData );

	time( &eTime);
	eTm = localtime( &eTime );

	sprintf((char*)buf, "[CAM#1 - %08lld] [%4d-%02d-%02d %02d:%02d:%02d] [%+010.06f / %+011.06f / %03d] [%+05d / %+05d / %+05d] [%+05.02f]",
		++frameCounter,
		eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec,
		gpsData.latitude, gpsData.longitude, (int)(gpsData.ground_speed * 1.852),
		gsensorData.x, gsensorData.y, gsensorData.z,
		(double)(powerData / 1000.)
	);

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
			DvrFileManagerPush( g_hEventFileManager, (char*)pEventData );
		}
		break;
	case DVR_NOTIFY_JPEGWRITING_DONE :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : Jpeg file writing done. ( %s )\n", __func__, pEventData);
			DvrFileManagerPush( g_hCaptureFileManager, (char*)pEventData );
		}
		pthread_mutex_lock( &g_hJpegLock );
		gstJpegDone = true;
		pthread_mutex_unlock( &g_hJpegLock );
		break;
	case DVR_NOTIFY_MOTION :
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

static void DvrSetConfigure( void )
{
	memset( &gstMediaConfig, 0x00, sizeof( NX_DVR_MEDIA_CONFIG ) );
	memset( &gstRecordConfig, 0x00, sizeof( NX_DVR_RECORD_CONFIG ) );
	memset( &gstDisplayConfig, 0x00, sizeof( NX_DVR_DISPLAY_CONFIG ) );

	gstMediaConfig.nVideoChannel	= !gDvrConfig.nChannel ? 1 : 2;
	gstMediaConfig.bEnableAudio		= gstAudioEnable;
	gstMediaConfig.bEnableUserData 	= gstUserDataEnable;
	gstMediaConfig.nContainer		= DVR_CONTAINER_TS;	

	gstMediaConfig.videoConfig[0].nPort 		= DVR_CAMERA_MIPI;
	gstMediaConfig.videoConfig[0].nSrcWidth		= 1024;
	gstMediaConfig.videoConfig[0].nSrcHeight	= 768;
	gstMediaConfig.videoConfig[0].bExternProc	= false;
	gstMediaConfig.videoConfig[0].nDstWidth		= gstMediaConfig.videoConfig[0].nSrcWidth;
	gstMediaConfig.videoConfig[0].nDstHeight	= gstMediaConfig.videoConfig[0].nSrcHeight;
	gstMediaConfig.videoConfig[0].nFps			= 15;
	gstMediaConfig.videoConfig[0].nBitrate		= 7000000;
	gstMediaConfig.videoConfig[0].nCodec		= DVR_CODEC_H264;
	
	gstMediaConfig.videoConfig[1].nPort			= DVR_CAMERA_VIP0;
	gstMediaConfig.videoConfig[1].nSrcWidth		= 1024;
	gstMediaConfig.videoConfig[1].nSrcHeight	= 768;
	gstMediaConfig.videoConfig[1].bExternProc	= false;
	gstMediaConfig.videoConfig[1].nDstWidth		= gstMediaConfig.videoConfig[1].nSrcWidth;
	gstMediaConfig.videoConfig[1].nDstHeight	= gstMediaConfig.videoConfig[1].nSrcHeight;
	gstMediaConfig.videoConfig[1].nFps			= 15;
	gstMediaConfig.videoConfig[1].nBitrate		= 5000000;
	gstMediaConfig.videoConfig[1].nCodec		= DVR_CODEC_H264;
	
	gstMediaConfig.textConfig.nBitrate			= 3000000;
	gstMediaConfig.textConfig.nInterval			= 200;

	gstMediaConfig.audioConfig.nChannel			= 2;
	gstMediaConfig.audioConfig.nFrequency		= 48000;
	gstMediaConfig.audioConfig.nBitrate			= 128000;
	gstMediaConfig.audioConfig.nCodec			= DVR_CODEC_AAC;

	gstRecordConfig.nNormalDuration				= 20000;
	gstRecordConfig.nEventDuration				= 10000;
	gstRecordConfig.nEventBufferDuration		= 10000;

	gstRecordConfig.networkType					= gDvrConfig.bHls ? DVR_NETWORK_HLS : DVR_NETWORK_NONE;
	gstRecordConfig.hlsConfig.nSegmentDuration 	= 10;
	gstRecordConfig.hlsConfig.nSegmentNumber	= 3;
	sprintf( (char*)gstRecordConfig.hlsConfig.MetaFileName,	"test.m3u8" );
	sprintf( (char*)gstRecordConfig.hlsConfig.SegmentFileName,	"segment" );
	sprintf( (char*)gstRecordConfig.hlsConfig.SegmentRootDir,	"/www" );

	gstRecordConfig.bMdEnable[0]				= false;
	gstRecordConfig.mdConfig[0].nMdThreshold	= 100;
	gstRecordConfig.mdConfig[0].nMdSensitivity	= 100;
	gstRecordConfig.mdConfig[0].nMdSampingFrame	= 1;

	gstRecordConfig.bMdEnable[1]				= gstMotionEnable;
	gstRecordConfig.mdConfig[1].nMdThreshold	= 100;
	gstRecordConfig.mdConfig[1].nMdSensitivity	= 100;
	gstRecordConfig.mdConfig[1].nMdSampingFrame	= 1;

	gstDisplayConfig.bEnable			= true;
	gstDisplayConfig.nChannel			= gDvrConfig.nPreview;
	gstDisplayConfig.nModule			= 0;
	gstDisplayConfig.cropRect.nLeft 	= 0;
	gstDisplayConfig.cropRect.nTop		= 0;
	gstDisplayConfig.cropRect.nRight 	= 800;
	gstDisplayConfig.cropRect.nBottom 	= 400;
	gstDisplayConfig.dspRect.nLeft 		= 0;
	gstDisplayConfig.dspRect.nTop		= 0;
	gstDisplayConfig.dspRect.nRight 	= 800;
	gstDisplayConfig.dspRect.nBottom 	= 400;
}

void DvrStart( void )
{
	pthread_mutex_init( &g_hJpegLock, NULL );
	pthread_mutex_lock( &g_hJpegLock );
	gstJpegDone = true;
	pthread_mutex_unlock( &g_hJpegLock );	

	DvrSetConfigure( );

	g_hNormalFileManager	= DvrFileManagerInit( (const char*)"/mnt/mmc/normal", 50, "ts" );
	g_hEventFileManager		= DvrFileManagerInit( (const char*)"/mnt/mmc/event", 20, "ts" );
	g_hCaptureFileManager	= DvrFileManagerInit( (const char*)"/mnt/mmc/capture", 1, "jpeg" );

	// g_hGpsManager			= DvrGpsManagerInit();
	// g_hGsensorManager 		= DvrGsensorManagerInit();
	// g_hPowerManager			= DvrPowerManagerInit();

	g_hAudio = NX_AudioInit();	
	g_hDvr = NX_DvrInit( &gstMediaConfig, &gstRecordConfig, &gstDisplayConfig );

	if( g_hGsensorManager )		DvrGsensorManagerRegCmd( g_hGsensorManager, g_hCmd );
	if( g_hGsensorManager )		DvrGsensorManagerMotionEnable( g_hGsensorManager, gstMotionEnable );

	if( g_hNormalFileManager )	DvrFileManagerStart( g_hNormalFileManager );
	if( g_hEventFileManager ) 	DvrFileManagerStart( g_hEventFileManager );
	if( g_hCaptureFileManager )	DvrFileManagerStart( g_hCaptureFileManager );

	if( g_hDvr )	NX_DvrRegisterFileNameCallback( g_hDvr, cbGetNormalFileName, cbGetEventFileName, NULL , cbGetCaptureFileName );
	if( g_hDvr )	NX_DvrRegisterTextOverlayCallback( g_hDvr, cbGetFrontTextOverlay, cbGetRearTextOverlay );
	if( g_hDvr )	NX_DvrRegisterUserDataCallback( g_hDvr, cbUserData );
	if( g_hDvr )	NX_DvrRegisterNotifyCallback( g_hDvr, cbNotifier );
	
	if( g_hAudio )	NX_AudioPlay( g_hAudio, "/root/wav/start.wav" );
	if( g_hDvr ) 	NX_DvrStart( g_hDvr, gstMode );

	if( g_hGpsManager )		DvrGpsManagerStart( g_hGpsManager );
	if( g_hGsensorManager )	DvrGsensorManagerStart( g_hGsensorManager );
	if( g_hPowerManager )	DvrPowerManagerStart( g_hPowerManager );
}

void DvrStop( void )
{		
	NX_AudioPlay( g_hAudio, "/root/wav/stop.wav" );
	usleep(2000000);

	if( g_hDvr ) NX_DvrStop( g_hDvr );
	if( g_hDvr ) NX_DvrDeinit( g_hDvr );

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

	if( g_hDvr )				g_hDvr = NULL;
	if( g_hGpsManager ) 		g_hGpsManager = NULL;
	if( g_hGsensorManager ) 	g_hGsensorManager = NULL;
	if( g_hPowerManager )		g_hPowerManager = NULL;
	if( g_hNormalFileManager )	g_hNormalFileManager = NULL;
	if( g_hEventFileManager )	g_hEventFileManager = NULL;
	if( g_hCaptureFileManager )	g_hCaptureFileManager = NULL;	

	if( g_hAudio ) 				NX_AudioStop( g_hAudio, false );
	if( g_hAudio ) 				NX_AudioDeinit( g_hAudio );
	if( g_hAudio ) 				g_hAudio = NULL;

	pthread_mutex_destroy( &g_hJpegLock );
}

void DvrCapture( void )
{
	pthread_mutex_lock( &g_hJpegLock );
	int32_t jpegDone = gstJpegDone;
	pthread_mutex_unlock( &g_hJpegLock );
	if( !jpegDone )
		return;

	gstJpegDone = false;
	if( g_hAudio)	NX_AudioPlay( g_hAudio, "/root/wav/capture.wav" );
	if( g_hDvr )	NX_DvrCapture( g_hDvr, 0 );
}
