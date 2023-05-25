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

#ifndef __CNX_USERDATAFILTER_H__
#define __CNX_USERDATAFILTER_H__

#include <CNX_BaseFilter.h>
#include <CNX_RefClock.h>
#include <NX_FilterConfigTypes.h>

#ifdef __cplusplus

#define MAX_USERDATA_SIZE		256

class	CNX_UserDataFilter
		: public CNX_BaseFilter
{
public:
	CNX_UserDataFilter(void);
	virtual ~CNX_UserDataFilter(void);

public:
	//------------------------------------------------------------------------
	virtual void		Init(NX_USERDATA_CONFIG *pConfig);
	virtual void		Deinit(void);
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive(CNX_Sample *pSample);
	virtual int32_t		ReleaseSample(CNX_Sample *pSample);

	virtual	int32_t		Run(void);
	virtual	int32_t		Stop(void);
	//------------------------------------------------------------------------

protected:
	virtual void		AllocateBuffer( int32_t numOfBuffer );
	virtual void		FreeBuffer(void);

	virtual	int32_t		GetSample(CNX_Sample **ppSample);			//	Get Sample From Input Queue
	virtual	int32_t		GetDeliverySample(CNX_Sample **ppSample);	//	Get Sample From Output Queue

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			void		GetUserDataFromCallback( CNX_MuxerSample *pSample );
			void		RegUserDataCallback( int32_t(*cbFunc)( uint8_t *, uint32_t ) );
			int32_t		SetPacketID( uint32_t PacketID );
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
	//	Output sample
	//------------------------------------------------------------------------
	uint32_t			m_PacketID;
	//------------------------------------------------------------------------
	//	Userdata
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 128 };
	CNX_MuxerSample		m_UserSample[MAX_BUFFER];
	uint8_t				m_UserBuffer[MAX_BUFFER][MAX_USERDATA_SIZE];
	
	uint32_t			m_UserBufferSize;
	uint32_t			m_UserInterval;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	int32_t				m_iNumOfBuffer;
	CNX_SampleQueue		m_SampleOutQueue;
	//------------------------------------------------------------------------
	//	Application callback functions for userdata
	//------------------------------------------------------------------------
	int32_t				(*UserDataCallbackFunc)( uint8_t *userDataBuf, uint32_t bufSize );
	//------------------------------------------------------------------------
	//	Statistics Infomation
	//------------------------------------------------------------------------
	pthread_mutex_t			m_hStatisticsLock;
	NX_FILTER_STATISTICS	m_FilterStatistics;
};

#endif //	__cplusplus

#endif // __CNX_USERDATAFILTER_H__

