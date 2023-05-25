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

#include <stdint.h>
#include "NX_FilterConfigTypes.h"
#include "CNX_BaseFilter.h"

#ifndef __CNX_IMAGEEFFECTFILTER_H__
#define __CNX_IMAGEEFFECTFILTER_H__

class CNX_ImageEffectFilter
	: public CNX_BaseFilter
{
public:
	CNX_ImageEffectFilter();
	~CNX_ImageEffectFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_IMAGE_EFFECT_CONFIG *pConfig );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual int32_t		Run( void );
	virtual int32_t		Stop( void );
	//------------------------------------------------------------------------

protected:
	virtual void		AllocateBuffer(int32_t width, int32_t height, int32_t alignx, int32_t aligny, int32_t numOfBuffer, uint32_t dwFourCC);
	virtual void		FreeBuffer( void );
	
	virtual	int32_t		GetSample(CNX_Sample **ppSample);
	virtual int32_t		GetDeliverySample(CNX_Sample **pSample);	//	Get Sample From Output Queue
	
protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void* arg );

private:
			int32_t		(*ImageEffectCallbackFunc)( NX_VID_MEMORY_INFO *pSrcMemory, NX_VID_MEMORY_INFO *pDstMemory );
			int32_t		GetImageEffectFromCallback( NX_VID_MEMORY_INFO *pSrcMemory, NX_VID_MEMORY_INFO *pDstMemory );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			void 		RegImageEffectCallback( int32_t(*cbFunc)( NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO *) );
			int32_t  	GetStatistics( NX_FILTER_STATISTICS *pStatistics );
protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t					m_bInit;
	int32_t					m_bRun;
	CNX_Semaphore			*m_pSemIn;
	CNX_Semaphore			*m_pSemOut;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t					m_bThreadExit;
	pthread_t				m_hThread;	
	//------------------------------------------------------------------------
	//	Image Effect
	//------------------------------------------------------------------------
	NX_IMAGE_EFFECT_CONFIG	m_ImageEffectConfig;
	uint32_t				m_FourCC;

	//------------------------------------------------------------------------
	//	Input / Output buffer
	//------------------------------------------------------------------------
	enum {	MAX_BUFFER = 16, NUM_ALLOC_BUFFER = 12 };

	int32_t					m_iNumOfBuffer;
	CNX_SampleQueue			m_SampleInQueue;			// Input queue
	CNX_SampleQueue			m_SampleOutQueue;			// Output queue
	NX_VID_MEMORY_HANDLE	m_VideoMemory[MAX_BUFFER];	// Video Memory Allocate
	CNX_VideoSample			m_VideoSample[MAX_BUFFER];
};	

#endif	//	__CNX_IMAGEEFFECTFILTER_H__ 