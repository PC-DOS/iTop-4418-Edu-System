#ifndef __SENSORTEST_H__
#define __SENSORTEST_H__

#include <pthread.h>

class CSensorTest
{
public:
	CSensorTest();
	~CSensorTest();

	bool Start();
	bool Stop();
	bool ReadValue( float &x, float &y, float &z );

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CSensorTest *pObj = (CSensorTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

private:
	float			m_X;
	float			m_Y;
	float			m_Z;

	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;
};

#endif // !__SENSORTEST_H__
