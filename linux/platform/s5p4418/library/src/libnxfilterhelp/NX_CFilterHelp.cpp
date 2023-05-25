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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NX_CFilterHelp.h"
#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"
//#include "NX_IMediaControl.h"

#include "NX_CDemuxFFmpegFilter.h"
#include "NX_CVideoDecoderFilter.h"
#include "NX_CVideoRenderFilter.h"
#include "NX_CAudioDecoderFFmpegFilter.h"
#include "NX_CAudioRenderFilter.h"
#include "NX_CAudioConvertFFmpegFilter.h"

#define FILTER_ID_NUM			6
typedef struct  _FilterDB{
	const char *pFilterID;
	NX_IBaseFilter *(*Creater)(const char *);
}FilterDB;

//////////////////////////////////////////////////////////////////////////////
//
//							NX_CFilterHelp
//

//--------------------------------------------------------------
//               Filter_1                 Filter_2
//			    ----------      (inpin) ---------
//			    |        |---        ---|       |
//              ---------- (outpin)     --------- 
//				
//-------------------------------------------------------------
int32_t ConnectFilter(NX_IBaseFilter *pFilter_1, NX_IBaseFilter *pFilter_2)
{
	int32_t					ret = 0;
	NX_CBasePin				*pOutPin = NULL;
	NX_CBasePin				*pInPin = NULL;
	NX_MediaType			*pMediaType = NULL;
	NX_PinInfo				PinInfo;
	int32_t					i = 0;

	memset(&PinInfo, 0, sizeof(NX_PinInfo));
	pFilter_1->PinInfo(&PinInfo);

	if (PinInfo.OutPutNum > 1)	//Demux
	{	
		for (i = 0; i < PinInfo.OutPutNum; i++)
		{
			ret = 0;
			pOutPin = pFilter_1->FindPin(i);
			pInPin = pFilter_2->FindPin(PIN_DIRECTION_INPUT);
			if ((NULL != pOutPin) && (NULL != pInPin))
			{
				if ((CFALSE == pOutPin->IsConnected()) && (CFALSE == pInPin->IsConnected()))
				{
					pOutPin->Connect(pInPin);
					pInPin->Connect(pOutPin);
					pOutPin->GetMediaType(&pMediaType);
					ret = pInPin->CheckMediaType(pMediaType);
					if (0 != ret){
						//printf("==Error CheckMediaType = %d\n", ret);
						pOutPin->Disconnect();
						pInPin->Disconnect();
						continue;
						//return -1;
					}
					//printf("= Connect Scuess !!! = %d\n", ret);
					return 0;
				}
			}
			else
			{
				//printf("=Error pOutPin = %p, pInPin = %p\n", pOutPin, pInPin);
			}
		}
	}
	else
	{
		pOutPin = pFilter_1->FindPin(PIN_DIRECTION_OUTPUT);
		pInPin = pFilter_2->FindPin(PIN_DIRECTION_INPUT);
	}

	if ((NULL != pOutPin) && (NULL != pInPin))
	{
		if ((CFALSE == pOutPin->IsConnected()) && (CFALSE == pInPin->IsConnected()))
		{
			pOutPin->Connect(pInPin);
			pInPin->Connect(pOutPin);
			pOutPin->GetMediaType(&pMediaType);
			ret = pInPin->CheckMediaType(pMediaType);
			if (0 != ret){
				//printf("=Error CheckMediaType = %d\n", ret);
				return -1;
			}
			return 0;
		}
	}
	else
	{
		//printf("=Error pOutPin = %p, pInPin = %p\n", pOutPin, pInPin);
		return -1;
	}

	return 0;
}


int32_t	AllFilterCreate(FilterPipeMag	*pFilterPipeMag, 
					int32_t ApinNum, int32_t VpinNum,
					int32_t ApinSelect, int32_t VpinSelect)
{
	int32_t i = 0, j = 0;

//	printf("====ApinSelect = %d, ApinNum = %d", ApinSelect, ApinNum);

	if (ApinNum > 0)
	{
		for (i = 0; i < FILTER_PIPE_NUM; i++)
		{
			pFilterPipeMag->AudioFilterPipe[j].Num = 0;
			for (i = 0; i < ARR_IBASEFILTER_NUM; i++)
			{
				pFilterPipeMag->AudioFilterPipe[j].pArrIBaseFilter[i] = NULL;
			}
		}		
		pFilterPipeMag->APipeNum = ApinSelect;
		for (i = 0; i < ApinSelect; i++)
		{
			pFilterPipeMag->AudioFilterPipe[j].pArrIBaseFilter[pFilterPipeMag->AudioFilterPipe[i].Num] = CreateAudioDecoderFilter(AUDIODECODER_FILTER_NAME);
			pFilterPipeMag->AudioFilterPipe[j].Num++;
			pFilterPipeMag->AudioFilterPipe[j].pArrIBaseFilter[pFilterPipeMag->AudioFilterPipe[i].Num] = CreateAudioConvertFilter(AUDIOCONVERT_FILTER_NAME);
			pFilterPipeMag->AudioFilterPipe[j].Num++;
			pFilterPipeMag->AudioFilterPipe[j].pArrIBaseFilter[pFilterPipeMag->AudioFilterPipe[i].Num] = CreateAudioRenderFilter(AUDIORENDER_FILTER_NAME);
			pFilterPipeMag->AudioFilterPipe[j].Num++;
		}
		
	}

	if (VpinNum > 0)
	{
		for (i = 0; i < FILTER_PIPE_NUM; i++)
		{
			pFilterPipeMag->VideoFilterPipe[j].Num = 0;
			for (i = 0; i < ARR_IBASEFILTER_NUM; i++)
			{
				pFilterPipeMag->VideoFilterPipe[j].pArrIBaseFilter[i] = NULL;
			}
		}

		pFilterPipeMag->VPipeNum = VpinSelect;
		for (i = 0; i < VpinSelect; i++)
		{
			pFilterPipeMag->VideoFilterPipe[j].pArrIBaseFilter[pFilterPipeMag->VideoFilterPipe[i].Num] = CreateVideoDecoderFilter(VIDEODECODER_FILTER_NAME);
			pFilterPipeMag->VideoFilterPipe[j].Num++;
			pFilterPipeMag->VideoFilterPipe[j].pArrIBaseFilter[pFilterPipeMag->VideoFilterPipe[i].Num] = CreateVideoRenderFilter(VIDEORENDER_FILTER_NAME);
			pFilterPipeMag->VideoFilterPipe[j].Num++;
		}		
	}

	return 0;
}

#if 0
int32_t FilterDelete(NX_IBaseFilter *pFilter)
{
	if (NULL == pFilter){
		return -1;
	}

	delete pFilter;
		
	return 0;
}
#endif
//
//////////////////////////////////////////////////////////////////////////////
