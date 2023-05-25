#include <pthread.h>
#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>

static SDL_Rect st_BGRect  = {   0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

#if 0	//	800x480 UI
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
static SDL_Rect st_ACTRect = { 275, 210, 250,  60 };
#endif

#if 1	//	1024x600 UI
static SDL_Rect st_OKRect  = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect = { 520, 480, 490, 110 };
static SDL_Rect st_ACTRect = { 280, 210, 460, 110 };
#endif

class vibratorTestWnd: public BaseAppWnd
{
public:
	vibratorTestWnd();
	~vibratorTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	int	PlayFlag;

private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;
	char	m_LastMessage[1024];
	int		m_bEnabled;

	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_BtnPColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;

};

vibratorTestWnd::vibratorTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	m_bEnabled = 0;
	PlayFlag =0;
	//
}

vibratorTestWnd::~vibratorTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
}

void vibratorTestWnd::Initialize()
{
	NXDiagButton *btn;

	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect , BUTTON_DISABLED, BTN_ACT_NONE , NULL ); btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect , BUTTON_NORMAL  , BTN_ACT_OK   , NULL ); btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect, BUTTON_NORMAL  , BTN_ACT_NOK  , NULL ); btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_ACTRect, BUTTON_NORMAL  , ACT_VIBRATION, NULL ); btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_BtnPColor = SDL_MapRGB(m_Surface->format, 0x85, 0x0, 0x0 );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int vibratorTestWnd::EventLoop( int lastResult )
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
				case SDL_KEYDOWN:
					//	printf("KeyDown\n");
					//	printf("Key == %d\n",sdl_event.key.keysym.sym);
					if( SDLK_HOME == sdl_event.key.keysym.sym ){
//						bExit = true;
//						processed = 1;
					}
					break;
			}

			if( btn->state != next_state )	update = 1;
			btn->state = next_state;
		}
		if( sdl_event.type == SDL_USEREVENT ){
			Action( &sdl_event.user );
			if(sdl_event.user.code != ACT_VIBRATION)
				bExit = true;

			update = 1;
		}

		if( update ){
			UpdateWindow();
		}

		if ( bExit )	break;
	}

	return m_TestResult;
}

void vibratorTestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );
	
	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );

	btn = btn->next;

	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "VIBRATION", btn->rect );
	SDL_Flip( m_Surface );

}

int	vibratorTestWnd::Action( SDL_UserEvent *usrEvent )
{
	switch(usrEvent->code)
	{
		case ACT_VIBRATION:
			system("echo 500 > /sys/class/timed_output/vibrator/subsystem/vibrator/enable");
			break;
		case BTN_ACT_OK:
			m_TestResult = TEST_PASSED;
			break;
		case BTN_ACT_NOK:
			m_TestResult = TEST_FAILED;
			break;
	}
	return 0;
}


int test_vibrator( int last_result )
{
	int result;

	vibratorTestWnd *vibratorWnd = new vibratorTestWnd();
	vibratorWnd->Initialize();
	vibratorWnd->UpdateWindow();
	result = vibratorWnd->EventLoop( last_result );
	delete vibratorWnd;
	return result;
}

//
static NXDiagPluginInfo VibratorTestPluginInfo = {
	"VIBRATOR",
	"Vibrator Diagnostic Application",
	"",
	test_vibrator,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &VibratorTestPluginInfo;
}
