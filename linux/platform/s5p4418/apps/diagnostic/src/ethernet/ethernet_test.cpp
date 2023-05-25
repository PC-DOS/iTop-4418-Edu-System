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
#include <debug_window.h>

static SDL_Rect st_BGRect  = {   0, 400, 800, 480 };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
static SDL_Rect st_DbgRect = {  80,  40, 720, 360 };

static bool bBtnEnable =0;
class EthPressTestWnd: public BaseAppWnd
{
public:
	EthPressTestWnd();
	~EthPressTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	//	Thread
	static void *ThreadStub( void *param );
	static void *ThreadStub2( void *param );
//	static void *ShellThread( void *param );
	void ShellThread();
	void TestThread();

	pthread_t	m_Thread;
	pthread_t	m_ShellThread;
	int			m_bThreadRunning;
	bool bExit;
	bool bExit1;
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
EthPressTestWnd::EthPressTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;

	m_bThreadRunning = 1;
	bExit = 0;
	bExit1 = 0;
	m_DbgWnd = new DebugWindow( &st_DbgRect, 18 );
}

EthPressTestWnd::~EthPressTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	if( m_DbgWnd )
		delete m_DbgWnd;
}

void EthPressTestWnd::Initialize()
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

int EthPressTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	//bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
	//int rFd=0;


	BUTTON_STATE next_state;
	m_TestResult = lastResult;
	//
	//	Test Thread
	//
#if 1
	if( pthread_create(&m_ShellThread, NULL, ThreadStub2, (void*)this ) < 0 ){
		m_bThreadRunning = 0;
		return TEST_FAILED;
	}
#endif

	while( !bExit )
	{
		update = 0;
		hit_test = 0;
		processed   = 0;

		SDL_MouseButtonEvent *event_btn = &sdl_event.button;
		SDL_WaitEvent( &sdl_event );
		
		for( btn = m_BtnList; 0!=btn&&0==processed; btn = btn->next )
		{
			if(bBtnEnable == 0) break;
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

			if( btn->state != next_state )	update = 0;
			btn->state = next_state;
			
		}
		

		if( sdl_event.type == SDL_USEREVENT ){
		printf("Btn Press : %d\n",bBtnEnable);
		if(bBtnEnable)
			{
				printf("Btnnable : %d\n",bBtnEnable);
				Action( &sdl_event.user );
				bExit = true;
				update = 1;
			}
		}

		if( update ){

			UpdateWindow();
		}

		if ( bExit ){
			break;
		}
	}

	if( m_bThreadRunning ){
		printf("wait Thread Exit\n");
	pthread_join( m_ShellThread, NULL );
		m_bThreadRunning = 0;
	}


	return m_TestResult;
}

void EthPressTestWnd::UpdateWindow()
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

int	EthPressTestWnd::Action( SDL_UserEvent *usrEvent )
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
void *EthPressTestWnd::ThreadStub2( void *param )
{
	EthPressTestWnd *pObj = (EthPressTestWnd *)param;
	pObj->ShellThread();
	return (void*)0xDEADDEAD;
}


void EthPressTestWnd::ShellThread( )
{
	int i;
	FILE *eFd;
	FILE *iFd;
	char msg[256]={0};
	char buf[256]={0};
	struct stat i_info;
	int usb0 = -1;
	{
		m_DbgWnd->DrawMessage("Test Start");
		printf("TEST START\n");
		usb0 = stat("/sys/class/net/usb0",&i_info);
		if(!usb0)
		{
			iFd = fopen("/mnt/mmc/eth.txt","r");
			if(iFd == NULL){
				perror("eth.txt open error ");
				m_DbgWnd->DrawMessage("Not Fine eth.txt");
				m_DbgWnd->DrawMessage("Test fail");
			}
			else
			{
				system("ifconfig usb0 up");

				fgets(buf,256,iFd);
				printf("%s\n",buf);
				for(i=0;i<256;i++)
				{
					if(buf[i]=='\n')
					{
						buf[i]=0;
					}
				}
				sprintf(msg,"ifconfig usb0 %s",buf);
				printf("%s\n",msg);
				system(msg);

				fgets(buf,256,iFd);
				for(i=0;i<256;i++)
				{
					if(buf[i]=='\n'){
						buf[i]=0;
					}
				}
				printf("%s\n",buf);
				sprintf(msg,"route add netmask %s",buf);
				printf("%s\n",msg);
				system(msg);

				fgets(buf,256,iFd);
				for(i=0;i<256;i++)
				{
					if(buf[i]=='\n'){
						buf[i]=0;
					}
				}
				printf("%s\n",buf);
				sprintf(msg,"route add gw %s",buf);
				printf("%s\n",msg);
				system(msg);

				fgets(buf,256,iFd);
				for(i=0;i<256;i++)
				{
					if(buf[i]=='\n'){
						buf[i]=0;
					}
				}
				printf("%s\n",buf);
				sprintf(msg,"ping -c 2 %s >t.txt",buf);
				printf("%s\n",msg);

				//			sprintf(msg,"ls -al");
				//			system(msg);

				fclose(iFd);
				printf("ifconfig ==");
				system("ifconfig > t.txt");

				printf("file open\n");
				eFd = fopen("t.txt","r");
				if(eFd == NULL){
					perror("ifconfig open error ");
				}
				else
				{
					while(!feof(eFd))
					{
						fgets(buf,256,eFd);
						for(i=0;i<256;i++)
						{
							if(buf[i]== '\n')
							{
								buf[i]= '\0';
							}
						}
						m_DbgWnd->DrawMessage(buf);
						memset(buf,0,256);
					}
					fclose(eFd);
				}
				m_DbgWnd->DrawMessage("Ping Test");
				system(msg);

				eFd = fopen("t.txt","r");
				if(eFd == NULL){
					perror("ping result open error ");
				}
				else
				{
					while(!feof(eFd))
					{
						fgets(buf,256,eFd);
						for(i=0;i<256;i++)
						{
							if(buf[i]== '\n')
							{
								buf[i]= '\0';
							}
						}
						m_DbgWnd->DrawMessage(buf);
						memset(buf,0,256);
					}
					fclose(eFd);
				}
			}
			printf("Test End\n");
			m_DbgWnd->DrawMessage("end");
		}
		else
		{
			printf("not pulg ethernet");
			m_DbgWnd->DrawMessage("Disconnect Lan card");
			m_DbgWnd->DrawMessage("Test fail");
			m_DbgWnd->DrawMessage("TEST END");
		}
	}
	system("ifconfig usb0 up");
	bBtnEnable=1;
	printf("shell thread Exit\n");
}

extern "C" int test_Eth_test( int last_result )
{
	int result;
	EthPressTestWnd *EthWnd = new EthPressTestWnd();
	EthWnd->Initialize();
	EthWnd->UpdateWindow();
	result = EthWnd->EventLoop( last_result );
	delete EthWnd;
	return result;
}
