#ifndef __AUDIOPLAYTEST_H__
#define __AUDIOPLAYTEST_H__

#include <pthread.h>

class CAudioPlayTest
{
public:
	CAudioPlayTest();
	~CAudioPlayTest();

	bool Start();
	bool Stop();

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CAudioPlayTest *pObj = (CAudioPlayTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

private:
	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;
};

#endif // !__AUDIOPLAYTEST_H__
