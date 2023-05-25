#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#include <linux/input.h>
#include <pthread.h>

#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>
#include <debug_window.h>


#define	EVENT_DEV_NAME	"/dev/input/event0"

enum{
	BTN_NONE        =  -1,
	BTN_VOLUME_UP   = 115,
	BTN_VOLUME_DOWN = 114,
	BTN_POWER       = 116,
};


#if 0
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
#endif

#if 1
static SDL_Rect st_BGRect     = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect     = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect    = { 520, 480, 490, 110 };
static SDL_Rect st_RESULTRect = { 280, 210, 460, 110 };
#endif


class KeyPressTestWnd: public BaseAppWnd
{
public:
	KeyPressTestWnd();
	~KeyPressTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	//	Thread
	static void *ThreadStub( void *param );
	void ThreadProc();

private:
	int  Action( SDL_UserEvent *usrEvent );
	void GetValue( int &buttonValue, char *btnStr );

	pthread_t		m_hThread;
	bool			m_bRunning;
	int				m_TestResult;
	int				m_BtnValue;
	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_pBtnColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;
	unsigned int 	id;


	pthread_mutex_t	m_hMutex;
};
KeyPressTestWnd::KeyPressTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	pthread_mutex_init( &m_hMutex, NULL );
	m_BtnValue = BTN_NONE;
}

KeyPressTestWnd::~KeyPressTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	pthread_mutex_destroy( &m_hMutex );
}

void KeyPressTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect     , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect     , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );		btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect    , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );		btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_RESULTRect , BUTTON_NORMAL  , BTN_ACT_NONE, NULL );		btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_pBtnColor = SDL_MapRGB(m_Surface->format, 0x00, 0xff, 0x00 );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
	
}

int KeyPressTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
	bool bExit = false;


	BUTTON_STATE next_state;
	m_TestResult = lastResult;
		
	//
	//	Test Thread
	//
	m_bRunning = true;
	if( pthread_create(&m_hThread, NULL, ThreadStub, (void*)this ) < 0 ){
		m_bRunning = false;
		return TEST_FAILED;
	}

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

			if( sdl_event.user.code == ACT_UPDATE )
			{
				update = 1;
			}
			else
			{
				Action( &sdl_event.user );
				bExit = true;
			}
		}

		if( update ){

			UpdateWindow();
		}

	}
	if( m_bRunning ){
		//	blocking
		m_bRunning = false;
		pthread_join( m_hThread, NULL );
	}
	return m_TestResult;
}


void KeyPressTestWnd::GetValue( int &buttonValue, char *btnStr )
{
	CNX_AutoLock lock(&m_hMutex);
	buttonValue = m_BtnValue;
	if( btnStr )
	{
		switch ( buttonValue )
		{
		case BTN_VOLUME_UP:
			strcpy(btnStr, "VOLUME UP");
			break;
		case BTN_VOLUME_DOWN:
			strcpy(btnStr, "VOLUME DOWN");
			break;
		case BTN_POWER:
			strcpy(btnStr, "POWER");
			break;
		case BTN_NONE:
			strcpy(btnStr, "NONE");
			break;
		default:
			strcpy(btnStr, "Unknown");
			break;
		}
	}
}

void KeyPressTestWnd::UpdateWindow()
{
	char str[80], btnStr[16];
	int btnValue;
	NXDiagButton *btn = m_BtnList;
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );

	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );

	//	Update Button String
	GetValue( btnValue, btnStr );
	snprintf(str, sizeof(str), "Button : %s(%d)", btnStr, btnValue);
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, str, btn->rect );
	SDL_Flip( m_Surface );
}

int	KeyPressTestWnd::Action( SDL_UserEvent *usrEvent )
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
void *KeyPressTestWnd::ThreadStub( void *param )
{
	KeyPressTestWnd *pObj = (KeyPressTestWnd *)param;

	pObj->ThreadProc();

	return (void*)0xDEADDEAD;
}


void KeyPressTestWnd::ThreadProc()
{
	int fd = open(EVENT_DEV_NAME, O_RDONLY);
    if (0 > fd) {
		printf("%s: device open failed!\n", __func__);
        return;
    }

    fd_set fdset;
	struct timeval timeout;
	while(m_bRunning)
	{
		FD_ZERO(&fdset);
   		FD_SET(fd, &fdset);

   		timeout.tv_sec  = 0;
   		timeout.tv_usec = 300000;
        if (select(fd + 1, &fdset, NULL, NULL, &timeout) > 0) 
		{
            if (FD_ISSET(fd, &fdset))
			{
				struct input_event evtBuf;
                size_t length = read(fd, &evtBuf, sizeof(struct input_event) );
                if (0 > (int)length)
				{
                    printf("%s(): read failed.\n", __func__);
                    close(fd);
					break;
                }

				//printf("1, length=%d, Type = %d, Code = %d, Value = %d\n", length, evtBuf.type, evtBuf.code, evtBuf.value);
				if( !evtBuf.value && evtBuf.code != 0 )
				{
					CNX_AutoLock lock(&m_hMutex);
					SDL_Event evt;
					switch ( evtBuf.code )
					{
					case BTN_VOLUME_UP:
						m_BtnValue = BTN_VOLUME_UP;
						evt.type = SDL_USEREVENT;
						evt.user.type = SDL_USEREVENT;
						evt.user.code = ACT_UPDATE;
						SDL_PushEvent(&evt);
						break;
					case BTN_VOLUME_DOWN:
						m_BtnValue = BTN_VOLUME_DOWN;
						evt.type = SDL_USEREVENT;
						evt.user.type = SDL_USEREVENT;
						evt.user.code = ACT_UPDATE;
						SDL_PushEvent(&evt);
						break;
					case BTN_POWER:
						m_BtnValue = BTN_POWER;
						evt.type = SDL_USEREVENT;
						evt.user.type = SDL_USEREVENT;
						evt.user.code = ACT_UPDATE;
						SDL_PushEvent(&evt);
						break;
					default:
						m_BtnValue = evtBuf.code;
						break;
					}
				}
            }
        }
    }
   	FD_CLR(fd, &fdset);

    close(fd);
}


int test_key( int last_result )
{
	int result;
	KeyPressTestWnd *KeyWnd = new KeyPressTestWnd();
	KeyWnd->Initialize();
	
	KeyWnd->UpdateWindow();
	result = KeyWnd->EventLoop( last_result );

	delete KeyWnd;

	return result;
}

//
static NXDiagPluginInfo KeyTestPluginInfo = {
	"KEY TEST",
	"Key Diagnostic Application",
	"",
	test_key,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &KeyTestPluginInfo;
}
