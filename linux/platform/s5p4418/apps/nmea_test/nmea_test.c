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
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/poll.h>

#include <nx_nmea_parser.h>
#include "gps_data.h"

#define GPS_DEV_NAME		"/dev/ttyAMA2"
#define MAX_BUFFER_SIZE		1024

#define GPS_DATA_SAMPLE

int parser_gprmc(int hParser, char *pBuf, struct nmea_gprmc *pGprmc)
{
	char buf[16] = "GPRMC";

	NX_NmeaParser(hParser, pBuf, MAX_BUFFER_SIZE);
	NX_NmeaType(hParser, buf, pGprmc, sizeof(struct nmea_gprmc));
	NX_NmeaGprmc(hParser, pGprmc);

	if(pGprmc->dat_valid == 'V') return -1;
	
	printf("[%04d-%02d-%02d %02d:%02d:%02d] latitude = %f, longitude = %f, speed = %d\n", 
		pGprmc->year, pGprmc->month, pGprmc->day, pGprmc->hour, pGprmc->minute, pGprmc->second,
		pGprmc->latitude, pGprmc->longitude, (int)(pGprmc->ground_speed * 1.852)
	);

	return 0;
}

#ifndef GPS_DATA_SAMPLE
int main(void)
{
	int hGps, hParser, hPoll;
	char buf[1024], buf_gprmc[16];

	struct termios	newtio, oldtio;
	struct pollfd	pollEvent;

	struct nmea_gprmc gprmc;

	if( 0 > (hGps = open(GPS_DEV_NAME, O_RDWR | O_NOCTTY)) ) {
		printf("device open failed. (%s) \n", GPS_DEV_NAME);
		return -1;
	}

	tcgetattr(hGps, &oldtio);
	memset( &newtio, 0x00, sizeof(newtio) );
	
	// Serial Port Setting (Canonical Mode)
	newtio.c_cflag      = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag      = IGNPAR | ICRNL;
	
	newtio.c_oflag      = 0;
	newtio.c_lflag      = ICANON;
	
	tcflush( hGps, TCIFLUSH );
	tcsetattr( hGps, TCSANOW, &newtio );
	
	if( 0 > (hParser = NX_NmeaInit()) ) {
		printf("nmea initialize failed. \n");
		goto ERROR;
	}
	
	pollEvent.fd		= hGps;
	pollEvent.events	= POLLIN | POLLERR;
	pollEvent.revents	= 0;

	while(1)
	{
		hPoll = poll( (struct pollfd*)&pollEvent, 1, 3000);

		if( hPoll < 0 ) {
			printf("poller error.\n");
			goto ERROR;
		}
		else if( hPoll > 0 ) {
			memset(buf, 0x00, sizeof(buf));
			if( 0 > read(hGps, buf, sizeof(buf)) ) {
				printf("gps data read failed.\n");
				goto ERROR;
			}
			else {
				memset(buf_gprmc, 0x00, sizeof(buf_gprmc));
				sscanf(buf, "%6s", buf_gprmc);

				if( !strcmp(buf_gprmc, "$GPRMC") ) {
					printf("%s", buf);
					if( !parser_gprmc( hParser, buf, &gprmc) ) {
					}
				}
			}
		}
		else {
			printf("time-out!!\n");
		}
	}

	tcsetattr(hGps, TCSANOW, &oldtio );
	
	if(hGps) close(hGps);
	if(hParser)	NX_NmeaExit(hParser);

	return 0;

ERROR :
	if(hGps)	close(hGps);
	if(hParser)	NX_NmeaExit(hParser);

	return -1;

}
#else
int main(void)
{
	int hGps, hParser;
	int buf_cnt = 0;

	struct termios	newtio, oldtio;

	struct nmea_gprmc gprmc;

	if( 0 > (hGps = open(GPS_DEV_NAME, O_RDWR | O_NOCTTY)) ) {
		printf("device open failed. (%s) \n", GPS_DEV_NAME);
		return -1;
	}

	tcgetattr(hGps, &oldtio);
	memset( &newtio, 0x00, sizeof(newtio) );
	
	// Serial Port Setting (Canonical Mode)
	newtio.c_cflag      = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag      = IGNPAR | ICRNL;
	
	newtio.c_oflag      = 0;
	newtio.c_lflag      = ICANON;
	
	tcflush( hGps, TCIFLUSH );
	tcsetattr( hGps, TCSANOW, &newtio );
	
	if( 0 > (hParser = NX_NmeaInit()) ) {
		printf("nmea initialize failed. \n");
		goto ERROR;
	}
	while(1)
	{
		if( !parser_gprmc( hParser, gps_test_data[buf_cnt], &gprmc) ) {
		}

		buf_cnt = (buf_cnt + 1) % (sizeof(gps_test_data) / sizeof(gps_test_data[0]));
		if( !buf_cnt ) break;
	}

	tcsetattr(hGps, TCSANOW, &oldtio );

	if(hGps) close(hGps);
	if( hParser )	NX_NmeaExit( hParser );

	return 0;

ERROR :
	if( hGps )		close( hGps );
	if( hParser )	NX_NmeaExit( hParser );

	return -1;
}
#endif
