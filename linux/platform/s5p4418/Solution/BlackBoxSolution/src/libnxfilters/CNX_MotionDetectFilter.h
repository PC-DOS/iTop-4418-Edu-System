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

#ifndef __CNX_MOTIONDETECTFILTER_H__
#define __CNX_MOTIONDETECTFILTER_H__

#include "CNX_BaseFilter.h"
#include "NX_FilterConfigTypes.h"

#ifdef __cplusplus
class CNX_MotionDetectFilter
	: public CNX_BaseFilter
{
public:
	CNX_MotionDetectFilter();
	virtual ~CNX_MotionDetectFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( NX_MOTION_DETECT_CONFIG *pConfig );
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
	virtual void		AllocateBuffer( void );
	virtual void		FreeBuffer( void );

	virtual	int32_t		GetSample( CNX_Sample **ppSample );
	virtual	int32_t		GetDeliverySample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

			int32_t		MotionDetect( NX_VID_MEMORY_INFO *pPrevMemory, NX_VID_MEMORY_INFO *pCurMemory );
			int32_t 	UserMotionDetect( NX_VID_MEMORY_INFO *pPrevMemory, NX_VID_MEMORY_INFO *pCurMemory );
			int32_t		(*MotionDetectCallbackFunc)( NX_VID_MEMORY_INFO *pPrevMemory, NX_VID_MEMORY_INFO *pCurMemory );

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			void		RegMotionDetectCallback( int32_t (*cbFunc)( NX_VID_MEMORY_INFO*, NX_VID_MEMORY_INFO* ) );
			int32_t		EnableMotionDetect( int32_t enable );

protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	int32_t				m_bEnabled;
	CNX_Semaphore		*m_pSemIn;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;
	//------------------------------------------------------------------------
	//	Motion Detect
	//------------------------------------------------------------------------
	int32_t				m_SamplingWidth;		// for InternalMotion
	int32_t				m_SamplingHeight;		// for InternalMotion
	int32_t				m_Threshold;			// for InternalMotion
	int32_t				m_Sensitivity;			// for InternalMotion
	int32_t				m_SamplingFrame;		// for UserMotion / for InternalMotion
	int32_t				m_SamplingFrameCnt;		// for UserMotion / for InternalMotion
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum {	MAX_BUFFER = 16 };

	CNX_SampleQueue			m_SampleInQueue;			//	Input queue
	CNX_VideoSample			*m_pPrevVideoSample;
};

#endif

#endif	// __CNX_MOTIONDETECTFILTER_H__