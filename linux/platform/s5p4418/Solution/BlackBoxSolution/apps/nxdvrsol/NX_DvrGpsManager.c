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
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <sys/poll.h>

#include "NX_DvrGpsSampleData.h"
#include "NX_DvrGpsManager.h"

#define	GPS_DEV_NAME		"/dev/ttyAMA2"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private Function
//
static void parser_gprmc( GPS_MANAGER_HANDLE hManager, char *pBuf )
{
	char buf[16] = "GPRMC";
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	NX_NmeaParser(hManager->hParser, pBuf, 1024);
	NX_NmeaType(hManager->hParser, buf, &hManager->gpsData, sizeof(struct nmea_gprmc));
	NX_NmeaGprmc(hManager->hParser, &hManager->gpsData);
	pthread_mutex_unlock( &hManager->hLock  );
}

#ifndef GPS_DATA_SAMPLE
void *DvrGpsManagerThread( void *arg )
{
	int32_t hPoll = 0;
	char buf[1024], buf_gprmc[16];

	struct termios  newtio, oldtio;
	struct pollfd   pollEvent;

	GPS_MANAGER_HANDLE hManager = (GPS_MANAGER_HANDLE)arg;

	tcgetattr( hManager->hDevice, &oldtio );
	memset( &newtio, 0x00, sizeof(newtio) );

	// Serial Port Setting (Canonical Mode)
	newtio.c_cflag      = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag      = IGNPAR | ICRNL;

	newtio.c_oflag      = 0;
	newtio.c_lflag      = ICANON;

	tcflush( hManager->hDevice, TCIFLUSH );
	tcsetattr( hManager->hDevice, TCSANOW, &newtio );

	pollEvent.fd        = hManager->hDevice;
	pollEvent.events    = POLLIN | POLLERR;
	pollEvent.revents   = 0;

    while( hManager->bThreadRun )
    {
        hPoll = poll( (struct pollfd*)&pollEvent, 1, 3000);

        if( hPoll < 0 ) {
            printf("%s(): poller error.\n", __func__);
            goto ERROR;
        }
        else if( hPoll > 0 ) {
            memset(buf, 0x00, sizeof(buf));
            if( 0 > read( hManager->hDevice, buf, sizeof(buf) ) ) {
                printf("%s(): gps data read failed.\n", __func__);
                goto ERROR;
            }
            else {
                memset(buf_gprmc, 0x00, sizeof(buf_gprmc));
                sscanf(buf, "%6s", buf_gprmc);

                if( !strcmp(buf_gprmc, "$GPRMC") ) {
					parser_gprmc( hManager, buf );
                }
            }
        }
        else {
			//printf("time-out!!\n");
        }
    }

ERROR :
    tcsetattr( hManager->hDevice, TCSANOW, &oldtio );

	return (void*)0xDEADDEAD;
}
#else
void *DvrGpsManagerThread( void *arg )
{
	int32_t buf_cnt = 0;
	GPS_MANAGER_HANDLE hManager = (GPS_MANAGER_HANDLE)arg;

	while( hManager->bThreadRun )
    {
		parser_gprmc( (GPS_MANAGER_HANDLE)arg, gps_test_data[buf_cnt] );
		buf_cnt = (buf_cnt + 1) % (sizeof(gps_test_data) / sizeof(gps_test_data[0]));
		usleep(500000);
	}

	return (void*)0xDEADDEAD;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface Function
//
GPS_MANAGER_HANDLE DvrGpsManagerInit( void )
{
	GPS_MANAGER_HANDLE hManager = (GPS_MANAGER_HANDLE)malloc( sizeof(GPS_MANAGER_INFO) );
	memset( hManager, 0x00, sizeof(GPS_MANAGER_INFO) );

	hManager->hThread		= 0;
	hManager->bThreadRun	= false;

	if( 0 > (hManager->hDevice = open(GPS_DEV_NAME, O_RDWR | O_NOCTTY)) ) { 
		printf("%s(): device open failed. ( %s )\n", __func__, GPS_DEV_NAME);
		goto ERROR; 
	}   

	if( 0 > (hManager->hParser = NX_NmeaInit()) ) { 
		printf("%s(): nmea initialize failed.\n", __func__);
		goto ERROR;
	}   

	pthread_mutex_init( &hManager->hLock, NULL );

	return hManager;

ERROR :
	if( hManager->hParser ) NX_NmeaExit( hManager->hParser );
	if( hManager->hDevice ) close( hManager->hDevice );

	return NULL;
}

void DvrGpsManagerDeinit( GPS_MANAGER_HANDLE hManager )
{
	assert( hManager );
	pthread_mutex_destroy( &hManager->hLock );

	if( hManager->hParser ) NX_NmeaExit( hManager->hParser );
	if( hManager->hDevice ) close( hManager->hDevice );
	
	if( hManager ) free( hManager );
}

int32_t DvrGpsManagerStart( GPS_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( hManager->bThreadRun ) {
		printf("%s(): Fail, Already running.\n", __func__);
		return -1;
	}
	
	hManager->bThreadRun = true;
	if( 0 > pthread_create( &hManager->hThread, NULL, &DvrGpsManagerThread, (void*)hManager) ) {
		printf("%s(): Fail, Create Thread.\n", __func__);
		return -1;
	}
	
	return 0;
}

int32_t DvrGpsManagerStop( GPS_MANAGER_HANDLE hManager )
{
	assert( hManager );
	if( !hManager->bThreadRun ) {
		printf("%s(): Fail, Already stopping.\n", __func__);
		return -1;
	}

	hManager->bThreadRun = false;
	pthread_join( hManager->hThread, NULL );

	return 0;
}

int32_t DvrGpsManagerRegCmd( GPS_MANAGER_HANDLE hManager, CMD_QUEUE_HANDLE hCmd )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	hManager->hCmd = hCmd;
	pthread_mutex_unlock( &hManager->hLock );

	return 0;
}

int32_t DvrGpsGetData( GPS_MANAGER_HANDLE hManager, struct nmea_gprmc *pGprmc )
{
	assert( hManager );
	pthread_mutex_lock( &hManager->hLock );
	
	pGprmc->dat_valid = hManager->gpsData.dat_valid;
	if( pGprmc->dat_valid == 'A' ) {
		pGprmc->hour			= hManager->gpsData.hour;
		pGprmc->minute			= hManager->gpsData.minute;
		pGprmc->second			= hManager->gpsData.second;
		pGprmc->latitude		= hManager->gpsData.latitude;
		pGprmc->longitude		= hManager->gpsData.longitude;
		pGprmc->ground_speed	= hManager->gpsData.ground_speed;
		pGprmc->course			= hManager->gpsData.course;
		pGprmc->day				= hManager->gpsData.day;
		pGprmc->month			= hManager->gpsData.month;
		pGprmc->year			= hManager->gpsData.year;
		pGprmc->mag_var			= hManager->gpsData.mag_var;
		pGprmc->count			= hManager->gpsData.count;
	}
	else {
		pGprmc->hour			= 0;
		pGprmc->minute			= 0;
		pGprmc->second			= 0;
		pGprmc->latitude		= 0.0;
		pGprmc->longitude		= 0.0;
		pGprmc->ground_speed	= 0.0;
		pGprmc->course			= 0.0;
		pGprmc->day				= 0;
		pGprmc->month			= 0;
		pGprmc->year			= 0;
		pGprmc->mag_var			= 0.0;
		pGprmc->count			= 0;
	}
	pthread_mutex_unlock( &hManager->hLock );

	return (pGprmc->dat_valid == 'A') ? true : false;
}
