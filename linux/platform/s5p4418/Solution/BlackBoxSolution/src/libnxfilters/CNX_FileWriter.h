
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

#ifndef __CNX_FILEWRITER_H__
#define __CNX_FILEWRITER_H__

#include <pthread.h>
#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>

#ifdef __cplusplus

#define MAX_FILE_NAME_SIZE		1024

class	CNX_FileWriter
		: public CNX_BaseFilter
{
public:
	CNX_FileWriter();
	virtual ~CNX_FileWriter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( void );
	virtual void		Deinit();
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );
	//------------------------------------------------------------------------

protected:
	virtual	void		AllocateBuffer( void );
	virtual	void		FreeBuffer( void );
	//------------------------------------------------------------------------
			void		ThreadLoop();
	static 	void*		ThreadMain( void *arg );


private:
			int32_t		StartWriting( void );
			int32_t		StopWriting( void );

			int32_t		(*FileNameCallbackFunc)( uint8_t *buf, uint32_t bufSize );
			int32_t		GetFileNameFromCallback( void );
			
public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t		RegFileNameCallback( int32_t (*cbFunc)(uint8_t *, uint32_t) );
			int32_t		EnableWriting( bool enable );
			int32_t		SetWritingMode( int32_t mode );
			uint64_t	GetTimeStamp( void );
			int32_t  	GetStatistics( NX_FILTER_STATISTICS *pStatistics );
protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;

	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t 			m_bThreadExit;
	pthread_t			m_hThread;

	//------------------------------------------------------------------------
	//	File Handling
	//------------------------------------------------------------------------
	enum { WRITING_MODE_NORMAL, WRITING_MODE_EVENT };
	enum {
		MAX_NUM_MEDIA_SAMPLES	= 32,
		NUM_WRITE_UNIT			= 32,
		SIZE_WRITE_UNIT			= 1024*1024,
	};

	int32_t				m_OutFd;
	//------------------------------------------------------------------------
	//	Buffer & Samples
	//------------------------------------------------------------------------
	uint8_t				m_FileName[1024];

	uint8_t				*m_pStreamBuffer[MAX_NUM_MEDIA_SAMPLES];
	CNX_BufferQueue		m_StreamQueue;
	CNX_BufferQueue		m_WriterQueue;
	CNX_Semaphore		*m_pSemWriter;

	uint8_t				*m_CurWriteBuffer;
	int32_t 			m_CurWritePos;

	int32_t 			m_WritingMode;
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	CNX_Statistics			*m_pStatistics;
	CNX_Statistics			*m_pStreamBuf, *m_pWriterBuf;
};

#endif //	__cplusplus

#endif //	__CNX_FILEWRITER_H__

