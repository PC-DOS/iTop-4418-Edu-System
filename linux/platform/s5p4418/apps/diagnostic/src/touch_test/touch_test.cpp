#include <pthread.h>
#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>

#if 0	//	800 x 480 UI
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
static SDL_Rect	st_TestRect= {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
#endif

#if 1	//	1024 x 600 UI
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect = { 520, 480, 490, 110 };
//static SDL_Rect	st_TestRect= {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
#endif


typedef struct TouchPoint TouchPoint;
struct TouchPoint {
	int x;
	int y;
	int valid;
};

class TouchTestWnd: public BaseAppWnd
{
public:
	TouchTestWnd();
	~TouchTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	void DrawTouchLine( bool contined );

private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;
	char	m_LastMessage[1024];
	int		m_bEnabled;

	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;
	unsigned int	m_TriColor;

	TouchPoint		m_OldPoint;
	TouchPoint		m_NewPoint;
	
};

TouchTestWnd::TouchTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	m_bEnabled = 0;

	//
	m_OldPoint.valid = m_OldPoint.x = m_OldPoint.y = 0;
	m_NewPoint.valid = m_NewPoint.x = m_NewPoint.y = 0;
}

TouchTestWnd::~TouchTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	if( m_BtnList )
	{
		NXDiagButton *btn = m_BtnList;
		NXDiagButton *next = NULL;
		while( btn )
		{
			next = btn->next;
			DestroyButton( btn );
			btn = next;
		}
	}
}

void TouchTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE, NULL ); btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK  , NULL ); btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK , NULL ); btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x00 );
	m_TriColor = SDL_MapRGB(m_Surface->format, 0x00, 0x00, 0xff );
}

int TouchTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
	bool bDrawLine = false;
	bool bContinued = false;
	BUTTON_STATE next_state;
	m_TestResult = lastResult;

	UpdateWindow();

	SDL_UpdateRect(m_Surface,0,0,800,400);

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
					m_OldPoint.valid = 1;
					m_OldPoint.x = event_btn->x;
					m_OldPoint.y = event_btn->y;
					break;
				case SDL_MOUSEMOTION:
					if( BUTTON_FOCUS_OUT == btn->state || BUTTON_FOCUS_IN == btn->state )
					{
						if( hit_test ) next_state = BUTTON_FOCUS_IN;
						else           next_state = BUTTON_FOCUS_OUT;
						processed = 1;
					}		
					m_NewPoint.valid = 1;
					m_NewPoint.x = event_btn->x;
					m_NewPoint.y = event_btn->y;
					bContinued = true;
					bDrawLine = true;
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
					m_NewPoint.valid = 1;
					m_NewPoint.x = event_btn->x;
					m_NewPoint.y = event_btn->y;
					bContinued = false;
					bDrawLine = true;
					break;
				case SDL_KEYDOWN:
					if( SDLK_HOME == sdl_event.key.keysym.sym ){
						bExit = true;
						processed = 1;
					}
					break;
			}

			if( btn->state != next_state )	update = 1;
			btn->state = next_state;
		}

		if( sdl_event.type == SDL_USEREVENT ){
			Action( &sdl_event.user );
			bExit = true;
			update = 1;
		}

		if( update ){
			UpdateWindow();
		}

		if( bDrawLine ){
			DrawTouchLine( bContinued );
			bDrawLine = false;
		}

		if ( bExit )	break;
	}

	return m_TestResult;
}

void TouchTestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );

	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );
	SDL_Flip( m_Surface );
}

int	TouchTestWnd::Action( SDL_UserEvent *usrEvent )
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

void TouchTestWnd::DrawTouchLine( bool continued )
{
	if( !m_NewPoint.valid || !m_OldPoint.valid )
		return;

	DrawLine( m_Surface, m_OldPoint.x, m_OldPoint.y, m_NewPoint.x, m_NewPoint.y, m_PenColor );

	if( continued )
	{
		m_OldPoint = m_NewPoint;
		m_NewPoint.valid = 0;
	}
	else
	{
		m_OldPoint.valid = 0;
		m_NewPoint.valid = 0;
	}
}

static int test_touch( int last_result )
{
	int result;

	TouchTestWnd *vipWnd = new TouchTestWnd();
	vipWnd->Initialize();
//	vipWnd->UpdateWindow();
	result = vipWnd->EventLoop( last_result );
	delete vipWnd;
	return result;
}

//
static NXDiagPluginInfo TouchTestPluginInfo = {
	"TOUCH TEST",
	"TOUCH Diagnostic Application",
	"",
	test_touch,
	NULL,
	0
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &TouchTestPluginInfo;
}
