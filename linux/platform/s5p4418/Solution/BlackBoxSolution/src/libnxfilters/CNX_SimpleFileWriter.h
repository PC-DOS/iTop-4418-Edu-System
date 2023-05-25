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

#ifndef __CNX_SIMPLEFILEWRITER_H__
#define __CNX_SIMPLEFILEWRITER_H__

#include <pthread.h>
#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>

#define OPTION_SYNC

#ifdef __cplusplus

#define MAX_FILE_NAME_SIZE		1024

class	CNX_SimpleFileWriter
		: public CNX_BaseFilter
{
public:
	CNX_SimpleFileWriter();
	virtual ~CNX_SimpleFileWriter();

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

private:
			int32_t		StartWriting( void );
			int32_t		StopWriting( void );

			int32_t		(*FileNameCallbackFunc)( uint8_t *buf, uint32_t bufSize );
			void		GetFileNameFromCallback( void );
			
public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t		RegFileNameCallback( int32_t (*cbFunc)(uint8_t *, uint32_t) );
			int32_t		EnableWriting( bool enable );
			int32_t		SetWritingMode( int32_t mode );
			uint64_t	GetTimeStamp( void );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	//------------------------------------------------------------------------
	//	File Handling
	//------------------------------------------------------------------------
	int32_t				m_OutFd;
	pthread_mutex_t		m_hWriteLock;
	enum {
		WRITING_MODE_NORMAL,
		WRITING_MODE_EVENT,
	};
	int32_t				m_nWritingMode;
	int32_t				m_bEnableWriting;
	//------------------------------------------------------------------------
	//	Buffer & Samples
	//------------------------------------------------------------------------
	uint8_t				m_FileName[1024];

	pthread_mutex_t		m_hTimeLock;
	uint64_t			m_CurTimeStamp;
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	CNX_Statistics			*m_pStatistics;
};

#endif //	__cplusplus

#endif //	__CNX_SIMPLEFILEWRITER_H__

