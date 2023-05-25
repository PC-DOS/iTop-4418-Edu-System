#ifndef __HDMITEST_H__
#define __HDMITEST_H__

#include <pthread.h>

class CHDMITest
{
public:
	CHDMITest();
	~CHDMITest();

	bool Start();
	bool Stop();
	bool IsConnected(){return m_bIsConnected;};

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CHDMITest *pObj = (CHDMITest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();
	
	int HDMIOpen();
	void HDMIClose();
	int HDMIDisplay();

private:
	bool			m_bIsConnected;

	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;

	int ion_fd;
	bool started_out;
	int out_q_count;
	int out_index;
};

#endif // !__HDMITEST_H__
