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

#ifndef __CNX_AACECODER_H__
#define __CNX_AACECODER_H__

#include "CNX_BaseFilter.h"
#include "NX_FilterConfigTypes.h"

#include <nx_aac_encoder.h>

#ifdef __cplusplus

class	CNX_AacEncoder
		: public CNX_BaseFilter
{
public:
	CNX_AacEncoder();
	virtual ~CNX_AacEncoder();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_AUDENC_CONFIG *pConfig );
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
			int32_t		EncodeAudio( CNX_MediaSample *pInSample, CNX_MuxerSample *pOutSample );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t		SetPacketID( uint32_t PacketID );
			int32_t		GetDsiInfo( uint8_t *dsiInfo, int32_t *dsiSize );
			int32_t  	GetStatistics( NX_FILTER_STATISTICS *pStatistics );
			
protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	CNX_Semaphore		*m_pSemIn;
	CNX_Semaphore		*m_pSemOut;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;
	//------------------------------------------------------------------------
	//	Output sample
	//------------------------------------------------------------------------
	uint32_t			m_PacketID;
	//------------------------------------------------------------------------
	//	AAC Audio Encoder
	//------------------------------------------------------------------------
	AacEncHandle		m_hAACenc;
	uint32_t			m_Channels;
	uint32_t			m_Frequency;
	uint32_t			m_Bitrate;
	uint32_t			m_bAdts;

	enum { MAX_ESDS_SIZE = 39 };
	uint8_t				m_EsdsInfo[MAX_ESDS_SIZE];
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 32, NUM_ALLOC_BUFFER = 16 };

	int32_t				m_iNumOfBuffer;
	unsigned char		m_OutBuf[MAX_BUFFER][9212];
	CNX_MuxerSample		m_OutSample[MAX_BUFFER];
	CNX_SampleQueue		m_SampleInQueue;	//	Input queue
	CNX_SampleQueue		m_SampleOutQueue;	//  Output queue
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	CNX_Statistics		*m_pInStatistics;
	CNX_Statistics		*m_pOutStatistics;
};

#endif //	__cplusplus

#endif // __CNX_AACECODER_H__

