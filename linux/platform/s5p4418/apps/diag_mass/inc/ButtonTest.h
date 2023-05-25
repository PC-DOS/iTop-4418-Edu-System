#ifndef __BUTTONTEST_H__
#define __BUTTONTEST_H__

#include <pthread.h>

class CButtonTest
{
public:
	CButtonTest();
	~CButtonTest();

	bool Start();
	bool Stop();

	void GetValue( int &buttonValue, char *btnStr );

	enum{
		BTN_NONE        =  -1,
		BTN_VOLUME_UP   = 115,
		BTN_VOLUME_DOWN = 114,
		BTN_POWER       = 116,
	};

	//	private functions
private:
	static void *ThreadStub(void *arg)
	{
		CButtonTest *pObj = (CButtonTest *)arg;
		pObj->ThreadProc();
		return (void*)0xDEADBEEF;
	}
	void ThreadProc();

private:
	bool			m_bRunning;
	pthread_t		m_hThread;
	pthread_mutex_t	m_hMutex;
	int				m_BtnValue;
};

#endif // !__BUTTONTEST_H__
