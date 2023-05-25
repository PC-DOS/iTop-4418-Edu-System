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

#ifndef __CNX_VRFILTER_H__
#define __CNX_VRFILTER_H__

#include <CNX_BaseFilter.h>
#include <NX_FilterConfigTypes.h>

#include <nx_dsp.h>

#ifdef __cplusplus

class	CNX_VRFilter
	: public CNX_BaseFilter
{
public:
	CNX_VRFilter();
	virtual ~CNX_VRFilter();

public:
	//------------------------------------------------------------------------
	virtual void	Init( NX_VIDRENDER_CONFIG *pConfig );
	virtual void	Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t	Receive( CNX_Sample *pSample );
	virtual int32_t	ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t	Run( void );
	virtual	int32_t	Stop( void );
	//------------------------------------------------------------------------

public:
	//------------------------------------------------------------------------
	//	External Interfaces
	//------------------------------------------------------------------------
			int32_t	EnableRender( uint32_t enable );
			int32_t EnableHdmiRender( uint32_t enable );
			int32_t SetRenderCrop( DSP_IMG_RECT *pCropRect );
			int32_t SetRenderPosition( DSP_IMG_RECT *pDspRect );
			
protected:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	int32_t				m_bEnable;
	int32_t				m_bEnableHdmi;
	pthread_mutex_t		m_hLock;
	//------------------------------------------------------------------------
	//	Display
	//------------------------------------------------------------------------
	DISPLAY_HANDLE		m_hDsp;
	DISPLAY_INFO		m_DisplayInfo;
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	CNX_VideoSample		*m_pPrevVideoSample;
};

#endif //	__cplusplus

#endif //	__CNX_VRFILTER_H__

