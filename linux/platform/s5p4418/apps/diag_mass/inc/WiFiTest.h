#ifndef __WiFiTEST_H__
#define __WiFiTEST_H__

#include <pthread.h>

typedef struct WIFI_AP_INFO{
	char *apName;
	int bssid[6];
	int  strength;
} WIFI_AP_INFO;

class CWiFiTest
{
public:
	CWiFiTest();
	~CWiFiTest();

	bool Start();
	bool Stop();

	int	GetWiFiApInfo( WIFI_AP_INFO *apInfo );

	enum{
		WIFI_MAX_ACCESS_POINT = 64,
		WIFI_MAX_AP_NAME_STR  = 128
	};

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CWiFiTest *pObj = (CWiFiTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

private:
	int				m_NumAPInfo;
	WIFI_AP_INFO	m_APInfo[WIFI_MAX_ACCESS_POINT];
	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;
};

#endif // !__WiFiTEST_H__
