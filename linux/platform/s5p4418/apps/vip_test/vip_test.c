#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <linux/fb.h>

#include <nx_fourcc.h>
#include <nx_vip.h>
#include <nx_dsp.h>

#include "queue.h"

typedef enum {
	NX_QUEUE_CLIPPER,
	NX_QUEUE_DECIMATOR,
} NX_QUEUE_TYPE;

enum {
	CLIPPER_MEMORY,
	DECIMATOR_MEMORY,
};

typedef struct {
	VIP_HANDLE		hVip;
	VIP_INFO		vipInfo;

	DISPLAY_HANDLE	hDsp;
	DISPLAY_INFO	dspInfo;	

	NX_QUEUE		clipQueue;		// clipper memory queue
	int32_t			clipMemNum;		// clipper memory number

	NX_QUEUE		deciQueue;		// decimator memory queue
	int32_t			deciMemNum;		// decimator memory number

	pthread_t		hThread;
	int32_t 		bThreadRun;
	pthread_mutex_t hLock;

	int32_t			bCapture;
	int32_t			bDebug;
	int32_t			dspMemory;
} APP_DATA;

//----------------------------------------------------------------------------------------------------
//
//	Signal Handler
//
static void signal_handler( int32_t sig )
{
	printf("Aborted by signal %s (%d)..\n", (char*)strsignal(sig), sig);

	switch( sig )
	{
		case SIGINT :
			printf("SIGINT..\n"); 	break;
		case SIGTERM :
			printf("SIGTERM..\n");	break;
		case SIGABRT :
			printf("SIGABRT..\n");	break;
		default :
			break;
	}

	exit(EXIT_FAILURE);
}

static void register_signal( void )
{
	signal( SIGINT,  signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

//----------------------------------------------------------------------------------------------------
//
//	Memory Allocator
//
static int32_t allocate_buffer( APP_DATA *pAppData, NX_QUEUE_TYPE type, int32_t width, int32_t height, uint32_t fourcc, int32_t num )
{
	int32_t i = 0;
	NX_QUEUE *pQueue	= (type == NX_QUEUE_CLIPPER) ? &pAppData->clipQueue : &pAppData->deciQueue;
	int32_t *pMemNum	= (type == NX_QUEUE_CLIPPER) ? &pAppData->clipMemNum : &pAppData->deciMemNum;

	NX_InitQueue( pQueue, num );
	for( i = 0; i < num; i++ )
	{
		NX_VID_MEMORY_HANDLE hVidMem = NULL;
		hVidMem = NX_VideoAllocateMemory( 4096, width, height, NX_MEM_MAP_LINEAR, fourcc);
		if( !hVidMem ) {
			printf("%s(): error allocate memory.\n", __func__);
			*pMemNum = i;
			return -1;
		}
		NX_PushQueue( pQueue, hVidMem );
	}

	*pMemNum = i;
	return 0;
}

static int32_t release_buffer( APP_DATA *pAppData )
{
	int32_t i = 0;

	for( i = 0; i < pAppData->clipMemNum; i++ )
	{
		NX_VID_MEMORY_HANDLE hVidMem = NULL;
		NX_PopQueue( &pAppData->clipQueue, (void**)&hVidMem );
		if( hVidMem ) NX_FreeVideoMemory( hVidMem );
	}

	for( i = 0; i < pAppData->deciMemNum; i++ )
	{
		NX_VID_MEMORY_HANDLE hVidMem = NULL;
		NX_PopQueue( &pAppData->deciQueue, (void**)&hVidMem );
		if( hVidMem ) NX_FreeVideoMemory( hVidMem );
	}
	if(pAppData->clipMemNum) NX_DeinitQueue( &pAppData->clipQueue );
	if(pAppData->deciMemNum) NX_DeinitQueue( &pAppData->deciQueue );

	pAppData->clipMemNum = 0;
	pAppData->deciMemNum = 0;

	return 0;
}

//----------------------------------------------------------------------------------------------------
//
//	VIP Operation
//
static void *vip_thread( void *data )
{
	APP_DATA *pAppData	= (APP_DATA*)data;

	NX_QUEUE *pQueue1 = NULL, *pQueue2 = NULL;
	NX_VID_MEMORY_INFO *pCurBuffer1 = NULL, *pTmpBuffer1 = NULL, *pPrvBuffer1 = NULL;
	NX_VID_MEMORY_INFO *pCurBuffer2 = NULL, *pTmpBuffer2 = NULL, *pPrvBuffer2 = NULL;
	int64_t timeStamp1, timeStamp2;
	
	uint64_t prvTime = 0;

	int32_t bCapture, bDebug;

	pthread_mutex_init( &pAppData->hLock, NULL );

	pAppData->hVip = NX_VipInit( &pAppData->vipInfo );
	pAppData->hDsp = NX_DspInit( &pAppData->dspInfo );

	if( pAppData->vipInfo.mode == VIP_MODE_CLIPPER ) {
		allocate_buffer( pAppData, NX_QUEUE_CLIPPER, pAppData->vipInfo.cropWidth, pAppData->vipInfo.cropHeight, FOURCC_MVS0, 4 );
	}
	else if( pAppData->vipInfo.mode == VIP_MODE_DECIMATOR || pAppData->vipInfo.mode == VIP_MODE_CLIP_DEC ) {
		allocate_buffer( pAppData, NX_QUEUE_DECIMATOR, pAppData->vipInfo.outWidth, pAppData->vipInfo.outHeight, FOURCC_MVS0, 4 );
	}
	else if( pAppData->vipInfo.mode == VIP_MODE_CLIP_DEC2 ) {
		allocate_buffer( pAppData, NX_QUEUE_CLIPPER, pAppData->vipInfo.cropWidth, pAppData->vipInfo.cropHeight, FOURCC_MVS0, 4 );
		allocate_buffer( pAppData, NX_QUEUE_DECIMATOR, pAppData->vipInfo.outWidth, pAppData->vipInfo.outHeight, FOURCC_MVS0, 4 );
	}

	if( pAppData->vipInfo.mode != VIP_MODE_CLIP_DEC2 ) {
		pQueue1 = (pAppData->vipInfo.mode == VIP_MODE_CLIPPER) ? &pAppData->clipQueue : &pAppData->deciQueue;
		NX_PopQueue( pQueue1, (void**)&pCurBuffer1 );
		NX_VipQueueBuffer( pAppData->hVip, pCurBuffer1 );
	}
	else {
		pQueue1 = &pAppData->clipQueue;
		pQueue2 = &pAppData->deciQueue;
		NX_PopQueue( pQueue1, (void**)&pCurBuffer1 );
		NX_PopQueue( pQueue2, (void**)&pCurBuffer2 );
		NX_VipQueueBuffer2( pAppData->hVip, pCurBuffer1, pCurBuffer2 );
	}

	while( pAppData->bThreadRun )
	{
		pthread_mutex_lock( &pAppData->hLock );
		bCapture 	= pAppData->bCapture;
		bDebug		= pAppData->bDebug;
		pAppData->bCapture = false;	
		pthread_mutex_unlock( &pAppData->hLock );

		if( pAppData->vipInfo.mode != VIP_MODE_CLIP_DEC2 ) {
			NX_PopQueue( pQueue1, (void**)&pCurBuffer1 );
			NX_VipQueueBuffer( pAppData->hVip, pCurBuffer1 );
			NX_VipDequeueBuffer( pAppData->hVip, &pTmpBuffer1, &timeStamp1 );
			
			if( !pTmpBuffer1 ) {
				printf("%s(): invalid memory buffer.\n", __func__);
				continue;
			}

			NX_DspQueueBuffer( pAppData->hDsp, pTmpBuffer1 );

			if( pPrvBuffer1 ) {
				NX_DspDequeueBuffer( pAppData->hDsp );
				NX_PushQueue( pQueue1, pPrvBuffer1 );
			}
			
			pPrvBuffer1 = pCurBuffer1;

		}
		else {
			NX_PopQueue( pQueue1, (void**)&pCurBuffer1 );
			NX_PopQueue( pQueue2, (void**)&pCurBuffer2 );
			NX_VipQueueBuffer2( pAppData->hVip, pCurBuffer1, pCurBuffer2 );
			NX_VipDequeueBuffer2( pAppData->hVip, &pTmpBuffer1, &pTmpBuffer2, &timeStamp1, &timeStamp2 );

			if( !pTmpBuffer1 || !pTmpBuffer2 ) {
				printf("%s(): invalid memory buffer.\n", __func__);
				continue;
			}

			NX_DspQueueBuffer( pAppData->hDsp, (pAppData->dspMemory == CLIPPER_MEMORY) ? pTmpBuffer1 : pTmpBuffer2 );

			if( pPrvBuffer1 && pPrvBuffer2 ) {
				NX_DspDequeueBuffer( pAppData->hDsp );
				NX_PushQueue( pQueue1, pPrvBuffer1 );
				NX_PushQueue( pQueue2, pPrvBuffer2 );
			}

			pPrvBuffer1 = pCurBuffer1;
			pPrvBuffer2 = pCurBuffer2;
		}

		// Capture.
		if( bCapture ) {
			NX_VID_MEMORY_INFO *pCaptureBuffer = NULL;

			char fileName[1024];
			time_t eTime;
			struct tm *eTm;

			FILE *fp;

			time( &eTime );
			eTm = localtime( &eTime );

			if( pAppData->vipInfo.mode == VIP_MODE_CLIP_DEC2 )
				pCaptureBuffer = (pAppData->dspMemory == CLIPPER_MEMORY) ? pCurBuffer1 : pCurBuffer2;
			else
				pCaptureBuffer = pCurBuffer1;

			sprintf( fileName, "dump_%04d%02d%02d_%02d%02d%02d_%dx%d.yuv",
				eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec,
				pCaptureBuffer->luStride, pCaptureBuffer->imgHeight );

			if( (fp = fopen( fileName, "wb+")) ) {
				fwrite( (char*)pCaptureBuffer->luVirAddr, 1, pCaptureBuffer->luStride * pCaptureBuffer->imgHeight, fp );
				fwrite( (char*)pCaptureBuffer->cbVirAddr, 1, pCaptureBuffer->cbStride * pCaptureBuffer->imgHeight / 2, fp );
				fwrite( (char*)pCaptureBuffer->crVirAddr, 1, pCaptureBuffer->crStride * pCaptureBuffer->imgHeight / 2, fp );
				fclose(fp);

				printf("Capture Done. ( name : %s, size : %d x %d )\n", fileName, pCaptureBuffer->luStride, pCaptureBuffer->imgHeight);
			}
		}

		// Debug Message.
		if( bDebug ) {
			NX_VID_MEMORY_INFO *pDebugBuffer = NULL;
			int64_t *pTimeStamp =  NULL;
			double	fps = 0.;
			
			if( pAppData->vipInfo.mode == VIP_MODE_CLIP_DEC2 ) {
				pDebugBuffer 	= (pAppData->dspMemory == CLIPPER_MEMORY) ? pTmpBuffer1 : pTmpBuffer2;
				pTimeStamp		= (pAppData->dspMemory == CLIPPER_MEMORY) ? &timeStamp1 : &timeStamp2;
			}
			else {
				pDebugBuffer 	= pTmpBuffer1;
				pTimeStamp		= &timeStamp1;
			}

			*pTimeStamp /= 1000;	// nSec -> uSec(*) -> mSec -> sec
			fps = 1000000. / (double)(*pTimeStamp - prvTime);
			prvTime = *pTimeStamp;

			printf("mem( 0x%08x ), %llduSec, %2.2ffps\n", (uint32_t)pDebugBuffer, *pTimeStamp, fps );
		}
	}
	pthread_mutex_destroy( &pAppData->hLock );

	if( pAppData->hVip ) NX_VipClose( pAppData->hVip );
	if( pAppData->hDsp ) NX_DspClose( pAppData->hDsp );

	if( pAppData->vipInfo.mode != VIP_MODE_CLIP_DEC2 ) {
		NX_PushQueue( pQueue1, pPrvBuffer1 );
	}
	else {
		NX_PushQueue( pQueue1, pPrvBuffer1 );
		NX_PushQueue( pQueue2, pPrvBuffer2 );
	}
	release_buffer( pAppData );
	
	return (void*)0xDEADDEAD;
}

static int32_t vip_start( APP_DATA *pAppData )
{
	printf("Run Vip.\n");

	if( pAppData->bThreadRun ) {
		printf("%s(): Already running thread. ( 0x%08x )\n", __func__, (uint32_t)pAppData->hThread );
		return -1;
	}

	pAppData->bThreadRun = true;
	if( 0 > pthread_create( &pAppData->hThread, NULL, vip_thread, (void*) pAppData) ) {
		printf("%s(): create thread, %s\n", __func__, strerror(errno) );
		pAppData->bThreadRun = false;
		return -1;
	}

	return 0;
}

static void vip_stop( APP_DATA *pAppData )
{
	printf(" Stop Vip.\n");

	pAppData->bThreadRun = false;
	if( pAppData->hThread )
		pthread_join( pAppData->hThread, NULL );

	pAppData->hThread = 0x00;

}

static void vip_capture( APP_DATA *pAppData )
{
	printf(" Capture Vip.\n");

	if( pAppData->hThread ) {
		pthread_mutex_lock( &pAppData->hLock );
		pAppData->bCapture = true;
		pthread_mutex_unlock( &pAppData->hLock );
	}
}

//----------------------------------------------------------------------------------------------------
//
//	Default Parameter Setting
//
static int32_t get_display_info( int32_t *pWidth, int32_t *pHeight )
{
	int32_t fd;
	struct fb_var_screeninfo fbvar;

	if( 0 > (fd = open("/dev/fb0", O_RDWR )) ) {
		printf("%s(): fbdev open fail.\n", __func__);
		return -1;
	}

	if( 0 > ioctl(fd, FBIOGET_VSCREENINFO, &fbvar) ) {
		printf("%s(): fbdev ioctl error.\n", __func__);
		close(fd);
		return -1;
	}
	
	*pWidth 	= fbvar.xres;
	*pHeight 	= fbvar.yres;

	//printf("%s(): fbdev - width(%d) x height(%d)\n", *pWidth, *pHeight);

	close(fd);
	return 0;
}

static void def_param( APP_DATA *pAppData )
{
	VIP_INFO 		vipInfo;
	DISPLAY_INFO	dspInfo;

	int32_t dspWidth = 0, dspHeight = 0;

	// a. Initialize Application Data 
	pAppData->hVip 			= NULL;
	pAppData->hDsp			= NULL;
	pAppData->deciMemNum	= 0;
	pAppData->clipMemNum	= 0;	
	pAppData->hThread 		= 0x00;
	pAppData->bThreadRun 	= false;
	pAppData->bCapture		= false;
	pAppData->bDebug 		= false;
	pAppData->dspMemory		= CLIPPER_MEMORY;		// CLIPPER_MEMORY / DECIMATOR_MEMORY

	// b. Initialize Vip Infomation
	memset( &vipInfo, 0x00, sizeof(vipInfo) );
	vipInfo.port 			= VIP_PORT_0;
	vipInfo.mode 			= VIP_MODE_CLIPPER;	// VIP_MODE_CLIPPER / VIP_MODE_DECIMATOR / VIP_MODE_CLIP_DEC / VIP_MODE_CLIP_DEC2
	
	vipInfo.width			= 1024;
	vipInfo.height			= 768;
	vipInfo.numPlane		= 1;
	vipInfo.fpsNum 			= 30;
	vipInfo.fpsDen 			= 1;
	
	vipInfo.cropX			= 0;
	vipInfo.cropY			= 0;
	vipInfo.cropWidth		= vipInfo.width;
	vipInfo.cropHeight		= vipInfo.height;
	
	vipInfo.outWidth		= vipInfo.width;
	vipInfo.outHeight		= vipInfo.height;

	// c. Initialize Display Information
	memset( &dspInfo, 0x00, sizeof(dspInfo) );
	dspInfo.port		= DISPLAY_PORT_LCD;
	dspInfo.module		= DISPLAY_MODULE_MLC0;
	dspInfo.numPlane	= vipInfo.numPlane;
	
	if( vipInfo.mode == VIP_MODE_CLIPPER ) {
		dspInfo.width				= vipInfo.cropWidth;
		dspInfo.height				= vipInfo.cropHeight;

		dspInfo.dspSrcRect.left		= 0;
		dspInfo.dspSrcRect.top		= 0;
		dspInfo.dspSrcRect.right	= vipInfo.cropWidth;
		dspInfo.dspSrcRect.bottom	= vipInfo.cropHeight;
	}
	else if( (vipInfo.mode == VIP_MODE_DECIMATOR) || (vipInfo.mode == VIP_MODE_CLIP_DEC) ) {
		dspInfo.width				= vipInfo.outWidth;
		dspInfo.height				= vipInfo.outHeight;

		dspInfo.dspSrcRect.left		= 0;
		dspInfo.dspSrcRect.top		= 0;
		dspInfo.dspSrcRect.right	= vipInfo.outWidth;
		dspInfo.dspSrcRect.bottom	= vipInfo.outHeight;
	}
	else {
		dspInfo.width				= (pAppData->dspMemory == CLIPPER_MEMORY) ? vipInfo.cropWidth : vipInfo.outWidth;
		dspInfo.height				= (pAppData->dspMemory == CLIPPER_MEMORY) ? vipInfo.cropHeight : vipInfo.outHeight;

		dspInfo.dspSrcRect.left		= 0;
		dspInfo.dspSrcRect.top		= 0;
		dspInfo.dspSrcRect.right	= (pAppData->dspMemory == CLIPPER_MEMORY) ? vipInfo.cropWidth : vipInfo.outWidth;
		dspInfo.dspSrcRect.bottom	= (pAppData->dspMemory == CLIPPER_MEMORY) ? vipInfo.cropHeight : vipInfo.outHeight;
	}

	get_display_info( &dspWidth, &dspHeight );
	dspInfo.dspDstRect.left		= 0;
	dspInfo.dspDstRect.top		= 0;
	dspInfo.dspDstRect.right	= dspWidth;
	dspInfo.dspDstRect.bottom	= dspHeight;

	// d. Application Data Update
	pAppData->vipInfo 	= vipInfo;
	pAppData->dspInfo 	= dspInfo;
}

//----------------------------------------------------------------------------------------------------
//
//	Menu list
//
#define STR_VIPPORT(a) 	(a == 0) ? "VIP0" : \
						(a == 1) ? "VIP1" : \
						(a == 2) ? "VIP2" : "MIPI"

#define STR_VIPMODE(a)	(a == 0) ? "Clipper" :		\
						(a == 1) ? "Decimator" : 	\
						(a == 2) ? "Clipper + Decimator(mem)" : "Clipper(mem) + Decimator(mem)"

#define STR_DSPMODE(a)	(a == 0) ? "Clipper memory display" : "Decimator memory display"

static void vip_config	( void *pData );
static void vip_run		( void *pData );

typedef struct {
	const char	*cmd;
	void		(*func)(void *pData);
	const char	*usage;
} APP_MENU;

static APP_MENU appMenu[] = {
	{ "0",	vip_run,		"cmd: 0 - vip run"				},
	{ "1",	vip_config,		"cmd: 1 - vip configuration"	},
	{ "h",	NULL,			"cmd: h - help"					},
	{ "q",	NULL,			"cmd: q - quit"					},
};

static void vip_run( void *pData )
{
	APP_DATA *pAppData = (APP_DATA*)pData;
	DISPLAY_INFO *pDspInfo = &pAppData->dspInfo;

	char key[8] = {'h', };

	while(1)
	{
		if( 'h' == key[0] ) {
			printf("+------------------------------------------------------+\n");
			printf("  VIP Run                                               \n");
			printf("+------------------------------------------------------+\n");
			printf(" 0 : Run                                                \n");
			printf(" 1 : Stop                                               \n");
			printf(" 2 : Capture                                            \n");
			printf(" 3 : Debug                                              \n");
			printf(" 4 : Display Crop = l: %4d, t: %4d, r: %4d, b: %4d      \n", pDspInfo->dspSrcRect.left, pDspInfo->dspSrcRect.top, pDspInfo->dspSrcRect.right, pDspInfo->dspSrcRect.bottom);
			printf(" 5 : Display Pos  = l: %4d, t: %4d, r: %4d, b: %4d      \n", pDspInfo->dspDstRect.left, pDspInfo->dspDstRect.top, pDspInfo->dspDstRect.right, pDspInfo->dspDstRect.bottom);
			printf(" q : quit                                               \n");
			printf("+------------------------------------------------------+\n");
		}
		
		printf("#> ");
		fgets(key, sizeof(key), stdin);

		if( '0' != key[0] && 
			'1' != key[0] && 
			'2' != key[0] && 
			'3' != key[0] && 
			'4' != key[0] && 
			'5' != key[0] && 
			'q' != key[0] ) {
			continue;
		}

		if( 'q' == key[0] ) {
			printf("Quit..\n");
			vip_stop( pAppData );
			break;
		}

		if( '0' == key[0] ) {
			vip_start( pAppData );
			continue;
		}

		if( '1' == key[0] ) {
			vip_stop( pAppData );
			continue;
		}

		if( '2' == key[0] ) {
			vip_capture( pAppData );
			continue;
		}
		
		if( '3' == key[0] ) {
			pAppData->bDebug = !pAppData->bDebug;
			printf(" debug : %s\n", pAppData->bDebug ? "enable" : "disable" );
			continue;
		}

		if( '4' == key[0] ) {
			DSP_IMG_RECT srcRect;
			memset( &srcRect, 0x00, sizeof(srcRect) );
			printf(" crop : left, top, right, bottom ? ");
			fscanf(stdin, "%d%d%d%d", &srcRect.left, &srcRect.top, &srcRect.right, &srcRect.bottom );
			NX_DspVideoSetSourceCrop( pAppData->hDsp, &srcRect );
			printf(" crop : l(%d), t(%d), r(%d), b(%d)\n", srcRect.left, srcRect.top, srcRect.right, srcRect.bottom );
			continue;
		}

		if( '5' == key[0] ) {
			DSP_IMG_RECT dstRect;
			memset( &dstRect, 0x00, sizeof(dstRect) );
			printf(" pos : left, top, right, bottom ? ");
			fscanf(stdin, "%d%d%d%d", &dstRect.left, &dstRect.top, &dstRect.right, &dstRect.bottom );
			NX_DspVideoSetPosition( pAppData->hDsp, &dstRect );
			printf(" pos : l(%d), t(%d), r(%d), b(%d)\n", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom );
			continue;
		}
	}
}

static void vip_config( void *pData )
{
	APP_DATA *pAppData = (APP_DATA*)pData;
	VIP_INFO *pVipInfo = &pAppData->vipInfo;
	DISPLAY_INFO *pDspInfo = &pAppData->dspInfo;

	char key[8] = {'h', };
	
	while(1)
	{
		if( 'h' == key[0] ) {
			printf("+------------------------------------------------------+\n");
			printf("  VIP Configuration                                     \n");
			printf("+------------------------------------------------------+\n");
			printf(" 0 : VIP port     = %s                                  \n", STR_VIPPORT(pVipInfo->port) );
			printf(" 1 : VIP Mode     = %s                                  \n", STR_VIPMODE(pVipInfo->mode) );
			if( pVipInfo->mode == VIP_MODE_CLIP_DEC2 )
				printf("                    --> %s                          \n", STR_DSPMODE(pAppData->dspMemory) );
			printf(" 2 : Camera       = %d x %d, %d fps                     \n", pVipInfo->width, pVipInfo->height, pVipInfo->fpsNum );
			printf(" 3 : Clipper      = x: %4d, y: %4d, w: %4d, h: %4d      \n", pVipInfo->cropX, pVipInfo->cropY, pVipInfo->cropWidth, pVipInfo->cropHeight );
			printf(" 4 : Decimator    = w: %4d, h: %4d                      \n", pVipInfo->outWidth, pVipInfo->outHeight);
			printf(" q : quit                                               \n");
			printf("+------------------------------------------------------+\n");
		}

		printf("#> ");
		fgets(key, sizeof(key), stdin);

		if( '0' != key[0] && 
			'1' != key[0] && 
			'2' != key[0] &&
			'3' != key[0] &&
			'4' != key[0] &&
			'q' != key[0] ) {
			continue;
		}

		if( 'q' == key[0] ) {
			printf("Quit..\n");
			vip_stop( pAppData );
			break;
		}

		if( '0' == key[0] )	{
			char buf[8];
			int32_t sel;
			printf(" vip port : 0 = VIP0, 1 = VIP1, 2 = VIP2, 3 = MIPI ? ");
			fgets( buf, sizeof(buf), stdin );
			sel = atoi( buf );
			switch( sel ) {
				case 0 : pVipInfo->port = VIP_PORT_0;	break;
				case 1 : pVipInfo->port = VIP_PORT_1;	break;
				case 2 : pVipInfo->port = VIP_PORT_2;	break;
				case 3 : pVipInfo->port = VIP_PORT_MIPI;break;
				default : break;
			}
			printf(" vip port : %s\n", STR_VIPPORT(pVipInfo->port) );
			continue;
		}

		if( '1' == key[0] ) {
			char buf[8];
			int32_t sel;
			printf(" vip mode : 0 = Clipper, 1 = Decimator, 2 = Clipper + Decimator(mem), 3 = Clipper(mem) + Decimator(mem) ? ");
			fgets( buf, sizeof(buf), stdin );
			sel = atoi( buf );
			switch( sel ) {
				case 0 : pVipInfo->mode = VIP_MODE_CLIPPER;		break;
				case 1 : pVipInfo->mode = VIP_MODE_DECIMATOR;	break;
				case 2 : pVipInfo->mode = VIP_MODE_CLIP_DEC;	break;
				case 3 : pVipInfo->mode = VIP_MODE_CLIP_DEC2;	break;
				default : break;
			}

			if( pVipInfo->mode == VIP_MODE_CLIP_DEC2 ) {
				printf(" display memory : 0 = Clipper, 1 = Decimator ? ");
				fgets( buf, sizeof(buf), stdin );
				sel = atoi( buf );
				switch( sel ) {
					case 0 : pAppData->dspMemory = CLIPPER_MEMORY;		break;
					case 1 : pAppData->dspMemory = DECIMATOR_MEMORY;	break;
					default : break;
				}
				pDspInfo->width		= (pAppData->dspMemory == CLIPPER_MEMORY) ? pVipInfo->cropWidth : pVipInfo->outWidth;
				pDspInfo->height	= (pAppData->dspMemory == CLIPPER_MEMORY) ? pVipInfo->cropHeight : pVipInfo->outHeight;

				pDspInfo->dspSrcRect.left	= 0;
				pDspInfo->dspSrcRect.top	= 0;
				pDspInfo->dspSrcRect.right	= (pAppData->dspMemory == CLIPPER_MEMORY) ? pVipInfo->cropWidth : pVipInfo->outWidth;
				pDspInfo->dspSrcRect.bottom	= (pAppData->dspMemory == CLIPPER_MEMORY) ? pVipInfo->cropHeight : pVipInfo->outHeight;
			}
			else {
				pDspInfo->width		= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropWidth : pVipInfo->outWidth;
				pDspInfo->height	= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropHeight : pVipInfo->outHeight;

				pDspInfo->dspSrcRect.left	= 0;
				pDspInfo->dspSrcRect.top	= 0;
				pDspInfo->dspSrcRect.right	= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropWidth : pVipInfo->outWidth;
				pDspInfo->dspSrcRect.bottom	= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropHeight : pVipInfo->outHeight;
			}
			printf(" vip mode : %s\n", STR_VIPMODE(pVipInfo->mode) );
			if( pVipInfo->mode == VIP_MODE_CLIP_DEC2 ) {
				printf("            --> %s\n", STR_DSPMODE(pAppData->dspMemory) );
			}
			continue;
		}

		if( '2' == key[0] ) {
			int32_t width = 0, height = 0, fps = 0;
			printf(" camera : width, height, fps -> ");
			fscanf(stdin, "%d%d%d", &width, &height, &fps);

			if( width <= 0 || height <= 0 || fps <= 0 ) {
				printf(" invalid value. ( width: %d, height: %d, fps: %d )\n", width, height, fps);
				continue;
			}

			pVipInfo->width		= width;
			pVipInfo->height	= height;
			pVipInfo->fpsNum	= fps;

			printf(" camera : %d x %d, %d fps\n", pVipInfo->width, pVipInfo->height, pVipInfo->fpsNum );
			continue;
		}

		if( '3' == key[0] ) {
			int32_t x = 0, y = 0, width = 0, height = 0;
			printf(" clipper : x, y, width, height ? ");
			fscanf(stdin, "%d%d%d%d", &x, &y, &width, &height );
			
			if( (x + width) > pVipInfo->width || (y + height) > pVipInfo->height || width <= 0 || height <= 0 ) {
				printf(" invalid value. ( x: %d, y: %d, w: %d, h: %d )\n", x, y, width, height );
				continue;
			}

			pVipInfo->cropX 		= x;
			pVipInfo->cropY 		= y;
			pVipInfo->cropWidth 	= width;
			pVipInfo->cropHeight	= height;

			pDspInfo->width			= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropWidth : pVipInfo->outWidth;
			pDspInfo->height		= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropHeight : pVipInfo->outHeight;

			pDspInfo->dspSrcRect.left	= 0;
			pDspInfo->dspSrcRect.top	= 0;
			pDspInfo->dspSrcRect.right	= pDspInfo->width;
			pDspInfo->dspSrcRect.bottom	= pDspInfo->height;

			printf(" clipper : x(%d), y(%d), w(%d), h(%d)\n", pVipInfo->cropX, pVipInfo->cropY, pVipInfo->cropWidth, pVipInfo->cropHeight);
			continue;
		}

		if( '4' == key[0] ) {
			int32_t width = 0, height = 0;
			printf(" decimator : width, height ? ");
			fscanf(stdin, "%d%d", &width, &height );
			
			pVipInfo->outWidth		= width;
			pVipInfo->outHeight		= height;

			if( width > pVipInfo->width || height > pVipInfo->height || width <= 0 || height <= 0 ) {
				printf(" invalid value. ( w: %d, h: %d )\n", width, height );
				continue;
			}

			pDspInfo->width			= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropWidth : pVipInfo->outWidth;
			pDspInfo->height		= (pVipInfo->mode == VIP_MODE_CLIPPER) ? pVipInfo->cropHeight : pVipInfo->outHeight;

			pDspInfo->dspSrcRect.left	= 0;
			pDspInfo->dspSrcRect.top	= 0;
			pDspInfo->dspSrcRect.right	= pDspInfo->width;
			pDspInfo->dspSrcRect.bottom	= pDspInfo->height;

			printf(" decimator : w(%d), h(%d)\n", pVipInfo->outWidth, pVipInfo->outHeight );
			continue;
		}
	}
}

static void print_menu( void )
{
	int32_t len = sizeof(appMenu) / sizeof(APP_MENU);
	int32_t i = 0;

  	printf("\n");
	printf("+------------------------------------------------------+\n");
	printf("  VIP Test Application (Build : %s, %s)                 \n", __TIME__, __DATE__);
	printf("+------------------------------------------------------+\n");
	for( i = 0; i < len; i++) printf(" %s                           \n", appMenu[i].usage);
	printf("+------------------------------------------------------+\n");
}

//----------------------------------------------------------------------------------------------------
//
//	Main Loop.
//
int main( int argc, char *argv[] )
{
	APP_DATA appData;

	char buf[256];
	int32_t i = 0;
	int32_t len = sizeof(appMenu) / sizeof(APP_MENU);

	register_signal();
	def_param( &appData );
	print_menu();

	while(1)
	{
		printf("#> ");
		fgets(buf, sizeof(buf), stdin);

		if( 'q' == buf[0] ) {
			printf("Exit.\n");
			break;
		} 
		else if( 'h' == buf[0] ) {
			print_menu();
			continue;
		}
		else if( 0x0A == buf[0] ) {
			continue;
		}
		else {
			for( i = 0; i < len; i++)
			{
				if (0 == strncmp(buf, appMenu[i].cmd, strlen(buf)-1)) {
					if( appMenu[i].func ) {
						appMenu[i].func(&appData);
						print_menu();
						break;
					}
				}
			}		
		}
	}

	return 0;
}

