//------------------------------------------------------------------------------
//
//	Copyright (C) 2014 Nexell Co. All Rights Reserved
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

#include <nx_dsp.h>
#include <INX_Mp4Manager.h>

uint32_t cbNotifier( uint32_t eventCode, uint8_t *pEventData, uint32_t dataSize );

static INX_Mp4Manager *pMp4Manager = NULL;

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
	
	if( pMp4Manager )
	{
		pMp4Manager->Stop();
		pMp4Manager->Deinit();
		ReleaseMp4ManagerHandle( pMp4Manager );
	}

	exit(EXIT_FAILURE);
}

static void register_signal(void)
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

static void shell_usage( void )
{
	printf("\n");
	//      0         1         2         3         4         5         6
	//      0123456789012345678901234567890123456789012345678901234567890123456789
	printf("----------------------------------------------------------------------\n");
	printf("                   MP4 Encoding Test Application                      \n");
	printf("----------------------------------------------------------------------\n");
	printf(" start or start [filename]     : encoding start.\n");
	printf(" stop                          : mp4 encoding stop.\n");
	printf(" capture or capture [filename] : jpeg capture.\n");
	printf(" exit                          : exit application.\n");
	printf("----------------------------------------------------------------------\n");
}

#define	SHELL_MAX_ARG	32
#define	SHELL_MAX_STR	1024
static int GetArgument( char *pSrc, char arg[][SHELL_MAX_STR] )
{
	int	i, j;

	// Reset all arguments
	for( i=0 ; i<SHELL_MAX_ARG ; i++ )
	{
		arg[i][0] = 0;
	}

	for( i=0 ; i<SHELL_MAX_ARG ; i++ )
	{
		// Remove space char
		while( *pSrc == ' ' )
			pSrc++;
		// check end of string.
		if( *pSrc == 0 || *pSrc == '\n' )
			break;

		j=0;
		while( (*pSrc != ' ') && (*pSrc != 0) && *pSrc != '\n' )
		{
			arg[i][j] = *pSrc++;
			j++;
			if( j > (SHELL_MAX_STR-1) )
				j = SHELL_MAX_STR-1;
		}
		arg[i][j] = 0;
	}
	return i;
}

static int32_t shell_main( void )
{
	static	char cmdString[SHELL_MAX_ARG * SHELL_MAX_STR];
	static 	char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	int cmdCnt;

	shell_usage();

	while( 1 )
	{
		printf(" > ");
		
		memset(cmd, 0x00, sizeof(cmd));
		fgets(cmdString, sizeof(cmdString), stdin);

		cmdCnt = GetArgument( cmdString, cmd );

		if( !strcmp( cmd[0], "exit" ) ) {
			printf("Exit.\n");
			break;
		}
		else if( !strcmp( cmd[0], "start" ) ) {
			printf("Start.\n");
			
			if( cmdCnt > 1 )
			{
				char fileName[1024];
				sprintf(fileName, "%s", cmd[1] );
				pMp4Manager->Init();
				pMp4Manager->RegisterNotifyCallback( cbNotifier );
				pMp4Manager->Start( fileName );
			}
			else
			{
				pMp4Manager->Init();
				pMp4Manager->RegisterNotifyCallback( cbNotifier );
				pMp4Manager->Start( NULL );
			}

		}
		else if( !strcmp( cmd[0], "stop") ) {
			printf("Stop.\n");
			pMp4Manager->Stop();
			pMp4Manager->Deinit();
		}
		else if( !strcmp( cmd[0], "capture") ) {
			printf("Capture.\n");
			if( cmdCnt > 1 )
			{
				char fileName[1024];
				sprintf(fileName, "%s", cmd[1] );
				pMp4Manager->Capture( fileName );
			}
			else
			{
				pMp4Manager->Capture( NULL );
			}
		}
	}

	return 0;
}

uint32_t cbNotifier( uint32_t eventCode, uint8_t *pEventData, uint32_t dataSize )
{
	switch( eventCode )
	{
	case NX_NOTIFY_FILEWRITING_DONE :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : File writing done. ( %s )\n", __func__, pEventData);
		}
		break;
	case NX_NOTIFY_JPEGWRITING_DONE :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : Capture writing done. ( %s )\n", __func__, pEventData);
		}
		break;
	case NX_NOTIFY_ERR_VIDEO_INPUT :
		break;
	case NX_NOTIFY_ERR_VIDEO_ENCODE :
		break;
	case NX_NOTIFY_ERR_OPEN_FAIL :
		if( pEventData && dataSize > 0 ) {
			printf("[%s] : File open failed. ( %s )\n", __func__, pEventData);
		}
		break;
	case NX_NOTIFY_ERR_WRITE :
		if( pEventData && dataSize > 0 ) {	
			printf("[%s] : File writing failed. ( %s )", __func__, pEventData);
		}
		break;
	}
	
	return 0;
}

int main(void)
{
	pMp4Manager = GetMp4ManagerHandle();

	printf("############################## STARTING APPLICATION ##############################\n");
	printf("MP4 Encoding Test Application\n");
	printf("Build Time : %s, %s\n", __TIME__, __DATE__);
	printf("Author     : Sung-won Jo\n");
	printf("Mail       : doriya@nexell.co.kr\n");
	printf("COPYRIGHT@2013 NEXELL CO. ALL RIGHT RESERVED.\n");
	printf("##################################################################################\n");

	NX_DspVideoSetPriority( DISPLAY_MODULE_MLC0, 0 );
	
	register_signal();
	shell_main();

	if( pMp4Manager )
	{
		pMp4Manager->Stop();
		pMp4Manager->Deinit();
		ReleaseMp4ManagerHandle( pMp4Manager );
	}

	return 0;
}