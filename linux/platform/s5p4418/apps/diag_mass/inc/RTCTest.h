#ifndef __RTCTEST_H__
#define __RTCTEST_H__

#include <pthread.h>

class CRTCTest
{
public:
	CRTCTest();
	~CRTCTest();

	bool Start();
	bool Stop();
	bool GetTime( int &year, int &month, int &day, int &hour, int &min, int &sec );

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CRTCTest *pObj = (CRTCTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

private:
	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;

	int				m_Year;
	int				m_Month;
	int				m_Day;
	int				m_Hour;
	int				m_Min;
	int				m_Sec;
};

#endif // !__RTCTEST_H__
