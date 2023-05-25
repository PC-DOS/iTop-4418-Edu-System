#ifndef __TOP_WINDOW_H__
#define	__TOP_WINDOW_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <pthread.h>

#include <nx_diag_type.h>
#include <diag_config.h>

#include <base_app_window.h>
#include <InfoWindow.h>


#include <AudioPlayTest.h>
#include <AudioRecTest.h>
#include <CameraTest.h>
#include <HDMITest.h>
#include <RTCTest.h>
#include <WiFiTest.h>
#include <SensorTest.h>
#include <ButtonTest.h>
#include <CpuRamTest.h>


class CDiagnosticWnd: public BaseAppWnd
{
public:
	CDiagnosticWnd();
	~CDiagnosticWnd();
	virtual void Initialize(){};
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	enum{
		COLOR_WHITE,
		COLOR_BLACK,
		COLOR_LIGHT_GRAY,
		COLOR_GRAY,
		COLOR_HEAVY_GRAY,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_BLUE,

		//	Special Color
		COLOR_COLOR_KEY,

		COLOR_MAX
	};

private:
	void	InitColor();

	void	UpdateAutomaticWnd( bool bAll = true );
	void	UpdateManualWnd( bool bAll = true );
	void	UpdateLCDWnd( bool bAll = true );
	void	UpdateMICWnd( bool bAll = true );
	void	UpdateCameraWnd( bool bAll = true );
	void	UpdateWiFiWnd( bool bAll = true );
	void	DrawTouchLine( bool continued );

	void	Start();
	void	Stop();

	static void *UpdateThreadStub( void *arg )
	{
		CDiagnosticWnd *pObj = (CDiagnosticWnd *)arg;
		pObj->UpdateThreadProc();
		return (void*)0xDEADBEEF;
	}

	void	UpdateThreadProc();

private:
	CAudioPlayTest	*m_pAudPlayTest;
	CAudioRecTest	*m_pAudRecTest;
	CCameraTest		*m_pCamTest;
	CHDMITest		*m_pHDMITest;
	CRTCTest		*m_pRTCTest;
	CWiFiTest		*m_pWiFiTest;
	CSensorTest		*m_pSensorTest;
	CButtonTest		*m_pButtonTest;

	CInformationWindow *m_pAutoResult;
	CInformationWindow *m_pManualResult;
	CInformationWindow *m_pWiFiResult;

	NXDiagButton	*m_pBtnCamToggle;
	NXDiagButton	*m_pBtnTouchMode;

	unsigned int	m_Colors[COLOR_MAX];
	SDL_Color		m_SDLColors[COLOR_MAX];

	//	Update Thread
	pthread_t		m_hUpdateThread;

	//	for Touch Test
	TouchPoint		m_OldPoint;
	TouchPoint		m_NewPoint;
	bool			m_bLineContinued;

	//	Button Test
	int				m_BtnValue;
	char			m_BtnString[16];

	//	WiFi Info
	int				m_NumAPInfo;
	WIFI_AP_INFO	m_APInfo[CWiFiTest::WIFI_MAX_ACCESS_POINT];


	//	Status Informations
private:
	int				m_Year, m_Month, m_Day, m_Hour, m_Min, m_Sec;
	bool			m_bHDMIIsConnected;

	pthread_mutex_t	m_hUpdateMutex;
};


#endif //__TOP_WINDOW_H__
