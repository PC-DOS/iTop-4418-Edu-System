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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <nx_audio.h>

#define MAX_STRING_SIZE		128

NX_AUDIO_HANDLE		hAudio;

static void signal_handler(int sig)
{
	printf("Aborted by signal %s (%d)..\n", (char*)strsignal(sig), sig);

	switch(sig)
	{
		case SIGINT :
			printf("SIGINT..\n");	break;
		case SIGTERM :
			printf("SIGTERM..\n");	break;
		case SIGABRT :
			printf("SIGABRT..\n");	break;
		default :
			break;
	}

	if( hAudio ) NX_AudioDeinit(hAudio);
	
	exit(EXIT_FAILURE);
}

static void register_signal(void)
{
	signal(SIGINT,	signal_handler);
	signal(SIGTERM,	signal_handler);
	signal(SIGABRT, signal_handler);
}

void print_usage(void)
{
	printf("##### Audio Library Test Application #####\n");
	printf("  play        :  play wav-audio\n");
	printf("  stop        :  stop wav-audio\n");
	printf("  up          :  volume up\n");
	printf("  dn          :  volume down\n");
	printf("  a           :  mic volume up\n");
	printf("  d           :  mic volume dn\n");
	printf("  quit        :  quit\n");
}

int shell_main(char *filename)
{
	char cmd[MAX_STRING_SIZE];
	
	int volume, mic;

	hAudio	= NX_AudioInit();
	volume	= NX_AudioGetVolume(hAudio, AUDIO_TYPE_PLAYBACK);
	mic		= NX_AudioGetVolume(hAudio, AUDIO_TYPE_CAPTURE);

	print_usage();

	while(1)
	{
		printf("# ");

		fgets(cmd, sizeof(cmd), stdin);
		if( strlen(cmd) != 0) cmd[strlen(cmd) - 1] = 0x00;

		if( !strcmp(cmd, "") ) {
			
		}
		else if( !strcmp(cmd, "quit") ) {
			break;
		}
		else if( !strcmp(cmd, "play") ) {
			if( hAudio ) {
				NX_AudioSetVolume(hAudio, AUDIO_TYPE_PLAYBACK, volume);
				NX_AudioPlay( hAudio, filename );
			}
		}
		else if( !strcmp(cmd, "stop") ) {
			
			if( hAudio ) {
				NX_AudioStop(hAudio, 1);
			}
			printf("stop\n");
		}
		else if( !strcmp(cmd, "up") ) {
			volume += 5;
			if( volume > 100 ) {
				volume = 100;
			}
				
			NX_AudioSetVolume(hAudio, AUDIO_TYPE_PLAYBACK, volume);
			printf("Expected Audio Volume(%d), Real Audio Volume(%d)\n", volume, NX_AudioGetVolume(hAudio, AUDIO_TYPE_PLAYBACK));
		}
		else if( !strcmp(cmd, "dn") ) {
			volume -= 5;
			if( volume < 0 ) {
				volume = 0;
			}

			NX_AudioSetVolume(hAudio, AUDIO_TYPE_PLAYBACK, volume);
			printf("Expected Audio Volume(%d), Real Audio Volume(%d)\n", volume, NX_AudioGetVolume(hAudio, AUDIO_TYPE_PLAYBACK));
		}
		else if( !strcmp(cmd, "a") ) {
			mic += 5;
			if( mic > 100 ) {
				mic = 100;
			}
			NX_AudioSetVolume(hAudio, AUDIO_TYPE_CAPTURE, mic);
			printf("Expected MIC Volume(%d), Real MIC Volume(%d)\n", mic, NX_AudioGetVolume(hAudio, AUDIO_TYPE_CAPTURE));
		}
		else if( !strcmp(cmd, "d") ) {
			mic -= 5;
			if( mic < 0 ) {
				mic = 0;
			}
			NX_AudioSetVolume(hAudio, AUDIO_TYPE_CAPTURE, mic);
			printf("Expected MIC Volume(%d), Real MIC Volume(%d)\n", mic, NX_AudioGetVolume(hAudio, AUDIO_TYPE_CAPTURE));
		}
		else {
			printf("Unknwon Command!\n");
		}
		fflush(stdin);
	}

	NX_AudioDeinit(hAudio);

	return 0;
}

int main ( int argc, char *argv[] )
{
	if( argc != 2 ) {
		printf(" Usage : %s [filename]\n", argv[0]);
		return -1;
	}
	
	register_signal();

	shell_main(argv[1]);

	return 0;
}
