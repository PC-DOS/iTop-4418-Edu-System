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
#include <string.h>
#include <unistd.h> 		// for open/close, getpid
#include <fcntl.h> 			// for O_RDWR
#include <sys/ioctl.h>	 	// for ioctl
#include <sys/time.h>
#include <asm/unistd.h> 	// __NR_gettid
#include <sched.h> 			// schedule
#include <linux/sched.h> 	// SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH

#include "NX_DebugMsg.h"
#include "NX_SystemCall.h"
#include "NX_CVideoRenderFilter.h"

#define TIMER_DEVNODE				"/dev/timer"

#define HDMI	0

//////////////////////////////////////////////////////////////////////////////
//
//							NX_CVideoRenderFilter
//
NX_CVideoRenderFilter::NX_CVideoRenderFilter(const char *filterName) 
	: NX_CBaseFilter(filterName)
	, m_pInputPin(NULL)
	, m_pInQueue(NULL)
	, m_pRender(NULL)
	, m_AverageTimePerFrame(1000/30)
	, m_MaxSleepTime( RENDERER_MAX_SLEEP_TIME )
	, m_Init(0)
{
	m_pInputPin = (NX_CVideoRenderInputPin*)new NX_CVideoRenderInputPin( this );
	m_pInQueue = new NX_CSampleQueue(MAX_NUM_VIDEO_OUT_BUF);
	m_pRender = new NX_CDisplay( this, m_SrcWidth, m_SrcHeight );

	memset(m_cArrFilterName, 0, sizeof(m_cArrFilterName));
	memset(m_cArrFilterID, 0, sizeof(m_cArrFilterID));
//	SetFilterName(filterName);

	if (pthread_mutex_init(&m_hMutexThread, NULL) < 0){
		assert(CFALSE);
	}
	
	if( !m_pInputPin || !m_pInQueue || !m_pRender ){
		assert( CFALSE  );
	}

#ifdef _DEBUG
	m_DbgInFrameCnt  =
	m_DbgOutFrameCnt =
	m_DbgDropFrameCnt=0;
#endif	//	_DEBUG
}

NX_CVideoRenderFilter::~NX_CVideoRenderFilter()
{
	if( m_pInputPin )
		delete m_pInputPin;
	if( m_pInQueue )
		delete m_pInQueue;
	if( m_pRender )
		delete m_pRender;

	pthread_mutex_destroy(&m_hMutexThread);
}

NX_CBasePin *NX_CVideoRenderFilter::GetPin( int pos )
{
	if( 0 == pos && NULL != m_pInputPin )
	{
		return m_pInputPin;
	}
	return NULL;
}

int32_t	NX_CVideoRenderFilter::RenderSample( NX_CSample *pSample )
{
	int32_t ret = -1;
	if( m_pInQueue )
	{
		pSample->AddRef();
//		NX_DbgMsg(DBG_MSG_ON, "[Filter|VRend]  RenderSample: pSample->GetRefCount = %d !!\n", pSample->GetRefCount());
		ret = m_pInQueue->PushSample( pSample );
		return ret;
	}
	return ret;
}

CBOOL NX_CVideoRenderFilter::Flush()
{
	return CTRUE;
}

CBOOL NX_CVideoRenderFilter::Run()
{
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend ] Run() ++\n");
	if( CFALSE == m_bRunning )
	{
		if (m_Init == 0){
			m_pRender->Init(m_SrcWidth, m_SrcHeight);
			NX_DbgMsg(DBG_MSG_ON, "[Filter|VRend]  m_pRender->Init!!\n");
			m_Init = 1;
		}

		m_pInQueue->ResetQueue();
		if( m_pInputPin )
			m_pInputPin->Active();

		m_bPaused = CFALSE;
		m_bExitThread = CFALSE;

		if( pthread_create( &m_hThread, NULL, ThreadStub, this ) < 0 )
		{
			return CFALSE;
		}
		m_bRunning = CTRUE;
	}
	else if (CTRUE == m_bRunning)
	{
		m_bPaused = CFALSE;
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend ] Run() --\n");
	return CTRUE;
}


CBOOL NX_CVideoRenderFilter::Stop()
{
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] Stop() ++\n");

	//	Set Thread End Command
	ExitThread();
	//	Send Queue Emd Message
	if( m_pInQueue )
		m_pInQueue->EndQueue();

	//	Input Inactive
	if( m_pInputPin )
		m_pInputPin->Inactive();

	if( CTRUE == m_bRunning )
	{
		NX_DbgMsg( DBG_MSG_OFF, "[Filter|VRend] Stop() : pthread_join() ++\n" );
		pthread_join( m_hThread, NULL );
		m_bRunning = CFALSE;
		m_hThread = 0;
		NX_DbgMsg( DBG_MSG_OFF, "[Filter|VRend] Stop() : pthread_join() --\n" );
	}

	m_Init = 0;

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] Stop() --\n");
	return CTRUE;
}

CBOOL NX_CVideoRenderFilter::Pause(void)
{
	m_bPaused = CTRUE;

	return CTRUE;
}
void NX_CVideoRenderFilter::Resume(void)
{
	m_bPaused = CFALSE;
	m_SemPause.Post();
}


void NX_CVideoRenderFilter::ChangeBufferingTime( uint32_t BufferingTime )
{
	m_MaxSleepTime = (int32_t)(BufferingTime);
}

void *NX_CVideoRenderFilter::ThreadStub( void *pObj )
{
	if( NULL != pObj )
	{
		((NX_CVideoRenderFilter*)pObj)->ThreadProc();
	}
	return (void*)(0);
}

void NX_CVideoRenderFilter::ThreadProc()
{
	NX_CSample *pSample;

	if( !m_pInQueue || !m_pRender )
		return ;

	m_pPrevSample = NULL;

	uint32_t *pBuf = NULL;
	int32_t Size;
	int64_t SampleOutTime, CurTime;

	while(1)
	{
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] ThreadProc START !!! \n");

		while (m_bPaused && !m_bExitThread)
		{
			usleep(3000);
		}

		pthread_mutex_lock(&m_hMutexThread);
		if (m_bExitThread){
			NX_DbgMsg(DBG_MSG_ON, "[Filter|VRend]  m_bExitThread = %d !!\n", m_bExitThread);
			pthread_mutex_unlock(&m_hMutexThread);			
			break;
		}
		pthread_mutex_unlock(&m_hMutexThread);

		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] ++ m_pInQueue->GetSampleCount = %d \n", m_pInQueue->GetSampleCount());
		if (m_pInQueue->PopSample(&pSample) < 0)
		{
			NX_DbgMsg(DBG_MSG_ON, "[Filter|VRend] m_pInQueue->PopSample(&pSample) < 0 \n");
			break;
		}
		NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] -- m_pInQueue->GetSampleCount = %d \n", m_pInQueue->GetSampleCount());

		if (!pSample)
		{
			break;
		}
#ifdef _DEBUG
		m_DbgInFrameCnt++;
#endif	//	_DEBUG
		if( pSample->IsEndOfStream() )
		{
			pSample->SetEndOfStream( CFALSE );
			pSample->Release();
			m_pEventNotify->SendEvent(EVENT_END_OF_STREAM, 1);
			NX_DbgMsg( DBG_MSG_ON, "[Filter|VRend] Deliver EndofStream\n" );
			break;
		}

		pSample->GetPointer( &pBuf );
		Size = pSample->GetActualDataLength();

		if (0 == pSample->GetTime(&SampleOutTime))
		{
			m_pRefClock->GetCurrTime(&CurTime);
			NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] SampleOutTime = %d, CurTime = %d\n", (int32_t)SampleOutTime, (int32_t)CurTime);
			if (0 == m_pRefClock->GetCurrTime(&CurTime) && CurTime != -1)
			{
				if (SampleOutTime >= CurTime)
				{
					int32_t SleepTime = (int32_t)(SampleOutTime - CurTime);
					if (SleepTime > m_MaxSleepTime){
						NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] warning : SleepTime(%d) > m_MaxSleepTime(%d)\n", SleepTime, m_MaxSleepTime);
					}
					else{
						if (SleepTime > 0){
							NX_Sleep(SleepTime);
							NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] NX_Sleep(%d)\n", SleepTime);
						}
					}
				}
				else    // Drop Sample
				{
					pSample->Release();
					NX_DbgMsg(DBG_MSG_OFF, "[Filter|VRend] Drop Video Frame\n");
					continue;
				}
			}
		}

		//printf("\n=======Size = %d\n", Size);
		m_pRender->Display( pBuf, Size, 1, CFALSE );

		if (m_pPrevSample)
		{
			uint8_t *pInBuf = NULL;
			m_pPrevSample->GetPointer((uint32_t**)&pInBuf);
			m_pPrevSample->Release();
		}
		m_pPrevSample = pSample;

	}
	m_pRender->Close();
	NX_DbgMsg( DBG_MSG_ON, "[Filter|VRend] Exit renderer thread!!\n" );
}


//
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//
//							External Interface
//
CBOOL NX_CVideoRenderFilter::SetVideoPosition(int32_t moudleId, int32_t port, int32_t X, int32_t Y, int32_t Width, int32_t Height)

{
	if( m_pRender ){
		return m_pRender->SetVideoPosition( moudleId, port, X, Y, Width, Height );
	}
	return CFALSE;
}

CBOOL NX_CVideoRenderFilter::SetDisplay(int32_t LCD_On_OFF, int32_t HDMI_ON_FF)		 		//	Pure Virtual
{
	m_pRender->SetDisplayer(LCD_On_OFF, HDMI_ON_FF);

	return CTRUE;
}

CBOOL NX_CVideoRenderFilter::PinInfo(NX_PinInfo *pPinInfo)
{
	pPinInfo->InPutNum = 1;
	pPinInfo->OutPutNum = 1;

	return 0;
}

NX_CBasePin *NX_CVideoRenderFilter::FindPin(int Pos)
{
	return GetPin(Pos);
}

CBOOL NX_CVideoRenderFilter::FindInterface(const char *InterfaceId, void **Interface)
{
//	if (!strcmp(InterfaceId, "IMediaControl")){
//		*Interface = (NX_IMediaControl *)(this);
//	}
//	else
	if (!strcmp(InterfaceId, "IVideoRenderControl")){
		*Interface = (NX_IDisplayControl *)(this);
	}
	
	return 0;
}

int32_t NX_CVideoRenderFilter::SetMediaType(void *Mediatype)
{
	NX_VideoMediaType *pType = NULL;

	pType = (NX_VideoMediaType *)Mediatype;

	m_SrcWidth = pType->Width;
	m_SrcHeight = pType->Height;

	return 0;
}

void NX_CVideoRenderFilter::SetClockReference(NX_CClockRef *pClockRef)
{
	SetClockRef(pClockRef);
}

void NX_CVideoRenderFilter::SetEventNotifi(NX_CEventNotifier *pEventNotifier)
{
	SetEventNotifier(pEventNotifier);
}



//
//////////////////////////////////////////////////////////////////////////////



#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////////
//
//						Debug Informations
//
void NX_CVideoRenderFilter::DisplayVersion()
{
}

void NX_CVideoRenderFilter::DisplayState()
{
}
void NX_CVideoRenderFilter::GetStatistics( uint32_t *InFrameCnt, uint32_t *OutFrameCnt, uint32_t *DropFrameCnt )
{
	*InFrameCnt   = m_DbgInFrameCnt;
	*OutFrameCnt  = m_DbgOutFrameCnt;
	*DropFrameCnt = m_DbgDropFrameCnt;
}
void NX_CVideoRenderFilter::ClearStatistics()
{
	m_DbgInFrameCnt   =
	m_DbgOutFrameCnt  =
	m_DbgDropFrameCnt = 0;
}
//
/////////////////////////////////////////////////////////////////////////////////
#endif


//////////////////////////////////////////////////////////////////////////////
//
//						Nexell CRender Input Pin Class
//
NX_CVideoRenderInputPin::NX_CVideoRenderInputPin(	NX_CVideoRenderFilter *pOwnerFilter ) :
	NX_CBaseInputPin( (NX_CBaseFilter*)pOwnerFilter )
{
}
NX_CVideoRenderInputPin::~NX_CVideoRenderInputPin()
{
}
int32_t NX_CVideoRenderInputPin::Receive( NX_CSample *pSample )
{
	//return 0;	//
	if( NULL == m_pOwnerFilter || CTRUE != IsActive() )
		return -1;

	return ((NX_CVideoRenderFilter*)m_pOwnerFilter)->RenderSample( pSample );
}

int32_t NX_CVideoRenderInputPin::CheckMediaType(void *Mediatype)
{
	NX_MediaType *MType = (NX_MediaType *)Mediatype;
	NX_VideoMediaType *pType = NULL;

	pType = (NX_VideoMediaType *)&MType->VMediaType;

	if (VIDEOTYPE != MType->MeidaType)
		return -1;

	if ((pType->Width == 0) || (pType->Height == 0)){
		NX_DbgMsg(DBG_TRACE, "[Filter|VDec ] Erro: Width = %d, Height = %d\n", pType->Width, pType->Height);
		return -1;
	}

	NX_CVideoRenderFilter *pOwner = (NX_CVideoRenderFilter *)m_pOwnerFilter;

	pOwner->SetMediaType(&MType->VMediaType);

	return 0;
}

//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Video Display API
//
NX_CDisplay::NX_CDisplay( NX_CVideoRenderFilter *pFilter, int32_t Width, int32_t Height ):
	m_SrcWidth( Width ),
	m_SrcHeight( Height ),
	m_DstX(0), m_DstY(0), m_DstWidth(Width), m_DstHeight(Height),
	m_Module( 0 ),			//	Default Display Primary Module
	m_Port( 0 ),
	m_Priority( 0 ),
	m_pOwnerFilter( pFilter ),
	m_LcdFlag(0),
	m_HdmiFlag(0)
{

}

NX_CDisplay::~NX_CDisplay()
{
	Close();

};

RENDER_INFO * NX_CDisplay::InitRender(int32_t Module, int32_t Port, int32_t Width, int32_t Height)
{
	m_SrcWidth = Width;
	m_SrcHeight = Height;

	RENDER_INFO *info = (RENDER_INFO *)malloc(sizeof(RENDER_INFO));
	DISPLAY_INFO dspInfo;
	memset(info, 0, sizeof(RENDER_INFO));

	//	Check Module ID
	if (Module != 0 && Module != 1)
	{
		printf("Error:%s:Line(%d) : Invalid Module ID (%d)\n", __FILE__, __LINE__, Module);
		goto ErrorExit;
	}

	//	Check Port ID
	if (Port != 0 && Port != 1)
	{
		printf("Error:%s:Line(%d) : Invalid Port ID (%d)\n", __FILE__, __LINE__, Port);
		goto ErrorExit;
	}

	m_DstWidth = 1280;
	m_DstHeight = 800;

	dspInfo.module = Module;
	dspInfo.port = Port;

	dspInfo.width = m_SrcWidth;
	dspInfo.height = m_SrcHeight;
	dspInfo.numPlane = 1;

	dspInfo.dspSrcRect.left = 0;
	dspInfo.dspSrcRect.top = 0;
	dspInfo.dspSrcRect.right = m_SrcWidth;
	dspInfo.dspSrcRect.bottom = m_SrcHeight;

	dspInfo.dspDstRect.left = m_DstX;
	dspInfo.dspDstRect.top = m_DstY;
	dspInfo.dspDstRect.right = m_DstX + m_DstWidth;
	dspInfo.dspDstRect.bottom = m_DstY + m_DstHeight;

	info->hDsp = NX_DspInit(&dspInfo);

	info->Init = 0;

	if (info->hDsp)
	{
		info->moduleId = Module;
		info->port = Port;
		info->bInit = 1;
		info->bEnabled = 0;
		info->dspInfo = dspInfo;
		NX_DspVideoSetPriority(info->moduleId, m_Priority);
		return info;
	}
	else
	{
		printf("ERROR:%s:Line(%d) : InitDisplay() failed \n", __FILE__, __LINE__);
	}
ErrorExit:

	return NULL;
}

CBOOL NX_CDisplay::Init(int32_t Width, int32_t Height)
{
	NX_CAutoLock Lock(m_DspMutex);

//	printf("===============m_LcdFlag = %d, m_HdmiFlag = %d", m_LcdFlag, m_HdmiFlag);
	//if (ON == m_LcdFlag)
		m_info = InitRender(0, 0, Width, Height);

	//if (ON == m_HdmiFlag)
#if HDMI
		m_info_hdmi = InitRender(1, 1, Width, Height);
#endif

	return CTRUE;
}
void NX_CDisplay::Display( uint32_t* pInBuf, int32_t InSize, CBOOL Valid, CBOOL bWait )
{
	NX_CAutoLock Lock(m_DspMutex);

	NX_VID_DEC_OUT *DecOut = NULL;
	NX_VID_MEMORY_INFO *image = NULL;

	DecOut = (NX_VID_DEC_OUT *)((void*)pInBuf);
	image = &DecOut->outImg;

	if( 0 != NX_DspQueueBuffer( m_info->hDsp, image ) )
	{
		printf("ERROR:%s:Line(%d) : DisplayQueueBuffer() Failed.!!!\n", __FILE__, __LINE__);
	}


	if (m_info->pPrevImage)
	{
		NX_DspDequeueBuffer( m_info->hDsp );
	}

	//
	//if(ON == m_HdmiFlag)
#if	HDMI
	if( 0 != NX_DspQueueBuffer( m_info_hdmi->hDsp, image ) )
	{
		printf("ERROR:%s:Line(%d) : DisplayQueueBuffer() Failed.!!!\n", __FILE__, __LINE__);
	}

	if (m_info_hdmi->pPrevImage)
	{
		NX_DspDequeueBuffer(m_info_hdmi->hDsp);
	}
	m_info_hdmi->pPrevImage = image;
#endif
	//

	m_info->pPrevImage = image;
}

CBOOL NX_CDisplay::Close()
{
	NX_CAutoLock Lock(m_DspMutex);

	if (m_info)
	{
		if (m_info->hDsp)
		{
			NX_DspClose(m_info->hDsp);
			m_info->hDsp = NULL;
		}
		free(m_info);
	}

#if	HDMI	
	if (m_info_hdmi)
	{
		if (m_info_hdmi->hDsp)
		{
			NX_DspClose(m_info_hdmi->hDsp);
			m_info_hdmi->hDsp = NULL;
		}
		free(m_info_hdmi);
	}
#endif

	return CTRUE;
}

CBOOL NX_CDisplay::SetVideoPosition(int32_t moudleId, int32_t port, int32_t X, int32_t Y, int32_t Width, int32_t Height)
{
	NX_CAutoLock Lock(m_DspMutex);
	m_DstX = X;
	m_DstY = Y;
	m_DstWidth = Width;
	m_DstHeight = Height;
	m_Module = moudleId;
	m_Port = port;

	if (!m_info)
		return CFALSE;

	if (m_Module != m_info->moduleId || m_Port != m_info->port)
	{
		printf("ERROR:%s:Line(%d) : Invalid Parameter\n", __FILE__, __LINE__);
		return -1;
	}

	//	Check paramter
	if (1>Width || 1>Height)
		return -1;

	if (m_info->dstX != X || m_info->dstY != Y || m_info->dstWidth != Width || m_info->dstHeight != Height)
	{
		DSP_IMG_RECT rect;
		rect.left	= X;
		rect.top	= Y;
		rect.right	= X + Width;
		rect.bottom	= Y + Height;
		//printf("*******************************: NX_DspVideoSetPosition() (x:%d, y:%d, w:%d, h:%d)\n", X, Y, Width, Height);
		if (NX_DspVideoSetPosition(m_info->hDsp, &rect)){
			printf("Error : NX_DspVideoSetPosition() failed.(x:%d, y:%d, w:%d, h:%d)\n", X, Y, Width, Height);
			return -1;
		}
		m_info->dstX = X;
		m_info->dstY = Y;
		m_info->dstWidth = Width;
		m_info->dstHeight = Height;
	}
	return CTRUE;
}

CBOOL NX_CDisplay::SetDisplayer(int32_t LCD_On_OFF, int32_t HDMI_ON_FF)		 		//	Pure Virtual
{
	m_LcdFlag = LCD_On_OFF;
	m_HdmiFlag = HDMI_ON_FF;

	return CTRUE;
}

//
//
//////////////////////////////////////////////////////////////////////////////



NX_IBaseFilter *CreateVideoRenderFilter(const char *filterName)
{
	return new NX_CVideoRenderFilter(filterName);
}