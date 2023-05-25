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

#include <unistd.h>
#include <pthread.h>

#include <CNX_BaseWindow.h>
#include <CNX_TextBox.h>
#include <CNX_PushButton.h>
#include <CNX_ProgressBar.h>
#include <CNX_ToggleButton.h>
#include <CNX_ListBox.h>

#include <NX_PlayerMain.h>

#include "NX_MenuPlayer.h"

static CNX_BaseWindow	*gstWnd 		= NULL;
static CNX_ProgressBar	*gstBarSeek		= NULL;
static CNX_TextBox		*gstTextDur		= NULL;
static CNX_PushButton	*gstBtnPrev		= NULL;
static CNX_PushButton	*gstBtnNext		= NULL;
static CNX_ToggleButton	*gstBtnPlay		= NULL;
static CNX_PushButton	*gstBtnExit		= NULL;

static SDL_Surface		*gstSurface		= NULL;
static TTF_Font			*gstFont		= NULL;

char strDuration[36] = "00:00/00:00";

extern CNX_BaseObject *GetMenuFileListHandle( SDL_Surface *pSurface, TTF_Font *pFont );

extern void FileListNextPlay(void);
extern void FileListPrevPlay(void);

static void Seek( void )
{
	printf("Seek!\n");
	PlayerSeek( gstBarSeek->GetPos( ) );
}

static void PlayPrevious( void )
{
	printf("Play Previous File.\n");
	FileListPrevPlay();
}

static void PlayNext( void )
{
	printf("Play Next File.\n");
	FileListNextPlay();
}

static void PlayPause( void )
{
	printf("Play / Pause.\n");
	PlayerPause();
}

static void Move2FileList( void )
{
	CNX_BaseWindow *pWnd = (CNX_BaseWindow*)GetMenuFileListHandle( gstSurface, gstFont );
	PlayerStop();
	pWnd->EventLoop();
}

static int32_t		bEventTaskRun = false;
static pthread_t	hEventTask = 0x00;
static pthread_mutex_t hEventLock;

void *EventTask( void *arg )
{
	uint32_t position, duration;
	int32_t ratio;
	int32_t flag;

	pthread_mutex_init( &hEventLock, NULL );

	while(bEventTaskRun)
	{
		usleep( 100000 );
		// zero is wait..
		flag = PopFlags();

		if(flag == 1) {		// running
			PlayerGetPos( &position, &duration );

			ratio = (int32_t)(((float)position / (float)duration) * 100.);
			sprintf( strDuration, "%02d:%02d/%02d:%02d",
				(int32_t)((float)position / 1000. / 60.), (int32_t)((float)position / 1000.),
				(int32_t)((float)duration / 1000. / 60.), (int32_t)((float)duration / 1000.)
				);

			//printf( "%s [%d]\n", strDuration, ratio );
			gstBarSeek->SetPos( ratio );
			gstTextDur->SetText( strDuration, 0x09, 0x09, 0x09 );
		}
		else if(flag == 3) {	// eos
			printf( "EOS!!\n" );
			PlayNext();
		}
	}

	pthread_mutex_destroy( &hEventLock );

	return (void*)0xDEADDEAD;
}

static int32_t EventTaskStart( void )
{
	if(bEventTaskRun)
		return -1;

	bEventTaskRun = true;
	if(0 > pthread_create( &hEventTask, NULL, &EventTask, NULL )) {
		printf( "%s(): Fail, Create Thread.\n", __func__ );
		return -1;
	}

	return 0;
}

static int32_t EventTaskStop( void )
{
	if(!bEventTaskRun)
		return -1;

	bEventTaskRun = false;
	pthread_join( hEventTask, NULL );
	hEventTask = 0x00;

	return 0;
}

static void BuildMenuPlayer( void )
{
	SDL_Rect	rtWnd			= {   0,   0, 800, 480 };
	SDL_Rect	rtBarSeek		= {  10,  20, 600,  30 };
	SDL_Rect	rtTextDur		= { 630,  20, 170,  30 };
	SDL_Rect	rtBtnPrev		= {  80, 400, 120,  50 };
	SDL_Rect	rtBtnNext		= { 230, 400, 120,  50 };
	SDL_Rect	rtBtnPlay		= { 450, 400, 120,  50 };
	SDL_Rect	rtBtnExit		= { 600, 400, 120,  50 };
	OBJ_ATTRIBUTE attr;

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtWnd;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x09, 0x09, 0x09);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x09, 0x09, 0x09);
	gstWnd->Create( &attr );
	gstWnd->SetChild( (CNX_BaseObject*)gstBarSeek );

	// Progress Bar0
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtBarSeek;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBarSeek->Create( &attr );
	
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtBarSeek;
	attr.status				= OBJ_STATUS_SELECT;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	gstBarSeek->Create( &attr );
	gstBarSeek->SetAction( OBJ_ACTION_EXEC, &Seek );
	gstBarSeek->SetPos( 0 );
	gstBarSeek->SetChild( (CNX_BaseObject*)gstTextDur );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtTextDur;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= strDuration;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB( gstSurface->format, 0x09, 0x09, 0x09 );
	attr.normalLineColor	= SDL_MapRGB( gstSurface->format, 0x09, 0x09, 0x09 );
	gstTextDur->Create( &attr );
	gstTextDur->SetChild( (CNX_BaseObject*)gstBtnPrev );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnPrev;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Previous";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnPrev->Create( &attr );
	gstBtnPrev->SetAction( OBJ_ACTION_EXEC, &PlayPrevious );	
	gstBtnPrev->SetChild( (CNX_BaseObject*)gstBtnNext );	

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnNext;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Next";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnNext->Create( &attr );
	gstBtnNext->SetAction( OBJ_ACTION_EXEC, &PlayNext );	
	gstBtnNext->SetChild( (CNX_BaseObject*)gstBtnPlay );	
	
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnPlay;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Pause";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnPlay->Create( &attr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtBtnPlay;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Play";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB( gstSurface->format, 0x00, 0xFF, 0x00 );
	attr.normalLineColor	= SDL_MapRGB( gstSurface->format, 0x00, 0x00, 0x00 );
	attr.focusBgColor		= SDL_MapRGB( gstSurface->format, 0x00, 0x00, 0xFF );
	attr.focusLineColor		= SDL_MapRGB( gstSurface->format, 0x00, 0x00, 0x00 );
	gstBtnPlay->Create( &attr );
	gstBtnPlay->SetAction( OBJ_ACTION_EXEC, &PlayPause );	
	gstBtnPlay->SetChild( (CNX_BaseObject*)gstBtnExit );		

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnExit;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Exit";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnExit->Create( &attr );
	gstBtnExit->SetAction( OBJ_ACTION_WINDOW, &Move2FileList );
}


CNX_BaseObject *GetMenuPlayerHandle( SDL_Surface *pSurface, TTF_Font *pFont )
{
	if( !gstWnd )
	{
		gstSurface	= pSurface;
		gstFont		= pFont;

		gstWnd 		= new CNX_BaseWindow();
		gstBarSeek	= new CNX_ProgressBar();
		gstTextDur	= new CNX_TextBox();
		gstBtnPrev	= new CNX_PushButton();
		gstBtnNext	= new CNX_PushButton();
		gstBtnPlay	= new CNX_ToggleButton();
		gstBtnExit	= new CNX_PushButton();
		
		BuildMenuPlayer();
	}
	
	EventTaskStart();

	return gstWnd;
}

#define SAFE_RELEASE(A)		if(A){delete A; A = NULL;}

void ReleaseMenuPlayerHandle( CNX_BaseObject *pBaseWindow )
{
	EventTaskStop();
	
	if( pBaseWindow )
	{
		SAFE_RELEASE( gstWnd );
		SAFE_RELEASE( gstBarSeek );
		SAFE_RELEASE( gstTextDur );
		SAFE_RELEASE( gstBtnPrev );
		SAFE_RELEASE( gstBtnNext );
		SAFE_RELEASE( gstBtnPlay );
		SAFE_RELEASE( gstBtnExit );
	}
}

static int32_t flags;

void PushFlags( int32_t type )
{
	pthread_mutex_lock( &hEventLock );
	flags = type;
	pthread_mutex_unlock( &hEventLock );
}

int32_t PopFlags( void )
{
	int32_t type;
	pthread_mutex_lock( &hEventLock );
	type = flags;
	pthread_mutex_unlock( &hEventLock );
	return type;
}