//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		: 
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {
	WIFI_MODE_STATION,
	WIFI_MODE_SOFTAP,
} WIFI_MODE;

int32_t	NX_AppIsRunning( const char *appName );
int32_t NX_KillApp( const char *appName );

typedef struct WIFI_WPSStatus{
	int32_t		id;
	uint32_t	bssid[6];
	uint32_t	ip_addr[4];
	char		ssid[64];
	char		pairwise_cipher[12];
	char		group_cipher[12];
	char		key_mgmt[12];
	char		wpa_state[12];
} WIFI_WPSStatus;

int32_t UpdateWiFiStatusInfo( WIFI_WPSStatus *status )
{
	FILE *fd;
	char buf[512];

	memset( status, 0, sizeof(WIFI_WPSStatus) );

	if( access("/var/run/wpa_supplicant", F_OK ) )
		return -1;

	fd = popen( "wpa_cli -iwlan0 -p/var/run/wpa_supplicant status", "r" );

	if( fd == NULL )
		return -1;

	if( fd )
	{
		while( fgets(buf, 512, fd ) != NULL )
		{
			if( 0 == strncmp( "bssid", buf, 5 ) )
			{
				sscanf( buf, "bssid=%02x:%02x:%02x:%02x:%02x:%02x", 
					&status->bssid[0], &status->bssid[1], &status->bssid[2], 
					&status->bssid[3], &status->bssid[4], &status->bssid[5] );
			}
			else if( 0 == strncmp( "ip_address", buf, 10 ) )
			{
				sscanf( buf, "ip_address=%d.%d.%d.%d",
					&status->ip_addr[0], &status->ip_addr[1], 
					&status->ip_addr[2], &status->ip_addr[3] );
			}
			else if( 0 == strncmp( "ssid", buf, 4 ) )
			{
				sscanf( buf, "ssid=%s", status->ssid );
			}
			else if( 0 == strncmp( "pairwise_cipher", buf, 15 ) )
			{
				sscanf( buf, "pairwise_cipher=%s", status->pairwise_cipher );
			}
			else if( 0 == strncmp( "group_cipher", buf, 12 ) )
			{
				sscanf( buf, "group_cipher=%s", status->group_cipher );
			}
			else if( 0 == strncmp( "key_mgmt", buf, 8 ) )
			{
				sscanf( buf, "key_mgmt=%s", status->key_mgmt );
			}
			else if( 0 == strncmp( "wpa_state", buf, 9 ) )
			{
				sscanf( buf, "wpa_state=%s", status->wpa_state );
			}
			else if( 0 == strncmp( "id", buf, 2 ) )
			{
				sscanf( buf, "id=%d", &status->id );
			}
		}
		fclose(fd);
	}
#ifdef _DEBUG
	printf( "[id]              = %d\n", status->id );
	printf( "[bssid]           = %02x:%02x:%02x:%02x:%02x:%02x \n",
		status->bssid[0], status->bssid[1], status->bssid[2], 
		status->bssid[3], status->bssid[4], status->bssid[5] );
	printf( "[ssid]            = %s\n", status->ssid );
	printf( "[pairwise_cipher] = %s\n", status->pairwise_cipher );
	printf( "[group_cipher]    = %s\n", status->group_cipher );
	printf( "[key_mgmt]        = %s\n", status->key_mgmt );
	printf( "[wpa_state]       = %s\n", status->wpa_state );
	printf( "[ip_address]      = %d.%d.%d.%d\n", status->ip_addr[0], status->ip_addr[1], status->ip_addr[2], status->ip_addr[3] );
#endif
	return 0;
}


int32_t main( int32_t argc, char *argv[] )
{
	WIFI_WPSStatus status;
	int32_t dhcpTryCnt = 0;

	WIFI_MODE mode = WIFI_MODE_SOFTAP;
	int32_t opt;

	while( -1 != (opt = getopt(argc, argv, "t:")) )
	{
		switch( opt )
		{
		case 't':
			if( !strncmp( "station", optarg, 9 ) )
			{
				mode = WIFI_MODE_STATION;
			}
			else if( !strncmp( "softap", optarg, 6) ) 
			{
				mode = WIFI_MODE_SOFTAP;
			}
			break;
		}
	}

	// Run http demon.
	if( 0 != NX_AppIsRunning("httpd") )
	{
		system("mkdir /tmp/www");
		system("ln -s /tmp/www /www");
		system("cp /root/www/index.html /www");
		system("httpd");
	}

	// Run nxdvrsol Application
	if( 0 != NX_AppIsRunning( "nxdvrsol" ) )
	{
		system( "/root/nxdvrsol -b -n &" );
	}


	if( mode == WIFI_MODE_SOFTAP )
	{
		// ifconfig wlan0 192.168.21.1 up
		// udhcpd
		// hostapd /etc/hostapd.conf -B	
		// ifconfig wlan0 192.168.21.1 up;udhcpd;hostapd /etc/hostapd.conf -B
		// ifconfig wlan0 192.168.21.1 up;udhcpd;hostapd /etc/hostapd.conf -B;mkdir /tmp/www;ln -s /tmp/www /www;cp /root/www/index.html /www;httpd;
		if( 0 != NX_AppIsRunning( "hostapd" ) )
		{
			system("ifconfig wlan0 192.168.21.1 up");
			system("udhcpd");
			system("hostapd /etc/hostapd.conf -B");
		}
		
		while(1)
		{
			usleep( 500000 );
		}		
	}
	else if( mode == WIFI_MODE_STATION ) 
	{
		while(1)
		{
			usleep( 500000 );
			
			if( 0 != UpdateWiFiStatusInfo(&status) )
			{
				printf("Cannot connect to wpa_supplication!!!\n");
				printf("Start wpa_supplication Application.\n");
				system("wpa_supplicant -i wlan0 -Dnl80211 -c /etc/wpa_supplicant.conf -B");
				continue;
			}

			if( 0 == strncmp( status.wpa_state, "COMPLETED", 9 ) )
			{
				if( status.ip_addr[0] == 0 && status.ip_addr[3] == 0 )
				{
					if( 0 != NX_AppIsRunning( "udhcpc" ) )
					{
						printf("Try DHCP Client...\n");
						system( "udhcpc -i wlan0 -b" );
					}
					dhcpTryCnt ++;

					if( dhcpTryCnt > 10 )
					{
						printf("DHCP Try Timeout( > 5sec ) : Kill udhcpc application!!!\n");
						NX_KillApp("udhcpc");
						dhcpTryCnt = 0;
					}
				}
				else
				{
					if( 0 == NX_AppIsRunning( "udhcpc" ) )
					{
						NX_KillApp( "udhcpc" );
					}
				}
			}
		}

	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
//						Linux Shell Script Tools
//

//	returns : 0(Running), -1(Not Running)
//	appName : upto 249 string.
int32_t	NX_AppIsRunning( const char *appName )
{
	FILE *fd;
	char buf[256];		//	Max App Name - 250
	int32_t pid = -1;

	memset( buf, 0, sizeof(buf) );
	sprintf( buf, "pidof %s", appName );

	fd = popen( buf, "r" );
	if( fd == NULL )
		return 0;
	memset( buf, 0, sizeof(buf) );
	while( fgets(buf, sizeof(buf), fd) != NULL )
	{
		pid = atoi( buf );
	}
	fclose( fd );
	if( pid > 1 )
		return 0;
	return -1;
}

//	returns : 0(Success), -1(Error), 1(App Not Exist)
int32_t NX_KillApp( const char *appName )
{
	char cmd[256];
	int32_t ret;
	sprintf(cmd, "kill -9 $(pidof %s)", appName );
	ret = system( cmd );
	return ret;
}

//
//
//////////////////////////////////////////////////////////////////////////////
