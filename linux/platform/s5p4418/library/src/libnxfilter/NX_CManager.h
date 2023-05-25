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
#ifndef __NX_CManager_h__
#define __NX_CManager_h__


#ifdef __cplusplus

#include "NX_MediaType.h"
#include "NX_IBaseFilter.h"
#include "NX_TypeDefind.h"
#include "NX_CFilterHelp.h"

#include <pthread.h>


#define FILTERTOTNUM
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

#define	FILTERMAX			20
typedef struct  _FilterMag{
	char FilterName[FILTERMAX];
	NX_IBaseFilter *pFilter;
}FilterMag;

typedef struct tag_PLAYER_EVENT_MESSAGE
{
	uint32_t		eventType;
	uint32_t		eventData;
} PLAYER_EVENT_MESSAGE;

//////////////////////////////////////////////////////////////////////////////
//
//						Nexell CInterface

class NX_CManager : public NX_CEventReceiver
{
public: 
	NX_CManager(void(*cb)(void *Obj, uint32_t EventType, uint32_t EventData, uint32_t parm), const char *uri);
	virtual ~NX_CManager();

public:
	//	Implementation Pure Virtual Fucntion for NX_CEnvetReceiver
	virtual void ProcessEvent(uint32_t EventCode, uint32_t Value);

	//	Media Control Meothods
	CBOOL			Run(void);
	CBOOL			Stop(void);
	CBOOL			Pause(void);
	int32_t			Resume(void);
	CBOOL			Seek(int64_t millsecond);
	int64_t			GetDuration(void);
	int64_t			GetPosition(void);
	int32_t			GetMediaInfo(MEDIA_INFO * pMediaInfo);
	//	Filter Management Functions
	int32_t			AddFilterManager(NX_IBaseFilter *pFilter);
	void		*NX_GetInterface(char *InterfaceID);
	//	Video Position
	int32_t SetVideoPosition(int32_t moudleId, int32_t port, int32_t X, int32_t Y, int32_t Width, int32_t Height);
	//	Audio Volume COntrol
	int32_t SetVolume(int32_t Volume);
	int32_t	FilterCreate(int32_t ApinSelectNum, int32_t VpinSelectNum);
	FilterPipeMag	m_FilterPipeMag;
	MEDIA_INFO		m_MediaInfo;
	NX_IBaseFilter			*m_pArrIBaseFilter[FILTERMAX];
	int32_t						m_BaseFilterNum;
	NX_IBaseFilter			*m_pDemux;
	NX_IBaseFilter			*m_pAudioDecode;
	NX_IBaseFilter			*m_pVideoDecode;
	NX_IBaseFilter			*m_pAudioRender;
	NX_IBaseFilter			*m_pVideoRender;
	NX_IBaseFilter			*m_pAudioConvert;
	NX_IDemuxControl		*m_pIDemuxCon;
	NX_IAudioControl		*m_pIAudioConvert;
	NX_IDisplayControl		*m_pIVideoRenderControl;
	NX_CBasePin				*m_pPinDmxAOut, *m_pPinDmxVOut;
	NX_CBasePin				*m_pPinVDecodeIn, *m_pPinVDecodeOut;
	NX_CBasePin				*m_pPinADecodeIn, *m_pPinADecodeOut;
	NX_CBasePin				*m_pPinARIn, *m_pPinVRIn;
	NX_CBasePin				*m_pPinAConvertIn, *m_pPinAConvertOut;
	NX_CClockRef			*m_pRefClock;
	NX_CEventNotifier		m_EventNotifier;
	bool					m_bRunning;
	bool					m_bPause;
	NX_MediaType			*m_pMediaType;
	//	End of stream checker
	CBOOL					m_bAudioEnd;
	CBOOL					m_bVideoEnd;

	static void *EventProcStub(void *param);
	void *EventProc();

	//	Event Callback Function Pointer
	void(*EventCallback)(void *Obj, uint32_t EventType, uint32_t EventData, uint32_t parm);

	int PushEvent(uint32_t EventCode, uint32_t Value);
	int PopEvent(uint32_t &EventCode, uint32_t &Value);

	void					*m_cbObj;
	CBOOL					m_bEventThreadExit;
	CBOOL					m_IsEventThread;
	pthread_t				m_hEventThread;
	NX_CSemaphore			*m_pEventSem;
	PLAYER_EVENT_MESSAGE	m_EventMsg[128];
	int						m_iHeadIndex, m_iTailIndex, m_MsgCount, m_QueueDepth;
	pthread_mutex_t			m_hEventLock;

	int32_t m_FilterTot;
	FilterMag m_FilterManager[20];

	void NX_SetEventNotifier();
	void NX_SetClockReference();

private:
	NX_CMutex				m_ControlLock;

	NX_CManager(NX_CManager& Ref);
	NX_CManager &operator=(NX_CManager &Ref);
};
//
//////////////////////////////////////////////////////////////////////////////



#endif	//	__cplusplus

#endif	//	__NX_CManager_h__
