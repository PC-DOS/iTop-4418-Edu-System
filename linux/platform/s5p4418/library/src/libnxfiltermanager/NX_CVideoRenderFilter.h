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
#ifndef __NX_CVideoRenderFilter_h__
#define __NX_CVideoRenderFilter_h__

#include "NX_MediaType.h"

#ifdef __cplusplus

#include <nx_fourcc.h>
#include <vpu_types.h>
#include <nx_video_api.h>
#include <nx_alloc_mem.h>
#include <nx_dsp.h>

#include "NX_CBaseFilter.h"
#include "NX_IBaseFilter.h"
#include "NX_IDisplayControl.h"

#include <pthread.h>

#define		MAX_NUM_VIDEO_OUT_BUF		(3)

NX_IBaseFilter *CreateVideoRenderFilter(const char *filterName);

typedef struct tagRENDER_INFO{
	DISPLAY_HANDLE hDsp;
	int32_t moduleId;			//	0:MLC0, 1:MLC1
	int32_t port;				//	0:LCD,  1:HDMI
	uint8_t fourcc;				//	Input Data Format
	int32_t srcWidth;			//	Input Image Width
	int32_t srcHeight;			//	Input Image Height
	int32_t dstX;				//	Display Position X
	int32_t dstY;				//	Display Position Y
	int32_t dstWidth;			//	Display Width
	int32_t dstHeight;			//	Display Height
	int32_t videoPriority;		//	Video Layer Priority
	uint32_t colorKey;			//	Color Key
	int32_t bInit;				//	0:Not Intialized, 1:Initialized
	int32_t bEnabled;			//	0:Disable, 1:Enable
	void *pPrevImage;			//	Previous Image Buffer
	int32_t Init;				//(image->luStride != src_width)
	DISPLAY_INFO dspInfo;
}RENDER_INFO;

typedef struct POSITION_INFO{
	int moduleId;			//	0:MLC0, 1:MLC1
	int port;				//	0:LCD,  1:HDMI
	int dstX;				//	Display Position X
	int dstY;				//	Display Position Y
	int dstWidth;			//	Display Width
	int dstHeight;			//	Display Height
}POS_INFO;

class NX_CDisplay;
class NX_CEventNotifier;
class NX_CVideoRenderInputPin;

//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Video Renderer Filter
//
class NX_CVideoRenderFilter : 
	public NX_CBaseFilter, public NX_IBaseFilter, public NX_IDisplayControl
{
public:
	NX_CVideoRenderFilter(const char *filterName);
	virtual ~NX_CVideoRenderFilter();

public:	//	for Video Rendering
	pthread_t			m_hThread;
	pthread_mutex_t		m_hMutexThread;
	CBOOL				m_bExitThread;
	CBOOL				m_bPaused;
	static void			*ThreadStub( void *pObj );
	void				ThreadProc();
	void ExitThread(){
		pthread_mutex_lock(&m_hMutexThread);
		m_bExitThread = CTRUE;
		pthread_mutex_unlock(&m_hMutexThread);
	}

	//	Override  BaseFilter
	virtual CBOOL		Run();
	virtual CBOOL		Stop();
	virtual	NX_CBasePin	*GetPin(int Pos);
	//

	void				Resume(void);
	void				ChangeBufferingTime( uint32_t BufferingTime );

public:
	//IBaseFilter Pure Virtual
	virtual CBOOL		PinInfo(NX_PinInfo *PinInfo);
	virtual NX_CBasePin	*FindPin(int Pos);
	virtual CBOOL		FindInterface(const char *InterfaceId, void **Interface);
	virtual void		SetClockReference(NX_CClockRef *pClockRef);
	virtual void		SetEventNotifi(NX_CEventNotifier *pEventNotifier);
	virtual CBOOL		Pause(void);
	//
	//IDisplayControl
	virtual CBOOL		SetVideoPosition(int moudleId, int port, int x, int y, int width, int height);		 		//	Pure Virtual
	virtual CBOOL		SetDisplay(int32_t LCD_On_OFF, int32_t HDMI_ON_FF);
	//
	int32_t				SetMediaType(void *Mediatype);
	int32_t				RenderSample( NX_CSample *pSample );
	CBOOL				Flush();


	NX_CVideoRenderInputPin		*m_pInputPin;
	NX_CSampleQueue				*m_pInQueue;
	NX_CDisplay					*m_pRender;
	int32_t						m_SrcWidth;
	int32_t						m_SrcHeight;
	int32_t						m_AverageTimePerFrame;
	NX_CSample					*m_pPrevSample;
	int32_t						m_MaxSleepTime;
	NX_CSemaphore				m_SemPause;
	int32_t						m_Init;

#ifdef _DEBUG
	/////////////////////////////////////////////////////////////////////////////
	//
	//						Debug Informations
	//
public:
	void DisplayVersion();
	void DisplayState();
	void DisplayStatistics();
	void GetStatistics( uint32_t *InFrameCnt, uint32_t *OutFrameCnt, uint32_t *DropFrameCnt );
	void ClearStatistics();
	uint32_t		m_DbgInFrameCnt;
	uint32_t		m_DbgOutFrameCnt;
	uint32_t		m_DbgDropFrameCnt;


	//
	/////////////////////////////////////////////////////////////////////////////
#endif

private:
	NX_CVideoRenderFilter (NX_CVideoRenderFilter &Ref);
	NX_CVideoRenderFilter &operator=(NX_CVideoRenderFilter &Ref);
};
//
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//				Nexell Video Renderer Input Pin for Linux
//

class NX_CVideoRenderInputPin: 
	public NX_CBaseInputPin
{
public:
	NX_CVideoRenderInputPin( NX_CVideoRenderFilter *pOwnerFilter );
	virtual ~NX_CVideoRenderInputPin();

protected:
	virtual int32_t Receive( NX_CSample *pSample );
	virtual int32_t	CheckMediaType(void *Mediatype);
};

//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//					Nexell Video Display API
//

class NX_CDisplay
{
public:
	NX_CDisplay( NX_CVideoRenderFilter *pFilter, int32_t Width, int32_t Height );
	virtual ~NX_CDisplay();

	//	Methods
public:
	CBOOL					Init(int32_t Width, int32_t Height);
	RENDER_INFO *			InitRender(int32_t Module, int32_t Port, int32_t Width, int32_t Height);
	void					Display( uint32_t* pInBuf, int32_t InSize, CBOOL Valid, CBOOL bWait );
	CBOOL					Close();
	//	Display Control Methods
	CBOOL					SetVideoPosition(int moudleId, int port, int x, int y, int width, int height);
	CBOOL					SetDisplayer(int32_t LCD_On_OFF, int32_t HDMI_ON_FF);

	int32_t					m_SrcWidth, m_SrcHeight;
	int32_t					m_DstX, m_DstY, m_DstWidth, m_DstHeight;
	int32_t					m_Module;
	int32_t					m_Port;
	int32_t					m_Priority;
	NX_CMutex				m_DspMutex;
	NX_CVideoRenderFilter	*m_pOwnerFilter;
	RENDER_INFO				*m_info;
	RENDER_INFO				*m_info_hdmi;
	int32_t					m_LcdFlag;
	int32_t					m_HdmiFlag;
};
//
//
//////////////////////////////////////////////////////////////////////////////


#endif	//	__cplusplus

#endif	//	__NX_CVideoRenderFilter_h__
