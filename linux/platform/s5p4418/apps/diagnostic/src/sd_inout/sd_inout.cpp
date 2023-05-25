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
#include <mntent.h>
#include <sys/mount.h>

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <SDL_ttf.h>
#define	SH_FILE_NAME	"sd_test.sh"


#define FILESIZE 1024

static SDL_Rect st_BGRect  = {   0, 0, 800, 480 };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
static SDL_Rect st_SDRect = { 250, 100, 300,  60 };

static SDL_Rect st_DbgRect = {  80,  40, 720, 360 };
struct uevent {
    const char *action;
    const char *path;
    const char *subsystem;
    const char *name;
    const char *links;
    const char *firmware;
    const char *partition_name;
    int udevlog;
    int seqnum;
    int partition_num;
    int major;
    int minor;
};

static int fd = -1;
bool bsdin;

class sdPressTestWnd: public BaseAppWnd
{
public:
	sdPressTestWnd();
	~sdPressTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();
 	
	//	Thread
	static void *ThreadStub( void *param );
	static void *ThreadStub2( void *param );
//	static void *ShellThread( void *param );
	void ShellThread();
	void TestThread();

	int uevent_init();
	int uevent_next_event(char* buffer, int buffer_length);
	static void parse_event(const char *msg, struct uevent *uevent);

	pthread_t	m_Thread;
	pthread_t	m_ShellThread;
	int			m_bExitTest;
	int			m_bThreadRunning;
	bool bExit;
	bool bExit1;
    bool bBtnEnable;
	TTF_Font *s_Font;
private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;

	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_SdinColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;
	unsigned int 	id;
	DebugWindow		*m_DbgWnd;
};
sdPressTestWnd::sdPressTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(MAINFONT, 18);
	s_Font = TTF_OpenFont(MAINFONT, 24);
	m_Surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;

	m_bThreadRunning = 1;
	m_bExitTest = 1;
	bExit = 0;
	bExit1 = 0;
	bBtnEnable=1;
	m_DbgWnd = new DebugWindow( &st_DbgRect, 20 );
}




sdPressTestWnd::~sdPressTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( s_Font )
		TTF_CloseFont( s_Font );	
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	if( m_DbgWnd )
		delete m_DbgWnd;
}

void sdPressTestWnd::Initialize()
{
	FILE *fp;
struct mntent *fs;
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK   );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK  ); btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_SDRect , BUTTON_NORMAL  , BTN_ACT_NONE  ); btn = btn->next;
	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_SdinColor = SDL_MapRGB(m_Surface->format, 0x00, 0xff, 0x00 );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );


	fp = setmntent("/proc/mounts","r");
		    if(fp == NULL)
			{
			    perror("open fail");
		    }
						  
		    while((fs = getmntent(fp)) != NULL)
		    {
		      printf("%s,   %s,   \n",fs->mnt_fsname,fs->mnt_dir);

			  if((strcmp(fs->mnt_dir,"/mnt/mmc")) == 0)
			  {
				  //printf("strcmp is ok \n");
				  bsdin =1;
			  }
		    }
			endmntent(fp);
}

int sdPressTestWnd::uevent_init()
{
    struct sockaddr_nl addr;
    int sz = 64*1024;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if(s < 0)
        return 0;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

    if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(s);
        return 0;
    }

    fd = s;
    return (fd > 0);
}

int sdPressTestWnd::uevent_next_event(char* buffer, int buffer_length)
{
	int i;
    //while (!m_bExitTest) {
    for(i=0;i<=2;i++) {
        struct pollfd fds;
        int nr;

        fds.fd = fd;
        fds.events = POLLIN;
        fds.revents = 0;
        nr = poll(&fds, 1, 100);

        if(nr > 0 && fds.revents == POLLIN) {
            int count = recv(fd, buffer, buffer_length, 0);
            if (count > 0) {
                return count;
            }
        }
    }

    // won't get here
    return 0;
}
void sdPressTestWnd::parse_event(const char *msg, struct uevent *uevent)
{
	const char *idx = NULL;

    uevent->action = "";
    uevent->path = "";
    uevent->subsystem = "";
    uevent->name = "";
    uevent->links = "";
    uevent->firmware = "";
	uevent->udevlog = -1;
    uevent->seqnum = -1;
    uevent->major = -1;
    uevent->minor = -1;
    uevent->partition_name = NULL;
    uevent->partition_num = -1;

        /* currently ignoring SEQNUM */
    while(*msg) {
    	idx = "ACTION=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->action = msg;
        }

        idx = "DEVPATH=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->path = msg;
        }

        idx = "SUBSYSTEM=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->subsystem = msg;
        }

        idx = "DEVNAME=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->name = msg;
        }

        idx = "DEVLINKS=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->links = msg;
        }

        idx = "FIRMWARE=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->firmware = msg;
        }

        idx = "UDEV_LOG=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->udevlog = atoi(msg);
        }

        idx = "SEQNUM=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->seqnum = atoi(msg);
        }

        idx = "MAJOR=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->major = atoi(msg);
        }

        idx = "MINOR=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->minor = atoi(msg);
        }

        idx = "PARTN=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->partition_num = atoi(msg);
        }

        idx = "PARTNAME=";
        if(!strncmp(msg, idx, strlen(idx))) {
            msg += strlen(idx);
            uevent->partition_name = msg;
        }

        /* advance to after the next \0 */
        while(*msg++);
    }
/*
	if (! strlen(uevent->action))
		return;
*/
    printf("event {\n"
    		"	action    : '%s'\n"
    		"	path      : '%s'\n"
    		"	subsystem : '%s'\n"
			"	name      : '%s'\n"
			"	links     : '%s'\n"
    		"	firmware  : '%s'\n"
    		"	udevlog   : '%d'\n"
    		"	seqnum    : '%d'\n"
    		"	major     : '%d'\n"
    		"	minor     : '%d'\n"
    		"}\n",
    		uevent->action,
    		uevent->path,
      		uevent->subsystem,
			uevent->name,
			uevent->links,
            uevent->firmware,
            uevent->udevlog,
            uevent->seqnum,
            uevent->major,
            uevent->minor);


            if((strcmp(uevent->action,"add")==0)&&(strcmp(uevent->subsystem,"mmc")==0))
            {
            	bsdin = 1;
            	//UpdateWindow();
            }
            else if((strcmp(uevent->action,"remove")==0)&&(strcmp(uevent->subsystem,"mmc")==0))
            {
            	bsdin = 0;
            	//UpdateWindow();
            }
}
int sdPressTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	//bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;


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
			if(bBtnEnable==0) break;
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
			if(bBtnEnable)
			{
				Action( &sdl_event.user );
				bExit = true;
				update = 1;
			}
//			printf("KEY ACTION\n");
		}

		if( update ){

			UpdateWindow();
		}

		if ( bExit ){

			m_bExitTest = 1;
			break;
		}
	}

	if( m_bThreadRunning ){
		printf("woit Thread Exit\n");
	pthread_join( m_ShellThread, NULL );
		m_bThreadRunning = 0;
	}


	return m_TestResult;
}

void sdPressTestWnd::UpdateWindow()
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
	if(bsdin)
	{
	DrawButton  ( m_Surface, &btn->rect, m_SdinColor );
	DrawString ( m_Surface, s_Font, "SD CARD Insert", btn->rect );
	}
	else
	{
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, s_Font, "SD CARD Remove", btn->rect );	
	}
	SDL_Flip( m_Surface );
}

int	sdPressTestWnd::Action( SDL_UserEvent *usrEvent )
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

void *sdPressTestWnd::ThreadStub2( void *param )
{
	sdPressTestWnd *pObj = (sdPressTestWnd *)param;
	pObj->ShellThread();
	return (void*)0xDEADDEAD;
}

//void *sdPressTestWnd::ShellThread( void *arg )
void sdPressTestWnd::ShellThread( )
{
	//	Check first
	//FILE *fd = fopen(SH_FILE_NAME, "r" );
	/*int i,j,wr;
	int tfd;
	FILE *fp;
    struct mntent *fs;
	int mountstate =0,rdresult = 0,wrresult=0;
	char mbuf[FILESIZE] ={'a'};
	*/
	char buffer[1024];
	int buf_sz = sizeof(buffer)/sizeof(buffer[0]);
	int ev_buf_sz;
	struct uevent uevent;
//printf("!!!!!!!!!!!!!!!!!!!!!!Shell Tread start!!!!!!!!!!!!!!!!\n\n");
//printf("m_bExitTest == %d\n\n",m_bExitTest);
	m_bExitTest =0;
	
	uevent_init();

	while(!m_bExitTest ) {
		memset(buffer, 0, sizeof(buffer));

		ev_buf_sz = uevent_next_event(buffer, buf_sz);

        printf("\nevent msg len [%d]\n", ev_buf_sz);
		if(ev_buf_sz >0)
			{
			parse_event(buffer, &uevent);
			printf("Update window");
			UpdateWindow();
			}
		}
	
		//	m_DbgWnd->DrawMessage("Test Start");
//	  			printf("Start");

		  //	m_DbgWnd->DrawMessage("Test End");
		//	fclose(eFd);
		printf("Close");
		close(fd);
		bBtnEnable = 1;	
		printf("Thread Exit");
	//return (void*)0xDEADDEAD;
}


extern "C" int test_sdin_Test( int last_result )
{
	int result;
	sdPressTestWnd *sdWnd = new sdPressTestWnd();
	sdWnd->Initialize();
	sdWnd->UpdateWindow();
	result = sdWnd->EventLoop( last_result );
	delete sdWnd;
	return result;
}
