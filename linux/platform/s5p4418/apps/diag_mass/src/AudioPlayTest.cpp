#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <AudioPlayTest.h>
#include <utils.h>
#include <diag_config.h>

CAudioPlayTest::CAudioPlayTest()
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CAudioPlayTest::~CAudioPlayTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CAudioPlayTest::Start()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		return false;
	}

	m_bRunning = true;
	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bRunning = false;
		return false;
	}
	return true;
}

bool CAudioPlayTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}


void CAudioPlayTest::ThreadProc()
{
	NX_DiagConfig *diagCfg = new NX_DiagConfig();

	char fileName[256];
	memset(fileName, 0, sizeof(fileName));
	snprintf(fileName, sizeof(fileName), "aplay %s/test.wav", diagCfg->GetDiagDataPath());

	while(m_bRunning)
	{
		system(fileName);
		usleep(500000);	//	500 msec
    }
	delete diagCfg;
}
