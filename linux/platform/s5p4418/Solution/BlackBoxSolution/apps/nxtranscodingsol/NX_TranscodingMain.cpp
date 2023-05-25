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

#include <INX_TranscodingManager.h>
#include <nx_dsp.h>

static INX_TranscodingManager *pTranscodingManager = NULL;

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
	
	if( pTranscodingManager )
	{
		pTranscodingManager->Stop();
		pTranscodingManager->Deinit();
		ReleaseTranscodingHandle( pTranscodingManager );
	}

	exit(EXIT_FAILURE);
}

static void register_signal(void)
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

static void Usage( void )
{
	printf("Nexell Transcoding Test Application. ( Filter Base )\n");
	printf("Decoding -> Preview -> Encoding(H264) -> FileWriting(MP4)\n");
	printf("Usage :\n");
	printf("   -h              : Show usage.\n");
	printf("   -i [filename]   : Input filename. (mandatory)\n");
	printf("   -o [filename]   : Output filename.\n");
	printf("   -f [framerate]  : Encoding framereate. default input-stream fps.\n");
	printf("   -b [bitrate]    : Encoding bitrate(Mbps). default 10M.\n");
}

int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt;
	
	char *pInFileName = NULL, *pOutFileName = NULL;
	int32_t fps = 0;
	int32_t bitrate = 10000000;

	while( -1 != (opt=getopt(argc, argv, "hi:o:f:b:")) )
	{
		switch( opt ) 
		{
			case 'h':
				Usage();
				goto END;
			case 'i':
				pInFileName = strdup( optarg );
				break;
			case 'o':
				pOutFileName = strdup( optarg );
				break;
			case 'f':
				fps = atoi( optarg );
				break;
			case 'b':
				bitrate = atoi( optarg ) * 1000000;
				break;
			default :
				break;
		}
	}

	if( pInFileName == NULL ) {
		printf("Error. Input file name is need. (-i [filename])\n");
		goto END;
	}

	NX_TRANSCODING_MGR_CONFIG transcodingConfig;

	memset( &transcodingConfig, 0x00, sizeof(NX_TRANSCODING_MGR_CONFIG) );
	
	transcodingConfig.pInFileName 	= (uint8_t*)pInFileName;
	transcodingConfig.pOutFileName	= (uint8_t*)pOutFileName;
	transcodingConfig.nEncFps		= fps;
	transcodingConfig.nEncBitrate 	= bitrate;

	pTranscodingManager = GetTranscodingHandle();

	printf("############################## STARTING APPLICATION ##############################\n");
	printf("Transcoding Test Application\n");
	printf("Build Time : %s, %s\n", __TIME__, __DATE__);
	printf("Author     : Sung-won Jo\n");
	printf("Mail       : doriya@nexell.co.kr\n");
	printf("COPYRIGHT@2014 NEXELL CO. ALL RIGHT RESERVED.\n");
	printf("##################################################################################\n");

	register_signal();
	
	NX_DspVideoSetPriority( DISPLAY_MODULE_MLC0, 0 );
	
	pTranscodingManager->Init( &transcodingConfig );
	pTranscodingManager->Start();

	while(1)
	{
		usleep(100000);
	}

	if( pTranscodingManager )
	{
		pTranscodingManager->Stop();
		pTranscodingManager->Deinit();
		ReleaseTranscodingHandle( pTranscodingManager );
	}

END:
	return 0;
}
