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

#ifndef __CNX_TSMUXERFILTER_H__
#define __CNX_TSMUXERFILTER_H__

#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>
#include <NX_TSMux.h>

#ifdef __cplusplus

#define MAX_PROGRAM_NUMBER			3		// Program Total Number == Video Total Number
#define MAX_PROGRAM_PID_LIST		4		// Video 2EA + Audio 1EA + User 1EA (MAX)

#define PRGORAM_NUMBER_BASE			0x01

#define PMT_PID_BASE				0x10

#define VIDEO_PID_BASE				0x20
#define AUDIO_PID_BASE				0x30
#define USER_PID_BASE				0x40

class CNX_TsMuxerFilter
		: public CNX_BaseFilter
{
public:
	CNX_TsMuxerFilter();
	virtual ~CNX_TsMuxerFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_TSMUXER_CONFIG *pConfig );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );
	//------------------------------------------------------------------------

protected:
			uint64_t	GetMuxerTime( uint64_t tickCounter );
			int32_t		MuxEncodedSample( CNX_MuxerSample *pSample );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	//------------------------------------------------------------------------
	//	MPEG2 TS Muxer
	//------------------------------------------------------------------------
	NX_HANDLE			m_hTsMuxer;
	NX_TSMUXER_CONFIG	m_MuxConfig;
	uint32_t			m_ProgramPidList[MAX_PROGRAM_NUMBER][MAX_PROGRAM_PID_LIST];
	//------------------------------------------------------------------------
	//	Writer Mutex Lock
	//------------------------------------------------------------------------
	pthread_mutex_t		m_hEncodeLock;
};

#endif //	__cplusplus

#endif // __CNX_TSMUXERFILTER_H__

