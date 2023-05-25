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

#ifndef __CNX_MP4MUXERFILTER_H__
#define __CNX_MP4MUXERFILTER_H__

#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>
#include <NX_MP4Mux.h>

#define CODEC_MPEG4			0x20
#define CODEC_H264			0x21
#define CODEC_MP3			0x6B
#define CODEC_AAC			0x40

#define OPTION_SYNC

#ifdef __cplusplus

#define MAX_USERDATA_SIZE	256
#define MAX_FILE_NAME_SIZE	1024

class CNX_Mp4MuxerFilter
		: public CNX_BaseFilter
{
public:
	CNX_Mp4MuxerFilter();
	virtual ~CNX_Mp4MuxerFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_MP4MUXER_CONFIG *pConfig );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );

protected:
	//------------------------------------------------------------------------
	virtual void		AllocateMemory( void );
	virtual void		FreeMemory( void );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

			
private:
	//------------------------------------------------------------------------
	//	File Writer Callback
	static	void		FileWriter( void *pObj, unsigned char *pBuffer, int bufSize );
	static	int			GetBuffer( void *pObj, unsigned char **pBuffer, int *bufSize );
	//------------------------------------------------------------------------
			int32_t		(*FileNameCallbackFunc)( uint8_t *buf, uint32_t bufSize );
			int32_t		MuxEncodedSample(CNX_MuxerSample *pSample);
			int32_t		SetMuxConfig( void );
			int32_t		StartMuxing( void );
			int32_t		StopMuxing( void );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t		SetFileName( const char *fileName );
			int32_t		EnableMp4Muxing( bool enable );
			int32_t		SetDsiInfo( uint32_t trackID, uint8_t *dsiInfo, int32_t dsiSize );

			int32_t		RegFileNameCallback( int32_t (*cbFunc)(uint8_t *, uint32_t) );
			int32_t		GetFileNameFromCallback( void );
			
private:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	CNX_Semaphore		*m_pSemStream;
	CNX_Semaphore		*m_pSemWriter;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;
	//------------------------------------------------------------------------
	//	MP4 Muxer
	//------------------------------------------------------------------------
	NX_HANDLE			m_hMp4Mux;
	int32_t				m_bEnableMux;
	
	//	MP4 Mux Config
	NX_MP4MUXER_CONFIG	m_MP4Config;
	MP4_STREAM_INFO		m_TrackInfo[MAX_VID_NUM + MAX_AUD_NUM + MAX_TXT_NUM];
	MP4MUX_TRACK_INFO	m_Mp4TrackInfo[MAX_VID_NUM + MAX_AUD_NUM + MAX_TXT_NUM];

	//  MP4 Decoder Specific Infomation
	enum { MAX_VID_DSI_SIZE = 22,MAX_AUD_DSI_SIZE = 2 };
	uint8_t				m_TrackDsiInfo[MAX_VID_NUM + MAX_AUD_NUM][MAX_VID_DSI_SIZE];

	int32_t				m_bTrackStart[MAX_VID_NUM + MAX_AUD_NUM + MAX_TXT_NUM];
	int32_t				m_bStartMuxing;
	uint64_t			m_MuxStartTime;	//	1000 msec
	//------------------------------------------------------------------------
	//	Input / Output Buffer (File Handling)
	//------------------------------------------------------------------------
	enum { WRITING_MODE_NORMAL, WRITING_MODE_EVENT };
	enum { NUM_STRM_BUFFER = 8, SIZE_WRITE_UNIT = 1024*1024 };

	int32_t				m_OutFd;

	CNX_BufferQueue		m_StreamQueue;
	CNX_BufferQueue		m_WriterQueue;
	
	uint8_t				*m_pStreamBuffer[NUM_STRM_BUFFER];
	uint8_t				*m_pTrackTempBuf[MAX_VID_NUM + MAX_AUD_NUM + MAX_TXT_NUM];
	uint8_t				m_FileName[MAX_FILE_NAME_SIZE];
	uint8_t				m_PrvFileName[MAX_FILE_NAME_SIZE];
	
	int32_t				m_Flags;
	int32_t 			m_WritingMode;
	//------------------------------------------------------------------------
	//	Writer Mutex Lock
	//------------------------------------------------------------------------
	pthread_mutex_t		m_hWriteLock;
};

#endif //	__cplusplus

#endif // __CNX_MP4MUXERFILTER_H__

