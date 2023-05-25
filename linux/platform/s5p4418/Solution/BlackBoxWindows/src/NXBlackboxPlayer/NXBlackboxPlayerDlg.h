
// NXBlackboxPlayerDlg.h : 헤더 파일
//

#pragma once

#include "ExplorerMap.h"
#include "BrowseFolderDialog.h"
#include "ZoomDlg.h"
#include "LineChartCtrl.h"
#include "CNX_BBFilterManager.h"

#include "afxcmn.h"
#include "afxwin.h"

#define ENABLE_ZOOM

// User Data Size
#define	GPS_LATITUDE_SIZE		10
#define	GPS_LONGITUDE_SIZE		11
#define	VELOCITY_SIZE			3

#define	GRA_X_SIZE				5
#define	GRA_Y_SIZE				5
#define	GRA_Z_SIZE				5

// Class for userdata
class CNX_UserData
{
public :
	unsigned long long	timeStamp;
	char				latitude[GPS_LATITUDE_SIZE + 1];
	char				longitude[GPS_LONGITUDE_SIZE + 1];
	char				velocity[VELOCITY_SIZE + 1];
	char				gravity_x[GRA_X_SIZE + 1];
	char				gravity_y[GRA_Y_SIZE + 1];
	char				gravity_z[GRA_Z_SIZE + 1];
};

// CNXBlackboxPlayerDlg 대화 상자
class CNXBlackboxPlayerDlg : public CDialogEx
{
// 생성입니다.
public:
	CNXBlackboxPlayerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_NXBLACKBOXPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CNX_BBFilterManager		*m_pPlayer;
	CExplorerMap			m_ExplorerMap;
	CLineChartCtrl			m_LineChart;
	CZoomDlg				*m_pZoomDlg;
	
	// For Player
	CString		m_StrPath;
	CStatic		m_StcVideo0;
	CStatic		m_StcVideo1;
	CSliderCtrl	m_SldSeek;
	CSliderCtrl	m_SdlVolume;
	CListCtrl	m_ListFile;

	void		Initialize(void);

	int			m_nPlayIndex;
	bool		GetFileList(CString strDrive);
	bool		OpenFile( CString strFileName );
	bool		m_bPlay;

	void		UpdateProgress( void );
	void		UpdateDuration( unsigned long current, unsigned long duration );

	// For UserData
	CList<CNX_UserData, CNX_UserData> m_UserData;
	CStatic		m_StcLatitude;
	CStatic		m_StcLongitude;
	CStatic		m_StcVelocity;
	CStatic		m_StcGravityX;
	CStatic		m_StcGravityY;
	CStatic		m_StcGravityZ;
	CStatic		m_StcDuration;
	
	int			m_nDrawMapSampling;
	bool		m_bOpenMap;

	bool		UserDataParser( CString strFileName, int FileType = CNX_BBFilterManager::FILE_TYPE_MP4 );
	bool		MP4UserDataParser( CString strFileName );

	static void TSUserDataCallback(void *privateData, unsigned char *pData, int size, __int64 timeStamp);
	bool		TSUserDataParser( CString strFileName );
	void		UpdateUserData( LONGLONG timeStamp );
	void		ClearUserData( void );
	
	int			CheckInternetConnection( void );
	void		ExecuteScript(CString &strFunction);
	bool		LoadMap( void );
	void		UpdateWebBrowser( char *latitude, char *logitude );
	void		RefreshMap(CString latitue, CString longitude);

	//	GraphNotify Message
	enum	{ SEEK_TIME=5000, WMU_GRAPHNOTIFY=(WM_USER+1024) };
	LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);

	// Message Handler
	afx_msg void OnNMDblclkListFile(NMHDR *pNMHDR, LRESULT *pResult);		//	List Control

	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnForward();
	afx_msg void OnBnClickedBtnBackward();
	afx_msg void OnBnClickedBtnNext();
	afx_msg void OnBnClickedBtnPrevious();
	afx_msg void OnBnClickedBtnMinimize();
	afx_msg void OnBnClickedBtnMaximize();
	afx_msg void OnBnClickedBtnZoom();

	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
