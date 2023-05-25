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
#include <libnxmem.h>
#include <libnxdpc.h>
#include <fourcc.h>
#include <debug_window.h>

#include <SDL.h>
#include <SDL_image.h>







#define UP_BMP ("/usr/local/diag/data/up_bmp")
#define UP_G_BMP ("/usr/local/diag/data/up_g.bmp")

#define DOWN_BMP ("/usr/local/diag/data/down_bmp")
#define DOWN_G_BMP ("/usr/local/diag/data/down_g.bmp")

#define LEFT_BMP ("/usr/local/diag/data/left_bmp")
#define LEFT_G_BMP ("/usr/local/diag/data/left_g.bmp")

#define RIGHT_BMP ("/usr/local/diag/data/right_bmp")
#define RIGHT_G_BMP ("/usr/local/diag/data/right_g.bmp")








//#define MAX_LINE		72
#define MAX_LINE		27
#define FIFO_FILE		"/tmp/t_fifo"
#define FIFO_NAME		"readinfo"
#define	SH_FILE_NAME	"/usr/local/diag/bin/inputtest"

#define LOG_NAME	"/root/diagnostics/Sensor_Test.log"


	SDL_Surface		*u_Surface;
	SDL_Surface		*d_Surface;
	SDL_Surface		*r_Surface;
	SDL_Surface		*l_Surface;
static SDL_Rect st_BGRect  = {   0, 0, 800, 480 };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
//static SDL_Rect st_DbgRect = {  80,  40, 720, 360 };

static SDL_Rect st_DbgRect = {  80,  40, 720, 60 };
//static SDL_Rect st_ABS_XRect = {  10,  210, 250, 60 };
//static SDL_Rect st_ABS_YRect = {  275,  210, 250, 60 };
//static SDL_Rect st_ABS_ZRect = {  540,  210, 250, 60 };

static SDL_Rect st_UPRect = { 350 ,50,0,0};
static SDL_Rect st_LRect = { 100 ,150,0,0};
static SDL_Rect st_RRect = { 600 ,150,0,0};
static SDL_Rect st_DWRect = { 350 ,250,0,0};

	char ABS_X[12] = {' ','A','B','S','_','X',' ',' ',' ',' ',' ','\0'};
	char ABS_Y[12] = {' ','A','B','S','_','Y',' ',' ',' ',' ',' ','\0'};
	char ABS_Z[12] = {' ','A','B','S','_','Z',' ',' ',' ',' ',' ','\0'};

	pthread_mutex_t w_mutex = PTHREAD_MUTEX_INITIALIZER;

class SensorPressTestWnd: public BaseAppWnd
{
public:
	SensorPressTestWnd();
	~SensorPressTestWnd();
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
	int			m_bExitTest;
	int			m_bThreadRunning;
	bool bExit;
	bool bExit1;

	bool Sen_up;
	bool Sen_down;
	bool Sen_right;
	bool Sen_left;
		
	struct timeval tv;
	fd_set	readfds;
	int state;

	bool window;  
private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;

	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;
	unsigned int 	id;
	DebugWindow		*m_DbgWnd;

	//pthread_mutex_t w_mutex;
};
SensorPressTestWnd::SensorPressTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	//m_Font = TTF_OpenFont("./DejaVuSansMono.ttf", 18);
	m_Font = TTF_OpenFont(MAINFONT, 20);
	m_Surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;

	m_bThreadRunning = 0;
	m_bExitTest = 0;
	bExit = 0;
	bExit1 = 0;
		

	Sen_up=0;
	Sen_down=0;
	Sen_right=0;
	Sen_left=0;

	m_DbgWnd = new DebugWindow( &st_DbgRect, 15 );
}

SensorPressTestWnd::~SensorPressTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	if( u_Surface )
		SDL_FreeSurface( u_Surface );
	if( d_Surface )
		SDL_FreeSurface( d_Surface );
	if( r_Surface )
		SDL_FreeSurface( r_Surface );
	if( l_Surface )
		SDL_FreeSurface( l_Surface );
	if( m_DbgWnd )
		delete m_DbgWnd;
}

void SensorPressTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK   );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK  ); btn = btn->next;
/*
	btn->next = CreateButton( (SDL_Rect*)&st_ABS_XRect , BUTTON_NORMAL  , BTN_ACT_NONE  ); btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_ABS_YRect , BUTTON_NORMAL  , BTN_ACT_NONE  ); btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_ABS_ZRect , BUTTON_NORMAL  , BTN_ACT_NONE  ); btn = btn->next;
*/
	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );
}

int SensorPressTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	//bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
//	int rFd=0;

	BUTTON_STATE next_state;
	m_TestResult = lastResult;

#if 0
	rFd = open(FIFO_FILE,O_RDWR);
	if(rFd < 0)
	{
		if(mkfifo(FIFO_FILE,0666)==-1)
		{
			printf("MKFIFO ERR\n\n");

		}
	}
	else
	{
		close(rFd);
	}
#endif
	/*
	rFd = open(FIFO_FILE,O_RDWR);
	perror("Open FIFO PIPE :"); 
//	printf("open fifo :%d\n\n",rFd);
	close(rFd);
	system("rm /tmp/t_fifo");
*/
		if(mkfifo(FIFO_FILE,0666)==-1)
		{
			printf("MKFIFO 1 ERR\n\n");
		}
		else
		{
				
			printf("MKFIFO OPEN\n\n");
		}
		
	//
	//	Test Thread
	//
#if 1 
	if( pthread_create(&m_Thread, NULL, ThreadStub, (void*)this ) < 0 ){
		m_bThreadRunning = 0;
		return TEST_FAILED;
	}
#endif
#if 1
	if( pthread_create(&m_ShellThread, NULL, ThreadStub2, (void*)this ) < 0 ){
		m_bThreadRunning = 0;
		return TEST_FAILED;
	}
#endif
#if 0 
 	if( pthread_create(&m_ShellThread, NULL, ShellThread, (void*)this ) < 0 ){
		return TEST_FAILED;
	}
	pthread_detach(m_ShellThread);
#endif
//printf("Eventloop _ !!!!!\n");//bok test
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
			//printf("EVENT : %d\n\n",sdl_event.type);
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
			bExit = true;
			update = 1;

		}

		if( update ){

			pthread_mutex_lock(&w_mutex);	
			UpdateWindow();
			pthread_mutex_unlock(&w_mutex);	
			update = 0;
		}

		if ( bExit ){
			m_bExitTest = 1;
			break;
		}
	}
	if( m_bThreadRunning ){
	pthread_join( m_Thread, NULL );
	pthread_join( m_ShellThread, NULL );
	m_bThreadRunning = 0;
	pthread_mutex_destroy(&w_mutex);
	}

	system("rm /tmp/t_fifo"); // Remove FIFO PIPE

	return m_TestResult;
}

void SensorPressTestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;
     SDL_FreeSurface( u_Surface );
     SDL_FreeSurface( d_Surface );
     SDL_FreeSurface( r_Surface );
     SDL_FreeSurface( l_Surface );

	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );
	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );

	if(Sen_up==0)
	u_Surface = SDL_LoadBMP(UP_BMP);
	else
	u_Surface = SDL_LoadBMP(UP_G_BMP);

	if(Sen_down ==0 )
	d_Surface = SDL_LoadBMP(DOWN_BMP);
	else
	d_Surface = SDL_LoadBMP(DOWN_G_BMP);

	if(Sen_right==0)
	r_Surface = SDL_LoadBMP(RIGHT_BMP);
	else
	r_Surface = SDL_LoadBMP(RIGHT_G_BMP);
	if(Sen_left==0)
	l_Surface = SDL_LoadBMP(LEFT_BMP);
	else
	l_Surface = SDL_LoadBMP(LEFT_G_BMP);
	
SDL_BlitSurface(u_Surface,NULL,m_Surface,&st_UPRect);
SDL_BlitSurface(d_Surface,NULL,m_Surface,&st_DWRect);
SDL_BlitSurface(r_Surface,NULL,m_Surface,&st_RRect);
SDL_BlitSurface(l_Surface,NULL,m_Surface,&st_LRect);
	SDL_Flip( m_Surface );
	
}

int	SensorPressTestWnd::Action( SDL_UserEvent *usrEvent )
{
	m_bExitTest = 1;	

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
void *SensorPressTestWnd::ThreadStub( void *param )
{
	SensorPressTestWnd *pObj = (SensorPressTestWnd *)param;
	pObj->TestThread();
	return (void*)0xDEADDEAD;
}
void *SensorPressTestWnd::ThreadStub2( void *param )
{
	SensorPressTestWnd *pObj = (SensorPressTestWnd *)param;
	pObj->ShellThread();
	return (void*)0xDEADDEAD;
}
void SensorPressTestWnd::TestThread()
{
	int rFd;
	static char line[MAX_LINE] = {0, };
//	char cmdBuf[128]={0, };
//	int cmdBufPos=0, i,j;
	size_t readSize;
	unsigned int cnt = 0;
	m_bThreadRunning = 1;
	//	char *ret=NULL;
	rFd = open( FIFO_FILE, O_RDWR );
	if( rFd <= 0 ){
		printf("Fifo Error __IN TestThread\n");
		return;
	}

	tv.tv_sec=1;
	tv.tv_usec=0;
	/*
		lFd = open(LOG_NAME,O_RDWR|O_CREAT|O_TRUNC);
		if( lFd <0)
		{
			printf("Log File error");
		}
	*/
	while(!m_bExitTest)
	{
		FD_ZERO(&readfds);
		FD_SET(rFd,&readfds);
		memset(line,0, MAX_LINE);
		tv.tv_sec=2;

		state = select(rFd+1,&readfds,NULL,NULL,&tv);
		//printf("State == %dTimeOut  =: %d\n",state,readSize);//bok test
		if(state > 0)
		{		
			readSize = read(rFd, line, MAX_LINE );
			printf("ReadSize = %d, ReadBuff : %s,[4] = %d \n",readSize,line,line[4]);
			if( readSize == 0 ){
				continue;
				if( cnt>500 ){
					printf("Time Out.\n");
					break;
				}
				usleep(1000);
				continue;
			}

			if(readSize >= 9)
			{
				if(line[0]=='X')
				{
					if(line[4] == '-')
					{
					Sen_up = 1;
					Sen_down = 0;
					}
					else if(line[4]==0)
					{
					Sen_up = 0;
					Sen_down = 0;
					}
					
					else
					{
					Sen_up = 0;
					Sen_down = 1;
					}
				//	for(i=0;i<9;i++)
				//	ABS_X[i]=line[i];
				}
				else if(line[0]=='Y')
				{
					if(line[4] == '-')
					{
					Sen_right = 1;
					Sen_left = 0;
					}
					
					else if(line[4]==0)
					{
					Sen_right = 0;
					Sen_left = 0;
					}
					else
					{
					Sen_right = 0;
					Sen_left = 1;
					}
				//	for(i=0;i<9;i++)
				//	ABS_Y[i]=line[i];
				}
				else if(line[0]=='Z')
				{
				//	for(i=0;i<9;i++)
				//	ABS_Z[i]=line[i];
				}
			}
			if(readSize == 18)
			{
			//	j=0;
				if(line[9]=='Y')
				{
					if(line[13] == '-')
					{
					Sen_right = 1;
					Sen_left = 0;
					}
					else if(line[4]==0)
					{
					Sen_right = 0;
					Sen_left = 0;
					}
					else
					{
					Sen_right = 0;
					Sen_left = 1;
					}
					//for(i=9;i<18;i++){
					//	ABS_Y[j++]=line[i];
					//}
				}
				else if(line[9]=='Z')
				{
					//for(i=9;i<18;i++)
					//ABS_Z[j++]=line[i];
				}
			}
		
			if(readSize == 27)
			{
			//	j=0;
				if(line[18]=='Z')
				{
				//	for(i=18;i<27;i++){
				//		ABS_Z[j++]=line[i];
				//	}
				}
			}

		if(!m_bExitTest)
		{
			pthread_mutex_lock(&w_mutex);	
			UpdateWindow();
			pthread_mutex_unlock(&w_mutex);	
		}
			cnt = 0;
		}/*(if state>1)*/
	}

	FD_CLR(rFd,&readfds);
	close(rFd);
	//close(lFd);
	bExit1= 1;
}

//void *SensorPressTestWnd::ShellThread( void *arg )
void SensorPressTestWnd::ShellThread( )
{
	//	Check first
	FILE *fd = fopen(SH_FILE_NAME, "r" );
	int nPid = -1;

//printf("!!!!!!!!!!!!!!!!!!!!!!Shell Tread start!!!!!!!!!!!!!!!!\n\n");
//printf("m_bExitTest == %d\n\n",m_bExitTest);
	if( fd == NULL ){
		FILE *wFd = fopen( FIFO_NAME, "w" );
		fwrite( "error! cannot find cript file", 1, strlen("error! cannot find cript file"), wFd );
		fclose(wFd);
	}
	else{
		fclose( fd );
		if((nPid=fork())==0)//bok test
		{
			execl(SH_FILE_NAME, SH_FILE_NAME, "-d","/dev/input/event2",NULL );
			sleep(1);
		}
		else if(nPid != -1)
		{
			int status;
	  	
		while(!m_bExitTest){
			sleep(1);
		}
		kill(nPid, 9);
		waitpid(nPid, &status,WUNTRACED);
		}
	}
	//return (void*)0xDEADDEAD;
}


extern "C" int test_Sensor_Test( int last_result )
{
	int result;
	SensorPressTestWnd *SensorWnd = new SensorPressTestWnd();
	SensorWnd->Initialize();
	SensorWnd->UpdateWindow();
	result = SensorWnd->EventLoop( last_result );
	delete SensorWnd;
	return result;
}
