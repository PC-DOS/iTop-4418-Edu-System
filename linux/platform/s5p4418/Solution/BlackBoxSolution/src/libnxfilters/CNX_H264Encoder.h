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

#ifndef __CNX_H264ENCODER_H__
#define __CNX_H264ENCODER_H__

#include <nx_video_api.h>

#include "CNX_BaseFilter.h"
#include "NX_FilterConfigTypes.h"

#ifdef __cplusplus

typedef NX_VID_ENC_INIT_PARAM		ENC_INFO;

class	CNX_H264Encoder
		: public CNX_BaseFilter
{
public:
	CNX_H264Encoder();
	virtual ~CNX_H264Encoder();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_VIDENC_CONFIG *pConfig );
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
	virtual void		AllocateBuffer( int numOfBuffer );
	virtual void		FreeBuffer(void);

	virtual	int32_t		GetSample(CNX_Sample **ppSample);
	virtual int32_t		GetDeliverySample(CNX_Sample **pSample);

protected:
			void		ThreadLoop(void);
	static 	void*		ThreadMain(void*arg);

private:
			int32_t		EncodeVideo( CNX_VideoSample *pInSample, CNX_MuxerSample *pOutSample );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t		GetDsiInfo( uint8_t *dsiInfo, int32_t *dsiSize );
			int32_t		SetPacketID( uint32_t packetID );
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
	//	H.264 Encoder
	//------------------------------------------------------------------------
	NX_VID_ENC_HANDLE	m_hEnc;
	ENC_INFO			m_EncInfo;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 32, NUM_ALLOC_BUFFER = 16, MAX_SEQ_BUF_SIZE = 4 * 1024 };
	
	int32_t				m_iNumOfBuffer;
	NX_VID_ENC_OUT		m_OutBuf[MAX_BUFFER];
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

#endif // __CNX_H264ENCODER_H__

