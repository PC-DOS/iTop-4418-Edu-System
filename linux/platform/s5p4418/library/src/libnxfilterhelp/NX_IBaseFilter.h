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
#ifndef __NX_IBaseFilter_h__
#define __NX_IBaseFilter_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"

#include <pthread.h>

typedef struct _NX_PinInfo{
	int32_t	InPutNum;
	int32_t	OutPutNum;
}NX_PinInfo;



//////////////////////////////////////////////////////////////////////////////
//
//						Nexell IBaseFilter Interface
class NX_IBaseFilter
{
public:
	NX_IBaseFilter(){};
	virtual ~NX_IBaseFilter(){};
public:
	virtual CBOOL		PinInfo(NX_PinInfo *PinInfo) = 0;		 	//	Pure Virtual
	virtual NX_CBasePin	*FindPin(int Pos) = 0;		 				//	Pure Virtual
	virtual CBOOL		FindInterface(const char *InterfaceId, void **Interface) = 0;		//	Pure Virtual
	virtual CBOOL		Run() = 0;		 							//	Pure Virtual
	virtual CBOOL		Stop() = 0;		 							//	Pure Virtual
	virtual CBOOL		Pause() = 0;		 						//	Pure Virtual
	virtual void		SetClockReference(NX_CClockRef *pClockRef) = 0;
	virtual void		SetEventNotifi(NX_CEventNotifier *pEventNotifier) = 0;
};

//
//////////////////////////////////////////////////////////////////////////////



#endif	//	__cplusplus

#endif	//	__NX_IBaseFilter_h__
