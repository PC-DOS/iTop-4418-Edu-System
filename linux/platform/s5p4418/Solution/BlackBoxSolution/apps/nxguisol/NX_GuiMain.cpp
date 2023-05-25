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
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <signal.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#include <CNX_BaseWindow.h>

#include <NX_DvrMain.h>
#include <NX_DvrConfig.h>

#include "NX_MenuTop.h"
#include "NX_MenuBlackBox.h"
#include "NX_MenuFileList.h"
#include "NX_MenuSetting.h"
#include "NX_MenuPlayer.h"

#include <nx_dsp.h>

static CNX_BaseWindow	*gstMenuTop			= NULL;
static CNX_BaseWindow	*gstMenuBlackBox	= NULL;
static CNX_BaseWindow	*gstMenuFileList	= NULL;
static CNX_BaseWindow	*gstMenuPlayer		= NULL;
static CNX_BaseWindow	*gstMenuSetting		= NULL;

static SDL_Surface		*gstSurface			= NULL;
static TTF_Font			*gstFont			= NULL;

static void signal_handler( int32_t sig )
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

	// SDL_Event event;
	// event.type = SDL_USEREVENT;
	// event.user.type = SDL_USEREVENT;
	// SDL_PushEvent(&event);

	if( gstSurface )	SDL_FreeSurface( gstSurface );
	SDL_Quit();
	TTF_CloseFont( gstFont );
	
	ReleaseMenuTopHandle( gstMenuTop );
	ReleaseMenuBlackBoxHandle( gstMenuBlackBox );
	ReleaseMenuFileListHandle( gstMenuFileList );
	ReleaseMenuSettingHandle( gstMenuSetting );

	exit( EXIT_FAILURE );
}

static void register_signal( void )
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

int32_t main( void )
{
	TTF_Init();
	gstFont = TTF_OpenFont("/root/DejaVuSansMono.ttf", 24);

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError() );
		exit(1);
	}

	//SDL_ShowCursor( true );
	SDL_ShowCursor( false );

	const SDL_VideoInfo *pInfo;
	pInfo = SDL_GetVideoInfo();
	gstSurface = SDL_SetVideoMode( pInfo->current_w, pInfo->current_h, pInfo->vfmt->BitsPerPixel, SDL_SWSURFACE );

	register_signal();

	NX_DspVideoSetPriority( DISPLAY_MODULE_MLC0, 2 );
	NX_DspSetColorKey( DISPLAY_MODULE_MLC0, 0x090909 );

	// Read Configuration
	if( 0 > DvrConfigRead("/mnt/mmc/config.dat" ) ) {
		DvrConfigWriteDefault("/mnt/mmc/config.dat" );
	}

	gstMenuTop		= (CNX_BaseWindow*)GetMenuTopHandle( gstSurface, gstFont );
	gstMenuBlackBox = (CNX_BaseWindow*)GetMenuBlackBoxHandle( gstSurface, gstFont );
	gstMenuFileList	= (CNX_BaseWindow*)GetMenuFileListHandle( gstSurface, gstFont );
	gstMenuSetting	= (CNX_BaseWindow*)GetMenuBlackBoxHandle( gstSurface, gstFont );
	gstMenuPlayer	= (CNX_BaseWindow*)GetMenuPlayerHandle( gstSurface, gstFont );

	gstMenuTop->EventLoop();

	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.type = SDL_USEREVENT;
	SDL_PushEvent(&event);

	if( gstSurface )	SDL_FreeSurface( gstSurface );
	SDL_Quit();
	TTF_CloseFont( gstFont );

	ReleaseMenuTopHandle( gstMenuTop );
	ReleaseMenuBlackBoxHandle( gstMenuBlackBox );
	ReleaseMenuFileListHandle( gstMenuFileList );
	ReleaseMenuSettingHandle( gstMenuSetting );
	ReleaseMenuPlayerHandle( gstMenuPlayer );

	return 0;
}
