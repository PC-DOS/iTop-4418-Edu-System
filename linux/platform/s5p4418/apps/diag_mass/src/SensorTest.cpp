#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <SensorTest.h>
#include <utils.h>

CSensorTest::CSensorTest()
	: m_X(0.)
	, m_Y(0.)
	, m_Z(0.)
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CSensorTest::~CSensorTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CSensorTest::Start()
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

bool CSensorTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}


bool CSensorTest::ReadValue( float &x, float &y, float &z )
{
	CNX_AutoLock lock(&m_hMutex);
	x = m_X;
	y = m_Y;
	z = m_Z;
	return true;
}


void CSensorTest::ThreadProc()
{
	srand( 100 );
	while(m_bRunning)
	{
		//	Update Sensor Value
		{
			CNX_AutoLock lock(&m_hMutex);
			int x = (rand() % 256) - 128;
			int y = (rand() % 256) - 128;
			int z = (rand() % 256) - 128;
			m_X = (float)x / 64.;
			m_Y = (float)y / 64.;
			m_Z = (float)z / 64.;
		}
		usleep(500000);	//	500 msec
    }
}
