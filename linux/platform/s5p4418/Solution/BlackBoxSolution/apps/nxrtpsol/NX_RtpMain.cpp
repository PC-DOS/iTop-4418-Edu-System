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
#include <stdbool.h>

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <INX_RtpManager.h>

static INX_RtpManager *pRtpManager = NULL;

// Signal Handler
static void signal_handler(int sig)
{
	printf("Aborted by signal %s (%d)...\n", (char*)strsignal(sig), sig);

	switch(sig)
	{   
		case SIGINT :
			printf("SIGINT..\n");   break;
		case SIGTERM :
			printf("SIGTERM..\n");  break;
		case SIGABRT :
			printf("SIGABRT..\n");  break;
		default :
			break;
	}   
	
	if( pRtpManager )
	{
		pRtpManager->Stop();
		pRtpManager->Deinit();
		ReleaseRtpHandle( pRtpManager );
	}

	exit(EXIT_FAILURE);
}

static void register_signal(void)
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

int main(void)
{
	NX_RTP_MGR_CONFIG 	rtpConfig;

	memset( &rtpConfig, 0x00, sizeof(NX_RTP_MGR_CONFIG) );
	rtpConfig.nPort				= 3;
	rtpConfig.nWidth			= 1024;
	rtpConfig.nHeight			= 768;
	rtpConfig.nFps				= 30;
	rtpConfig.nBitrate			= 5000000;
	rtpConfig.nDspWidth			= 720;
	rtpConfig.nDspHeight		= 480;

	pRtpManager = GetRtpHandle();

	printf("############################## STARTING APPLICATION ##############################\n");
	printf("HLS Test Application\n");
	printf("Build Time : %s, %s\n", __TIME__, __DATE__);
	printf("Author     : Sung-won Jo\n");
	printf("Mail       : doriya@nexell.co.kr\n");
	printf("COPYRIGHT@2013 NEXELL CO. ALL RIGHT RESERVED.\n");
	printf("##################################################################################\n");

	register_signal();
	
	pRtpManager->Init( &rtpConfig );
	pRtpManager->Start();

	while(1)
	{
		usleep(100000);
	}

	if( pRtpManager )
	{
		pRtpManager->Stop();
		pRtpManager->Deinit();
		ReleaseRtpHandle( pRtpManager );
	}

	return 0;
}
