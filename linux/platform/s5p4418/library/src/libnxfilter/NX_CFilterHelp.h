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
#ifndef __NX_CFilterHelp_h__
#define __NX_CFilterHelp_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include "NX_IBaseFilter.h"
#include "NX_CDemuxFFmpegFilter.h"
#include "NX_CVideoDecoderFilter.h"
#include "NX_CVideoRenderFilter.h"
#include "NX_CAudioDecoderFFmpegFilter.h"
#include "NX_CAudioRenderFilter.h"
#include "NX_CAudioConvertFFmpegFilter.h"

#include <pthread.h>

//---------------------------------------
#define FILTER_PIPE_NUM			6
#define ARR_IBASEFILTER_NUM		6
typedef struct FilterPipe {
	NX_IBaseFilter *pArrIBaseFilter[ARR_IBASEFILTER_NUM];
	int32_t		Num;
} FilterPipe;

typedef struct _FilterPipeMag {
	FilterPipe AudioFilterPipe[FILTER_PIPE_NUM];
	FilterPipe VideoFilterPipe[FILTER_PIPE_NUM];
	int32_t		APipeNum;
	int32_t		VPipeNum;
} FilterPipeMag;
//---------------------------------------

//Filter ID
#define DEMUX_FILTER_ID				"Demux"
#define VIDEODECODER_FILTER_ID		"VideoDecode"
#define VIDEORENDER_FILTER_ID		"VideoRender"
#define AUDIODECODER_FILTER_ID		"AudioDecode"
#define AUDIORENDER_FILTER_ID		"AudioRender"
#define AUDIOCONVERT_FILTER_ID		"AudioConvert"

//Filter Name
#define DEMUX_FILTER_NAME			"DemuxFilter"
#define VIDEODECODER_FILTER_NAME	"VideoDecFilter"
#define VIDEORENDER_FILTER_NAME		"VideoRenderFilter"
#define AUDIODECODER_FILTER_NAME	"AudioDecFilter"
#define AUDIORENDER_FILTER_NAME		"AudioRenderFilter"
#define AUDIOCONVERT_FILTER_NAME	"AudioConvertFilter"

//InterFace
#define IDEMUX_CONTROL			"IDemuxControl"
#define IMEDIA_CONTROL			"IMediaControl"
#define IVIDEORENDER_CONTROL	"IVideoRenderControl"
#define IAUDIOCONVERT_CONTROL	"IAudioConvertControl"

void FilterRegister(const char *FilterNameID, NX_IBaseFilter *(*Creater)(const char *));
NX_IBaseFilter *FilterCreate(const char *FilterNameID, const char *FilterName);
int32_t FilterDelete(NX_IBaseFilter *pFilter);
CBOOL ConnectPin(NX_CBasePin *pOutPin, NX_CBasePin *pInPin, void *Mediatype);

int32_t	AllFilterCreate(FilterPipeMag	*pFilterPipeMag,
	int32_t ApinNum, int32_t VpinNum,
	int32_t ApinSelect, int32_t VpinSelect);
int32_t ConnectFilter(NX_IBaseFilter *pFilter_1, NX_IBaseFilter *pFilter_2);

#endif	//	__cplusplus

#endif	//	__NX_CFilterHelp_h__
