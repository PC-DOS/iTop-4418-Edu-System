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
#ifndef __NX_IAudioControlControl_h__
#define __NX_IAudioControlControl_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include <pthread.h>


//////////////////////////////////////////////////////////////////////////////
//
//						Nexell IAudioControlControl Interface
class NX_IAudioControl
{
public:
	NX_IAudioControl(){};
	virtual ~NX_IAudioControl(){};
	virtual CBOOL	SetVolume(int32_t Volume) = 0;		 		//	Pure Virtual
private:
};
//
//////////////////////////////////////////////////////////////////////////////



#endif	//	__cplusplus

#endif	//	__NX_IDemuxControl_h__
