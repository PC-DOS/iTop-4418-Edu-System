//------------------------------------------------------------------------------
//
//	Copyright (C) 2010 Nexell co., Ltd All Rights Reserved
//	Nexell Proprietary & Confidential
//
//	Module     : 
//	File       : 
//	Description:
//	Author     : RayPark
//	History    :
//------------------------------------------------------------------------------
#ifndef __NX_IVideoRenderControl_h__
#define __NX_IVideoRenderControl_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include <pthread.h>
//////////////////////////////////////////////////////////////////////////////
//
//						Nexell IVideoRenderControl Interface
class NX_IDisplayControl
{
public:
	NX_IDisplayControl(){};
	virtual ~NX_IDisplayControl(){};
	virtual CBOOL	SetVideoPosition(int moudleId, int port, int x, int y, int width, int height) = 0;		 		//	Pure Virtual
	virtual CBOOL	SetDisplay(int32_t LCD_On_OFF, int32_t HDMI_ON_FF) = 0;		 									//	Pure Virtual
};
//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__cplusplus

#endif	//	__NX_IVideoRenderControl_h__
