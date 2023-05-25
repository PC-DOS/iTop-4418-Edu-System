#include <pthread.h>
#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>

#define COLORINDEX 9 
static SDL_Rect st_BGRect      = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };

#if 0	//	800 x 480 UI
static SDL_Rect st_OKRect      = {  10, 410, 250,  60 };
static SDL_Rect st_NOKRect     = { 540, 410, 250,  60 };
static SDL_Rect st_NEXTRect    = { 275, 410, 250,  60 };

static SDL_Rect	st_Color0Rect  = {   0,   0, 100, 400 };
static SDL_Rect	st_Color1Rect  = { 100,   0, 100, 400 };
static SDL_Rect	st_Color2Rect  = { 200,   0, 100, 400 };
static SDL_Rect	st_Color3Rect  = { 300,   0, 100, 400 };
static SDL_Rect	st_Color4Rect  = { 400,   0, 100, 400 };
static SDL_Rect	st_Color5Rect  = { 500,   0, 100, 400 };
static SDL_Rect	st_Color6Rect  = { 600,   0, 100, 400 };
static SDL_Rect	st_Color7Rect  = { 700,   0, 100, 400 };
static SDL_Rect st_MessageRect = {  20,  20, 200,  60 };
#endif

#if 1
static SDL_Rect st_OKRect      = {  10, 480, 320, 110 };
static SDL_Rect st_NEXTRect    = { 350, 480, 320, 110 };
static SDL_Rect st_NOKRect     = { 690, 480, 320, 110 };
static SDL_Rect	st_Color0Rect  = {   0,   0, 128, 460 };
static SDL_Rect	st_Color1Rect  = { 128,   0, 128, 460 };
static SDL_Rect	st_Color2Rect  = { 256,   0, 128, 460 };
static SDL_Rect	st_Color3Rect  = { 384,   0, 128, 460 };
static SDL_Rect	st_Color4Rect  = { 512,   0, 128, 460 };
static SDL_Rect	st_Color5Rect  = { 640,   0, 128, 460 };
static SDL_Rect	st_Color6Rect  = { 768,   0, 128, 460 };
static SDL_Rect	st_Color7Rect  = { 896,   0, 128, 460 };
static SDL_Rect st_MessageRect = {  20,  20, 200,  60 };
#endif

class LCDTestWnd: public BaseAppWnd
{
public:
	LCDTestWnd();
	~LCDTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	void DrawLCDLine( bool contined );
	int		index;
private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;
	char	m_LastMessage[1024];
	int		m_bEnabled;

	unsigned int	m_BgColor[COLORINDEX];;
	unsigned int	m_BtnColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;
};

LCDTestWnd::LCDTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	m_bEnabled = 0;
	index  =0;
	//
}

LCDTestWnd::~LCDTestWnd()
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

void LCDTestWnd::Initialize()
{
	NXDiagButton *btn;

	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect     , BUTTON_DISABLED, BTN_ACT_NONE, NULL );btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect     , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect    , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NEXTRect   , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;

	btn->next = CreateButton( (SDL_Rect*)&st_Color0Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color1Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color2Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color3Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color4Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color5Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color6Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_Color7Rect , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_MessageRect, BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );btn = btn->next;

	m_BgColor[0] = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	m_BgColor[1] = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x00 );
	m_BgColor[2] = SDL_MapRGB(m_Surface->format, 0x00, 0xff, 0x00 );
	m_BgColor[3] = SDL_MapRGB(m_Surface->format, 0x00, 0x00, 0xff );
	m_BgColor[4] = SDL_MapRGB(m_Surface->format, 0x00, 0x00, 0x00 );
	m_BgColor[5] = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0x00 );
	m_BgColor[6] = SDL_MapRGB(m_Surface->format, 0x00, 0xff, 0xff );
	m_BgColor[7] = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int LCDTestWnd::EventLoop( int lastResult )
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
			if(sdl_event.user.code != BTN_ACT_NEXT)
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

void LCDTestWnd::UpdateWindow()
{
	int i = 0;
	NXDiagButton *btn = m_BtnList;
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor[0] );
	
	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NEXT", btn->rect );
	if(index == 0)
	{ 
		for(i = 0;i <8;i++)
		{	
			btn = btn->next;
			SDL_FillRect( m_Surface, &btn->rect, m_BgColor[i] );
		}
		
	}

	else
	{
		for(i=0;i<8;i++)
		{
			btn = btn->next;
			SDL_FillRect( m_Surface, &btn->rect, m_BgColor[index-1] );
		}
	}

	btn = btn->next;

	SDL_FillRect( m_Surface, &btn->rect, m_BgColor[4] );

		switch(index)
		{
			case 0:
			DrawString ( m_Surface, m_Font, "ColorBar", btn->rect );
			break;
			case 1:
			DrawString ( m_Surface, m_Font,"WHITE", btn->rect );
			break;
			case 2:
			DrawString ( m_Surface, m_Font, "RED", btn->rect );
			break;
			case 3:
			DrawString ( m_Surface, m_Font, "GREEN", btn->rect );
			break;
			case 4:
			DrawString ( m_Surface, m_Font, "BLUE", btn->rect );
			break;
			case 5:
			DrawString ( m_Surface, m_Font, "BLACK", btn->rect );
			break;
			case 6:
			DrawString ( m_Surface, m_Font, "RG COLOR", btn->rect );
			break;
			case 7:
			DrawString ( m_Surface, m_Font, "GB Color", btn->rect );
			break;
			case 8:
			DrawString ( m_Surface, m_Font, "RB Color", btn->rect );
			break;
		}
	SDL_Flip( m_Surface );

}

int	LCDTestWnd::Action( SDL_UserEvent *usrEvent )
{
	switch(usrEvent->code)
	{
		case BTN_ACT_NEXT:
			index= (index+1)%6;
			//printf("Click Next, index = %d\n",index);
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

void LCDTestWnd::DrawLCDLine( bool continued )
{

}

int test_LCD( int last_result )
{
	int result;
	LCDTestWnd *LcdWnd = new LCDTestWnd();
	LcdWnd->Initialize();
	LcdWnd->UpdateWindow();
	result = LcdWnd->EventLoop( last_result );
	delete LcdWnd;
	return result;
}

//
static NXDiagPluginInfo LCDTestPluginInfo = {
	"LCD TEST",
	"LCD Diagnostic Application",
	"",
	test_LCD,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &LCDTestPluginInfo;
}
