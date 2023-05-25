#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <WiFiTest.h>
#include <utils.h>


CWiFiTest::CWiFiTest()
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CWiFiTest::~CWiFiTest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CWiFiTest::Start()
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

bool CWiFiTest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}


void CWiFiTest::ThreadProc()
{
	system( "insmod /root/wlan.ko" );		//	Down WiFi network interface
	usleep( 1000000 );
	system( "ifconfig wlan0 up" );
	system( "wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wpa_supplicant.conf &" );
	usleep( 1000000 );

	FILE *fd;
	bool bScan = false;
	char buf[512];

	system( "wpa_cli -p/var/run/wpa_supplicant scan_interval 2" );
	while(m_bRunning)
	{
		fd = popen( "wpa_cli -p/var/run/wpa_supplicant scan", "r" );
		if( fd )
		{
			while( fgets(buf, 512, fd ) != NULL )
			{
				//printf("%s", buf);
				if( 0 == strncmp( "OK", buf, 2 ) )
				{
					//printf("Scan OK!!!\n");
					bScan = true;
					break;
				}
				else
					bScan = false;
			}
			fclose(fd);
		}

		if( bScan )
		{
			fd = popen( "wpa_cli -p/var/run/wpa_supplicant scan_result", "r" );
			if( fd )
			{
				CNX_AutoLock lock(&m_hMutex);
				m_NumAPInfo = 0;
				while( fgets(buf, 512, fd ) != NULL )
				{
					if( 0 == strncmp( buf, "bssid", 5 ) )
					{
					}
					else if( 0 == strncmp( buf, "Selected", 8 ) )
					{
					}
					else
					{
						char paraStr[8][64];
						int len = strlen(buf);
						int idx = 0;
						char *tmp;
						tmp = paraStr[idx++];
						for( int i=0 ; i<len ; i++ )
						{
							if( buf[i] == '\t' )
							{
								*tmp = 0;
								tmp = paraStr[idx++];
							}
							else if( buf[i] == '\n' )
							{
								*tmp = 0;
								break;
							}
							else
							{
								*tmp++ = buf[i];
							}
						}
//						sprintf(str, "%s    %s", paraStr[4], paraStr[2]);

						if( m_NumAPInfo < WIFI_MAX_ACCESS_POINT )
						{
							WIFI_AP_INFO *pApInfo = &m_APInfo[m_NumAPInfo++];
							sscanf( paraStr[0], "%2x:%2x:%2x:%2x:%2x:%2x\n"
								, &pApInfo->bssid[0], &pApInfo->bssid[1], &pApInfo->bssid[2]
								, &pApInfo->bssid[3], &pApInfo->bssid[4], &pApInfo->bssid[5] );
							if( pApInfo->apName )
							{
								free(pApInfo->apName);
							}
							pApInfo->apName   = strdup(paraStr[4]);
							pApInfo->strength = atoi(paraStr[2]);
						}
					}
				}
				fclose(fd);
			}
		}

		usleep(1000000);
	}
	KillAppByName("wpa_supplicant");
	system( "ifconfig wlan0 down" );
	system( "rmmod wlan" );
}

int	CWiFiTest::GetWiFiApInfo( WIFI_AP_INFO *apInfo )
{
	CNX_AutoLock lock(&m_hMutex);
	int numAp = m_NumAPInfo;

	for( int i=0 ; i<numAp ; i++ )
	{
		memcpy(&apInfo[i], &m_APInfo[i], sizeof(m_APInfo[i]));
		apInfo[i].apName   = strdup( m_APInfo[i].apName );
	}
	return numAp;
}
