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

#include "NX_CVideoDecoderFilter.h"
#include "NX_CodecID.h"


//////////////////////////////////////////////////////////////////////////////
//
//							Nexell VideoDecoder Filter
//
//							----------
//			(input pin)   --|        |--- (out putpin)
//							----------

#define MAX_VIDEO_NUM_BUF 30

NX_CVideoDecoderFilter::NX_CVideoDecoderFilter(const char *filterName)
	: m_pInQueue(NULL)
	, m_hCodec(NULL)
	, m_pInputPin(NULL)
	, m_pOutputPin(NULL)
	, m_Discontinuity(CFALSE)
	, m_bInit(0)
{
	m_pInputPin = new NX_CVideoDecoderInputPin(this);
	m_pInQueue = new NX_CSampleQueue(MAX_VIDEO_NUM_BUF);
	m_pOutputPin = new NX_CVideoDecoderOutputPin(this);

	memset(m_cArrFilterName, 0, sizeof(m_cArrFilterName));
	memset(m_cArrFilterID, 0, sizeof(m_cArrFilterID));
	SetFilterName(filterName);	

	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}

}

NX_CVideoDecoderFilter::~NX_CVideoDecoderFilter()
{
	if( m_pInputPin )	delete m_pInputPin;
	if( m_pOutputPin )	delete m_pOutputPin;
	if (m_pInQueue)		delete m_pInQueue;

	pthread_mutex_destroy(&m_hMutexThread);
}

NX_CBasePin *NX_CVideoDecoderFilter::GetPin(int Pos)
{
	if( PIN_DIRECTION_INPUT == Pos )
		return m_pInputPin;
	if( PIN_DIRECTION_OUTPUT == Pos )
		return m_pOutputPin;
	return NULL;
}

int32_t	NX_CVideoDecoderFilter::DecoderSample(NX_CSample *pSample)
{
	int32_t ret = -1;
	if (m_pInQueue)
	{
		pSample->AddRef();
		ret = m_pInQueue->PushSample(pSample);
		return ret;
	}
	return ret;
}

CBOOL NX_CVideoDecoderFilter::Run()
{
	if (CFALSE == m_bRunning)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec ] Run() ++\n");

		m_pInQueue->ResetQueue();
		if (m_pInputPin && m_pInputPin->IsConnected())
		{
			m_pInputPin->Active();
		}
		if (m_pOutputPin && m_pOutputPin->IsConnected())
		{
			m_pOutputPin->Active();
		}

		m_bPaused = CFALSE;
		m_bExitThread = CFALSE;

		if (pthread_create(&m_hThread, NULL, ThreadStub, this) < 0)
		{
			return CFALSE;
		}
		m_bRunning = CTRUE;

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec ] Run() --\n");
	}

	return CTRUE;
}

CBOOL NX_CVideoDecoderFilter::Stop()
{
	NX_DbgMsg( DBG_MSG_OFF, "[Filter|VDec ] Stop() ++\n" );

	//	Set Thread End Command
	ExitThread();
	//	Send Queue Emd Message
	if (m_pInQueue)
		m_pInQueue->EndQueue();

	if( m_pInputPin && m_pInputPin->IsConnected() )
	{
		m_pInputPin->Inactive();
	}
	if( m_pOutputPin && m_pOutputPin->IsConnected() )
	{
		m_pOutputPin->Inactive();
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec ] Stop() --\n");

	CloseDecoder(m_hCodec);

	m_bInit = 0;	

	if (m_VMediaType.pSeqData)
	{
		//free(m_VMediaType.pSeqData);
		//m_VMediaType.pSeqData = NULL;
	}

	if (CTRUE == m_bRunning)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] Stop() : pthread_join() ++\n");
		pthread_join(m_hThread, NULL);
		m_bRunning = CFALSE;
		m_hThread = 0;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] Stop() : pthread_join() --\n");
	}

	return CTRUE;
}

CBOOL NX_CVideoDecoderFilter::Pause()
{
	return CTRUE;
}


CBOOL NX_CVideoDecoderFilter::Flush()
{

	return CTRUE;
}

void *NX_CVideoDecoderFilter::ThreadStub(void *pObj)
{
	if (NULL != pObj)
	{
		((NX_CVideoDecoderFilter*)pObj)->ThreadProc();
	}
	return (void*)(0);
}

void NX_CVideoDecoderFilter::CloseDecoder(NX_VID_DEC_HANDLE hCodec)
{
	if (hCodec)
	{
		//NX_VidDecFlush(hCodec);
		NX_VidDecClose(hCodec);
	}
}

int NX_CVideoDecoderFilter::FlushDecoder(NX_VID_DEC_HANDLE hCodec)
{
	if (hCodec)
	{
		NX_VidDecFlush(hCodec);
	}

	return 0;
}


#define VDFILEWRITE	0
#if VDFILEWRITE	//File Write
static FILE	*hFile = NULL;
static int32_t count = 0;
static int32_t init = 0;
static int32_t i = 0;
#endif

void NX_CVideoDecoderFilter::ThreadProc()
{
	uint32_t *pInBuf = NULL;
	int32_t inSize = 0;
	NX_CSample *pInSample = NULL;
	NX_CSample *pOutSample = NULL;
	uint32_t *pOutBuf = NULL;

	VID_ERROR_E vidRet;
	NX_VID_DEC_IN decIn;
	NX_VID_DEC_OUT decOut;

	int32_t outCount = 0;
	int64_t timeStamp = -1; 

	if (!m_pInQueue)
		return;

#if VDFILEWRITE	//File Write
	if (init == 0)
	{
		hFile = fopen("/root/filter_vdecoder_pcm.out", "wb");
		init = 1;
	}
#endif


	while(1)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec]  ThreadProc START !!\n");

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){
			NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec]  m_bExitThread = %d !!\n", m_bExitThread);
			pthread_mutex_unlock(&m_hMutexThread);
			break;
		}
		pthread_mutex_unlock(&m_hMutexThread);

		pInSample = NULL;

		//	Get Input Sample
		if (m_pInQueue->PopSample(&pInSample) < 0)
		{
			break;
		}
		if (!pInSample)
			break;

#ifdef _DEBUG
		m_DbgInFrameCnt++;
#endif	//	_DEBUG

		//	Deliver End of stream
		if (pInSample->IsEndOfStream()){
			if (0 != m_pOutputPin->GetDeliveryBuffer(&pOutSample)){
				NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec ] GetDeliveryBuffer Failed!!!(LINE:%d)\n", __LINE__);
				return;
			}
			pOutSample->SetEndOfStream(CTRUE);
			m_pOutputPin->Deliver(pOutSample);
			pInSample->SetEndOfStream(CFALSE);
			pOutSample->Release();
			m_pEventNotify->SendEvent(EVENT_END_OF_STREAM, 1);
			NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec ] Deliver EndofStream\n");
			break;
		}

		//	Get Input Bitstream Informations
		pInSample->GetPointer(&pInBuf);
		pInSample->GetTime(&timeStamp);
		inSize = pInSample->GetActualDataLength();

		//	Check Initialization
		if (0 == m_bInit)
		{
			NX_VID_SEQ_IN seqIn;
			NX_VID_SEQ_OUT seqOut;
			memset(&seqIn, 0, sizeof(seqIn));
			memcpy(m_VideoMaxBuf, m_SeqData, m_SeqDataSize);
			memcpy(m_VideoMaxBuf + m_SeqDataSize, pInBuf, inSize);
			seqIn.seqInfo = m_VideoMaxBuf;
			seqIn.seqSize = m_SeqDataSize + inSize;

			seqIn.enableUserData = 0;
			seqIn.disableOutReorder = 0;
			m_hCodec = NX_VidDecOpen((VID_TYPE_E)m_VpuCodecType, m_Mp4Class, 0, NULL);

#if VDFILEWRITE	//File Write
			count++;
			fwrite(&count, sizeof(count), 1, hFile);
			fwrite(&m_VpuCodecType, sizeof(m_VpuCodecType), 1, hFile);
			fwrite(&m_Mp4Class, sizeof(m_Mp4Class), 1, hFile);
			fwrite(&seqIn.seqSize, sizeof(seqIn.seqSize), 1, hFile);
			fwrite(seqIn.seqInfo, 1, seqIn.seqSize, hFile);
			fclose(hFile);
			exit(1);
#endif		

			vidRet = NX_VidDecInit(m_hCodec, &seqIn, &seqOut);
			m_pOutputPin->SetHandle(m_hCodec);

			if (vidRet == 1)
			{
				//	TODO : Implementation Need More Buffer
				//
				printf("********************************* More buffer\n");

			}

			if (0 != vidRet)
			{
				printf("Initialize Failed!!!\n");
				m_pEventNotify->SendEvent(EVENT_VIDEO_DECODER_ERROR, 1);
				//return;
			}

			InitVideoTimeStamp();
			m_bInit = 1;
			continue;
		}

		//	Decode Frame
		memset(&decIn, 0, sizeof(decIn));
		decIn.strmBuf = (unsigned char*)pInBuf;
		decIn.strmSize = inSize;
		{
#if 0	//File Write
//#if VDFILEWRITE	//File Write
			count++;
			fwrite(&count, sizeof(count), 1, hFile);
			fwrite(&decIn.strmSize, sizeof(decIn.strmSize), 1, hFile);
			fwrite(decIn.strmBuf, 1, decIn.strmSize, hFile);
			if (count == 100){
				printf("=====exit ==== \n");
				fclose(hFile);
				exit(1);
			}
#endif		

		}
		decIn.timeStamp = timeStamp;
		decIn.eos = 0;
		PushVideoTimeStamp(timeStamp, m_iInFlag);
		vidRet = NX_VidDecDecodeFrame(m_hCodec, &decIn, &decOut);

#if VDFILEWRITE	//File Write
		count++;
		fwrite(&count, sizeof(count), 1, hFile);
		fwrite(decIn.strmBuf, 1, decIn.strmSize, hFile);
		if (count == 100){
			printf("=====exit ==== \n");
			fclose(hFile);
			exit(1);
		}
#endif		


		if (vidRet == VID_ERR_NEED_MORE_BUF)
		{
			printf("VID_NEED_MORE_BUF NX_VidDecDecodeFrame\n");
			continue;
		}
		if (vidRet < 0)
		{
			printf("Decoding Error!!!\n");
			m_pEventNotify->SendEvent(EVENT_VIDEO_DECODER_ERROR, 1);
		}

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] OutFrame[%5d]: Type = %d, DspIdx=%2d, timeStamp=%7lld\n", outCount, decOut.picType, decOut.outImgIdx, decOut.timeStamp);
		outCount++;

		pInSample->Release();

		if (decOut.outImgIdx >= 0)
		{
			int32_t *ptr;
			uint32_t	Flag = 0;
			//	Exist valid stream
			PopVideoTimeStamp(&timeStamp, &Flag);
//			NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec] OutFrame[%5d]: Type = %d, DspIdx=%2d, timeStamp=%7lld ++\n", outCount, decOut.picType, decOut.outImgIdx, timeStamp);
//outCount++;
			NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] decOut.outImgIdx = %d\n", decOut.outImgIdx);

			if (0 != m_pOutputPin->GetDeliveryBuffer(&pOutSample)){
				NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec ] GetDeliveryBuffer Failed!!!(LINE:%d)\n", __LINE__);
				return;
			}
			pOutSample->GetPointer(&pOutBuf);

			memcpy((uint8_t*)pOutBuf, &decOut, sizeof(NX_VID_DEC_OUT));
			ptr = (int32_t *)(pOutBuf + sizeof(NX_VID_DEC_OUT));
			memcpy((uint8_t*)ptr, &decOut.outImgIdx, 4);
			ptr = (int32_t *)(pOutBuf + sizeof(NX_VID_DEC_OUT));

			pOutSample->SetActualDataLength(sizeof(NX_VID_DEC_OUT)+4);	
//			pOutSample->SetTime(decOut.timeStamp);
			pOutSample->SetTime(timeStamp);
			pOutSample->SetDiscontinuity(0);
			pOutSample->SetValid(CTRUE);
#ifdef _DEBUG
			m_DbgOutFrameCnt++;
#endif
			m_pOutputPin->Deliver(pOutSample);
			if (pOutSample){
				pOutSample->Release();
			}
		}

	}	//while
}

void NX_CVideoDecoderFilter::InitVideoTimeStamp()
{
	int32_t i;
	for (i = 0; i<NX_MAX_BUF; i++)
	{
		m_OutTimeStamp[i].flag = (int32_t)-1;
		m_OutTimeStamp[i].timestamp = (int32_t)0;
	}
	m_iInFlag = 0;
	m_iOutFlag = 0;
}

void NX_CVideoDecoderFilter::PushVideoTimeStamp(int64_t timestamp, uint32_t flag)
{
	int32_t i = 0;
	if (-1 != timestamp)
	{
		m_iInFlag++;
		if (NX_MAX_BUF <= m_iInFlag)
			m_iInFlag = 0;

		for (i = 0; i<NX_MAX_BUF; i++)
		{
			if (m_OutTimeStamp[i].flag == (int32_t)-1)
			{
				m_OutTimeStamp[i].timestamp = timestamp;
				m_OutTimeStamp[i].flag = flag;
				break;
			}
		}
	}
}

int32_t NX_CVideoDecoderFilter::PopVideoTimeStamp(int64_t *timestamp, uint32_t *flag)
{
	int32_t i = 0;
	int64_t minTime = 0x7FFFFFFFFFFFFFFFll;
	int32_t minIdx = -1;

	for (i = 0; i<NX_MAX_BUF; i++)
	{
		if (m_OutTimeStamp[i].flag != (uint32_t)-1)
		{
			if (minTime > m_OutTimeStamp[i].timestamp)
			{
				minTime = m_OutTimeStamp[i].timestamp;
				minIdx = i;
			}
		}
	}
	if (minIdx != -1)
	{
		*timestamp = m_OutTimeStamp[minIdx].timestamp;
		*flag = m_OutTimeStamp[minIdx].flag;
		m_OutTimeStamp[minIdx].flag = (int32_t)-1;
		return 0;
	}
	else
	{
		//printf("Cannot Found Time Stamp!!!\n");
		return -1;
	}
}

CBOOL NX_CVideoDecoderFilter::PinInfo(NX_PinInfo *pPinInfo)
{
	pPinInfo->InPutNum = 1;
	pPinInfo->OutPutNum = 1;
	return 0;
}

NX_CBasePin *NX_CVideoDecoderFilter::FindPin(int Pos)
{
	return GetPin(Pos);	
}

CBOOL NX_CVideoDecoderFilter::FindInterface(const char *InterfaceId, void **Interface)
{
//	if(!strcmp(InterfaceId, "IMediaControl")){
//		*Interface = (NX_IMediaControl *)(this);
//	}

	return 0;
}

int32_t NX_CVideoDecoderFilter::SetMediaType(void *Mediatype)
{

	memcpy(&m_VMediaType, Mediatype, sizeof(NX_VideoMediaType));

	m_VpuCodecType = m_VMediaType.VpuCodecType;
	m_Mp4Class = m_VMediaType.Mp4Class;
	m_Width = m_VMediaType.Width;
	m_Height = m_VMediaType.Height;

	m_SeqDataSize = m_VMediaType.SeqDataSize;
	memcpy(m_SeqData, m_VMediaType.pSeqData, m_VMediaType.SeqDataSize);

	NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec ] m_Mp4Class = %d, m_Height = %d, m_Width = %d\n", m_Mp4Class, m_Height, m_Width);

	return 0;
}

void NX_CVideoDecoderFilter::SetClockReference(NX_CClockRef *pClockRef)
{
	SetClockRef(pClockRef);
}

void NX_CVideoDecoderFilter::SetEventNotifi(NX_CEventNotifier *pEventNotifier)
{
	SetEventNotifier(pEventNotifier);
}

//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							NX_CVideoDecoderInputPin
//
NX_CVideoDecoderInputPin::NX_CVideoDecoderInputPin(NX_CVideoDecoderFilter *pOwner) :
	NX_CBaseInputPin( (NX_CBaseFilter*)pOwner )
{
}


int32_t NX_CVideoDecoderInputPin::Receive(NX_CSample *pInSample)
{
	int32_t ret = -1;
	//return 0;	//
	if( NULL == m_pOwnerFilter && CTRUE != IsActive() )
		return -1;
	
	ret = ((NX_CVideoDecoderFilter*)m_pOwnerFilter)->DecoderSample(pInSample);
	return ret;
}

#define CODE_ID_MAX		30
static int codec_name_tab[CODE_ID_MAX] = {
	CODEC_H264,
	CODEC_H263,
	CODEC_MPEG1VIDEO,
	CODEC_MPEG2VIDEO,
	CODEC_MPEG4,
	CODEC_MSMPEG4V3,
	CODEC_FLV1,
	CODEC_WMV1,
	CODEC_WMV2,
	CODEC_WMV3,
	CODEC_VC1,
	CODEC_RV30,
	CODEC_RV40,
	CODEC_THEORA,
	CODEC_VP8,
	CODEC_RA_144,
	CODEC_RA_288,
	CODEC_MP2,
	CODEC_MP3,
	CODEC_AAC,
	CODEC_AC3,
	CODEC_DTS,
	CODEC_VORBIS,
	CODEC_WMAV1,
	CODEC_WMAV2,
	CODEC_WMAPRO,
	CODEC_FLAC,
	CODEC_COOK,
	CODEC_APE,
	CODEC_AAC_LATM,
};

int32_t NX_CVideoDecoderInputPin::CheckMediaType(void *Mediatype)
{
	NX_MediaType *MType = (NX_MediaType *)Mediatype;
	NX_VideoMediaType *pType = NULL;
	int i = 0;
	int32_t FindCodec = CFALSE;
	
	pType = (NX_VideoMediaType *)&MType->VMediaType;

	if (VIDEOTYPE != MType->MeidaType)
		return -1;

	for (i = 0; i < CODE_ID_MAX; i++){
		if (pType->CodecID == codec_name_tab[i]){
//			NX_DbgMsg(DBG_MSG_ON, "[BB|ADec ] Not Support CodecID = %d\n", pType->CodecID);
			FindCodec = CTRUE;
			break;
		}
	}
	if (CFALSE == FindCodec){
		NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec ] Not Support CodecID = %d\n", pType->CodecID);
		return -1;
	}
	if ((pType->Width == 0) || (pType->Height == 0)){
		NX_DbgMsg(DBG_MSG_ON, "[Filter|VDec ] Erro: Width = %d, Height = %d\n", pType->Width, pType->Height);
		return -1;
	}

	NX_CVideoDecoderFilter *pOwner = (NX_CVideoDecoderFilter *)m_pOwnerFilter;

	pOwner->SetMediaType(&MType->VMediaType);

	memcpy(&pOwner->m_pOutputPin->MediaType, Mediatype, sizeof(NX_MediaType));

	return 0;
}


//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							NX_CVideoDecoderOutputPin
//
NX_CVideoDecoderOutputPin::NX_CVideoDecoderOutputPin(NX_CVideoDecoderFilter *pOwner)
	: NX_CBaseOutputPin( (NX_CBaseFilter *)pOwner )
	, m_hCodec(NULL)
	, m_pSampleQ( NULL )
	, m_pSampleList( NULL )
{
	m_pSampleQ = new NX_CSampleQueue(MAX_NUM_VIDEO_OUT_BUF);
	AllocBuffers();
}

NX_CVideoDecoderOutputPin::~NX_CVideoDecoderOutputPin()
{
	if( m_pSampleQ ){
		delete m_pSampleQ;
	}
	DeallocBuffers();
}


CBOOL NX_CVideoDecoderOutputPin::Active()
{

	if (CFALSE == m_bActive)
	{
		m_bExitThread = CFALSE;

		if (m_pSampleQ)
		{
			m_pSampleQ->ResetQueue();
			NX_CSample *pSample = m_pSampleList;
			pSample->Reset();

			for (int i = 0; i < MAX_NUM_VIDEO_OUT_BUF; i++)
			{
				if (NULL != pSample)
				{
					pSample->Reset();
					m_pSampleQ->PushSample(pSample);
					pSample = pSample->m_pNext;
				}
			}
		}
	}

	m_bActive = CTRUE;

	return NX_CBaseOutputPin::Active();
}

CBOOL NX_CVideoDecoderOutputPin::Inactive()
{
	//	Set Thread End Command
	ExitThread();
	if( m_pSampleQ )	m_pSampleQ->EndQueue();

	if (CTRUE == m_bActive)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] NX_CVideoDecoderOutputPin Wait join ++\n");
		m_bActive = CFALSE;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] NX_CVideoDecoderOutputPin Wait join --\n");
	}

	return NX_CBaseOutputPin::Inactive();
}

int32_t NX_CVideoDecoderOutputPin::CheckMediaType(void *Mediatype)
{

	return 0;
}


int32_t	NX_CVideoDecoderOutputPin::ReleaseSample(NX_CSample *pSample)
{
	int32_t ret = 0;

	NX_VID_DEC_OUT *pDecOut = NULL;
	int32_t *ptr = NULL;
	pSample->GetPointer((uint32_t **)&pDecOut);
	int32_t index = -1;

	ptr = (int32_t *)pDecOut;
	index = *(ptr + sizeof(NX_VID_DEC_OUT));
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] ReleaseSample: index = %d\n", index);
	if (index != -1)
	{
		NX_VidDecClrDspFlag(m_hCodec, &pDecOut->outImg, index);
	}

	if (m_pSampleQ){
		uint8_t *pInBuf = NULL;
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] m_pSampleQ->GetSampleCount() = %d\n", m_pSampleQ->GetSampleCount());
		pSample->GetPointer((uint32_t**)&pInBuf);

		ret = m_pSampleQ->PushSample(pSample);
		return ret;
	}
	return 0;
}


int32_t NX_CVideoDecoderOutputPin::GetDeliveryBuffer(NX_CSample **ppSample)
{
	if( !IsActive() )
		return -1;

	*ppSample = NULL;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VDec] GetDeliveryBuffer : m_pSampleQ->GetSampleCount() = %d\n", m_pSampleQ->GetSampleCount());
	if( m_pSampleQ->GetSampleCount() < 0 ){
		return -1;
	}

	m_pSampleQ->PopSample( ppSample );
	if( NULL != *ppSample )
	{
		(*ppSample)->AddRef();
	}

	if( !IsActive() )
	{
		if( NULL != *ppSample )
		{
			(*ppSample)->Release();
		}
		return -1;
	}
	return 0;
}

int32_t	NX_CVideoDecoderOutputPin::AllocBuffers()
{
	uint32_t *pBuf;
	NX_CSample *pNewSample = NULL;
	NX_CSample *pOldSample = m_pSampleList;

	int i=0;

	pOldSample = new NX_CSample(this);
	pBuf = new uint32_t[ (sizeof(NX_VID_DEC_OUT) / sizeof(uint32_t) ) + 4];
	pOldSample->SetBuffer(pBuf, sizeof(NX_VID_DEC_OUT) + 4, sizeof(uint32_t));
	m_pSampleList = pOldSample;

	for (i = 1; i<MAX_NUM_VIDEO_OUT_BUF; i++)
	{
		pNewSample = new NX_CSample(this);
		pBuf = new uint32_t[(sizeof(NX_VID_DEC_OUT) / sizeof(uint32_t)) + 4];
		pNewSample->SetBuffer(pBuf, sizeof(NX_VID_DEC_OUT) + 4, sizeof(uint32_t));
		pOldSample->m_pNext = pNewSample;
		pOldSample = pNewSample;
	}

	NX_DbgMsg( DBG_MSG_OFF, "[Filter|VDec ] Allocated Buffers = %d\n", i );
	return 0;
}

int32_t	NX_CVideoDecoderOutputPin::DeallocBuffers()
{
	int k = 0;
	while(1)
	{
		if( NULL == m_pSampleList )
		{
			break;
		}
		NX_CSample *TmpSampleList = m_pSampleList;
		m_pSampleList = m_pSampleList->m_pNext;
		delete TmpSampleList;
		k++;
	}
	NX_DbgMsg( DBG_MSG_OFF, "[Filter|VDec ] Deallocated Buffers = %d\n", k );
	return 0;
}

int32_t	NX_CVideoDecoderOutputPin::SetHandle(NX_VID_DEC_HANDLE hCodec)
{
	m_hCodec = hCodec;
	return 0;
}
//
//////////////////////////////////////////////////////////////////////////////


NX_IBaseFilter *CreateVideoDecoderFilter(const char *filterName)
{
	return new NX_CVideoDecoderFilter(filterName);
}
