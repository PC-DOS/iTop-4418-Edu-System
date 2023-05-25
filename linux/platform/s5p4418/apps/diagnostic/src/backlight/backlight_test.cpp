#include  <unistd.h>
#include <pthread.h>

#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>

static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };

#if 0	//	800 x 480 UI
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
static SDL_Rect st_VALRect = { 275, 210, 250,  60 };
static SDL_Rect st_DOWNRect= {  10, 210, 250,  60 };
static SDL_Rect st_UPRect  = { 540, 210, 250,  60 };
#endif

#if 1	//	1024 x 600 UI
static SDL_Rect st_OKRect  = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect = { 520, 480, 490, 110 };

static SDL_Rect st_DOWNRect= {  70, 210, 280,  80 };
static SDL_Rect st_VALRect = { 370, 210, 280,  80 };
static SDL_Rect st_UPRect  = { 670, 210, 280,  80 };
#endif

static char str[3] ={' ','5','\0'};
class BacklightTestWnd: public BaseAppWnd
{
public:
	BacklightTestWnd();
	~BacklightTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	int bright;
private:
	void Brightness(int a);
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

BacklightTestWnd::BacklightTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	bright = 5;

	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	m_bEnabled = 0;
	
	//
}

BacklightTestWnd::~BacklightTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
}

void BacklightTestWnd::Initialize()
{
	NXDiagButton *btn;

	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_VALRect , BUTTON_NORMAL  , BTN_ACT_NONE, NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_DOWNRect, BUTTON_NORMAL  , BTN_ACT_DOWN, NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_UPRect  , BUTTON_NORMAL  , BTN_ACT_UP  , NULL );	btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_BtnPColor= SDL_MapRGB(m_Surface->format, 0x85,  0x0,  0x0 );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int BacklightTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
	
	BUTTON_STATE next_state;
	m_TestResult = lastResult;

	Brightness(5);
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
		//printf("Backlight Event : %d\n\n",sdl_event.type);//bok test
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
			if(sdl_event.user.code <= BTN_ACT_NOK)
				bExit = true;

			update = 1;
		}

		if( update ){
			UpdateWindow();
		}
					
		if ( bExit ){
			bright = 5;
			Brightness(bright);
			break;
		}
	}

	return m_TestResult;
}

void BacklightTestWnd::UpdateWindow()
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
	DrawString ( m_Surface, m_Font, str, btn->rect );

	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "BRIGHT DOWN", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "BRIGHT UP", btn->rect );
	SDL_Flip( m_Surface );
}

void BacklightTestWnd::Brightness(int a)
{
	switch(bright)
	{
		case 0:
		system("echo 0 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 1:
		system("echo 25 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 2:
		system("echo 50 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 3:
		system("echo 75 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 4:
		system("echo 100 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 5:
		system("echo 128 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 6:
		system("echo 150 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 7:
		system("echo 175 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 8:
		system("echo 200 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 9:
		system("echo 225 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
		case 10:
		system("echo 255 > /sys/class/backlight/pwm-backlight/brightness");	
		break;
	}
		if (a >= 10)
		{
			str[0]='1';
			str[1]= a%10 + '0';
		}
		else if(a <10){
		str[0]= ' ';	
		str[1] =  a + '0';
		}
//		UpdateWindow();
}
int	BacklightTestWnd::Action( SDL_UserEvent *usrEvent )
{
	
	switch(usrEvent->code)
	{
		case BTN_ACT_DOWN:
			if(--bright < 1)
			bright = 1;	
			Brightness(bright);
			break;
		case BTN_ACT_UP:
		if(++bright > 10)
			bright = 10;	
			Brightness(bright);
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
int test_Backlight( int last_result )
{
	int result;

	BacklightTestWnd *BacklightWnd = new BacklightTestWnd();
	BacklightWnd->Initialize();
	BacklightWnd->UpdateWindow();
	result = BacklightWnd->EventLoop( last_result );
	delete BacklightWnd;
	return result;
}

//
static NXDiagPluginInfo BacklightTestPluginInfo = {
	"BACKLIGHT TEST",
	"Backlight Diagnostic Application",
	"",
	test_Backlight,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &BacklightTestPluginInfo;
}
