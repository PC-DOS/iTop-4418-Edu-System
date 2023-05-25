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

#include <CNX_BaseWindow.h>
#include <CNX_TextBox.h>
#include <CNX_PushButton.h>
#include <CNX_ProgressBar.h>
#include <CNX_ToggleButton.h>
#include <CNX_ListBox.h>

#include <NX_DvrMain.h>
#include <nx_dsp.h>

#include "NX_MenuBlackBox.h"

static CNX_BaseWindow		*gstWnd			= NULL;
static CNX_TextBox			*gstText		= NULL;
static CNX_PushButton		*gstBtnCapture	= NULL;
static CNX_PushButton		*gstBtnExit		= NULL;

static SDL_Surface			*gstSurface		= NULL;
static TTF_Font				*gstFont		= NULL;

extern CNX_BaseObject *GetMenuTopHandle( SDL_Surface *pSurface, TTF_Font *pFont );

static void Move2MenuTop( void )
{
	CNX_BaseWindow *pWnd = (CNX_BaseWindow*)GetMenuTopHandle( gstSurface, gstFont );
	DvrStop();
	pWnd->EventLoop();
}

static void BlackBoxCapture( void )
{
	DvrCapture();
}

static void BuildMenuBlackBox( void )
{
	SDL_Rect	rtWnd			= {   0,   0, 800, 480 };
	SDL_Rect	rtText			= { 700,  10, 100,  50 };
	SDL_Rect	rtBtnCapture	= { 500, 420, 120,  40 };
	SDL_Rect	rtBtnExit		= { 650, 420, 120,  40 };

	OBJ_ATTRIBUTE attr;

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtWnd;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x09, 0x09, 0x09);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x09, 0x09, 0x09);
	gstWnd->Create( &attr );
	gstWnd->SetChild( (CNX_BaseObject*)gstText );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtText;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Rec";
	attr.fontColor			= (SDL_Color){ 0xFF, 0x00, 0x00, 0 };
	gstText->Create( &attr );
	gstText->SetChild( (CNX_BaseObject*)gstBtnCapture );	

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnCapture;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Capture";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnCapture->Create( &attr );
	gstBtnCapture->SetAction( OBJ_ACTION_EXEC, &BlackBoxCapture );	
	gstBtnCapture->SetChild( (CNX_BaseObject*)gstBtnExit );	

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
	gstBtnExit->SetAction( OBJ_ACTION_WINDOW, &Move2MenuTop );	
}

CNX_BaseObject *GetMenuBlackBoxHandle( SDL_Surface *pSurface, TTF_Font *pFont )
{
	if( !gstWnd )
	{
		gstSurface	= pSurface;
		gstFont		= pFont;

		gstWnd 			= new CNX_BaseWindow();
		gstText			= new CNX_TextBox();
		gstBtnCapture	= new CNX_PushButton();
		gstBtnExit		= new CNX_PushButton();
		
		BuildMenuBlackBox();
	}
	
	return gstWnd;
}

#define SAFE_RELEASE(A)		if(A){delete A; A = NULL;}

void ReleaseMenuBlackBoxHandle( CNX_BaseObject *pBaseWindow )
{
	if( pBaseWindow )
	{
		SAFE_RELEASE( gstWnd );
		SAFE_RELEASE( gstText );
		SAFE_RELEASE( gstBtnCapture );
		SAFE_RELEASE( gstBtnExit );
	}
}

