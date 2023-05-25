#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>		//	ioctl()
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include <RTCTest.h>
#include <utils.h>

#define	RTC_DEV_NAME 		"/dev/rtc0"

CRTCTest::CRTCTest()
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CRTCTest::~CRTCTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CRTCTest::Start()
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

bool CRTCTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}

bool CRTCTest::GetTime( int &year, int &month, int &day, int &hour, int &min, int &sec )
{
	CNX_AutoLock lock(&m_hMutex) ;
	year  = m_Year;
	month = m_Month;
	day   = m_Day;
	hour  = m_Hour;
	min   = m_Min;
	sec   = m_Sec;
	return true;
}


void CRTCTest::ThreadProc()
{
	int fd, ret;
	const char * device = RTC_DEV_NAME;
	struct rtc_time tm;

	/* open rtc device */
	fd = open(device, O_RDONLY);
	if (0 > fd) {
		fprintf(stderr, "fail open %s\n\n", device);
		perror(device);
	}

	while(m_bRunning)
	{
		//	Read the RTC time/date
		ret = ioctl(fd,RTC_RD_TIME,&tm);
		if(0 > ret) {
			printf("Failed : ioctl( RTC_RD_TIME ) Failed ~~~~~~~~\n");
			break;
		}
		{
			CNX_AutoLock lock(&m_hMutex) ;
			m_Year  = tm.tm_year + 1900;
			m_Month = tm.tm_mon + 1;
			m_Day   = tm.tm_mday;
			m_Hour  = tm.tm_hour;
			m_Min   = tm.tm_min;
			m_Sec   = tm.tm_sec;
		}
		//printf("Time : %02d-%02d-%04d, %02d:%02d:%02d \n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		usleep(500000);	//	500 msec
    }
	close(fd);
}
