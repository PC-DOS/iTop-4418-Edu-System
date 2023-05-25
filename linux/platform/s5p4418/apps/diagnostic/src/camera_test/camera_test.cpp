#include <pthread.h>
#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>
#include <unistd.h>
#include <errno.h>

#include <nx_fourcc.h>
#include <nx_vip.h>			//	VIP
#include <nx_dsp.h>		//	Display

#if 1	//	1024 x 600 UI
static SDL_Rect st_BGRect     = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect     = {  10, 480, 320, 110 };
static SDL_Rect st_TOGGLERect = { 350, 480, 320, 110 };
static SDL_Rect st_NOKRect    = { 690, 480, 320, 110 };
#endif

#define	MAX_CAMERA_PORTS 2
int const CameraPortList[MAX_CAMERA_PORTS] =
{
	VIP_PORT_0,
	VIP_PORT_MIPI
};

class CameraTestWnd: public BaseAppWnd
{
public:
	CameraTestWnd();
	~CameraTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

private:
	static void *ThreadStub(void *arg);
	void ThreadProc();
	void CameraToggle();

private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;
	char	m_LastMessage[1024];
	int		m_bEnabled;

	unsigned int	m_BgColor;;
	unsigned int	m_BtnColor;
	unsigned int	m_BtnPColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;

	int				m_bRunning;
	pthread_t		m_Thread;

	int				m_CameraToggleIdx;
	int				m_CameraPort;		//	0 or 1
};

CameraTestWnd::CameraTestWnd()
	: m_bEnabled(0)
	, m_bRunning(1)
	, m_CameraToggleIdx(0)
	, m_CameraPort(CameraPortList[m_CameraToggleIdx++])
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_BtnList = NULL;
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	if(pthread_create(&m_Thread,NULL,ThreadStub,(void*)this) < 0){
		m_bRunning = 0;
	}
}

CameraTestWnd::~CameraTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );

	if( m_bRunning )
	{
		m_bRunning = 0;
		pthread_join( m_Thread, NULL );
	}
}

void CameraTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect     , BUTTON_DISABLED, BTN_ACT_NONE  , NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect     , BUTTON_NORMAL  , BTN_ACT_OK    , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_TOGGLERect , BUTTON_NORMAL  , BTN_ACT_TOGGLE, NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect    , BUTTON_NORMAL  , BTN_ACT_NOK   , NULL );	btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor  = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_BtnPColor = SDL_MapRGB(m_Surface->format, 0x85, 0x00, 0x00 );
	m_PenColor  = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int CameraTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;

	BUTTON_STATE next_state;
	m_TestResult = lastResult;
	
	while( !bExit )
	{
		update = 0;
		hit_test = 0;
		processed   = 0;

		SDL_MouseButtonEvent *event_btn = &sdl_event.button;
		SDL_WaitEvent( &sdl_event );
		for( btn = m_BtnList; 0!=btn&&0==processed; btn = btn->next )
		{
			if( btn->state == BUTTON_DISABLED ) continue;
			hit_test =	(btn->rect.x<=event_btn->x) &&
						(btn->rect.y<=event_btn->y) &&
						((btn->rect.x+btn->rect.w)>event_btn->x) &&
						((btn->rect.y+btn->rect.h)>event_btn->y) ;

			next_state = btn->state;
		//printf("Camera Event : %d\n\n",sdl_event.type);//bok test
			switch( sdl_event.type )
			{
				case SDL_MOUSEBUTTONDOWN:
					if( BUTTON_NORMAL == btn->state && hit_test ){
						processed = 1;
						next_state = BUTTON_FOCUS_IN;
					}
					break;
				case SDL_MOUSEMOTION:
					if( BUTTON_FOCUS_OUT == btn->state || BUTTON_FOCUS_IN == btn->state )
					{
						if( hit_test ) next_state = BUTTON_FOCUS_IN;
						else           next_state = BUTTON_FOCUS_OUT;
						processed = 1;
					}		
					break;
				case SDL_MOUSEBUTTONUP:
					if( BUTTON_FOCUS_OUT == btn->state || BUTTON_FOCUS_IN == btn->state ){
						next_state = BUTTON_NORMAL;
						processed = 1;
						//	send event
						if( hit_test && BTN_ACT_NONE != btn->action )
						{
							// send message
							SDL_Event evt;
							evt.type = SDL_USEREVENT;
							evt.user.type = SDL_USEREVENT;
							evt.user.code = btn->action;
							evt.user.data1 = btn;
							//event.user.data2= ;
							SDL_PushEvent(&evt);
						}
					}
					break;
			}

			if( btn->state != next_state )	update = 1;
			btn->state = next_state;
		}
		if( sdl_event.type == SDL_USEREVENT ){
			Action( &sdl_event.user );
			if(sdl_event.user.code <= BTN_ACT_NOK)
			{
				bExit = true;
				processed = 1;
			}
			update = 1;
		}

		if( update ){
			UpdateWindow();
			update = 0;
		}
					
		if ( bExit ){
			break;
		}
	}

	return m_TestResult;
}

void CameraTestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );
	
	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "TOGGLE", btn->rect );
	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );
	SDL_Flip( m_Surface );
}

int	CameraTestWnd::Action( SDL_UserEvent *usrEvent )
{
	switch(usrEvent->code)
	{
		case BTN_ACT_OK:
			m_TestResult = TEST_PASSED;
			break;
		case BTN_ACT_NOK:
			m_TestResult = TEST_FAILED;
			break;
		case BTN_ACT_TOGGLE:
			CameraToggle();
			break;
	}
	return 0;
}

void *CameraTestWnd::ThreadStub( void *param )
{
	CameraTestWnd *pObj = (CameraTestWnd *)param;
	pObj->ThreadProc();
	return (void*)0xDEADDEAD;
}


#define	CAP_BUFFERS		4
#define	CAP_WIDTH		1024
#define	CAP_HEIGHT		768
void CameraTestWnd::ThreadProc()
{
	//	Camera
	VIP_HANDLE hVip;
	VIP_INFO vipInfo;

	//	Display
	DISPLAY_HANDLE hDsp;
	DISPLAY_INFO dspInfo;

	NX_VID_MEMORY_HANDLE hMem[CAP_BUFFERS];

	NX_VID_MEMORY_INFO *pPrevDsp = NULL;
	NX_VID_MEMORY_INFO *pCurCapturedBuf = NULL;

	int width  = CAP_WIDTH;
	int height = CAP_HEIGHT;
	int pos = 0;
	long long timeStamp;

	//	Allocation Memory
	for( int i=0; i<CAP_BUFFERS ; i++ )
	{
		hMem[i] = NX_VideoAllocateMemory( 4096, width, height, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );
	}

	//	Initialize Video Input Processor
	memset( &vipInfo, 0, sizeof(vipInfo) );
	vipInfo.port       = m_CameraPort;
	vipInfo.mode       = VIP_MODE_CLIPPER;
	//	Sensor Input Size
	vipInfo.width      = width;
	vipInfo.height     = height;
	vipInfo.numPlane   = 1;
	//	Clipper Setting
	vipInfo.cropX      = 0;
	vipInfo.cropY      = 0;
	vipInfo.cropWidth  = width;
	vipInfo.cropHeight = height;
	//	Fps
	vipInfo.fpsNum = 30;
	vipInfo.fpsDen = 1;
	hVip = NX_VipInit( &vipInfo );

	//	Initailize Display
	dspInfo.port              = DISPLAY_PORT_LCD;	//	Display Port( DISPLAY_PORT_LCD or DISPLAY_PORT_HDMI )
	dspInfo.module            = 0;					//	MLC Module (0 or 1)
	dspInfo.width             = width;				//	Source Width
	dspInfo.height            = height;				//	Source Height
	dspInfo.numPlane          = 1;					//	Image Plane Number
	dspInfo.dspSrcRect.left   = 0;					//	Clipper Setting
	dspInfo.dspSrcRect.top    = 0;
	dspInfo.dspSrcRect.right  = width;
	dspInfo.dspSrcRect.bottom = height;
	dspInfo.dspDstRect.left   = 0;					//	MLC Scaler Setting
	dspInfo.dspDstRect.top    = 0;
	dspInfo.dspDstRect.right  = 1024;
	dspInfo.dspDstRect.bottom = 465;
	hDsp = NX_DspInit( &dspInfo );

	NX_DspVideoSetPriority( 0, 0 );

	NX_VipQueueBuffer( hVip, hMem[pos] );

	while(m_bRunning)
	{
		NX_VipQueueBuffer( hVip, hMem[pos] );
		NX_VipDequeueBuffer( hVip, &pCurCapturedBuf, &timeStamp );

		NX_DspQueueBuffer( hDsp, pCurCapturedBuf );
		if( pPrevDsp )
		{
			NX_DspDequeueBuffer( hDsp );
		}
		pPrevDsp = pCurCapturedBuf;
		pos = (pos + 1)%CAP_BUFFERS;
	}

	NX_DspVideoSetPriority( 0, 2 );

	NX_DspClose( hDsp );
	NX_VipClose( hVip );
}

void CameraTestWnd::CameraToggle()
{
	if( m_bRunning )
	{
		m_bRunning = 0;
		pthread_join( m_Thread, NULL );
	}

	m_CameraPort = CameraPortList[m_CameraToggleIdx++];
	m_CameraToggleIdx %= MAX_CAMERA_PORTS;

	m_bRunning = 1;
	if(pthread_create(&m_Thread,NULL,ThreadStub,(void*)this) < 0){
		m_bRunning = 0;
	}
}


int test_Camera( int last_result )
{
	int result;
	CameraTestWnd *CameraWnd = new CameraTestWnd();
	CameraWnd->Initialize();
	CameraWnd->UpdateWindow();
	result = CameraWnd->EventLoop( last_result );
	delete CameraWnd;
	return result;
}

//
static NXDiagPluginInfo CameraTestPluginInfo = {
	"CAMERA TEST",
	"Camera Diagnostic Application",
	"",
	test_Camera,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &CameraTestPluginInfo;
}
