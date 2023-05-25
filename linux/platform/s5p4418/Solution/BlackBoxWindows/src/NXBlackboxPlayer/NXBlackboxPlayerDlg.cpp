
// NXBlackboxPlayerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "NXBlackboxPlayer.h"
#include "NXBlackboxPlayerDlg.h"
#include "afxdialogex.h"
#include "NX_ParserUserData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAP_PATH_DEBUG		L"C:"
#define MAP_NAME			L"map-google.html"

#define TIMER_PROGRESS		101

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNXBlackboxPlayerDlg 대화 상자




CNXBlackboxPlayerDlg::CNXBlackboxPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNXBlackboxPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNXBlackboxPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPLORER_MAP, m_ExplorerMap);
	DDX_Control(pDX, IDC_LIST_FILE, m_ListFile);
	DDX_Control(pDX, IDC_STC_LATITUDE, m_StcLatitude);
	DDX_Control(pDX, IDC_STC_LONGITUDE, m_StcLongitude);
	DDX_Control(pDX, IDC_STC_VELOCITY, m_StcVelocity);
	DDX_Control(pDX, IDC_STC_GRAVITY_X, m_StcGravityX);
	DDX_Control(pDX, IDC_STC_GRAVITY_Y, m_StcGravityY);
	DDX_Control(pDX, IDC_STC_GRAVITY_Z, m_StcGravityZ);
	DDX_Control(pDX, IDC_STC_DURATION, m_StcDuration);
	DDX_Control(pDX, IDC_SLD_SEEK, m_SldSeek);
	DDX_Control(pDX, IDC_SLD_VOLUME, m_SdlVolume);
	DDX_Control(pDX, IDC_STC_VIDEO0, m_StcVideo0);
	DDX_Control(pDX, IDC_STC_VIDEO1, m_StcVideo1);
}

BEGIN_MESSAGE_MAP(CNXBlackboxPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WMU_GRAPHNOTIFY, OnGraphNotify)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILE, &CNXBlackboxPlayerDlg::OnNMDblclkListFile)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CNXBlackboxPlayerDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CNXBlackboxPlayerDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CNXBlackboxPlayerDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_FORWARD, &CNXBlackboxPlayerDlg::OnBnClickedBtnForward)
	ON_BN_CLICKED(IDC_BTN_BACKWARD, &CNXBlackboxPlayerDlg::OnBnClickedBtnBackward)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CNXBlackboxPlayerDlg::OnBnClickedBtnNext)
	ON_BN_CLICKED(IDC_BTN_PREVIOUS, &CNXBlackboxPlayerDlg::OnBnClickedBtnPrevious)
	ON_BN_CLICKED(IDC_BTN_MINIMIZE, &CNXBlackboxPlayerDlg::OnBnClickedBtnMinimize)
	ON_BN_CLICKED(IDC_BTN_MAXIMIZE, &CNXBlackboxPlayerDlg::OnBnClickedBtnMaximize)
	ON_BN_CLICKED(IDC_BTN_ZOOM, &CNXBlackboxPlayerDlg::OnBnClickedBtnZoom)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CNXBlackboxPlayerDlg 메시지 처리기

BOOL CNXBlackboxPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// Resotre Configuration(Last Path)
	TCHAR pathValue[255];
	GetPrivateProfileString(L"Owner", L"LastPath", NULL, pathValue, sizeof(pathValue), L".\\NXBlackboxPlayer.ini");
	m_StrPath.Format(L"%s", pathValue);

	// Vraialbe Initialize
	m_bOpenMap = false;
	m_nDrawMapSampling = 0;
	
	
	// Zoom Dialog Initliaze
	m_pZoomDlg	= NULL; 
	m_pPlayer = NULL;

	// Control Initialize
	Initialize();

	// Static Text Initliaze
	ClearUserData();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CNXBlackboxPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CNXBlackboxPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CNXBlackboxPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#define LIST_COLUMN_FILENAME		0
#define LIST_COLUMN_DATE			1
#define LIST_COLUMN_SIZE			2
#define LIST_COLUMN_PATH			3

#define LINECHART_MAX_RANGE			100
#define LINECHART_CENTER			50
#define LINECHART_DRAW_MARGIN		10

void CNXBlackboxPlayerDlg::Initialize( void )
{
	// Initialize List Control
	m_ListFile.GetHeaderCtrl()->EnableWindow(FALSE);
	//m_ListFile.SetBkColor(RGB(101, 82, 67));

	m_ListFile.InsertColumn( LIST_COLUMN_FILENAME, L"이름", LVCFMT_LEFT, 210 );
	m_ListFile.InsertColumn( LIST_COLUMN_DATE, L"수정한 날짜", LVCFMT_CENTER | LVCFMT_WRAP, 90 );
	m_ListFile.InsertColumn( LIST_COLUMN_SIZE, L"크기", LVCFMT_CENTER, 80 );
	m_ListFile.InsertColumn( LIST_COLUMN_PATH, L"경로", LVCFMT_CENTER, 0 );

	m_ListFile.SetRedraw(TRUE);
	ListView_SetExtendedListViewStyle( m_ListFile.m_hWnd, LVS_EX_FULLROWSELECT );

	// Initliaze Map
	m_bOpenMap = LoadMap();
	::ShowWindow( m_ExplorerMap.GetSafeHwnd(), SW_SHOW );

	// Initialize Line Chart
	m_LineChart.SubclassDlgItem( IDC_LINE_CHART, this );

	// Line Chart Color / Range :: Add( COLOR, MAX, MIN )
	m_LineChart.Add( RGB(144, 195, 31),	LINECHART_MAX_RANGE, LINECHART_DRAW_MARGIN );
	m_LineChart.Add( RGB(230, 0, 18),	LINECHART_MAX_RANGE, LINECHART_DRAW_MARGIN );
	m_LineChart.Add( RGB(248, 182, 43),	LINECHART_MAX_RANGE, LINECHART_DRAW_MARGIN );

	m_LineChart.SetPos(0, 0 + LINECHART_DRAW_MARGIN);
	m_LineChart.SetPos(1, 0 + LINECHART_DRAW_MARGIN);
	m_LineChart.SetPos(2, 0 + LINECHART_DRAW_MARGIN);

	// Initalize flags
	m_bPlay = false;
}


bool CNXBlackboxPlayerDlg::GetFileList(CString strDrive)
{
	m_ListFile.SetRedraw(false);

	CFileFind fileFinder;
	
	CString strFilename;
	CString strTime, strFileSize;
	CString strPath;

	BOOL bWorking = fileFinder.FindFile( strDrive + L"\\*.*" );
	
	int nItemCnt = m_ListFile.GetItemCount() ;
	
	while(bWorking)
	{
		bWorking = fileFinder.FindNextFile();

		if( fileFinder.IsDirectory() )
		{
			strFilename = fileFinder.GetFileName();
			
			if( strFilename == L"normal" || strFilename == L"event" )
			{
				GetFileList( strDrive + L"\\" + strFilename );
			}
		}
		else
		{
			strFilename = fileFinder.GetFileName();
			CString strFileExtention = strFilename.Right( 3 );

			strFileExtention.MakeUpper();

			if ( strFileExtention == L"MP4" || strFileExtention == L".TS" )
			{
				CTime timeCreation;
				m_ListFile.InsertItem( nItemCnt, strFilename );
				
				fileFinder.GetLastWriteTime( timeCreation );	// Last Modification Date
				strTime.Format( L"%02d.%02d.%02d", timeCreation.GetYear(), timeCreation.GetMonth(), timeCreation.GetDay() );
				m_ListFile.SetItemText( nItemCnt, LIST_COLUMN_DATE, strTime );
				
				ULONGLONG fileSize = fileFinder.GetLength() / 1024;
				strFileSize.Format( L"%d MB", int(fileSize / 1024) );
				m_ListFile.SetItemText( nItemCnt, LIST_COLUMN_SIZE, strFileSize );

				strPath.Format( L"%s", fileFinder.GetFilePath() );
				m_ListFile.SetItemText( nItemCnt, LIST_COLUMN_PATH, strPath );

				nItemCnt++;
			}
		}
	}
	fileFinder.Close();
	m_ListFile.SetRedraw( TRUE );

	return false;
}


int CALLBACK cbSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl *pListCtrl = (CListCtrl *)lParamSort;
	int nFirst = 0, nSecond = 0;

	LV_FINDINFO find_data;
	find_data.flags = LVFI_PARAM | LVFI_WRAP;

	find_data.lParam = lParam1;
	nFirst = pListCtrl->FindItem( &find_data );

	find_data.lParam = lParam2;
	nSecond = pListCtrl->FindItem( &find_data, nFirst );

	// Get String Data
	CString strData1 = pListCtrl->GetItemText( nFirst, LIST_COLUMN_FILENAME );
	CString strData2 = pListCtrl->GetItemText( nSecond, LIST_COLUMN_FILENAME );

	// return sorting value
	return strData1.Compare( strData2 ) * 1;	// 1(Asending), -1(Descending)
}


bool CNXBlackboxPlayerDlg::OpenFile( CString strFileName )
{
	CString fileExtension;
	int fileType = CNX_BBFilterManager::FILE_TYPE_MP4;

	fileExtension = strFileName.Right( 3 );

	fileExtension.MakeUpper();

	if( fileExtension == L"MP4" )
		fileType = CNX_BBFilterManager::FILE_TYPE_MP4;
	else if( fileExtension == L".TS" )
		fileType = CNX_BBFilterManager::FILE_TYPE_MP2TS;

	//	Step 1. Delete Previeous Player Control
	if( m_pPlayer )
	{
		delete m_pPlayer;
		m_pPlayer = NULL;
	}
	//	Step 2. Create Play Control
	m_pPlayer = new CNX_BBFilterManager(fileType);

	char fileName[2048];
	memset( fileName, 0, sizeof(fileName) );
	WideCharToMultiByte( CP_ACP, 0, strFileName.GetBuffer(), strFileName.GetLength(), fileName, sizeof(fileName), NULL, NULL );

	m_pPlayer->Init( fileName, (OAHWND)this->GetSafeHwnd(), WMU_GRAPHNOTIFY, (OAHWND)m_StcVideo0.GetSafeHwnd(), (OAHWND)m_StcVideo1.GetSafeHwnd(), NULL );

	//	Step 3. User Data Parsing
	UserDataParser( strFileName, fileType );

	//	Step 4. Control Reset
	unsigned long duration;
	m_pPlayer->GetDuration( &duration );
	m_SldSeek.SetRange( 0, (int)duration );
	m_SldSeek.SetPos( 0 );
	
	return true;
}


void CNXBlackboxPlayerDlg::UpdateProgress( void )
{
	unsigned long nCurrentTime, nDurationTime;
	m_pPlayer->GetCurTime( &nCurrentTime );
	m_pPlayer->GetDuration( &nDurationTime );

	m_SldSeek.SetPos( (int)nCurrentTime );
	UpdateDuration( nCurrentTime, nDurationTime );
	UpdateUserData( nCurrentTime );
}


void CNXBlackboxPlayerDlg::UpdateDuration( unsigned long current, unsigned long duration )
{
	int hh, mm, ss;
	CString curStr, durStr;

	current /= 1000;
	hh = (int)(current / 3600);
	mm = (int)(current % 3600) / 60;
	ss = (int)(current % 3600) % 60;
	curStr.Format( L"%02d:%02d:%02d", hh, mm, ss );

	duration /= 1000;
	hh = (int)(duration / 3600);
	mm = (int)(duration % 3600) / 60;
	ss = (int)(duration % 3600) % 60;
	durStr.Format( L"%02d:%02d:%02d", hh, mm, ss );

	m_StcDuration.SetWindowTextW( curStr + L" / " + durStr );
}


static FILE *pFile;

static int cbFileRead( long long int pos, long int len, unsigned char *pBuf)
{
	if( !pFile )
		return -1;

	fseek( pFile, (long)pos, SEEK_SET );
	fread( pBuf, 1, len, pFile );

	return 0;
}


static int cbFileLength( long long *totalSize, long long *availSize )
{
	unsigned int size;

	fseek( pFile, 0, SEEK_END );
	size = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	*totalSize = size;
	*availSize = size;

	return 0;
}

///////////////////////////////////////////////////////////////////////
//
//	MP4 Parser User Data Parsing
//
#include <afxconv.h> 
bool CNXBlackboxPlayerDlg::MP4UserDataParser( CString strFileName )
{
	unsigned long long timeStamp = 0;
	unsigned long size = 0;;
	unsigned char *pBuf = NULL, *pBufTemp = NULL;

	int hParser = 0;
	PARSER_INFO parserInfo;
		
	char fileName[2048];
	memset( fileName, 0, sizeof(fileName) );
	WideCharToMultiByte( CP_ACP, 0, strFileName.GetBuffer(), strFileName.GetLength(), fileName, sizeof(fileName), NULL, NULL );

	fopen_s( &pFile, fileName, "rb");
	m_UserData.RemoveAll();
	hParser	= NXParserUserDataInit( fileName, 64 * 1024 );
	NXParserUserDataWrapFunc( hParser, cbFileRead, cbFileLength );
	NXParserUserDataGetInfo( hParser, &parserInfo );
	while( 1 )
	{
		if( 0 != NXParserGetIndex( hParser, &size, &timeStamp) )
			break;
		
		pBuf = (unsigned char*)malloc( size + 1 );
		pBufTemp = pBuf;

		NXParserUserDataGetFrame( hParser, &pBuf, &size, &timeStamp );
		pBuf[size] = NULL;
		
		CNX_UserData userData;
		memset( &userData, 0x00, sizeof(userData) );

		userData.timeStamp = timeStamp;
		CopyMemory( userData.latitude, pBufTemp, GPS_LATITUDE_SIZE );
		pBufTemp += GPS_LATITUDE_SIZE + 1;
		CopyMemory( userData.longitude, pBufTemp, GPS_LONGITUDE_SIZE );
		pBufTemp += GPS_LONGITUDE_SIZE + 1;
		CopyMemory( userData.velocity, pBufTemp, VELOCITY_SIZE );
		pBufTemp += VELOCITY_SIZE + 1;

		CopyMemory( userData.gravity_x, pBufTemp, GRA_X_SIZE );
		pBufTemp += GRA_X_SIZE + 1;
		CopyMemory( userData.gravity_y, pBufTemp, GRA_Y_SIZE );
		pBufTemp += GRA_Y_SIZE + 1;
		CopyMemory( userData.gravity_z, pBufTemp, GRA_Z_SIZE );

		free( pBuf );
		m_UserData.AddTail( userData );
	}

	NXParserUserDataClose( hParser );
	fclose( pFile );
	return 0;
}
//
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
//
//	MPEG2 TS User Data Parser
//
#include "ParseUserData.h"
#define TS_IN_BUF_SIZE	(188*3)

void CNXBlackboxPlayerDlg::TSUserDataCallback(void *privateData, unsigned char *pData, int size, __int64 timeStamp)
{
	CNXBlackboxPlayerDlg *pObj = (CNXBlackboxPlayerDlg *)privateData;
	CNX_UserData userData;
	memset( &userData, 0x00, sizeof(userData) );
	userData.timeStamp = timeStamp;
	CopyMemory( userData.latitude,  pData, GPS_LATITUDE_SIZE  ); pData += GPS_LATITUDE_SIZE  + 1;
	CopyMemory( userData.longitude, pData, GPS_LONGITUDE_SIZE ); pData += GPS_LONGITUDE_SIZE + 1;
	CopyMemory( userData.velocity,  pData, VELOCITY_SIZE      ); pData += VELOCITY_SIZE      + 1;
	CopyMemory( userData.gravity_x, pData, GRA_X_SIZE         ); pData += GRA_X_SIZE         + 1;
	CopyMemory( userData.gravity_y, pData, GRA_Y_SIZE         ); pData += GRA_Y_SIZE         + 1;
	CopyMemory( userData.gravity_z, pData, GRA_Z_SIZE         );
	pObj->m_UserData.AddTail( userData );
}

bool CNXBlackboxPlayerDlg::TSUserDataParser( CString strFileName )
{
	FILE *fd;
	char fileName[2048];
	memset( fileName, 0, sizeof(fileName) );
	WideCharToMultiByte( CP_ACP, 0, strFileName.GetBuffer(), strFileName.GetLength(), fileName, sizeof(fileName), NULL, NULL );

	fopen_s( &fd, fileName, "rb");

	if( fd == NULL )
		return false;

	m_UserData.RemoveAll();
	CParseUserData *pParaer = new CParseUserData();
	pParaer->Init( &TSUserDataCallback, this );
	size_t readSize;
	int pos = 0, ret;
	unsigned char *pBuf;
	unsigned char buffer[TS_IN_BUF_SIZE];
	do{
		readSize = fread( buffer+pos, 1, TS_IN_BUF_SIZE-pos, fd );

		if( readSize<=0 )
			break;

		readSize += pos;
		pBuf = buffer;
		while( readSize > 189 ){
			if( pBuf[0] == 0x47 && pBuf[188] == 0x47 )
			{
				//	put data to parser
				ret = pParaer->Parse( pBuf );
				if( ret<0 )
				{
					wchar_t msg[80];
					wsprintf( msg, TEXT("[NEXELL][USR_DATA] ret = %d\n"), ret );
					OutputDebugString( msg );
				}
				pBuf += 188;
				readSize -= 188;

			}else{
				pBuf ++;
				readSize --;
			}
		}
		memcpy( buffer, pBuf, readSize );
		pos = readSize;

	}while(1);
	delete pParaer;
	fclose( fd );
	return 0;
}
//
///////////////////////////////////////////////////////////////////////

bool CNXBlackboxPlayerDlg::UserDataParser( CString strFileName, int fileType )
{
	if( fileType == CNX_BBFilterManager::FILE_TYPE_MP4 )
		return MP4UserDataParser(strFileName);
	else if( fileType == CNX_BBFilterManager::FILE_TYPE_MP2TS )
		return TSUserDataParser(strFileName);
	else
		return false;
}


void CNXBlackboxPlayerDlg::UpdateUserData( LONGLONG timeStamp )
{
	POSITION pos = m_UserData.GetHeadPosition();
	for(int i = 0; i < m_UserData.GetCount(); i++) {
		CNX_UserData userData= m_UserData.GetNext( pos );

		if( userData.timeStamp > ((unsigned long long)timeStamp) - 1000) {
			CString strTemp;
			
			strTemp.Format(L"Latitude   : %S", userData.latitude);
			m_StcLatitude.SetWindowTextW( strTemp );

			strTemp.Format(L"Longitude : %S", userData.longitude);
			m_StcLongitude.SetWindowTextW( strTemp );

			strTemp.Format(L"Velocity   : %S", userData.velocity);
			m_StcVelocity.SetWindowTextW( strTemp );
			
			strTemp.Format(L"GravityX  : %S", userData.gravity_x);
			m_StcGravityX.SetWindowTextW( strTemp );

			strTemp.Format(L"GravityY  : %S", userData.gravity_y);
			m_StcGravityY.SetWindowTextW( strTemp );

			strTemp.Format(L"GravityZ  : %S", userData.gravity_z);
			m_StcGravityZ.SetWindowTextW( strTemp );

			if( (m_bOpenMap) && !(m_nDrawMapSampling % 5 )) {
				UpdateWebBrowser( userData.latitude, userData.longitude );
			}
			m_nDrawMapSampling++;

			int temp;
			char digit[5];

			memcpy( digit, userData.gravity_x + 1, sizeof(digit) );
			temp = ((userData.gravity_x[0] == '+') ? 1 : -1) * atoi( digit ) / 100;
			if( temp > LINECHART_MAX_RANGE ) temp = LINECHART_MAX_RANGE;
			m_LineChart.SetPos( 0, temp + LINECHART_CENTER );

			memcpy( digit, userData.gravity_y + 1, sizeof(digit) );
			temp = ((userData.gravity_y[0] == '+') ? 1 : -1) * atoi( digit ) / 100;
			if( temp > LINECHART_MAX_RANGE ) temp = LINECHART_MAX_RANGE;
			m_LineChart.SetPos( 1, temp + LINECHART_CENTER );

			memcpy( digit, userData.gravity_z + 1, sizeof(digit) );
			temp = ((userData.gravity_z[0] == '+') ? 1 : -1) * atoi( digit ) / 100;
			if( temp > LINECHART_MAX_RANGE ) temp = LINECHART_MAX_RANGE;
			m_LineChart.SetPos( 2, temp + LINECHART_CENTER );
			
			m_LineChart.Invalidate( true );
			m_LineChart.Go();

			break;
		}
	}
}


void CNXBlackboxPlayerDlg::ClearUserData( void )
{
	m_StcLatitude.SetWindowTextW( L"Latitude   : +00.000000" );
	m_StcLongitude.SetWindowTextW( L"Longitude : +000.000000" );
	m_StcVelocity.SetWindowTextW( L"Velocity   : 000" );
	m_StcGravityX.SetWindowTextW( L"GravityX  : +0000" );
	m_StcGravityY.SetWindowTextW( L"GravityY  : +0000" );
	m_StcGravityZ.SetWindowTextW( L"GravityZ  : +0000" );
	m_StcDuration.SetWindowTextW( L"00:00:00 / 00:00:00" );
}


#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x)	HeapAlloc( GetProcessHeap(), 0, (x) )
#define FREE(x)		HeapFree( GetProcessHeap(), 0, (x) )

int CNXBlackboxPlayerDlg::CheckInternetConnection( void )
{
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	char strText[256], strKeyword[256];
	bool bOperation = false;

	MIB_IFTABLE	*pIfTable;
	MIB_IFROW	*pIfRow;

	pIfTable = (MIB_IFTABLE *)MALLOC( sizeof (MIB_IFTABLE) );
	if( NULL == pIfTable )
	{
		//TRACE("Error allocating memory needed to call GetIfTable.");
		return -1;
	}

    dwSize = sizeof( MIB_IFTABLE );
    if ( GetIfTable( pIfTable, &dwSize, FALSE ) == ERROR_INSUFFICIENT_BUFFER )
	{
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *)MALLOC( dwSize );
        
		if (pIfTable == NULL) {
            //TRACE("Error allocating memory.");
            return -1;
        }
    }

	if ( NO_ERROR == (dwRetVal = GetIfTable(pIfTable, &dwSize, 0)) )
	{
		if ( pIfTable->dwNumEntries > 0 )
		{
			pIfRow = (MIB_IFROW *)MALLOC( sizeof(MIB_IFROW) );
			if (pIfRow == NULL)
			{
				//TRACE("Error allocating memory.");
				
				if( pIfTable != NULL )
				{
					FREE(pIfTable);
					pIfTable = NULL;
				}
				return -1;
			}

			for (int i = 0; i < (int) pIfTable->dwNumEntries; i++)
			{
				pIfRow->dwIndex = pIfTable->table[i].dwIndex;
				if( (dwRetVal = GetIfEntry(pIfRow)) == NO_ERROR )
				{
					if(pIfRow->dwType != MIB_IF_TYPE_ETHERNET)
						continue;

					memset(strText, 0x00, sizeof(strText));

					for( int j = 0; j < (signed)pIfRow->dwDescrLen; j++ )
						strText[j] = pIfRow->bDescr[j];

					sscanf_s( strText, "%s", strKeyword, sizeof(strKeyword) );
					if( strcmp(strKeyword, "Bluetooth") && strcmp(strKeyword, "WAN") && strcmp(strKeyword, "VMware"))
					{
						if( MIB_IF_OPER_STATUS_OPERATIONAL == pIfRow->dwOperStatus )
						{
							//TRACE("Network Description : %s", strText);
							//TRACE("Network Operational.");
							
							if( pIfRow != NULL )
							{
								FREE( pIfRow );
								pIfRow = NULL;
							}

							if (pIfTable != NULL)
							{
								FREE( pIfTable );
								pIfTable = NULL;
							}
							return 0;
						}
					}
				}
			}
		}
	}
	
	return -1;
}


#include <mshtmlc.h>

void CNXBlackboxPlayerDlg::ExecuteScript(CString &strFunction)
{
	if(strFunction.GetLength() == 0) return;

	HRESULT					hr;
	CComVariant				vtResult(0);
	CComPtr<IDispatch>      spDisp = NULL;
	CComPtr<IHTMLDocument2> spDocument = NULL;
	CComPtr<IHTMLWindow2>   spWindow = NULL;

	spDisp = m_ExplorerMap.get_Document();		// Get Document Interface
	if( spDisp == NULL ) return;
	
	hr = spDisp->QueryInterface(IID_IHTMLDocument2, (void **)&spDocument);
	spDisp = NULL;
	if( FAILED(hr) || spDocument == NULL )
		return;
	
	hr = spDocument->get_parentWindow(&spWindow);
	spDocument = NULL;
	if( SUCCEEDED(hr) && spWindow != NULL )
	{
		spWindow->execScript(CComBSTR((LPCTSTR)strFunction), CComBSTR(_T("JScript")), &vtResult);
		spWindow = NULL;
	}
}


bool CNXBlackboxPlayerDlg::LoadMap( void )
{
	CFile file;

	HKEY hKey;
	TCHAR szBuffer[256] = {'\0', };

	DWORD dwType = REG_SZ;
	DWORD dwSize = 128;
	
	// Get Application Location
	CString strReg = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
	CString strExe = L"NXBlackboxPlayer.exe";
	CString strPath;

	// Check Internet Connection
	if( CheckInternetConnection() ) {
		AfxMessageBox(L"Internet is not connected.");
		return false;
	}

	// Reference to System Top directory ( C:\ ) :: for debugging
	strPath.Format( L"%s/%s", MAP_PATH_DEBUG, MAP_NAME );
	if( file.Open((LPCTSTR)strPath, CFile::modeRead) )
	{
		file.Close();
		m_ExplorerMap.Navigate( L"file://" + strPath, NULL, NULL, NULL, NULL );
		::ShowWindow( m_ExplorerMap.GetSafeHwnd(), SW_HIDE );
		return true;;
	}

	// Reference to Installation Directory
	LONG IResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, strReg + strExe, 0, KEY_READ, &hKey );
	if( ERROR_SUCCESS == IResult )
	{
		RegQueryValueEx(hKey, L"", NULL, &dwType, (LPBYTE)szBuffer, &dwSize);
		RegCloseKey(hKey);
	}
	
	strPath.Format(L"%s", szBuffer);
	strPath =  strPath.Left( strPath.GetLength() - strExe.GetLength() );

	strPath.Replace(L"\\", L"/");
	strPath.AppendFormat(MAP_NAME);
	
	if( file.Open( (LPCTSTR)strPath, CFile::modeRead) )
	{
		file.Close();
		m_ExplorerMap.Navigate( L"file://" + strPath, NULL, NULL, NULL, NULL );
		::ShowWindow( m_ExplorerMap.GetSafeHwnd(), SW_HIDE );
		return true;
	}

	AfxMessageBox(L"Unable to load map.");
	
	return false;
}


void CNXBlackboxPlayerDlg::UpdateWebBrowser(char *latitude, char *longitude)
{
	CString tmpStr1, tmpStr2;
	if( (((latitude[0] == '+') || (latitude[0] == '-')) && (latitude[3] == '.')) &&
		(((longitude[0] == '+') || (longitude[0] == '-')) && (longitude[4] == '.')) )
	{
			tmpStr1.Format( L"%S", latitude );		
			tmpStr2.Format( L"%S", longitude );
			RefreshMap( tmpStr1, tmpStr2 );
	}
}


void CNXBlackboxPlayerDlg::RefreshMap(CString latitue, CString longitude)
{
	CString strText;
	double tempLatitue = _tstof( latitue );
	double tempLongitude = _tstof( longitude );
	
	strText.Format((L"search(%s,%s)"), latitue, longitude);

	if( !CheckInternetConnection() )
	{
		if( tempLatitue != 0.0 && tempLongitude != 0.0 )
		{
			ExecuteScript( strText );
		} 
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Event Handler
//
LRESULT CNXBlackboxPlayerDlg::OnGraphNotify(WPARAM wParam, LPARAM lParam)
{
	if( m_pPlayer )
	{
		int ret;
		unsigned int eventCode;
		ret = m_pPlayer->FilterEventProc(&eventCode);

		switch( eventCode )
		{
		case EVT_CODE_EOS:
			//	Process Event Code
			OnBnClickedBtnNext();
			break;
		}
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Message Handler
//
void CNXBlackboxPlayerDlg::OnNMDblclkListFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CString strFileName = m_ListFile.GetItemText(pNMItemActivate->iItem, 3);

	if (strFileName != L"")
	{
		m_nPlayIndex = pNMItemActivate->iItem;
		if( OpenFile(strFileName) )
		{
			OnBnClickedBtnPlay();
			m_ListFile.SetHotItem(m_nPlayIndex);
			m_ListFile.SetSelectionMark(m_nPlayIndex);		

			for( int iRow = 0; iRow <= m_ListFile.GetItemCount(); iRow++ )
			{
				m_ListFile.SetItemState(iRow, 0, LVIS_SELECTED | LVIS_DROPHILITED);				
				m_ListFile.SetFocus();
			}

			m_ListFile.SetSelectionMark(m_nPlayIndex);
			m_ListFile.SetItemState(m_nPlayIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_ListFile.SetFocus();
			m_ListFile.Invalidate();
		}
		else
		{
			m_nPlayIndex = -1;
		}
	}
	*pResult = 0;
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CBrowseFolderDialog dlg;
	
	if( dlg.DoModal(m_StrPath) == IDOK )
	{
		m_ListFile.DeleteAllItems();
		dlg.GetSelectStr(m_StrPath);
		
		GetFileList(m_StrPath);

		for( int i = 0; i < m_ListFile.GetItemCount(); i++ )
		{
			m_ListFile.SetItemData( i, i );
		}
		m_ListFile.SortItems( cbSortFunc, (LPARAM)&m_ListFile );
	}
}

//	Play & Pause
void CNXBlackboxPlayerDlg::OnBnClickedBtnPlay()
{
	//	Step 1. Play/Pause
	if( m_pPlayer )
	{
		if( m_bPlay == false ) {
			m_pPlayer->Play();
			SetTimer( TIMER_PROGRESS, 400, NULL );
			GetDlgItem(IDC_BTN_PLAY)->SetWindowTextW(L"Pause");
			GetDlgItem(IDC_BTN_PLAY)->Invalidate(false);
			m_bPlay = true;
		}
		else {
			m_pPlayer->Pause();
			KillTimer( TIMER_PROGRESS );
			GetDlgItem(IDC_BTN_PLAY)->SetWindowTextW(L"Play");
			GetDlgItem(IDC_BTN_PLAY)->Invalidate(false);
			m_bPlay = false;
		}
	}
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnStop()
{
	//	Step 1. Delete Current Player Control
	if( m_pPlayer )
	{
		GetDlgItem(IDC_BTN_PLAY)->SetWindowTextW(L"Play");
		GetDlgItem(IDC_BTN_PLAY)->Invalidate(false);

		m_pPlayer->Stop();
		m_pPlayer->Close();
		KillTimer( TIMER_PROGRESS );
		m_bPlay = false;
	}
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnForward()
{
	//	Step 1. Seek Forward
	if( m_pPlayer )
	{
		unsigned long curTime;
		m_pPlayer->GetCurTime( &curTime );
		m_pPlayer->Seek(curTime + SEEK_TIME);
	}
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnBackward()
{
	//	Step 1. Seek Backward
	if( m_pPlayer )
	{
		unsigned long curTime;
		m_pPlayer->GetCurTime( &curTime );
		if( curTime < SEEK_TIME )
			m_pPlayer->Seek( 0 );
		else
			m_pPlayer->Seek( curTime-SEEK_TIME );
	}
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnNext()
{
	if( m_ListFile.GetItemCount() < 1 )
		return;

	//	Find Next Item
	if( m_nPlayIndex >= m_ListFile.GetItemCount() - 1)
	{
		m_nPlayIndex = 0;
	}
	else
	{
		m_nPlayIndex++;
	}

	//	Get File Name
	CString strFileName = m_ListFile.GetItemText(m_nPlayIndex, LIST_COLUMN_PATH);
	if( OpenFile(strFileName) )
	{
		OnBnClickedBtnPlay();
		for( int iRow=0; iRow <= m_ListFile.GetItemCount(); iRow++ )
		{
			m_ListFile.SetItemState(iRow, 0, LVIS_SELECTED|LVIS_DROPHILITED);	
			m_ListFile.SetFocus();
		}
		m_ListFile.SetItemState(m_nPlayIndex,LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		m_ListFile.EnsureVisible(m_nPlayIndex, FALSE);
		m_ListFile.Invalidate();
	}
	else
	{
		m_nPlayIndex = -1;
	}
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnPrevious()
{
	if( m_ListFile.GetItemCount() < 1 )
		return;

	//	Find Next Item
	if( m_nPlayIndex <= 0 )
	{
		m_nPlayIndex = m_ListFile.GetItemCount() - 1;
	}
	else
	{
		m_nPlayIndex--;
	}
	
	//	Get File Name
	CString strFileName = m_ListFile.GetItemText( m_nPlayIndex, LIST_COLUMN_PATH);
	if( OpenFile(strFileName) )
	{
		OnBnClickedBtnPlay();

		for(int iRow=0; iRow <= m_ListFile.GetItemCount(); iRow++)
		{
			m_ListFile.SetItemState(iRow, 0, LVIS_SELECTED|LVIS_DROPHILITED);	
			m_ListFile.SetFocus();
		}
		m_ListFile.SetItemState(m_nPlayIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		m_ListFile.EnsureVisible(m_nPlayIndex, FALSE);
		m_ListFile.Invalidate();
	}
	else
	{
		m_nPlayIndex = -1;
	}
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnMinimize()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// Not Implemetation
	//ShowWindow( SW_MINIMIZE );
}


void CNXBlackboxPlayerDlg::OnBnClickedBtnMaximize()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// Not Implemetation
	//ShowWindow( SW_MAXIMIZE );
}

#define ZOOM_DIALOG_WIDTH		500
#define ZOOM_DIALOG_HEIGHT		350

void CNXBlackboxPlayerDlg::OnBnClickedBtnZoom()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_pZoomDlg == NULL ) {
		m_pZoomDlg = new CZoomDlg;
		m_pZoomDlg->Create( IDD_ZOOM_DIALOG );

		RECT clientRect;
		
		GetWindowRect( &clientRect );
		m_pZoomDlg->MoveWindow( clientRect.right - ZOOM_DIALOG_WIDTH, clientRect.bottom - ZOOM_DIALOG_HEIGHT, ZOOM_DIALOG_WIDTH, ZOOM_DIALOG_HEIGHT );
	} 
	
	m_pZoomDlg->ShowWindow( SW_NORMAL );
	m_pZoomDlg->Invalidate();
}


void CNXBlackboxPlayerDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( m_pZoomDlg )
		delete m_pZoomDlg;

	if( m_pPlayer )
		delete m_pPlayer;

	WritePrivateProfileString( L"Owner", L"LastPath", m_StrPath, L".\\NXBlackboxPlayer.ini" );

	KillTimer( TIMER_PROGRESS );

	CDialogEx::OnClose();
}


void CNXBlackboxPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( (nIDEvent == TIMER_PROGRESS) ) {
		UpdateProgress();
	}

	CDialogEx::OnTimer(nIDEvent);
}


BOOL CNXBlackboxPlayerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if(pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_RETURN )
			return TRUE;
		
		if( pMsg->wParam == VK_ESCAPE )
			return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

