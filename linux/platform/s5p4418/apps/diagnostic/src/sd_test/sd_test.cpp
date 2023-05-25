#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <sys/mount.h>			//	mount / umount
#include <sys/stat.h>			//	mkdir
#include <sys/types.h>
#include <mntent.h>				//	setmntent

#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>
#include <debug_window.h>

#include <disk_test.h>


#define	MMC_MASS_DEV_PREFIX		"/dev/mmcblk"
#define	MMC_MOUNT_POSITION		"/mnt/mmc"

#define	MMC_BLOCK_NUM			0
#define	MMC_PARTITION_NUM		1


#if 1	//	1024x600 UI
static SDL_Rect st_BGRect    = {   0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect    = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect   = { 520, 480, 490, 110 };
//static SDL_Rect st_ACTRect   = { 280, 210, 460, 110 };
static SDL_Rect st_STATERect = { 280, 210, 460, 110 };
#endif


class sdPressTestWnd: public BaseAppWnd
{
public:
	sdPressTestWnd();
	~sdPressTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

private:
	static void *ThreadStub(void *arg)
	{
		sdPressTestWnd *pObj = (sdPressTestWnd*)arg;
		pObj->ThreadProc();
		return (void*)0xDEADDEAD;
	}
	void ThreadProc();

private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;

	unsigned int	m_BgColor;
	unsigned int	m_BtnColor;
	unsigned int	m_FourCC;
	unsigned int	m_PenColor;

	pthread_t		m_hThread;
	bool			m_bThreadRunning;
	DISK_INFO		m_DiskInfo;
};
sdPressTestWnd::sdPressTestWnd()
	: m_bThreadRunning(true)
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();

	memset(&m_DiskInfo, 0, sizeof(m_DiskInfo));

	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
}

sdPressTestWnd::~sdPressTestWnd()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
}

void sdPressTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect   , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect   , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect  , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_STATERect, BUTTON_NORMAL  , BTN_ACT_NONE, NULL );	btn = btn->next;

	//	Background color
	m_BgColor = SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff );
	//	Button color
	m_BtnColor = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f );
	m_PenColor = SDL_MapRGB(m_Surface->format, 0xff, 0x00, 0x10 );

	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bThreadRunning = false;
	}
}

int sdPressTestWnd::EventLoop( int lastResult )
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
			}

			if( btn->state != next_state )	update = 1;
			btn->state = next_state;
		}

		if( sdl_event.type == SDL_USEREVENT ){
			Action( &sdl_event.user );
			if(sdl_event.user.code <= BTN_ACT_NOK)
				bExit = true;
		}

		if( update ){
			UpdateWindow();
		}

		if ( bExit ){
			break;
		}
	}

	if( m_bThreadRunning )
	{
		m_bThreadRunning = false;
		pthread_join(m_hThread, NULL);
	}

	return m_TestResult;
}

void sdPressTestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;
	char strStatus[64];
	SDL_FillRect( m_Surface, &btn->rect, m_BgColor );

	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );
	btn = btn->next;
	snprintf(strStatus, sizeof(strStatus), "SD DISK : %.1fMB/%.1fMB", (double)m_DiskInfo.avail/(1024.*1024.), (double)m_DiskInfo.total/(1024.*1024.));
	DrawButton  ( m_Surface, &btn->rect, m_BtnColor );
	DrawString ( m_Surface, m_Font, strStatus, btn->rect );
	SDL_Flip( m_Surface );
}

int	sdPressTestWnd::Action( SDL_UserEvent *usrEvent )
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


void sdPressTestWnd::ThreadProc()
{
	bool bFound = false;
	char buf[128];

	//
	//	Find Pre-Mounted USB Storage
	//
	{
		mntent *pEntry;
		FILE *fd2 = setmntent("/proc/mounts", "r");
		if( fd2 )
		{
			while( (pEntry = getmntent( fd2 )) != NULL )
			{
				if( 0 == strncmp(pEntry->mnt_dir, MMC_MOUNT_POSITION, sizeof(MMC_MOUNT_POSITION)-1 ) )
				{
					bFound = true;
					StorageTestUtil *pDisUtil = new StorageTestUtil(MMC_MOUNT_POSITION);
					pDisUtil->GetDiskInfo(m_DiskInfo);
					printf("Device Node : %s, Mount Position = /mnt/usb\n", buf);
					printf("Disk Size   : %.1fMB/%.1fMB\n", (double)m_DiskInfo.avail/(1024.*1024.), (double)m_DiskInfo.total/(1024.*1024.));
					delete pDisUtil;
					//UpdateWindow();
					break;
				}
			}
			fclose(fd2);
		}
	}

	while(m_bThreadRunning)
	{
		if( !bFound )
		{
			FILE *fd = popen( "find /dev/", "r" );
			if( fd )
			{
				char mountPosName[128];
				sprintf( mountPosName, "%s%dp%d", MMC_MASS_DEV_PREFIX, MMC_BLOCK_NUM, MMC_PARTITION_NUM );
				printf("SDCARD Mount Pos Name : %s\n", mountPosName );
				while( fgets(buf, 128, fd) != NULL )
				{
					if( 0 == strncmp( mountPosName, buf, strlen(mountPosName) ) )
					{
						buf[strlen(buf)-1] = 0;
						if( 0 != mount( buf, MMC_MOUNT_POSITION, "ext4", MS_SYNCHRONOUS, NULL ) )
						{
							printf("mount failed (%s) : %d(%s)\n", buf, errno, strerror(errno) );
						}
						else
						{
							bFound = true;
							StorageTestUtil *pDisUtil = new StorageTestUtil(MMC_MOUNT_POSITION);
							pDisUtil->GetDiskInfo(m_DiskInfo);
							printf("Device Node : %s, Mount Position = %s\n", buf, MMC_MOUNT_POSITION);
							printf("Disk Size   : %.1fMB/%.1fMB\n", (double)m_DiskInfo.avail/(1024.*1024.), (double)m_DiskInfo.total/(1024.*1024.));
							delete pDisUtil;
							UpdateWindow();
						}
					}
				}
				fclose(fd);
			}
		}
		usleep(1000000);
	}

	if( bFound == true )
	{
		umount( MMC_MOUNT_POSITION );
	}

}

extern "C" int test_sd( int last_result )
{
	int result;
	sdPressTestWnd *sdWnd = new sdPressTestWnd();
	sdWnd->Initialize();
	sdWnd->UpdateWindow();
	result = sdWnd->EventLoop( last_result );
	delete sdWnd;
	return result;
}

//
static NXDiagPluginInfo SDTestPluginInfo = {
	"SD TEST",
	"SD Diagnostic Application",
	"",
	test_sd,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &SDTestPluginInfo;
}
