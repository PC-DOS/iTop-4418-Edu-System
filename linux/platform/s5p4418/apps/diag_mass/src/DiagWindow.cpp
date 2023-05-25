#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <utils.h>
#include <disk_test.h>
#include <DiagWindow.h>


//
//	Automatic Test Window
//
//   -------------------------------------------------
//  |          Automatic Test Title Window            |
//   -------------------------------------------------
//  |                                                 |
//  |                                                 |
//  |                                                 |
//  |                                                 |
//  |                                                 |
//   -------------------------------------------------
//
//static SDL_Rect st_AutomaticTestRect      = {   0,   0, 512, 200 };
static SDL_Rect st_AutomaticTestTitleRect = {   0,   0, 512,  40 };
static SDL_Rect st_AutoResultRect         = {   0,  40, 512, 160 };

//
//	Manual Test Rects
//
//   -------------------------------------------------
//  |          Manually Test Title Window             |
//   -------------------------------------------------
//  |                                                 |
//  |                                                 |
//  |                                                 |
//  |                                                 |
//  |                                                 |
//   -------------------------------------------------
//
//static SDL_Rect st_ManuallyTestRect      = {   0, 200, 512, 200 };
static SDL_Rect st_ManuallyTestTitleRect = {   0, 200, 512,  40 };
static SDL_Rect st_ManuResultRect        = {   0, 240, 512, 160 };

//
//	LCD Test Rects
//
//   -------------------------------------------------
//  |               |                |                |
//  |               |                |                |
//  |               |                |                |
//  |     RED       |    GREEN       |     BLUE       |
//  |               |                |                |
//  |               |                |                |
//  |               |                |                |
//   -------------------------------------------------
//static SDL_Rect st_LCDTestRect      = {   0, 400, 512, 200 };
static SDL_Rect st_LCDTestRedRect   = {   0, 400, 171, 200 };
static SDL_Rect st_LCDTestGreedRect = { 171, 400, 171, 200 };
static SDL_Rect st_LCDTestBlueRect  = { 342, 400, 170, 200 };
static SDL_Rect st_TouchModeBtnRect = { 388, 405, 120,  40 };

//
//	MIC Test Rects
//
//   ---------------
//  |               |
//  |               |
//  |               |
//  |     MIC_L     |
//  |               |
//  |               |
//  |               |
//   ---------------
//  |               |
//  |               |
//  |               |
//  |     MIC_R     |
//  |               |
//  |               |
//  |               |
//   ---------------
static SDL_Rect st_MICTestRect      = { 512,   0, 100, 600 };
static SDL_Rect st_MICTestLeftRect  = { 512,   0, 100, 300 };
static SDL_Rect st_MICTestRightRect = { 512, 300, 100, 300 };


//
//	Result Window Rects
//
//   -----------------------------------------
//  |          WiFi Title Window              |
//   -----------------------------------------
//  |                                         |
//  |                                         |
//  |          Display AP Scan Result         |
//  |                                         |
//  |                                         |
//   -----------------------------------------
//static SDL_Rect st_WiFiTestRect      = { 612, 300, 412, 300 };
static SDL_Rect st_WiFiTestTitleRect = { 612, 300, 412,  40 };
static SDL_Rect st_WiFiResultRect    = { 612, 340, 412, 260 };

//
//	Camera Test Rects
//
//   -----------------------------------------
//  |                                         |
//  |                                         |
//  |                                         |
//  |         Camera Preview Window           |
//  |                                         |
//  |                                         |
//  |                                         |
//   -----------------------------------------
static SDL_Rect st_CameraTestRect   = { 612,   0, 412, 300 };
static SDL_Rect st_CamToggleBtnRect = { 900, 255, 120,  40 };


CDiagnosticWnd::CDiagnosticWnd()
	: m_pAudPlayTest(new CAudioPlayTest())
	, m_pAudRecTest (new CAudioRecTest() )
	, m_pCamTest    (new CCameraTest()   )
	, m_pHDMITest   (new CHDMITest()     )
	, m_pRTCTest    (new CRTCTest()      )
	, m_pWiFiTest   (new CWiFiTest()     )
	, m_pSensorTest (new CSensorTest()   )
	, m_pButtonTest (new CButtonTest()   )
	, m_pAutoResult(NULL)
	, m_pManualResult(NULL)
	, m_pWiFiResult(NULL)
	, m_Year(0), m_Month(0), m_Day(0), m_Hour(0), m_Min(0), m_Sec(0)
	, m_bHDMIIsConnected(false)

{
	//	Initialize SDL
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );

	//	Intialize Colors
	InitColor();

	m_pAutoResult   = new CInformationWindow(&st_AutoResultRect, 17, m_Colors[COLOR_BLACK], m_SDLColors[COLOR_WHITE]);
	m_pManualResult = new CInformationWindow(&st_ManuResultRect, 17, m_Colors[COLOR_BLACK], m_SDLColors[COLOR_WHITE]);
	m_pWiFiResult   = new CInformationWindow(&st_WiFiResultRect, 17, m_Colors[COLOR_BLACK], m_SDLColors[COLOR_WHITE]);

	//	Create Buttons
	m_pBtnCamToggle = CreateButton( &st_CamToggleBtnRect, BUTTON_NORMAL, BTN_ACT_CAM_TOGGLE, NULL);
	m_pBtnTouchMode = CreateButton( &st_TouchModeBtnRect, BUTTON_NORMAL, BTN_ACT_TOUCH_MODE, NULL);
	//	Make Button List
	m_BtnList = m_pBtnCamToggle;
	m_pBtnCamToggle->next = m_pBtnTouchMode;
	m_pBtnTouchMode->next = NULL;

	m_OldPoint.valid = m_OldPoint.x = m_OldPoint.y = 0;
	m_NewPoint.valid = m_NewPoint.x = m_NewPoint.y = 0;

	m_BtnValue = -1;
	strcpy(m_BtnString, "NONE");

	m_NumAPInfo = 0;

	pthread_mutex_init(&m_hUpdateMutex, NULL);

	UpdateWindow();

	Start();
}

CDiagnosticWnd::~CDiagnosticWnd()
{
	if( m_pAutoResult   ) delete m_pAutoResult;
	if( m_pManualResult ) delete m_pManualResult;
	if( m_pWiFiResult   ) delete m_pWiFiResult;

	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );

	pthread_mutex_destroy(&m_hUpdateMutex);
}

SDL_Color MakeSDLColor( unsigned char r, unsigned char g, unsigned char b )
{
	SDL_Color color = {r,g,b,0};
	return color;
}

void CDiagnosticWnd::InitColor()
{
	//	SDL Map RGB Colors
	m_Colors[COLOR_WHITE     ] = SDL_MapRGB( m_Surface->format, 0xFF, 0xFF, 0xFF );
	m_Colors[COLOR_BLACK     ] = SDL_MapRGB( m_Surface->format, 0x00, 0x00, 0x00 );
	m_Colors[COLOR_LIGHT_GRAY] = SDL_MapRGB( m_Surface->format, 0xC0, 0xC0, 0xC0 );
	m_Colors[COLOR_GRAY      ] = SDL_MapRGB( m_Surface->format, 0x80, 0x80, 0x80 );
	m_Colors[COLOR_HEAVY_GRAY] = SDL_MapRGB( m_Surface->format, 0x40, 0x40, 0x40 );
	m_Colors[COLOR_RED       ] = SDL_MapRGB( m_Surface->format, 0xFF, 0x00, 0x00 );
	m_Colors[COLOR_GREEN     ] = SDL_MapRGB( m_Surface->format, 0x00, 0xFF, 0x00 );
	m_Colors[COLOR_BLUE      ] = SDL_MapRGB( m_Surface->format, 0x00, 0x00, 0xFF );
	m_Colors[COLOR_COLOR_KEY ] = SDL_MapRGB( m_Surface->format, 0x09, 0x09, 0x09 );

	//	SDL Colors
	m_SDLColors[COLOR_WHITE     ] = MakeSDLColor( 0xFF, 0xFF, 0xFF );
	m_SDLColors[COLOR_BLACK     ] = MakeSDLColor( 0x00, 0x00, 0x00 );
	m_SDLColors[COLOR_LIGHT_GRAY] = MakeSDLColor( 0xC0, 0xC0, 0xC0 );
	m_SDLColors[COLOR_GRAY      ] = MakeSDLColor( 0x80, 0x80, 0x80 );
	m_SDLColors[COLOR_HEAVY_GRAY] = MakeSDLColor( 0x40, 0x40, 0x40 );
	m_SDLColors[COLOR_RED       ] = MakeSDLColor( 0xFF, 0x00, 0x00 );
	m_SDLColors[COLOR_GREEN     ] = MakeSDLColor( 0x00, 0xFF, 0x00 );
	m_SDLColors[COLOR_BLUE      ] = MakeSDLColor( 0x00, 0x00, 0xFF );
	m_SDLColors[COLOR_COLOR_KEY ] = MakeSDLColor( 0x09, 0x09, 0x09 );

}

int CDiagnosticWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
	bool bDrawLine = false;
	m_bLineContinued = false;

	BUTTON_STATE next_state;

	while( !bExit )
	{
		update = 0;
		hit_test = 0;
		processed   = 0;

		SDL_MouseButtonEvent *event_btn = &sdl_event.button;
		SDL_WaitEvent( &sdl_event );

		//	Mouse Button Event
		if( sdl_event.type == SDL_MOUSEMOTION		||
			sdl_event.type == SDL_MOUSEBUTTONDOWN	||
			sdl_event.type == SDL_MOUSEBUTTONUP )
		{
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
						m_OldPoint.valid = 1;
						m_OldPoint.x = event_btn->x;
						m_OldPoint.y = event_btn->y;
						break;
					case SDL_MOUSEMOTION:
						if( BUTTON_FOCUS_OUT == btn->state || BUTTON_FOCUS_IN == btn->state )
						{
							if( hit_test ) next_state = BUTTON_FOCUS_IN;
							else           next_state = BUTTON_FOCUS_OUT;
							processed = 1;
						}		
						m_NewPoint.valid = 1;
						m_NewPoint.x = event_btn->x;
						m_NewPoint.y = event_btn->y;
						m_bLineContinued = true;
						bDrawLine = true;
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
						m_NewPoint.valid = 1;
						m_NewPoint.x = event_btn->x;
						m_NewPoint.y = event_btn->y;
						m_bLineContinued = false;
						bDrawLine = true;
						break;
				}

				if( btn->state != next_state )	update = 0;
				btn->state = next_state;
			}
		}//	Mouse Event
		//	User Event
		else if( sdl_event.type == SDL_USEREVENT  )
		{
			switch( sdl_event.user.code )
			{
			case BTN_ACT_CAM_TOGGLE:
				if( m_pCamTest )
					m_pCamTest->CameraToggle();
				break;
			case BTN_ACT_TOUCH_MODE:
				update = 1;
				break;
			case ACT_UPDATE:
				switch( (int)sdl_event.user.data1 )
				{
				case UPDATE_ALL:
					update = 1;
					break;
				case UPDATE_AUTO:
					UpdateAutomaticWnd(false);
					break;
				case UPDATE_MANU:
					UpdateManualWnd(false);
					break;
				case UPDATE_WIFI:
					UpdateWiFiWnd(false);
					break;
				case UPDATE_MIC:
					UpdateMICWnd(true);
					break;
				default:
					break;
				}
				break;
			}
		}
		else if( sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP )
		{
			printf("Key Pressed\n");
		}

		if( update ){
			UpdateWindow();
		}

		if( bDrawLine ){
			DrawTouchLine( m_bLineContinued );
			bDrawLine = false;
		}

		if ( bExit ){
			break;
		}
	}

	return 0;
}

void CDiagnosticWnd::UpdateWindow()
{
	CNX_AutoLock lock(&m_hUpdateMutex);
	UpdateAutomaticWnd();
	UpdateManualWnd();
	UpdateLCDWnd();
	UpdateMICWnd();
	UpdateCameraWnd();
	UpdateWiFiWnd();
	SDL_Flip( m_Surface );
}


//////////////////////////////////////////////////////////////////////////////
//
//				Update Test Windows Functions
//


void CDiagnosticWnd::UpdateAutomaticWnd( bool bAll )
{
	char str[80];

	unsigned int numCpu=0, maxCpuFreq=0, ramSize=0, ramFreq=0;
	SDL_Color cpuFontColor, ramFontColor;
	long long mainDiskSize;
	float x, y, z;

	if( bAll )	//	Draw Title BOX
	{
		DrawBox( m_Surface, &st_AutomaticTestTitleRect, m_Colors[COLOR_GRAY  ], m_Colors[COLOR_BLACK] );
		DrawString2( m_Surface, m_Font, "Automatic Test Result", st_AutomaticTestTitleRect, m_SDLColors[COLOR_BLACK] );
	}

	// Clear Automatic String Data's
	m_pAutoResult->Clear();
	//
	//	Update CPU Data
	//
	if( CCpuRamTest::GetCpuInfo( numCpu, maxCpuFreq ) )
		cpuFontColor = m_SDLColors[COLOR_BLUE];
	else
		cpuFontColor = m_SDLColors[COLOR_WHITE];
	snprintf( str, sizeof(str), "CPU : %d core, %.1fGHz", numCpu, (float)maxCpuFreq/1000000. );
	m_pAutoResult->AddString(str, cpuFontColor);

	//
	//	Update Ram Data
	//
	if( CCpuRamTest::GetRamInfo( ramFreq, ramSize ) )
		ramFontColor = m_SDLColors[COLOR_BLUE];
	else
		ramFontColor = m_SDLColors[COLOR_WHITE];
	snprintf( str, sizeof(str), "RAM : %d MB, %.1fMHz", ramSize, (float)ramFreq/1000. );
	m_pAutoResult->AddString(str, ramFontColor);

	//
	//	Update RTC Information
	//
	if( m_Year == 0 )
	{
		snprintf( str, sizeof(str), "RTC Time : %4d/%02d/%02d-%02d:%02d:%02d",
			m_Year, m_Month, m_Day, m_Hour, m_Min, m_Sec );
		m_pAutoResult->AddString(str, m_SDLColors[COLOR_WHITE]);
	}
	else
	{
		snprintf( str, sizeof(str), "RTC Time : %4d/%02d/%02d-%02d:%02d:%02d",
			m_Year, m_Month, m_Day, m_Hour, m_Min, m_Sec );
		m_pAutoResult->AddString(str, m_SDLColors[COLOR_BLUE]);
	}

	//
	//	Get Main Storage Information
	//
	if( StorageTestUtil::GetDeviceSize(SD_STORAGE, 0, mainDiskSize ) )
	{
		snprintf( str, sizeof(str), "Main Storage : %.1fGB", (double)mainDiskSize/1000000000.);
		m_pAutoResult->AddString(str, m_SDLColors[COLOR_BLUE]);
	}
	else
	{
		m_pAutoResult->AddString("Main Storage : Cannot Access", m_SDLColors[COLOR_RED]);
	}

	//
	//	Get Main Storage Information
	//
	if( m_pSensorTest->ReadValue(x, y, z) )
	{
		snprintf( str, sizeof(str), "Sensor Value : (%.2f, %.2f %.2f)", x, y, z);
		m_pAutoResult->AddString(str, m_SDLColors[COLOR_BLUE]);
	}
	else
	{
		m_pAutoResult->AddString("Sensor value : (0.0, 0.0, 0.0)", m_SDLColors[COLOR_WHITE]);
	}

	//	Draw Result Window
	m_pAutoResult->Update();
}


void CDiagnosticWnd::UpdateManualWnd( bool bAll )
{
	char str[80];
	long long usbDiskSize;
	if( bAll )
	{
		DrawBox( m_Surface, &st_ManuallyTestTitleRect, m_Colors[COLOR_GRAY  ], m_Colors[COLOR_BLACK] );
		DrawString2( m_Surface, m_Font, "Manually Test Result", st_ManuallyTestTitleRect, m_SDLColors[COLOR_BLACK] );
	}

	m_pManualResult->Clear();
	snprintf( str, sizeof(str), "HDMI Status : %s",
		m_bHDMIIsConnected?"Connected":"Disconnted" );
	m_pManualResult->AddString(str, m_SDLColors[COLOR_BLUE]);

	if( StorageTestUtil::GetDeviceSize(USB_STORAGE, 0, usbDiskSize ) )
	{
		snprintf( str, sizeof(str), "USB Storage : %.1fGB", (double)usbDiskSize/1000000000.);
		m_pManualResult->AddString(str, m_SDLColors[COLOR_BLUE]);
	}
	else
	{
		m_pManualResult->AddString("USB Storage : Cannot Access", m_SDLColors[COLOR_RED]);
	}

	//
	//	Update Touch Information
	//
	if( m_NewPoint.x == 0 && m_NewPoint.y == 0 )
	{
		snprintf( str, sizeof(str), "Touch : (%d,%d)", m_NewPoint.x, m_NewPoint.y );
		m_pManualResult->AddString(str, m_SDLColors[COLOR_WHITE]);
	}
	else
	{
		snprintf( str, sizeof(str), "Touch : (%d,%d)", m_NewPoint.x, m_NewPoint.y );
		m_pManualResult->AddString(str, m_SDLColors[COLOR_BLUE]);
	}

	if( m_BtnValue != -1)
	{
		snprintf( str, sizeof(str), "Button : %s(%d)", m_BtnString, m_BtnValue );
		m_pManualResult->AddString(str, m_SDLColors[COLOR_BLUE]);
	}
	else
	{
		snprintf( str, sizeof(str), "Button : %s(%d)", m_BtnString, m_BtnValue );
		m_pManualResult->AddString(str, m_SDLColors[COLOR_WHITE]);
	}

	m_pManualResult->Update();
}

void CDiagnosticWnd::UpdateLCDWnd( bool bAll )
{
	if( bAll )
	{
		DrawBox( m_Surface, &st_LCDTestRedRect  , m_Colors[COLOR_RED  ], m_Colors[COLOR_BLACK] );
		DrawBox( m_Surface, &st_LCDTestGreedRect, m_Colors[COLOR_GREEN], m_Colors[COLOR_BLACK] );
		DrawBox( m_Surface, &st_LCDTestBlueRect , m_Colors[COLOR_BLUE ], m_Colors[COLOR_BLACK] );

		//	Touch Clear Button
		DrawButton( m_Surface, &m_pBtnTouchMode->rect, m_Colors[COLOR_GRAY] );
		DrawString2( m_Surface, m_Font, "Clear", m_pBtnTouchMode->rect, m_SDLColors[COLOR_BLACK] );
	}
}
void CDiagnosticWnd::UpdateMICWnd( bool bAll )
{
	SDL_Rect left = st_MICTestLeftRect;
	SDL_Rect right = st_MICTestRightRect;
	if( bAll )
	{
		DrawBox( m_Surface, &left , m_Colors[COLOR_LIGHT_GRAY], m_Colors[COLOR_BLACK] );
		DrawBox( m_Surface, &right, m_Colors[COLOR_LIGHT_GRAY], m_Colors[COLOR_BLACK] );

		DrawString2( m_Surface, m_Font, "MIC_L", left , m_SDLColors[COLOR_BLACK] );
		DrawString2( m_Surface, m_Font, "MIC_R", right, m_SDLColors[COLOR_BLACK] );
	}

	if( m_pAudRecTest )
	{
		int leftSum, rightSum;
		if( m_pAudRecTest->GetLastSumInfo( leftSum, rightSum ) )
		{
			//	Draw Left Rect
			int lFillHeight = left.h  -(left.h  * leftSum )/32767;
			left.y = left.y + lFillHeight;
			left.h = left.h - lFillHeight;
			DrawBox( m_Surface, &left , m_Colors[COLOR_GREEN], m_Colors[COLOR_BLACK] );

			//	Draw Right Rect
			int rFillHeight = right.h -(right.h * rightSum)/32767;
			right.y = right.y + rFillHeight;
			right.h = right.h - rFillHeight;
			DrawBox( m_Surface, &right, m_Colors[COLOR_GREEN], m_Colors[COLOR_BLACK] );

			//printf("Sum(%d,%d), Height(%d,%d)\n", leftSum, rightSum, lFillHeight, rFillHeight);
			SDL_UpdateRect( m_Surface, st_MICTestRect.x, st_MICTestRect.y, st_MICTestRect.w, st_MICTestRect.h );
		}
	}
}
void CDiagnosticWnd::UpdateCameraWnd( bool bAll )
{
	if( bAll )
	{
		SDL_Rect cameraRect  = { 612,   0, 410, 300 };
		DrawBox( m_Surface, &cameraRect, m_Colors[COLOR_COLOR_KEY], m_Colors[COLOR_BLACK] );

	}
	//	ToDo : Draw Toggle Button
	DrawButton( m_Surface, &m_pBtnCamToggle->rect, m_Colors[COLOR_GRAY] );
	DrawString2( m_Surface, m_Font, "Font/Rear", m_pBtnCamToggle->rect, m_SDLColors[COLOR_BLACK] );
}
void CDiagnosticWnd::UpdateWiFiWnd( bool bAll )
{
	if( bAll )
	{
		DrawBox( m_Surface, &st_WiFiTestTitleRect, m_Colors[COLOR_GRAY  ], m_Colors[COLOR_BLACK] );
		DrawString2( m_Surface, m_Font, "WiFi Test Result", st_WiFiTestTitleRect, m_SDLColors[COLOR_BLACK] );
	}
	m_pWiFiResult->Clear();
	{
		char str[80];
		for( int i=0 ; i<m_NumAPInfo && i<8 ; i++ )
		{
			WIFI_AP_INFO *pApInfo = &m_APInfo[i];
			snprintf( str, sizeof(str), "%02x:%02x:%02x:%02x:%02x:%02x %d %s",
				pApInfo->bssid[0],pApInfo->bssid[1],pApInfo->bssid[2],pApInfo->bssid[3],pApInfo->bssid[4],pApInfo->bssid[5],
				pApInfo->strength, pApInfo->apName);
			m_pWiFiResult->AddString( str, m_SDLColors[COLOR_BLUE] );
		}
	}
	m_pWiFiResult->Update();
}
//
//////////////////////////////////////////////////////////////////////////////


void CDiagnosticWnd::DrawTouchLine( bool continued )
{
	if( !m_NewPoint.valid || !m_OldPoint.valid )
		return;
	DrawLine( m_Surface, m_OldPoint.x, m_OldPoint.y, m_NewPoint.x, m_NewPoint.y, m_Colors[COLOR_RED]);
	if( continued )
	{
		m_OldPoint = m_NewPoint;
		m_NewPoint.valid = 0;
	}
	else
	{
		m_OldPoint.valid = 0;
		m_NewPoint.valid = 0;
	}
}

void CDiagnosticWnd::Start()
{
	m_pAudPlayTest->Start();
	m_pAudRecTest ->Start();
	m_pCamTest    ->SetDspRect(st_CameraTestRect.x, st_CameraTestRect.y, st_CameraTestRect.w, st_CameraTestRect.h);
	m_pCamTest    ->Start();
	m_pHDMITest   ->Start();
	m_pRTCTest    ->Start();
	m_pWiFiTest   ->Start();
	m_pButtonTest ->Start();
	m_pSensorTest ->Start();

	if( 0 != pthread_create( &m_hUpdateThread, NULL, UpdateThreadStub, this ) )
	{
		printf("Error : Cannot create UpdateThread!!!!\n");
	}
}

void CDiagnosticWnd::Stop()
{
	m_pAudPlayTest->Stop();
	m_pAudRecTest ->Stop();
	m_pCamTest    ->Stop();
	m_pHDMITest   ->Stop();
	m_pRTCTest    ->Stop();
	m_pWiFiTest   ->Stop();
	m_pButtonTest ->Stop();
	m_pSensorTest ->Stop();
}

long long GetCurrentTimeMSec()
{
	long long result;
	struct timeval time;
	gettimeofday(&time, NULL);
	result = (long long)time.tv_sec * 1000 + (long long)time.tv_usec/1000;
	return result;
}

void CDiagnosticWnd::UpdateThreadProc()
{
	long long newTime, lastAutoTime, lastManuTime, lastWiFiTime;
	lastAutoTime = lastManuTime = lastWiFiTime = GetCurrentTimeMSec();

	while(1)
	{
		newTime = GetCurrentTimeMSec();
		{
			//	ToDo : Add Storage Information

			if( !m_bLineContinued )
			{
				SDL_Event evt;
				// send message
				if( newTime-lastAutoTime > 500 )
				{
					//	Get Time
					if( m_pRTCTest )
					{
						CNX_AutoLock lock(&m_hUpdateMutex);
						m_pRTCTest->GetTime(m_Year, m_Month, m_Day, m_Hour, m_Min, m_Sec);
					}
					evt.type = SDL_USEREVENT;
					evt.user.type = SDL_USEREVENT;
					evt.user.code = ACT_UPDATE;
					evt.user.data1 = (void*)UPDATE_AUTO;
					SDL_PushEvent(&evt);
					lastAutoTime = newTime;
				}
				if( newTime-lastManuTime > 300 )
				{
					//	Get Button
					if( m_pButtonTest )
					{
						CNX_AutoLock lock(&m_hUpdateMutex);
						m_pButtonTest->GetValue( m_BtnValue, m_BtnString );
					}
					//	Get HDMI
					if( m_pHDMITest )
					{
						CNX_AutoLock lock(&m_hUpdateMutex);
						m_bHDMIIsConnected = m_pHDMITest->IsConnected();
					}
					evt.type = SDL_USEREVENT;
					evt.user.type = SDL_USEREVENT;
					evt.user.code = ACT_UPDATE;
					evt.user.data1 = (void*)UPDATE_MANU;
					SDL_PushEvent(&evt);
					lastManuTime = newTime;
				}
				if( newTime-lastWiFiTime > 1000 )
				{
					//	Get WiFi AP Scan Result
					if( m_pWiFiTest )
					{
						CNX_AutoLock lock(&m_hUpdateMutex);
						m_NumAPInfo = m_pWiFiTest->GetWiFiApInfo( &m_APInfo[0] );
					}
					evt.type = SDL_USEREVENT;
					evt.user.type = SDL_USEREVENT;
					evt.user.code = ACT_UPDATE;
					evt.user.data1 = (void*)UPDATE_WIFI;
					SDL_PushEvent(&evt);
					lastWiFiTime = newTime;
				}
				if( m_pAudRecTest )
				{
					evt.type = SDL_USEREVENT;
					evt.user.type = SDL_USEREVENT;
					evt.user.code = ACT_UPDATE;
					evt.user.data1 = (void*)UPDATE_MIC;
					SDL_PushEvent(&evt);
				}
			}
		}
		usleep(50000);
	}
}

