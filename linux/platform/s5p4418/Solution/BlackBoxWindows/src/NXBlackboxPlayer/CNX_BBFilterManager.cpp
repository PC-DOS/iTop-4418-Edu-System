#include "stdafx.h"
#include "CNX_BBFilterManager.h"
#include "NX_FilterGuid.h"

//----------------------------------------------------------------------------
#define	MODULE_NAME	TEXT("Filter Manager")
#define	SAFE_RELEASE(OBJ) if(OBJ){OBJ->Release();OBJ=NULL;}
#define REGISTER_FILTERGRAPH


#define	VIDEO_DECODER_NAME	L"NEXELL Mpeg4 Decode Filter"
#define	AUDIO_DECODER_NAME	L"NEXELL AUDIO Decode Filter"

#if(0)
#define	DEMUX_VIDEO_PIN_NAME_0	L"Video_0"
#define	DEMUX_VIDEO_PIN_NAME_1	L"Video_1"
#define	DEMUX_VIDEO_PIN_NAME_2	L"Video_2"
#define	DEMUX_AUDIO_PIN_NAME	L"Audio_0"
#else
#define	DEMUX_VIDEO_PIN_NAME_0	L"Video Out_0"
#define	DEMUX_VIDEO_PIN_NAME_1	L"Video Out_1"
#define	DEMUX_VIDEO_PIN_NAME_2	L"Video Out_2"
#define	DEMUX_AUDIO_PIN_NAME	L"Audio Out"
#endif


#ifdef REGISTER_FILTERGRAPH
//----------------------------------------------------------------------------
static DWORD g_dwGraphRegister;
//----------------------------------------------------------------------------
static HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) 
    {
        return E_FAIL;
    }

	WCHAR wsz[128];
	wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
			GetCurrentProcessId());

    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}
//----------------------------------------------------------------------------
static void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}
#endif



//============================================================================
CNX_BBFilterManager::CNX_BBFilterManager( int FileType )
	: m_pGB(NULL)				//	Graph Builder
	, m_pMC(NULL)				//	Basic Media Control
	, m_pME(NULL)				//	Media Event
	, m_pMS(NULL)				//	Media Seeking
	, m_pVW(NULL)				//	Video Window Interface for First Window
	, m_pVW2(NULL)				//	Video WIndow Interface for Second Window
	, m_pVW3(NULL)				//	Video WIndow Interface for Third Window
	, m_pBA(NULL)				//	Basic Audio Interface   (Volume Control)
	, m_pDemuxFilter(NULL)		//	Demuxer Filter
	, m_pAudioDecFilter(NULL)
	, m_NumVideoTrack(0)
	, m_FileType(FileType)
{
	if(FAILED(CoInitialize(NULL)))
	{
        exit(1);
	}
	for( int i=0 ; i<MAX_VIDEO_TRACKS ; i++ )
	{
		m_pVideoDecFilter[i] = NULL;
	}
}

//============================================================================
CNX_BBFilterManager::~CNX_BBFilterManager()
{
	Close();
	CoUninitialize();
}


//============================================================================
//	Init
//		Step 1. Create Filter Graph Manager
//		Step 2. Load Demuxer Filter & Load File
//		Step 3. Find Audio/Video Pins
//		Step 4. Load Audio/Video Decoder Filters
//		Step 5. Render File & Get Common Control Interface for Display
int CNX_BBFilterManager::Init( const char *fileName, OAHWND hEventOwner, unsigned long eventID, OAHWND hFirstOwner, OAHWND hSecondOwner, OAHWND hThirdOwner )
{
	//	TODO : Check Input Paramters


	//
	//	Step 1. Create Filter Graph Manger & Common Control Interfaces
	//
    if( CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&m_pGB) != S_OK )
	{
		MessageBox( NULL, TEXT("IGraphBuilder Create Failed"), TEXT("Error!!!"), MB_OK );
		goto ErrorExit;
	}

	//	Get Common Media Cotnrols
	m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
	m_pGB->QueryInterface(IID_IMediaEventEx, (void **)&m_pME);
	m_pME->SetNotifyWindow( hEventOwner, eventID, NULL );

	//
	//	Step 2. Create Demuxer Filter & Load File
	//
	if( m_FileType == FILE_TYPE_MP4 )
	{
		if( S_OK != LoadFilter( &m_pASyncReader, CLSID_AsyncReader, NULL ) )
		{
			MessageBox( NULL, TEXT("File AsyncReader Create Failed"), TEXT("Error!!!"), MB_OK );
			goto ErrorExit;
		}
		else
		{
			//	Open File
			IFileSourceFilter *pIFileSrc;
			if( m_pASyncReader->QueryInterface( IID_IFileSourceFilter, (void**)&pIFileSrc ) != S_OK )
			{
				MessageBox( NULL, TEXT("IID_IFileSourceFilter QueryInterface() Failed!!"), TEXT("Error!!!"), MB_OK );
				goto ErrorExit;
			}
			WCHAR wsFileName[1024];
			memset( wsFileName, 0, sizeof(wsFileName) );
			MultiByteToWideChar( CP_ACP, NULL, fileName, strlen(fileName), wsFileName, sizeof(wsFileName)/sizeof(WCHAR) );

			pIFileSrc->Load( wsFileName, NULL );
			pIFileSrc->Release();
		}

		if( S_OK != LoadFilter( &m_pDemuxFilter, CLSID_NXMP4PARSER, L"NEXELL Demux Filter for BlackBox(MP4)" ) )
		{
			MessageBox( NULL, TEXT("DemuxFilter Create Failed(MP4)"), TEXT("Error!!!"), MB_OK );
			goto ErrorExit;
		}
	}
	else if( m_FileType == FILE_TYPE_MP2TS )
	{
		if( S_OK != LoadFilter( &m_pDemuxFilter, CLSID_NXTSDemuxer, L"NEXELL Demux Filter for BlackBox(TS)" ) )
		{
			MessageBox( NULL, TEXT("DemuxFilter Create Failed(TS)"), TEXT("Error!!!"), MB_OK );
			goto ErrorExit;
		}
		//	Open File
		IFileSourceFilter *pIFileSrc;
		if( m_pDemuxFilter->QueryInterface( IID_IFileSourceFilter, (void**)&pIFileSrc ) != S_OK )
		{
			MessageBox( NULL, TEXT("IID_IFileSourceFilter QueryInterface() Failed!!"), TEXT("Error!!!"), MB_OK );
			goto ErrorExit;
		}
		WCHAR wsFileName[1024];
		memset( wsFileName, 0, sizeof(wsFileName) );
		MultiByteToWideChar( CP_ACP, NULL, fileName, strlen(fileName), wsFileName, sizeof(wsFileName)/sizeof(WCHAR) );

		pIFileSrc->Load( wsFileName, NULL );
		pIFileSrc->Release();
	}

	//
	//	Step 3. Find Audio/Video Pins
	//
	m_NumVideoTrack = 3;

	//
	//	Step 4. Load Audio/Video Decoder Filters
	//
	for( int i=0 ; i<m_NumVideoTrack ; i++ )
	{
		if( S_OK != LoadFilter( &m_pVideoDecFilter[i], CLSID_NX_VIDEODECODER, NULL ) )
		{
			MessageBox( NULL, TEXT("Video Decoder Create Failed"), TEXT("Error!!!"), MB_OK );
			goto ErrorExit;
		}
	}

	if( S_OK != LoadFilter( &m_pAudioDecFilter, CLSID_NXAudioDecoderFilter, VIDEO_DECODER_NAME ) )
	{
		MessageBox( NULL, TEXT("Video Decoder Create Failed"), TEXT("Error!!!"), MB_OK );
		goto ErrorExit;
	}

	//
	//	Step 5. Render File & Get Common Control Interface for Display
	//
	if( m_FileType == FILE_TYPE_MP4 )
		RenderFilter( m_pASyncReader );
	else if( m_FileType == FILE_TYPE_MP2TS )
		RenderFilter( m_pDemuxFilter );
	else
		goto ErrorExit;

	{
		//	Find Video Window 0
		IPin *pIPin, *pConPin;
		{
			if( m_pDemuxFilter->FindPin( DEMUX_VIDEO_PIN_NAME_0, &pIPin ) == S_OK ){
				if( pIPin->ConnectedTo( &pConPin ) == S_OK )
				{
					PIN_INFO pinInfo;
					pConPin->QueryPinInfo( &pinInfo );
					if( FindVideoWindowInterface( pinInfo.pFilter, &m_pVW ) != S_OK )
					{
						MessageBox( NULL, TEXT("Cannot Find VideoWindow for Video 0 Port"), TEXT("Error"), MB_OK );
					}
					pConPin->Release();
					pinInfo.pFilter->Release();
				}
				pIPin->Release();
			}
		}

		//	Find Video Window 2
		{
			if( m_pDemuxFilter->FindPin(DEMUX_VIDEO_PIN_NAME_1, &pIPin ) == S_OK ){
				if( pIPin->ConnectedTo( &pConPin ) == S_OK )
				{
					PIN_INFO pinInfo;
					pConPin->QueryPinInfo( &pinInfo );
					if( FindVideoWindowInterface( pinInfo.pFilter, &m_pVW2 ) != S_OK )
					{
						MessageBox( NULL, TEXT("Cannot Find VideoWindow for Video 1 Port"), TEXT("Error"), MB_OK );
					}
					pConPin->Release();
					pinInfo.pFilter->Release();
				}
				pIPin->Release();
			}
		}

		//	Find Video Window 3
		{
			if( m_pDemuxFilter->FindPin(DEMUX_VIDEO_PIN_NAME_2, &pIPin ) == S_OK ){
				if( pIPin->ConnectedTo( &pConPin ) == S_OK )
				{
					PIN_INFO pinInfo;
					pConPin->QueryPinInfo( &pinInfo );
					if( FindVideoWindowInterface( pinInfo.pFilter, &m_pVW3 ) != S_OK )
					{
						MessageBox( NULL, TEXT("Cannot Find VideoWindow for Video 2 Port"), TEXT("Error"), MB_OK );
					}
					pConPin->Release();
					pinInfo.pFilter->Release();
				}
				pIPin->Release();
			}
		}
	}

	//	Get Media Seeking Interface
	m_pGB->QueryInterface(IID_IMediaSeeking, (void **)&m_pMS);
	m_pGB->QueryInterface(IID_IBasicAudio, (void **)&m_pBA);

#define DEBUG_DSP_WINDOW	0
	//	Set Default Video Window (optional)
	{
		//	Set Window Style & Postion
		RECT rect;
		long style;
		if( m_pVW ){
#if !DEBUG_DSP_WINDOW
			m_pVW->put_Owner( hFirstOwner );
			GetWindowRect( (HWND)hFirstOwner, &rect );
			style = 0x40000000;
			m_pVW->put_WindowStyle( style );
			m_pVW->SetWindowPosition( 2, 2, rect.right - rect.left - 8, rect.bottom - rect.top - 8);
#else	//	Debugging
			GetWindowRect( (HWND)hFirstOwner, &rect );
			m_pVW->get_WindowStyle( &style );
			m_pVW->put_WindowStyle( style & ~(WS_BORDER) );
			m_pVW->SetWindowPosition( 10, 10, rect.right - rect.left, rect.bottom - rect.top );
#endif
		}
		if( m_pVW2 ){
#if !DEBUG_DSP_WINDOW
			m_pVW2->put_Owner( hSecondOwner );
			GetWindowRect( (HWND)hSecondOwner, &rect );
			style = 0x40000000;
			m_pVW2->put_WindowStyle( style );
			m_pVW2->SetWindowPosition( 2, 2, rect.right - rect.left - 8, rect.bottom - rect.top - 8);
#else
			GetWindowRect( (HWND)hSecondOwner, &rect );
			m_pVW2->get_WindowStyle( &style );
			m_pVW2->put_WindowStyle( style & ~(WS_BORDER) );
			m_pVW2->SetWindowPosition( 660, 10, 640, 480 );
#endif
		}
		if( m_pVW3 ){
#if !DEBUG_DSP_WINDOW
			m_pVW3->put_Owner( hThirdOwner );
			GetWindowRect( (HWND)hThirdOwner, &rect );
			style = 0x40000000;
			m_pVW3->put_WindowStyle( style );
			m_pVW3->SetWindowPosition( 0, 0, rect.right - rect.left, rect.bottom - rect.top );
#else
			GetWindowRect( (HWND)hThirdOwner, &rect );
			m_pVW3->get_WindowStyle( &style );
			m_pVW3->put_WindowStyle( style & ~(WS_BORDER) );
			m_pVW3->SetWindowPosition( 660, 10, 640, 480 );
#endif
		}
	}

#ifdef REGISTER_FILTERGRAPH
	if (FAILED(AddGraphToRot(m_pGB, &g_dwGraphRegister)))
	{
		MessageBox(NULL, TEXT("Failed to register filter graph with ROT!"), TEXT("ERROR"), MB_OK );
		g_dwGraphRegister = 0;
	}
#endif

	return 0;

ErrorExit:
	Close();
	MessageBox( NULL, TEXT("Init Error!!!"), MODULE_NAME, MB_OK );
	return -1;
}

//============================================================================
int CNX_BBFilterManager::Close( void )
{
#ifdef REGISTER_FILTERGRAPH
	RemoveGraphFromRot(g_dwGraphRegister);
	g_dwGraphRegister = 0;
#endif

	Stop();

	SAFE_RELEASE( m_pDemuxFilter );
	for( int i=0 ; i<m_NumVideoTrack ; i++ )
	{
		SAFE_RELEASE( m_pVideoDecFilter[i] );
	}
	SAFE_RELEASE( m_pAudioDecFilter );
	SAFE_RELEASE( m_pMC );
	SAFE_RELEASE( m_pME );
	SAFE_RELEASE( m_pMS );
	SAFE_RELEASE( m_pVW );
	SAFE_RELEASE( m_pVW2);
	SAFE_RELEASE( m_pVW3);
	SAFE_RELEASE( m_pBA );
	SAFE_RELEASE( m_pGB );

	return 0;
}

//============================================================================
//	Play Control
int CNX_BBFilterManager::Play( void )
{
	if( m_pMC )
	{
		HRESULT ret = m_pMC->Run();
		if( ret == S_OK )
			m_bPaused = false;
		return ret;
	}
	return -1;
}
//============================================================================
int CNX_BBFilterManager::Stop( void )
{
	if( m_pMC )
	{
		HRESULT ret = m_pMC->Stop();
		if( ret == S_OK )
			m_bPaused = false;
		return ret;
	}
	return -1;
}
//============================================================================
int CNX_BBFilterManager::Pause( void )
{
	if( m_pMC )
	{
		HRESULT ret = m_pMC->Pause();
		if( ret == S_OK )
			m_bPaused = true;
		return ret;
	}
	return -1;
}
//============================================================================
int CNX_BBFilterManager::FrameStep( int nStep )
{
	if( m_pGB )
	{
		IVideoFrameStep *pIFrameStep;
		if( m_pGB->QueryInterface( IID_IVideoFrameStep, (void**)&pIFrameStep ) == S_OK )
		{
			pIFrameStep->Step( nStep, NULL );
			pIFrameStep->Release();
		}
	}
	return 0;
}

//	Stream Control & Seeking
//============================================================================
int CNX_BBFilterManager::Seek( unsigned long time )
{
	if( m_pMS )
	{
		long long seekTime = (long long)time * 10000;
		if( m_pMS->SetPositions( &seekTime,AM_SEEKING_AbsolutePositioning, NULL, 0 ) != S_OK )
		{
			return -1;
		}
	}
	return -1;
}

//============================================================================
int CNX_BBFilterManager::GetDuration( unsigned long *time )
{
	if( m_pMS )
	{
		long long duration;
		if( m_pMS->GetDuration( &duration ) == S_OK )
		{
			if( duration == 0 )
				return -1;
			*time = (unsigned long)(duration / 10000);
			return 0;
		}
	}
	return -1;
}

//============================================================================
int CNX_BBFilterManager::GetCurTime( unsigned long *time )
{
	if( m_pMS )
	{
		long long current;
		if( m_pMS->GetCurrentPosition( &current ) == S_OK )
		{
			if( current == 0 ){
				*time = 0;
			}else{
				*time = (unsigned long)(current / 10000);
			}
			return 0;
		}
	}
	return -1;
}

//============================================================================
//	Audio Volume Control
int CNX_BBFilterManager::GetVolume( long *volume )
{
	if( m_pBA )
	{
		m_pBA->get_Volume( volume );
		return 0;
	}
	return -1;
}

int CNX_BBFilterManager::SetVolume( long volume )
{
	if( m_pBA )
	{
		m_pBA->put_Volume( volume );
		return 0;
	}
	return -1;
}

//============================================================================
int CNX_BBFilterManager::SetWindowPosition( int channel, int x, int y, int width, int height )
{
	if( channel==0 && m_pVW )
	{
		m_pVW->SetWindowPosition(x, y, width, height);
	}
	if( channel==1 && m_pVW2 )
	{
		m_pVW2->SetWindowPosition(x, y, width, height);
	}
	if( channel==2 && m_pVW3 )
	{
		m_pVW3->SetWindowPosition(x, y, width, height);
	}
	return 0;
}

int CNX_BBFilterManager::SetFullScreen( int channel, BOOL bEnable )
{
	return 0;
}

int CNX_BBFilterManager::SetAspectRatio( int channel, BOOL bEnable )
{
	return 0;
}


//============================================================================
int CNX_BBFilterManager::FilterEventProc( unsigned int *eventCode )
{
	LONG evCode, evParam1, evParam2;
	HRESULT hr=S_OK;

	if( !m_pGB || !m_pME )
		return -1;
	*eventCode = 0;
	while(SUCCEEDED(m_pME->GetEvent(&evCode, &evParam1, &evParam2, 0)))
	{
		// Spin through the events
		hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);
		switch( evCode )
		{
			case EC_COMPLETE:
				if( m_pMC ){
					m_pMC->Stop();
				}
				*eventCode = EVT_CODE_EOS;
				break;
		}
	}
	return hr;
}


void CNX_BBFilterManager::Move( OAHWND hWnd, int x, int y )
{
}



//----------------------------------------------------------------------------
HRESULT CNX_BBFilterManager::LoadFilter( IBaseFilter **pFilter, REFCLSID clsid, LPCWSTR pName )
{
	CString msg;
	HRESULT hr = S_OK;

	hr = CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)pFilter );
	if( hr != S_OK ){
		msg.Format(TEXT("CoCreateInstance Failed.(Instance Name:%s)"), pName);
		MessageBox( NULL, msg.GetBuffer(), TEXT("Error"), MB_OK );
		return hr;
	}
	hr = m_pGB->AddFilter( *pFilter, pName );
	if( hr != S_OK ){
		MessageBox( NULL, TEXT("AddFilter failed"), TEXT("Error"), MB_OK );
	}
	return hr;
}


//----------------------------------------------------------------------------
//	Mode
HRESULT CNX_BBFilterManager::RenderFilter( IBaseFilter *pFilter )
{
	IPin *pOutPin = NULL;
	IPin *pIPin;
	IEnumPins *pIEnum;
	PIN_INFO pinInfo;

	if( pFilter->EnumPins( &pIEnum ) == S_OK )
	{
		while( (pIEnum->Next( 1, &pIPin, NULL )==S_OK) )
		{
			pIPin->QueryPinInfo( &pinInfo );
			if( pinInfo.dir == PINDIR_OUTPUT )
			{
				m_pGB->Render( pIPin );
			}
			pinInfo.pFilter->Release();
			pIPin->Release();
		}
		pIEnum->Release();
	}
	return S_OK;
}

//----------------------------------------------------------------------------
//
//	Desc.
//		; 해당 filter 이하에 IVideoWindow Interface를 지원하는 Filter가 있는지
//		검색하는 역할을 한다.
//
HRESULT	CNX_BBFilterManager::FindVideoWindowInterface ( IBaseFilter *pFilter, IVideoWindow **ppIVW )
{
	HRESULT hr = E_FAIL;
	IPin *pIPin, *pConPin;
	IEnumPins *pIEnum;
	PIN_INFO pinInfo;

	if( pFilter->QueryInterface( IID_IVideoWindow, (void**)ppIVW ) == S_OK ){
		return S_OK;
	}

	if( pFilter->EnumPins( &pIEnum ) == S_OK )
	{
		while( (pIEnum->Next( 1, &pIPin, NULL )==S_OK) )
		{
			pIPin->QueryPinInfo( &pinInfo );
			if( pinInfo.dir == PINDIR_OUTPUT )
			{
				//	현재 filter의 출력 핀과 연결되어 있는 pConPin을 찾는다.
				if( pIPin->ConnectedTo( &pConPin ) == S_OK )
				{
					pinInfo.pFilter->Release();
					pConPin->QueryPinInfo( &pinInfo );
					pConPin->Release();
					hr = FindVideoWindowInterface( pinInfo.pFilter, ppIVW );
					if( hr == S_OK ){
						pinInfo.pFilter->Release();
						pIPin->Release();
						break;
					}
				}
			}
			pinInfo.pFilter->Release();
			pIPin->Release();
		}
		pIEnum->Release();
	}
	return hr;
}
