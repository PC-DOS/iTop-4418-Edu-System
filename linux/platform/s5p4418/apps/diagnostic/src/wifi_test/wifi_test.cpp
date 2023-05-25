#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>

#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>
#include <debug_window.h>


static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect = { 520, 480, 490, 110 };
static SDL_Rect st_DBGRect = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT - 110 - 15 };

class WiFiSacnTestWnd: public BaseAppWnd
{
public:
	WiFiSacnTestWnd();
	~WiFiSacnTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	//	Thread
	static void *ThreadStub( void *param );
	void ThreadProc();

	pthread_t	m_hThread;
	int			m_bRunning;
private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;

	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;
	unsigned int 	id;
	DebugWindow		*m_DbgWnd;
};

WiFiSacnTestWnd::WiFiSacnTestWnd()
	: m_bRunning(1)
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;

	m_DbgWnd = new DebugWindow( &st_DBGRect, 18 );

	if(pthread_create(&m_hThread,NULL,ThreadStub,(void*)this) < 0){
		m_bRunning = 0;
	}

}

WiFiSacnTestWnd::~WiFiSacnTestWnd()
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
	if( m_DbgWnd )
		delete m_DbgWnd;
}

void WiFiSacnTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );	btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int WiFiSacnTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;

	BUTTON_STATE next_state;
	m_TestResult = lastResult;

	m_DbgWnd->DrawMessage("Test");

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

			if( btn->state != next_state )	update = 0;
			btn->state = next_state;
		}
		
		if( sdl_event.type == SDL_USEREVENT )
		{
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
		}

		if ( bExit ){
			break;
		}
	}

	return m_TestResult;
}

void WiFiSacnTestWnd::UpdateWindow()
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

int	WiFiSacnTestWnd::Action( SDL_UserEvent *usrEvent )
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

void *WiFiSacnTestWnd::ThreadStub( void *param )
{
	WiFiSacnTestWnd *pObj = (WiFiSacnTestWnd *)param;
	pObj->ThreadProc();
	return (void*)0xDEADDEAD;
}

//	returns : 0(Success), -1(Error), 1(App Not Exist)
int32_t NX_KillApp( const char *appName )
{
	char cmd[256];
	int32_t ret;
	sprintf(cmd, "kill -9 $(pidof %s)", appName );
	ret = system( cmd );
	return ret;
}


void WiFiSacnTestWnd::ThreadProc()
{
	system( "insmod /root/wlan.ko" );		//	Down WiFi network interface
	usleep( 1000000 );
	system( "ifconfig wlan0 up" );
	system( "wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wpa_supplicant.conf &" );
	usleep( 1000000 );

	FILE *fd;
	bool bScan = false;
	char buf[512];
	char str[512];

	system( "wpa_cli -p/var/run/wpa_supplicant scan_interval 2" );
	while(m_bRunning)
	{
		fd = popen( "wpa_cli -p/var/run/wpa_supplicant scan", "r" );
		if( fd )
		{
			while( fgets(buf, 512, fd ) != NULL )
			{
				printf("%s", buf);
				if( 0 == strncmp( "OK", buf, 2 ) )
				{
					printf("Scan OK!!!\n");
					bScan = true;
					break;
				}
				else
					bScan = false;
			}
			fclose(fd);
		}

		if( bScan )
		{
			fd = popen( "wpa_cli -p/var/run/wpa_supplicant scan_result", "r" );
			if( fd )
			{
				m_DbgWnd->Clear();
				while( fgets(buf, 512, fd ) != NULL )
				{
					if( 0 == strncmp( buf, "bssid", 5 ) )
					{
					}
					else if( 0 == strncmp( buf, "Selected", 8 ) )
					{
					}
					else
					{
						char paraStr[8][64];
						int len = strlen(buf);
						int idx = 0;
						char *tmp;
						tmp = paraStr[idx++];
						for( int i=0 ; i<len ; i++ )
						{
							if( buf[i] == '\t' )
							{
								*tmp = 0;
								tmp = paraStr[idx++];
							}
							else if( buf[i] == '\n' )
							{
								*tmp = 0;
								break;
							}
							else
							{
								*tmp++ = buf[i];
							}
						}
						sprintf(str, "%s    %s", paraStr[4], paraStr[2]);
//						sprintf(str, "%s %s %s %s %s", paraStr[0], paraStr[1], paraStr[2], paraStr[3], paraStr[4]);
						m_DbgWnd->DrawMessage(str);
					}
				}
				fclose(fd);
			}
		}

		usleep(1000000);
	}
	NX_KillApp("wpa_supplicant");
	system( "ifconfig wlan0 down" );
	system( "rmmod wlan" );
}


//////////////////////////////////////////////////////////////////////////////
//
//						Linux Shell Script Tools
//


int WiFi_Test( int last_result )
{
	int result;
	WiFiSacnTestWnd *WifiWnd = new WiFiSacnTestWnd();
	WifiWnd->Initialize();
	WifiWnd->UpdateWindow();
	result = WifiWnd->EventLoop( last_result );
	delete WifiWnd;
	return result;
}

//
static NXDiagPluginInfo WiFiTestPluginInfo = {
	"WIFI TEST",
	"WiFi Diagnostic Application",
	"",
	WiFi_Test,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &WiFiTestPluginInfo;
}
