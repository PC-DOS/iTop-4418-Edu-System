#include <unistd.h>
#include <pthread.h>


#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>

#if 0
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
static SDL_Rect st_RECRect = {10, 210,250,60};
static SDL_Rect st_PLAYRect = {540, 210,250,60};
#endif

#if 1	//	1024 600
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect = { 520, 480, 490, 110 };

static SDL_Rect st_RECRect = { 230, 140, 250, 200 };
static SDL_Rect st_PLAYRect= { 530, 140, 250, 200 };
#endif

int m_bExitTest=0;
pthread_mutex_t w_mutex = PTHREAD_MUTEX_INITIALIZER;
class AudioRecTestWnd: public BaseAppWnd
{
public:
	AudioRecTestWnd();
	~AudioRecTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();
	static void *ThreadStub2( void *param );

	void DrawAudioRecLine( bool contined );
	void ShellThread();

	int	PlayFlag;
	pthread_t   m_ShellThread;
	int         m_bThreadRunning;

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

AudioRecTestWnd::AudioRecTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	m_bEnabled = 0;
	PlayFlag =0;

	if(pthread_create(&m_ShellThread,NULL,ThreadStub2,(void*)this) < 0){
		m_bThreadRunning=0 ;
	}
}

AudioRecTestWnd::~AudioRecTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
}

void AudioRecTestWnd::Initialize()
{
	NXDiagButton *btn;

	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_RECRect, BUTTON_NORMAL   , BTN_ACT_REC , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_PLAYRect, BUTTON_NORMAL  , BTN_ACT_PLAY, NULL );	btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor  = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_BtnPColor = SDL_MapRGB(m_Surface->format, 0x85, 0x00, 0x00 );
	m_PenColor  = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int AudioRecTestWnd::EventLoop( int lastResult )
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
			if(PlayFlag !=0) break;
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
					}
					break;
			}

			if( btn->state != next_state )	update = 1;
			btn->state = next_state;
		}
		if( sdl_event.type == SDL_USEREVENT ){
			Action( &sdl_event.user );
			if(sdl_event.user.code == BTN_ACT_OK || sdl_event.user.code == BTN_ACT_NOK )
				bExit = true;
			update = 1;
		}

		if( update ){
			pthread_mutex_lock(&w_mutex);
		    UpdateWindow();
		    pthread_mutex_unlock(&w_mutex);

		}
		
		if ( bExit ){
			m_bExitTest = 1;
			break;
		}
	}
	if(m_bThreadRunning){
		printf("wait Thread Exit\n");
		pthread_join(m_ShellThread,NULL);
		pthread_mutex_unlock(&w_mutex);
		m_bThreadRunning = 0;
	}

	return m_TestResult;
}

void AudioRecTestWnd::UpdateWindow()
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
	if(PlayFlag ==1)
		DrawButton  ( m_Surface, &btn->rect, m_BtnPColor );
	else
		DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "REC", btn->rect );
	btn = btn->next;
	if(PlayFlag ==2)
		DrawButton  ( m_Surface, &btn->rect, m_BtnPColor );
	else
		DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "PLAY", btn->rect );
	SDL_Flip( m_Surface );

}

int	AudioRecTestWnd::Action( SDL_UserEvent *usrEvent )
{
	printf("Button");
	switch(usrEvent->code)
	{
		case BTN_ACT_REC:
			pthread_mutex_lock(&w_mutex);
			PlayFlag=1;	
			pthread_mutex_unlock(&w_mutex);
			break;
		case BTN_ACT_PLAY:
			pthread_mutex_lock(&w_mutex);
			PlayFlag=2;
			pthread_mutex_unlock(&w_mutex);
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
void *AudioRecTestWnd::ThreadStub2( void *param )
{
	AudioRecTestWnd *pObj = (AudioRecTestWnd *)param;
	pObj->ShellThread();
	return (void*)0xDEADDEAD;
}
void AudioRecTestWnd::ShellThread( )
{
	char outFileName[512];
	char cmdBuf[512];
	m_bThreadRunning=1;

	memset( outFileName, 0, sizeof(outFileName) );
	strcpy( outFileName, m_DiagCfg.GetDiagOutPath() );
	strcat( outFileName, "/rec.wav" );

	while(!m_bExitTest)
	{
		if(PlayFlag == 1)
		{
			pthread_mutex_lock(&w_mutex);
			UpdateWindow();
			memset( cmdBuf, 0, sizeof(cmdBuf) );
			sprintf(cmdBuf, "arecord -f S16_LE -r 48000 -c 2 -d 2 > %s", outFileName );
			printf("Recording(cmd :%s)\n", cmdBuf);
			system(cmdBuf);
			printf("record end\n");
			PlayFlag=0;
			UpdateWindow();
			pthread_mutex_unlock(&w_mutex);
		}
		else if(PlayFlag == 2)
		{
			pthread_mutex_lock(&w_mutex);
			UpdateWindow();
			memset( cmdBuf, 0, sizeof(cmdBuf) );
			sprintf(cmdBuf, "aplay %s", outFileName );
			system(cmdBuf);
			PlayFlag=0;
			UpdateWindow();
			pthread_mutex_unlock(&w_mutex);
		}
		else
		{
			usleep(1000);
		}
	}
	memset( cmdBuf, 0, sizeof(cmdBuf) );
	sprintf(cmdBuf, "rm -rf %s", outFileName );
	system(cmdBuf);
	printf("Shell Threa Exit\n");
}

int test_AudioRec( int last_result )
{
	int result;

	AudioRecTestWnd *AudioRecWnd = new AudioRecTestWnd();
	AudioRecWnd->Initialize();
	AudioRecWnd->UpdateWindow();
	result = AudioRecWnd->EventLoop( last_result );
	delete AudioRecWnd;
	return result;
}


//
static NXDiagPluginInfo AudioRecTestPluginInfo = {
	"AUDIO REC TEST",
	"Audio Recoding Diagnostic Application",
	"",
	test_AudioRec,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &AudioRecTestPluginInfo;
}
