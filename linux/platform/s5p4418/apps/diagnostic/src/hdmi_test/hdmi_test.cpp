#include <pthread.h>
#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#include <uevent.h>

#if 1	//	1024 x 600 UI
static SDL_Rect st_BGRect    = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect    = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect   = { 520, 480, 490, 110 };
static SDL_Rect st_STATERect = { 280, 210, 460, 110 };
#endif

class HDMITestWnd: public BaseAppWnd
{
public:
	HDMITestWnd();
	~HDMITestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

private:
	static void *ThreadStub(void *arg);
	void ThreadProc();

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

	char			m_HDMIStateStr[64];
	NXDiagButton	*m_pStateBtn;

	pthread_t		m_hThread;
	int				m_bRunning;
};

HDMITestWnd::HDMITestWnd()
	: m_bEnabled(0)
	, m_bRunning(1)
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_BtnList = NULL;
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	strcpy( m_HDMIStateStr, "HDMI : Not connected" );

	if(pthread_create(&m_hThread,NULL,ThreadStub,(void*)this) < 0){
		m_bRunning=0 ;
	}
}

HDMITestWnd::~HDMITestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	if( m_bRunning )
	{
		m_bRunning = 0;
		pthread_join( m_hThread, NULL );
	}
}

void HDMITestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect   , BUTTON_DISABLED, BTN_ACT_NONE  , NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect   , BUTTON_NORMAL  , BTN_ACT_OK    , NULL );		btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect  , BUTTON_NORMAL  , BTN_ACT_NOK   , NULL );		btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_STATERect, BUTTON_NORMAL  , BTN_ACT_TOGGLE, NULL );		btn = btn->next;
	m_pStateBtn = btn;		//	State Button

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor  = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_BtnPColor = SDL_MapRGB(m_Surface->format, 0x85, 0x00, 0x00 );
	m_PenColor  = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int HDMITestWnd::EventLoop( int lastResult )
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

void HDMITestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;

	//	Fill BackGround
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );			btn = btn->next;

	//	OK Button
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;

	//	NOK Button
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );
	btn = btn->next;

	//	Status Button
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, m_HDMIStateStr, btn->rect );
	btn = btn->next;
	SDL_Flip( m_Surface );
}

int	HDMITestWnd::Action( SDL_UserEvent *usrEvent )
{
	switch(usrEvent->code)
	{
		case BTN_ACT_OK:
			m_TestResult = TEST_PASSED;
			break;
		case BTN_ACT_NOK:
			m_TestResult = TEST_FAILED;
			break;
	}
	return 0;
}

void *HDMITestWnd::ThreadStub( void *param )
{
	HDMITestWnd *pObj = (HDMITestWnd *)param;
	pObj->ThreadProc();
	return (void*)0xDEADDEAD;
}


#define HDMI_STATE_FILE     "/sys/class/switch/hdmi/state"
void HDMITestWnd::ThreadProc()
{
	int err;	
	int fd;
	struct pollfd fds[1];
	static unsigned char uevent_desc[2048];
	char val;

	uevent_init();
	fd = open(HDMI_STATE_FILE, O_RDONLY);
	if( fd > 0 )
	{
		if (read(fd, (char *)&val, 1) == 1 && val == '1')
		{
			strcpy( m_HDMIStateStr, "HDMI : Connected" );
		}
		else
		{
			strcpy( m_HDMIStateStr, "HDMI : Disconnected" );
		}
		close(fd);
	}

    fds[0].fd = uevent_get_fd();
    fds[0].events = POLLIN;
	while(m_bRunning)
	{
        err = poll(fds, 1, 500);

		if (err > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				int len = uevent_next_event((char *)uevent_desc, sizeof(uevent_desc) - 2);
				if( len < 0 )
					continue;

				int isHdmiEvent = !strcmp((const char *)uevent_desc, (const char *)"change@/devices/virtual/switch/hdmi");
				if (isHdmiEvent)
				{
					fd = open(HDMI_STATE_FILE, O_RDONLY);
					if (fd < 0)
					{
						printf("failed to open hdmi state fd: %s", HDMI_STATE_FILE);
					}
					if( fd > 0 )
					{
						if (read(fd, &val, 1) == 1 && val == '1') {
							strcpy( m_HDMIStateStr, "HDMI : Connected" );
						}
						else
						{
							strcpy( m_HDMIStateStr, "HDMI : Disconnected" );
						}
						close(fd);
					}
				}
			}
		}
		else if (err == -1)
		{
			printf("error in vsync thread \n");
		}
    }
}


int test_HDMI( int last_result )
{
	int result;
	HDMITestWnd *HDMIWnd = new HDMITestWnd();
	HDMIWnd->Initialize();
	HDMIWnd->UpdateWindow();
	result = HDMIWnd->EventLoop( last_result );
	delete HDMIWnd;
	return result;
}

//
static NXDiagPluginInfo HDMITestPluginInfo = {
	"HDMI TEST",
	"HDMI Diagnostic Application",
	"",
	test_HDMI,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &HDMITestPluginInfo;
}
