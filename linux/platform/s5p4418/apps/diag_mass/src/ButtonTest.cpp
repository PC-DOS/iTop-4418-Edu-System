#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

#include <linux/input.h>

#include <ButtonTest.h>
#include <utils.h>

#define	EVENT_DEV_NAME	"/dev/input/event0"


CButtonTest::CButtonTest()
	: m_BtnValue(BTN_NONE)
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CButtonTest::~CButtonTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CButtonTest::Start()
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

bool CButtonTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}

void CButtonTest::GetValue( int &buttonValue, char *btnStr )
{
	CNX_AutoLock lock(&m_hMutex);
	buttonValue = m_BtnValue;
	if( btnStr )
	{
		switch ( buttonValue )
		{
		case BTN_VOLUME_UP:
			strcpy(btnStr, "VOLUME UP");
			break;
		case BTN_VOLUME_DOWN:
			strcpy(btnStr, "VOLUME DOWN");
			break;
		case BTN_POWER:
			strcpy(btnStr, "POWER");
			break;
		case BTN_NONE:
			strcpy(btnStr, "NONE");
			break;
		default:
			strcpy(btnStr, "Unknown");
			break;
		}
	}
}

void CButtonTest::ThreadProc()
{
	int    fd;
    struct input_event evtBuf;
    size_t length;

    fd_set fdset;
	struct timeval timeout;

	fd = open(EVENT_DEV_NAME, O_RDONLY);
    if (0 > fd) {
		printf("%s: device open failed!\n", __func__);
        return;
    }

	while(m_bRunning)
	{
		FD_ZERO(&fdset);
   		FD_SET(fd, &fdset);

   		timeout.tv_sec  = 1000;
   		timeout.tv_usec = 0;
        if (select(fd + 1, &fdset, NULL, NULL, &timeout) > 0) 
		{
            if (FD_ISSET(fd, &fdset))
			{
                length = read(fd, &evtBuf, sizeof(struct input_event) );
                if (0 > (int)length)
				{
                    printf("%s(): read failed.\n", __func__);
                    close(fd);
					break;
                }
				//printf("1, length=%d, Type = %d, Code = %d, Value = %d\n", length, evtBuf.type, evtBuf.code, evtBuf.value);
				if( !evtBuf.value && evtBuf.code != 0 )
				{
					CNX_AutoLock lock(&m_hMutex);
					switch ( evtBuf.code )
					{
					case BTN_VOLUME_UP:
						m_BtnValue = BTN_VOLUME_UP;
						break;
					case BTN_VOLUME_DOWN:
						m_BtnValue = BTN_VOLUME_DOWN;
						break;
					case BTN_POWER:
						m_BtnValue = BTN_POWER;
						break;
					default:
						m_BtnValue = evtBuf.code;
						break;
					}
				}
            }
        }
		else
		{
		}
    }
   	FD_CLR(fd, &fdset);

    close(fd);
}
