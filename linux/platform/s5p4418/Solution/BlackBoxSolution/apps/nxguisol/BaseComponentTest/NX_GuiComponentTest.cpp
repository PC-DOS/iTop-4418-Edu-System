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
#include <termios.h>
#include <term.h>
#include <curses.h>
#include <unistd.h>
#include <signal.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#include <CNX_BaseWindow.h>
#include <CNX_TextBox.h>
#include <CNX_PushButton.h>
#include <CNX_ProgressBar.h>
#include <CNX_ToggleButton.h>
#include <CNX_ListBox.h>

#include <NX_GuiUtils.h>

static CNX_BaseWindow	*pTopWnd 	= new CNX_BaseWindow();
static CNX_TextBox		*pText		= new CNX_TextBox();

static CNX_PushButton	*pTopBtn0	= new CNX_PushButton();
static CNX_PushButton	*pTopBtn1	= new CNX_PushButton();
static CNX_PushButton	*pTopExit	= new CNX_PushButton();

static CNX_ProgressBar	*pBar0		= new CNX_ProgressBar();
static CNX_ProgressBar	*pBar1		= new CNX_ProgressBar();

static CNX_BaseWindow	*pSubWnd	= new CNX_BaseWindow();
static CNX_PushButton	*pSubBtn0	= new CNX_PushButton();
static CNX_PushButton	*pSubBtn1	= new CNX_PushButton();

static CNX_ToggleButton	*pToggle0	= new CNX_ToggleButton();
static CNX_ToggleButton	*pToggle1	= new CNX_ToggleButton();

static CNX_ListBox		*pListBox	= new CNX_ListBox();

static SDL_Surface		*pSurface 	= NULL;
static TTF_Font			*pFont		= NULL;

static SDL_Rect			wndRect		= { 0, 0, 1280, 720 };
static SDL_Rect			textRect	= {  10,  10, 300,  30 };
static SDL_Rect			btn0Rect 	= { 100, 100, 200, 100 };
static SDL_Rect			btn1Rect 	= { 400, 100, 200, 100 };
static SDL_Rect			btnExitRect	= { 700, 100, 200, 100 };
static SDL_Rect			bar0Rect	= { 100, 500, 900, 50 };
static SDL_Rect			bar1Rect	= { 100, 650, 900, 50 };
static SDL_Rect			toggle0Rect	= { 100, 300, 200, 100 };
static SDL_Rect			toggle1Rect = { 400, 300, 200, 100 };
static SDL_Rect			listboxRect	= { 700, 100, 400, 600 };

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
			printf("SIgABRT..\n");	break;
		default :
			break;
	}

	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.type = SDL_USEREVENT;
	SDL_PushEvent(&event);

	if( pSurface )	SDL_FreeSurface( pSurface );
	SDL_Quit();

	TTF_CloseFont( pFont );

	if( pTopWnd )	delete pTopWnd;
	if( pText )		delete pText;
	if( pTopBtn0 )	delete pTopBtn0;
	if( pTopBtn1 )	delete pTopBtn1;
	if( pTopExit )	delete pTopExit;

	if( pBar0 ) 	delete pBar0;
	if( pBar1 ) 	delete pBar1;

	if( pToggle0 )	delete pToggle0;
	if( pToggle1 )	delete pToggle1;

	if( pSubWnd )	delete pSubWnd;
	if( pSubBtn0 )	delete pSubBtn0;
	if( pSubBtn1 )	delete pSubBtn1;
	if( pListBox )	delete pListBox;

	exit( EXIT_FAILURE );
}

static void register_signal( void )
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

void testAction( void )
{
	printf("Test Action!\n");
	
	int32_t pos;
	pos = pBar0->GetPos();
	pos += 10;
	if( pos > 100) pos = 0;
	pBar0->SetPos( pos );	

	// for( int32_t i = 0; i < 101; i++)
	// {
	// 	pBar0->SetPos( i );	
	// 	usleep(10000);
	// }
}

void testProgress1( void )
{
	printf("%s():: Touch Action!\n", __func__);
}

void testProgress2( void )
{
	printf("%s():: Touch Action!\n", __func__);
}

void Top2Sub( void )
{
	pSubWnd->EventLoop();
}

void Sub2Top( void )
{
	pTopWnd->EventLoop();
}

void testListBox( void )
{
	int32_t curItem = pListBox->GetCurItem();

	curItem++;
	curItem %= pListBox->GetItemNum();

	pListBox->SetCurItem( curItem );
}

void BuildTopWnd( void )
{
	OBJ_ATTRIBUTE attr;

	// Base Window
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= wndRect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0xFF, 0xFF, 0xFF);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0xFF, 0xFF, 0xFF);

	pTopWnd->Create( &attr );
	pTopWnd->SetChild( (CNX_BaseObject*)pTopBtn0 );

	// Btn0
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= pSurface;
	attr.rect				= btn0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Button0";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pTopBtn0->Create( &attr );
	pTopBtn0->SetChild( (CNX_BaseObject*)pTopBtn1 );
	pTopBtn0->SetAction( OBJ_ACTION_EXEC, &testAction );

	// Btn1
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= pSurface;
	attr.rect				= btn1Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "to Sub Menu";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);

	pTopBtn1->Create( &attr );
	pTopBtn1->SetChild( (CNX_BaseObject*)pTopExit );
	pTopBtn1->SetAction( OBJ_ACTION_WINDOW, &Top2Sub );

	// BtnExit
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= pSurface;
	attr.rect				= btnExitRect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Exit";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pTopExit->Create( &attr );
	pTopExit->SetAction( OBJ_ACTION_EXIT, NULL );
	pTopExit->SetChild( (CNX_BaseObject*)pBar0 );

	// Progress Bar0
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= bar0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pBar0->Create( &attr );
	
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= bar0Rect;
	attr.status				= OBJ_STATUS_SELECT;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	pBar0->Create( &attr );
	pBar0->SetAction( OBJ_ACTION_EXEC, &testProgress1 );
	pBar0->SetPos( 0 );
	pBar0->SetChild( (CNX_BaseObject*)pBar1 );
	
	// Progress Bar1
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= bar1Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pBar1->Create( &attr );
	
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= bar1Rect;
	attr.status				= OBJ_STATUS_SELECT;
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	pBar1->Create( &attr );
	pBar1->SetAction( OBJ_ACTION_EXEC, &testProgress2 );
	pBar1->SetPos( 0 );
	pBar1->SetChild( (CNX_BaseObject*)pToggle0 );

	// Toggle Button0
	//status0
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= toggle0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Toggle-Step0";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pToggle0->Create( &attr );
	
	//status1
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= toggle0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Toggle-Step1";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pToggle0->Create( &attr );

	//status2
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= toggle0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Toggle-Step2";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pToggle0->Create( &attr );

	//status3
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= toggle0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Toggle-Step3";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pToggle0->Create( &attr );
	pToggle0->SetChild( (CNX_BaseObject*)pToggle1 );
	pToggle0->SetCurPos( 1 );

	//status0
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= toggle1Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Toggle-Step0";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pToggle1->Create( &attr );	
	pToggle1->SetChild( (CNX_BaseObject*)pText );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= textRect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "GUI TEST APPLICATION";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	pText->Create( &attr );	

}

void BuildSubWnd( void )
{
	OBJ_ATTRIBUTE attr;

	// Base Window
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= pSurface;
	attr.rect				= wndRect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0xBB, 0xBB, 0xBB);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0xBB, 0xBB, 0xBB);

	pSubWnd->Create( &attr );
	pSubWnd->SetChild( (CNX_BaseObject*)pSubBtn0 );

	// Btn0
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= pSurface;
	attr.rect				= btn0Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "to Top Menu";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pSubBtn0->Create( &attr );
	pSubBtn0->SetAction( OBJ_ACTION_WINDOW, &Sub2Top );	
	pSubBtn0->SetChild( (CNX_BaseObject*)pSubBtn1 );

	// Btn1
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= pSurface;
	attr.rect				= btn1Rect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "Test";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pSubBtn1->Create( &attr );
	pSubBtn1->SetAction( OBJ_ACTION_EXEC, &testListBox );	
	pSubBtn1->SetChild( (CNX_BaseObject*)pListBox );

	// ListBox
	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= pSurface;
	attr.rect				= listboxRect;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= pFont;
	attr.pString			= "to Top Menu";
	attr.fontColor			= (SDL_Color){ 0xFF, 0xFF, 0xFF, 0 };
	attr.normalBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(pSurface->format, 0x00, 0x00, 0x00);
	pListBox->Create( &attr );

	char str[256][256] = { {0x00,}, };

	for(int32_t i = 0; i < 30; i++)
	{
		sprintf(str[i], "ListBox Component #%d", i );
		pListBox->AddString((uint8_t*)str[i]);
	}
	
	pListBox->SetPageItemNum( 10 );
	pListBox->SetCurItem(1);
}

int32_t main( void )
{
	TTF_Init();
	pFont = TTF_OpenFont("/root/DejaVuSansMono.ttf", 24);

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError() );
		exit(1);
	}

	SDL_ShowCursor( true );
	//SDL_ShowCursor( false );
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	//SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);

	const SDL_VideoInfo *pInfo;
	pInfo = SDL_GetVideoInfo();
	pSurface = SDL_SetVideoMode( pInfo->current_w, pInfo->current_h, pInfo->vfmt->BitsPerPixel, SDL_SWSURFACE );

	BuildTopWnd();
	BuildSubWnd();

	register_signal();

	// Loop
	pTopWnd->EventLoop();

	if( pSurface )	SDL_FreeSurface( pSurface );
	SDL_Quit();

	TTF_CloseFont( pFont );

	if( pTopWnd )	delete pTopWnd;
	if( pText )		delete pText;

	if( pTopBtn0 )	delete pTopBtn0;
	if( pTopBtn1 )	delete pTopBtn1;
	if( pTopExit )	delete pTopExit;
	
	if( pBar0 ) 	delete pBar0;
	if( pBar1 ) 	delete pBar1;

	if( pToggle0 )	delete pToggle0;
	if( pToggle1 )	delete pToggle1;

	if( pSubWnd )	delete pSubWnd;
	if( pSubBtn0 )	delete pSubBtn0;
	if( pSubBtn1 )	delete pSubBtn1;
	if( pListBox )	delete pListBox;

	return 0;
}
