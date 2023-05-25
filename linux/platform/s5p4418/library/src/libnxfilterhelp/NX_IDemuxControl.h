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
#ifndef __NX_IDemuxControl_h__
#define __NX_IDemuxControl_h__

#include "NX_MediaType.h"

#ifdef __cplusplus


#include <pthread.h>

typedef struct  _InPinType{
	int32_t APinNum;
	int32_t VPinNum;
}InPinType;
typedef struct  _OutPinType{
	int32_t APinNum;
	int32_t VPinNum;
}OutPinType;
typedef struct  _PinType{
	InPinType InPinNum;
	OutPinType OutPinNum;
}PinType;

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell IDemuxControl Interface
class NX_IDemuxControl
{
public:
	NX_IDemuxControl(){}
	virtual ~NX_IDemuxControl(){}

public:
	virtual CBOOL	SetFileName(const char *pBuf) = 0;		 		//	Pure Virtual
	virtual CBOOL	PinCreate() = 0;		 						//	Pure Virtual
	virtual CBOOL	SelectPin(int32_t VPin, int32_t Apin) = 0;		 		//	Pure Virtual
	virtual CBOOL	PinInfo(PinType *pininfo) = 0;		 			//	Pure Virtual
	virtual int32_t	GetMediaInfo(MEDIA_INFO * pMediaInfo) = 0;
	virtual CBOOL   GetMediaType(void *MediaType, int32_t Type) = 0;
	virtual CBOOL	SeekStream(int64_t seekTime) = 0;
private:
};
//
//////////////////////////////////////////////////////////////////////////////



#endif	//	__cplusplus

#endif	//	__NX_IDemuxControl_h__
