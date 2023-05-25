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

#ifndef __CNX_AUDCAPTUREFILTER_H__
#define __CNX_AUDCAPTUREFILTER_H__

#include <CNX_BaseFilter.h>
#include <CNX_RefClock.h>
#include <NX_FilterConfigTypes.h>

#include <alsa/asoundlib.h>

#ifdef __cplusplus

class	CNX_AudCaptureFilter
		: public CNX_BaseFilter
{
public:
	CNX_AudCaptureFilter();
	virtual ~CNX_AudCaptureFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_AUDCAPTURE_CONFIG *pConfig );
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
	virtual void		AllocateBuffer( int32_t numOfBuffer );
	virtual void		FreeBuffer( void );

	virtual	int32_t		GetSample( CNX_Sample **ppSample );
	virtual	int32_t		GetDeliverySample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

private:
			int32_t		InitAudCapture( uint32_t channels, uint32_t frequency );
			int32_t		CaptureAudioSample( CNX_MediaSample *pSample );
			void		CloseAudCapture( void );
public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t  	GetStatistics( NX_FILTER_STATISTICS *pStatistics );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	CNX_Semaphore		*m_pSemOut;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;
	//------------------------------------------------------------------------
	//	Audio Sample
	//------------------------------------------------------------------------
	uint32_t 			m_hDevice;
	uint32_t			m_Channels;
	uint32_t			m_Frequency;
	uint32_t			m_Samples;
	uint64_t			m_PrevAudioSampleTime;
	uint64_t			m_TotalReadSampleSize;
	int64_t				m_ClockCorrectThreshold;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 128 };
	
	uint8_t				*m_pSampleBuffer;
	int32_t				m_iNumOfBuffer;
	CNX_MediaSample		m_AudioSample[MAX_BUFFER];
	CNX_SampleQueue		m_SampleOutQueue;
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	CNX_Statistics		*m_pOutStatistics;
	
private:
	snd_pcm_t			*m_hAudCapture;
};

#endif //	__cplusplus

#endif // __CNX_AUDCAPTUREFILTER_H__

