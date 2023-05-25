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

#ifndef __CNX_DVRMANAGER_H__
#define __CNX_DVRMANAGER_H__

#include <stdint.h>
#include <pthread.h>
#include <nx_alloc_mem.h>

#include <NX_FilterConfigTypes.h>
#include "NX_DvrControl.h"

#define VERSION_MAJOR		1
#define VERSION_MINOR		0
#define VERSION_REVISION	0

//#define SIMPLE_WRITER

class CNX_RefClock;
class CNX_DvrNotify;
class CNX_VIPFilter;
class CNX_ImageEffectFilter;
class CNX_TextOverlayFilter;
class CNX_VRFilter;
class CNX_H264Encoder;
class CNX_AudCaptureFilter;
class CNX_AacEncoder;
class CNX_Mp3Encoder;
class CNX_UserDataFilter;
class CNX_InterleaverFilter;
class CNX_BufferingFilter;
class CNX_Mp4MuxerFilter;
class CNX_TsMuxerFilter;
class CNX_FileWriter;
class CNX_SimpleFileWriter;
class CNX_HLSFilter;
class CNX_RTPFilter;
class CNX_MotionDetectFilter;

enum {
	MP4_CODEC_TYPE_MPEG4	= 0x20,
	MP4_CODEC_TYPE_H264		= 0x21,
	MP4_CODEC_TYPE_AAC		= 0x40,
	MP4_CODEC_TYPE_MP3		= 0x6B,
};

enum {
	TS_CODEC_TYPE_MPEG4		= 0x10,
	TS_CODEC_TYPE_H264		= 0x1B,
	TS_CODEC_TYPE_MP3		= 0x04,
	TS_CODEC_TYPE_AAC		= 0x0F,
};

enum {
	FRAME_SIZE_AAC			= 1024,
	FRAME_SIZE_MP3			= 1152,
};

typedef enum {
	DVR_STATUS_NORMAL_START,
	DVR_STATUS_NORMAL_WAIT,
	DVR_STATUS_NORMAL_EVENT,
	DVR_STATUS_NORMAL_EVENT_WAIT,
	DVR_STATUS_MOTION_START,
	DVR_STATUS_MOTION_WAIT,
	DVR_STATUS_MOTION_EVENT,
	DVR_STATUS_MOTION_EVENT_WAIT,
} DVR_STATUS;

typedef struct tag_FILTER_STATISTICS
{
	NX_FILTER_STATISTICS	vip[MAX_VID_NUM];
	NX_FILTER_STATISTICS	vidEnc[MAX_VID_NUM];
	NX_FILTER_STATISTICS	audCapture[MAX_AUD_NUM];
	NX_FILTER_STATISTICS	audEnc[MAX_AUD_NUM];
	NX_FILTER_STATISTICS	userData;
	NX_FILTER_STATISTICS	buffering;
	NX_FILTER_STATISTICS	fileWriter;
} FILTER_STATISTICS;

class CNX_DvrManager
{
public:
	CNX_DvrManager();
	~CNX_DvrManager();

public:
	// Control Function
	int32_t Init( NX_DVR_MEDIA_CONFIG *pMediaConfig, NX_DVR_RECORD_CONFIG *pRecordConfig, NX_DVR_DISPLAY_CONFIG *pDisplayConfig );
	int32_t Deinit( void );

	int32_t Start( NX_DVR_ENCODE_TYPE encodeType );
	int32_t Stop( void );

	int32_t SetEvent( void );
	int32_t SetCapture( int32_t channel );
	int32_t SetDisplay( NX_DVR_DISPLAY_CONFIG *pDisplayConfig );
	int32_t SetPreview( int32_t channel );
	int32_t SetPreviewHdmi( int32_t channel );
	
	int32_t ChangeMode( NX_DVR_ENCODE_TYPE mode );
	// Control Function ( Not Implemetation )
	
	//int32_t GetStatisctics( void );

	// Callback Function
	int32_t RegisterGetFileNameCallback( 
		int32_t (*cbNormalFileName)	(uint8_t*, uint32_t),
		int32_t (*cbEventFileName)		(uint8_t*, uint32_t),
		int32_t (*cbParkingFileName)	(uint8_t*, uint32_t),
		int32_t (*cbJpegFileName)		(uint8_t*, uint32_t)
	);
	int32_t	RegisterUserDataCallback(
		int32_t (*cbUserData)			(uint8_t*, uint32_t)
	);
	int32_t	RegisterTextOverlayCallback(
		int32_t (*cbFrontTextOverlay)	(uint8_t*, uint32_t*, uint32_t*, uint32_t*),
		int32_t (*cbRearTextOverlay)	(uint8_t*, uint32_t*, uint32_t*, uint32_t*)
	);
	int32_t RegisterNotifyCallback(
		int32_t (*cbNotify)				(uint32_t, uint8_t*, uint32_t)
	);
	int32_t RegisterImageEffectCallback( 
		int32_t (*cbFrontImageEffect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *),
		int32_t (*cbRearImageEffect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *)
	);
	int32_t RegisterMotionDetectCallback(
		int32_t (*cbFrontMotionDetect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *),
		int32_t (*cbRearMotionDetect)	(NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *)
	);

	void 			GetAPIVersion( int32_t *pMajor, int32_t *pMinor, int32_t *pRevision, uint8_t *pBuildDate = NULL, uint8_t *pBuildTime = NULL, uint8_t *pBuildAuthor = NULL );
	void			ChangeDebugLevel( int32_t dbgLevel );
	void 			GetStatistics( FILTER_STATISTICS *pFilterStatistics );

private:
	int32_t			BuildFilter( void );
	int32_t			SetConfig( NX_DVR_MEDIA_CONFIG *pMediaConfig, NX_DVR_RECORD_CONFIG *pRecordConfig, NX_DVR_DISPLAY_CONFIG *pDisplayConfig );
	void			SetNotifier( void );
	
	int32_t			StartManager( void );
	int32_t			StopManager( void );

	void			ThreadLoop( void );
	static void*	ThreadMain( void* arg );

	void			StartNormalWriting( void );
	void			StartMotionWriting( void );
	void 			ChangeNormalWriting( void );
	void 			ChangeEventWriting( void );
	void			StartWriting( void );
	void 			StopWriting( void );

private:
	CNX_RefClock			*m_pRefClock;
	CNX_DvrNotify			*m_pNotifier;

	CNX_VIPFilter			*m_pVipFilter[MAX_VID_NUM];
	CNX_ImageEffectFilter	*m_pEffectFilter[MAX_VID_NUM];
	CNX_MotionDetectFilter 	*m_pMdFilter[MAX_VID_NUM];
	CNX_TextOverlayFilter	*m_pOverlayFilter[MAX_VID_NUM];
	CNX_VRFilter			*m_pVrFilter[MAX_VID_NUM];
	CNX_H264Encoder			*m_pAvcEncFilter[MAX_VID_NUM];
	//CNX_Mp4EncFilter		*m_pMp4EncFilter[MAX_VID_NUM];

	CNX_AudCaptureFilter	*m_pAudCapFilter[MAX_AUD_NUM];
	CNX_AacEncoder			*m_pAacEncFilter[MAX_AUD_NUM];
	CNX_Mp3Encoder			*m_pMp3EncFilter[MAX_AUD_NUM];

	CNX_UserDataFilter		*m_pUserDataFilter[MAX_TXT_NUM];

	CNX_InterleaverFilter	*m_pInterleaverFilter;
	CNX_BufferingFilter		*m_pBufferingFilter;
	
	CNX_Mp4MuxerFilter		*m_pMp4MuxerFilter;
	CNX_TsMuxerFilter		*m_pTsMuxerFilter;
#ifdef SIMPLE_WRITER	
	CNX_SimpleFileWriter	*m_pFileWriter;
#else	
	CNX_FileWriter			*m_pFileWriter;
#endif
	CNX_HLSFilter			*m_pHlsFilter;
	CNX_RTPFilter			*m_pRtpFilter;

	// Configuration
	NX_VIP_CONFIG			m_VipConfig[MAX_VID_NUM];
	NX_MOTION_DETECT_CONFIG	m_MdConfig[MAX_VID_NUM];
	NX_IMAGE_EFFECT_CONFIG	m_EffectConfig[MAX_VID_NUM];
	NX_VIDRENDER_CONFIG		m_VidRenderConfig[MAX_VID_NUM];
	NX_VIDENC_CONFIG		m_VidEncConfig[MAX_VID_NUM];

	NX_AUDCAPTURE_CONFIG	m_AudCapConfig[MAX_AUD_NUM];
	NX_AUDENC_CONFIG		m_AudEncConfig[MAX_AUD_NUM];

	NX_USERDATA_CONFIG		m_UserDataConfig[MAX_TXT_NUM];

	NX_MP4MUXER_CONFIG		m_Mp4MuxerConfig;

	NX_TSMUXER_CONFIG		m_TsMuxerConfig;

	NX_INTERLEAVER_CONFIG	m_InterleaverConfig;
	NX_BUFFERING_CONFIG		m_BufferingConfig;

	NX_HLS_CONFIG			m_HlsConfig;
	NX_RTP_CONFIG			m_RtpConfig;

private:
	int32_t					m_bInit;
	int32_t					m_bRun;
	
	int32_t					m_nMode;
	int32_t					m_bChageMode;
	int32_t					m_bEvent;

	int32_t					m_VideoNum, m_AudioNum, m_TextNum;
	int32_t					m_VideoCodec, m_AudioCodec;
	int32_t					m_Container;
	int32_t					m_DisplayChannel;
	int32_t					m_ExternProc[MAX_VID_NUM];
	int32_t					m_NormalDuration, m_EventDuration, m_EventBufferDuration;
	uint32_t				m_DisplayEnable;
	uint32_t				m_HlsEnable;
	uint32_t				m_RtpEnable;
	uint32_t				m_MdEnable[MAX_VID_NUM];

	int32_t					m_bThreadExit;
	pthread_t				m_hThread;

	int32_t					(*NormalFileNameCallbackFunc)	( uint8_t *buf, uint32_t bufSize );
	int32_t					(*EventFileNameCallbackFunc)	( uint8_t *buf, uint32_t bufSize );
	int32_t					(*ParkingFileNameCallbackFunc)	( uint8_t *buf, uint32_t bufSize );

	FILTER_STATISTICS		m_FilterStatistics;	
	pthread_mutex_t			m_hLock;
};

#endif // __CNX_DVRMANAGER_H__
