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
#include <unistd.h>

#include "NX_CManager.h"
#include "NX_IDemuxControl.h"
#include "NX_IAudioControl.h"
#include "NX_IDisplayControl.h"


//////////////////////////////////////////////////////////////////////////////

NX_CManager::NX_CManager(void(*cb)(void *Obj, uint32_t EventType, uint32_t EventData, uint32_t parm), const char *uri)
	: m_pDemux(NULL)
	, m_pAudioDecode(NULL)
	, m_pVideoDecode(NULL)
	, m_pAudioRender(NULL)
	, m_pVideoRender(NULL)
	, m_pAudioConvert(NULL)
	, m_pIDemuxCon(NULL)
	, m_pIAudioConvert(NULL)
	, m_pIVideoRenderControl(NULL)
	, m_bRunning(CFALSE)
	, m_bPause(CFALSE)
	, m_bAudioEnd(CFALSE)
	, m_bVideoEnd(CFALSE)
	, EventCallback(NULL)
	, m_bEventThreadExit(CTRUE)
	, m_IsEventThread(CTRUE)
	, m_iHeadIndex(0)
	, m_iTailIndex(0)
	, m_MsgCount(0)
	, m_QueueDepth(128)
	, m_FilterTot(0)
{
	m_pDemux = CreateDemuxFilter(DEMUX_FILTER_NAME);

	if (m_pDemux)
	{
		m_pDemux->FindInterface(IDEMUX_CONTROL, (void**)&m_pIDemuxCon);
		m_pIDemuxCon->SetFileName(uri);
		m_pIDemuxCon->GetMediaInfo(&m_MediaInfo);
	}

	m_pRefClock = new NX_CClockRef;

	m_bEventThreadExit = CFALSE;
	pthread_mutex_init(&m_hEventLock, 0);
	if (0 > pthread_create(&m_hEventThread, NULL, EventProcStub, this))
	{
		NX_ErrMsg(("%s:%d : Cannot create event process thread!!\n", __FILE__, __LINE__));
	}
	//--------------------------
	EventCallback = cb;

}

int32_t	NX_CManager::FilterCreate(int32_t ApinSelectNum, int32_t VpinSelectNum)
{
	int32_t i = 0;
	NX_VideoMediaType VMediaType;
	NX_AudioMediaType AMediaType;

	// Demux Pin Create
	m_pIDemuxCon->PinCreate();

	if (ApinSelectNum > 0)
		if (m_MediaInfo.AudioTrackTotNum <= 0)
			ApinSelectNum = 1;

		if (VpinSelectNum > 0)
		if (m_MediaInfo.VideoTrackTotNum <= 0)
			VpinSelectNum = 1;

	m_pIDemuxCon->SelectPin(VpinSelectNum, ApinSelectNum);

	m_FilterPipeMag.APipeNum = 0;
	m_FilterPipeMag.VPipeNum = 0;
	AllFilterCreate(&m_FilterPipeMag, m_MediaInfo.AudioTrackTotNum, m_MediaInfo.VideoTrackTotNum, ApinSelectNum, VpinSelectNum);

	//Find InterFace
	if (m_MediaInfo.VideoTrackTotNum > 0)
	{
		m_pVideoRender = m_FilterPipeMag.VideoFilterPipe[0].pArrIBaseFilter[1];
		m_pVideoRender->FindInterface(IVIDEORENDER_CONTROL, (void**)&m_pIVideoRenderControl);
	}
	if (m_MediaInfo.AudioTrackTotNum > 0)
	{
		m_pAudioConvert = m_FilterPipeMag.AudioFilterPipe[0].pArrIBaseFilter[1];
		m_pAudioConvert->FindInterface(IAUDIOCONVERT_CONTROL, (void**)&m_pIAudioConvert);
	}


	//	Set Event Notifier & Clock Reference
	NX_SetClockReference();
	NX_SetEventNotifier();

	//
	m_pIDemuxCon->GetMediaType(&VMediaType, VIDEOTYPE);
	//printf("== Width = %d, Height = %d, Size = %d\n", VMediaType.Width, VMediaType.Height, VMediaType.SeqDataSize);
	m_pIDemuxCon->GetMediaType(&AMediaType, AUDIOTYPE);
	//printf("== CodecID = %d, SampleRate = %d, Channels = %d, sample_fmt = %d\n", AMediaType.CodecID, AMediaType.Samplerate, AMediaType.Channels, AMediaType.FormatType);

	
	if (m_MediaInfo.AudioTrackTotNum > 0)
	{
		ConnectFilter(m_pDemux, m_FilterPipeMag.AudioFilterPipe[0].pArrIBaseFilter[0]);
		ConnectFilter(m_FilterPipeMag.AudioFilterPipe[0].pArrIBaseFilter[0], m_FilterPipeMag.AudioFilterPipe[0].pArrIBaseFilter[1]);
		ConnectFilter(m_FilterPipeMag.AudioFilterPipe[0].pArrIBaseFilter[1], m_FilterPipeMag.AudioFilterPipe[0].pArrIBaseFilter[2]);
	}
	if (m_MediaInfo.VideoTrackTotNum > 0)
	{
		ConnectFilter(m_pDemux, m_FilterPipeMag.VideoFilterPipe[0].pArrIBaseFilter[0]);
		ConnectFilter(m_FilterPipeMag.VideoFilterPipe[0].pArrIBaseFilter[0], m_FilterPipeMag.VideoFilterPipe[0].pArrIBaseFilter[1]);
	}
	//

	return 0;
}

int32_t	NX_CManager::GetMediaInfo(MEDIA_INFO * pMediaInfo)
{
	memcpy(pMediaInfo, &m_MediaInfo, sizeof(MEDIA_INFO));

	return 0;
}

NX_CManager::~NX_CManager()
{
	int32_t i = 0, j = 0;
	if (m_bRunning)
		Stop();
	
	for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
	{
		for (i = 0; i < m_FilterPipeMag.AudioFilterPipe[j].Num; i++)
		{
			if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
				delete m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i];
		}
	}

	for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
	{
		for (i = 0; i < m_FilterPipeMag.VideoFilterPipe[j].Num; i++)
		{
			if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
				delete m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i];
		}
	}

	if (m_pDemux)			delete m_pDemux;
	if (m_pRefClock)		delete m_pRefClock;

	//Event
	m_bEventThreadExit = CTRUE;
	if (m_pEventSem){
		pthread_join(m_hEventThread, NULL);
	}

	pthread_mutex_destroy(&m_hEventLock);
	
}

void NX_CManager::NX_SetEventNotifier()
{
	int32_t i = 0, j = 0;
	m_EventNotifier.SetEventReceiver(this);

	if (NULL != m_pDemux)			m_pDemux->SetEventNotifi(&m_EventNotifier);

	for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
	{
		for (i = 0; i < m_FilterPipeMag.AudioFilterPipe[j].Num; i++)
		{
			if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
			{
				m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->SetEventNotifi(&m_EventNotifier);
			}
		}
	}

	for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
	{
		for (i = 0; i < m_FilterPipeMag.VideoFilterPipe[j].Num; i++)
		{
			if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
			{
				m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->SetEventNotifi(&m_EventNotifier);
			}
		}
	}

}


void NX_CManager::NX_SetClockReference()
{
	int32_t i = 0, j = 0;

	if (NULL != m_pRefClock  && NULL != m_pDemux)
	{
		m_pDemux->SetClockReference(m_pRefClock);
	}

	for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
	{
		for (i = 0; i < m_FilterPipeMag.AudioFilterPipe[j].Num; i++)
		{
			if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
			{
				m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->SetClockReference(m_pRefClock);
			}
		}
	}

	for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
	{
		for (i = 0; i < m_FilterPipeMag.VideoFilterPipe[j].Num; i++)
		{
			if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
			{
				m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->SetClockReference(m_pRefClock);
			}
		}
	}

}


CBOOL NX_CManager::Run(void)
{
	NX_CAutoLock Lock(m_ControlLock);
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Run() ++\n");

	if( (m_bRunning == CFALSE && m_bPause == CFALSE) ||
		(m_bRunning == CTRUE && m_bPause == CTRUE) ){
		int32_t i = 0, j = 0;

		for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
		{
			for (i = m_FilterPipeMag.VideoFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->Run();
				}
			}
		}

		for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
		{
			for (i = m_FilterPipeMag.AudioFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->Run();
				}
			}
		}

		if (m_pDemux)	{
			m_pDemux->Run();
		}

		m_bAudioEnd = CFALSE;
		m_bVideoEnd = CFALSE;
		m_bPause = CFALSE;
		m_bRunning = CTRUE;
	}

	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Run() --\n");

	return CTRUE;	
}

CBOOL NX_CManager::Stop(void)
{
	NX_CAutoLock Lock(m_ControlLock);
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Stop() ++\n");
	if (m_bRunning)
	{
		int32_t i =0, j = 0;

		for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
		{
			for (i = m_FilterPipeMag.VideoFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->Stop();
				}
			}
		}

		for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
		{
			for (i = m_FilterPipeMag.AudioFilterPipe[j].Num - 1; i >= 0; i--)
			{
				if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->Stop();
				}
			}
		}

		if (m_pDemux){
			m_pDemux->Stop();
		}
		if (m_pRefClock){
			m_pRefClock->Reset();
		}

	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Stop() --\n");
	m_bRunning = CFALSE;

	return CTRUE;
}

CBOOL NX_CManager::Pause(void)
{
	NX_CAutoLock Lock(m_ControlLock);
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Pause() ++\n");
	if (m_bRunning)
	{
		int32_t i = 0, j = 0;
		if (m_pRefClock)				m_pRefClock->PauseRefTime();

		for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
		{
			for (i = m_FilterPipeMag.VideoFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->Pause();
				}
			}
		}

		for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
		{
			for (i = m_FilterPipeMag.AudioFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->Pause();
				}
			}
		}

		m_bPause = CTRUE;
	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Pause() --\n");

	return CTRUE;
}

int32_t NX_CManager::Resume(void)
{
#if 0
	NX_CAutoLock Lock(m_ControlLock);
	NX_DbgMsg(DBG_MSG_OFF, "[BB|API] Resume() ++\n");
	if (m_bRunning)
	{
		if (m_pRefClock)		m_pRefClock->ResumeRefTime();
		//if (m_pAudioRender)		m_pAudioRender->Resume();
		//if (m_pVideoRender)		m_pVideoRender->Resume();
	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Resume() --\n");
#endif
	return 0;
}


CBOOL NX_CManager::Seek(int64_t millsecond)
{
	NX_CAutoLock Lock(m_ControlLock);
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Seek() ++\n");
	if (m_bRunning){
		int32_t i = 0, j = 0;
		//Stop

		for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
		{
			for (i = m_FilterPipeMag.VideoFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->Stop();
				}
			}
		}

		for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
		{
			for (i = m_FilterPipeMag.AudioFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->Stop();
				}
			}
		}
		if (m_pDemux)		m_pDemux->Stop();
		if (m_pRefClock)	m_pRefClock->Reset();

		//	seek 
		if (m_pDemux)
		{
			if (m_pIDemuxCon)
			{
				m_pIDemuxCon->SeekStream((int64_t)millsecond);
			}
		}

		//Run
		for (j = 0; j < m_FilterPipeMag.VPipeNum; j++)
		{
			for (i = m_FilterPipeMag.VideoFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.VideoFilterPipe[j].pArrIBaseFilter[i]->Run();
				}
			}
		}

		for (j = 0; j < m_FilterPipeMag.APipeNum; j++)
		{
			for (i = m_FilterPipeMag.AudioFilterPipe[j].Num-1; i >= 0; i--)
			{
				if (m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i])
				{
					m_FilterPipeMag.AudioFilterPipe[j].pArrIBaseFilter[i]->Run();
				}
			}
		}
		if (m_pDemux)		m_pDemux->Run();
	}
	else{
		//	seek 
		if (m_pDemux)
			if (m_pIDemuxCon)
				m_pIDemuxCon->SeekStream((int64_t)millsecond);

	}
	NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Seek() --\n");

	return CTRUE;
}

int32_t NX_CManager::SetVideoPosition(int32_t moudleId, int32_t port, int32_t X, int32_t Y, int32_t Width, int32_t Height)
{
	NX_CAutoLock Lock(m_ControlLock);
	m_pIVideoRenderControl->SetVideoPosition(moudleId, port, X, Y, Width, Height);
	return 0;
}
int32_t NX_CManager::SetVolume(int32_t Volume)
{
	NX_CAutoLock Lock(m_ControlLock);
	m_pIAudioConvert->SetVolume(Volume);

	return 0;
}

int64_t NX_CManager::GetDuration(void)
{
	NX_CAutoLock Lock(m_ControlLock);

	int64_t curTime = 0;
	curTime = m_MediaInfo.Duration;
	return curTime;
}

int64_t NX_CManager::GetPosition(void)
{
	NX_CAutoLock Lock(m_ControlLock);
	if (m_bRunning){
		if (m_pRefClock){
			int64_t curTime = 0;
			if (m_pRefClock->GetMediaTime(&curTime) == 0){
				return curTime;
			}
		}
	}
	return -1;
}


int32_t NX_CManager::AddFilterManager(NX_IBaseFilter *pFilter)
{
	int32_t index = 0;

	index = m_FilterTot;
	m_FilterManager[index].pFilter = pFilter;

	return 0;
}


void * NX_CManager::NX_GetInterface(char *InterfaceID)
{
	return NULL;
}


int NX_CManager::PushEvent(uint32_t EventCode, uint32_t Value)
{
	int ret = 0;

	pthread_mutex_lock(&m_hEventLock);
	if (m_bEventThreadExit){
		ret = -1;
		goto exit_unlock;
	}
	m_EventMsg[m_iTailIndex].eventType = EventCode;
	m_EventMsg[m_iTailIndex].eventData = Value;
	m_iTailIndex = (m_iTailIndex + 1) % m_QueueDepth;
	m_MsgCount++;

exit_unlock:
	pthread_mutex_unlock(&m_hEventLock);

	return ret;
}


int NX_CManager::PopEvent(uint32_t &EventCode, uint32_t &Value)
{
	int ret = 0;

	pthread_mutex_lock(&m_hEventLock);

	if (m_bEventThreadExit){
		ret = -1;
		goto exit_unlock;
	}

	EventCode = m_EventMsg[m_iHeadIndex].eventType;
	Value = m_EventMsg[m_iHeadIndex].eventData;
	m_EventMsg[m_iHeadIndex].eventType = 0;
	m_EventMsg[m_iHeadIndex].eventData = 0;
	m_iHeadIndex = (m_iHeadIndex + 1) % m_QueueDepth;
	m_MsgCount--;

exit_unlock:
	pthread_mutex_unlock(&m_hEventLock);

	return ret;
}


void NX_CManager::ProcessEvent(uint32_t EventCode, uint32_t Value)
{
	PushEvent(EventCode, Value);
}

void *NX_CManager::EventProcStub(void *param)
{
	NX_CManager *pMgr = (NX_CManager *)param;
	if (pMgr){
		return pMgr->EventProc();
	}
	return (void*)NULL;
}

void *NX_CManager::EventProc()
{
	uint32_t EventCode, Value;
	m_IsEventThread = CTRUE;
	while (!m_bEventThreadExit)
	{
		//if( m_pEventSem->Pend(1000000) < 0 )
		//	break;

		if (m_MsgCount <= 0)
		{
			usleep(100000);
			continue;
		}

		if (m_bEventThreadExit)
			break;

		//	processing
		if (0 == PopEvent(EventCode, Value))
		{
			if (m_bEventThreadExit)
				break;
			switch (EventCode)
			{

			case EVENT_DEMUX_ERROR:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_DEMUX_ERROR !!! \n");
				break;
			case EVENT_VIDEO_DECODER_ERROR:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_VIDEO_DECODER_ERROR !!! \n");
				break;
			case EVENT_VIDEO_RENDER_ERROR:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_VIDEO_RENDER_ERROR !!! \n");
				break;
			case EVENT_AUDIO_DECODER_ERROR:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_AUDIO_DECODER_ERROR !!! \n");
				break;
			case EVENT_AUDIO_CONVERT_ERROR:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_AUDIO_CONVERT_ERROR !!! \n");
				break;
			case EVENT_AUDIO_RENDER_ERROR:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_AUDIO_RENDER_ERROR !!! \n");
				break;
			case EVENT_ENABLE_VIDEO:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_ENABLE_VIDEO !!! \n");
				break;
			case EVENT_CHANGE_STREAM_INFO:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVENT_CHANGE_STREAM_INFO !!! \n");
				break;
			case EVNT_CHANGE_DEFAULT_BUFFERING_TIME:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] EVNT_CHANGE_DEFAULT_BUFFERING_TIME !!! \n");
				break;
			case EVENT_END_OF_STREAM:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] End of stream(%d)\n", Value);
				if (Value == 0)
					m_bAudioEnd = CTRUE;
				if (Value == 1)
					m_bVideoEnd = CTRUE;
				if (EventCallback){
					m_cbObj = this;
					EventCallback(m_cbObj, EVENT_END_OF_STREAM, 0, 0);
				}
				break;
			default:
				NX_DbgMsg(DBG_MSG_OFF, "[Filter|Manager] Unknwon defined event !!! \n");
				break;
			}
		}
	}

	return (void*)0xDEADDEAD;
}

//
//////////////////////////////////////////////////////////////////////////////

