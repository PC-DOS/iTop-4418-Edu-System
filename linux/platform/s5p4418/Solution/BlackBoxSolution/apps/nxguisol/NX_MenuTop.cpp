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

#include <NX_DvrConfig.h>

#include "NX_MenuTop.h"
#include "NX_MenuFileList.h"

static CNX_BaseWindow 	*gstWnd			= NULL;
static CNX_TextBox		*gstTxtTitle	= NULL;
static CNX_PushButton 	*gstBtnDvr		= NULL;
static CNX_PushButton 	*gstBtnPlayer	= NULL;
static CNX_PushButton 	*gstBtnSetting	= NULL;
static CNX_PushButton 	*gstBtnExit		= NULL;

static SDL_Surface		*gstSurface		= NULL;
static TTF_Font			*gstFont		= NULL;

extern CNX_BaseObject *GetMenuBlackBoxHandle( SDL_Surface *pSurface, TTF_Font *pFont );
extern CNX_BaseObject *GetMenuFileListHandle( SDL_Surface *pSurface, TTF_Font *pFont );
extern CNX_BaseObject *GetMenuSettingHandle( SDL_Surface *pSurface, TTF_Font *pFont );

static void Move2MenuBlackBox( void )
{
	CNX_BaseWindow *pWnd = (CNX_BaseWindow*)GetMenuBlackBoxHandle( gstSurface, gstFont );
	
	DvrStart();

	pWnd->Draw( OBJ_STATUS_NORMAL );
	pWnd->EventLoop();
}

static void Move2Player( void )
{
	CNX_BaseWindow *pWnd = (CNX_BaseWindow*)GetMenuFileListHandle( gstSurface, gstFont );

	FileListNormal();

	pWnd->EventLoop();
}

extern CNX_ToggleButton *gstBtnChannel;
extern CNX_ToggleButton *gstBtnPreview;
extern CNX_ToggleButton *gstBtnQuality;
extern CNX_ToggleButton *gstBtnEnableHls;

static void Move2Setting( void )
{
	CNX_BaseWindow *pWnd = (CNX_BaseWindow*)GetMenuSettingHandle( gstSurface, gstFont );

	gstBtnChannel->SetCurPos( gDvrConfig.nChannel );
	gstBtnPreview->SetCurPos( gDvrConfig.nPreview );
	gstBtnQuality->SetCurPos( gDvrConfig.nEncQuality );
	gstBtnEnableHls->SetCurPos( gDvrConfig.bHls );

	pWnd->EventLoop();
}

static void BuildMenuTop( void )
{
	SDL_Rect	rtWnd			= {   0,   0, 800, 480 };
	SDL_Rect	rtTxtTitle		= {  10,  10, 800,  50 };
	SDL_Rect	rtBtnDvr		= { 150, 120, 200, 120 };
	SDL_Rect	rtBtnPlayer		= { 450, 120, 200, 120 };
	SDL_Rect	rtBtnSetting	= { 150, 300, 200, 120 };
	SDL_Rect	rtBtnExit		= { 450, 300, 200, 120 };

	OBJ_ATTRIBUTE attr;

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtWnd;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0xBB, 0xBB, 0xBB);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0xBB, 0xBB, 0xBB);
	gstWnd->Create( &attr );
	gstWnd->SetChild( (CNX_BaseObject*)gstTxtTitle );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtTxtTitle;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "- Blackbox Test Application (800 x 480) -";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	gstTxtTitle->Create( &attr );
	gstTxtTitle->SetChild( (CNX_BaseObject*)gstBtnDvr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnDvr;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Blackbox";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnDvr->Create( &attr );
	gstBtnDvr->SetAction( OBJ_ACTION_WINDOW, &Move2MenuBlackBox );
	gstBtnDvr->SetChild( (CNX_BaseObject*)gstBtnPlayer );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnPlayer;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Player";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnPlayer->Create( &attr );
	gstBtnPlayer->SetAction( OBJ_ACTION_WINDOW, &Move2Player );
	gstBtnPlayer->SetChild( (CNX_BaseObject*)gstBtnSetting );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnSetting;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Setting";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnSetting->Create( &attr );
	gstBtnSetting->SetAction( OBJ_ACTION_WINDOW, &Move2Setting );
	gstBtnSetting->SetChild( (CNX_BaseObject*)gstBtnExit );

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
	gstBtnExit->SetAction( OBJ_ACTION_EXIT, NULL );
}

CNX_BaseObject *GetMenuTopHandle( SDL_Surface *pSurface, TTF_Font *pFont )
{
	if( !gstWnd )
	{
		gstSurface	= pSurface;
		gstFont		= pFont;

		gstWnd 			= new CNX_BaseWindow();
		gstTxtTitle		= new CNX_TextBox();
		gstBtnDvr		= new CNX_PushButton();
		gstBtnPlayer	= new CNX_PushButton();
		gstBtnSetting	= new CNX_PushButton();
		gstBtnExit		= new CNX_PushButton();		
		
		BuildMenuTop();
	}
	
	return gstWnd;
}

#define SAFE_RELEASE(A)		if(A){delete A; A = NULL;}

void ReleaseMenuTopHandle( CNX_BaseObject *pBaseWindow )
{
	if( pBaseWindow )
	{
		SAFE_RELEASE(gstWnd);
		SAFE_RELEASE(gstTxtTitle);
		SAFE_RELEASE(gstBtnDvr);
		SAFE_RELEASE(gstBtnPlayer);
		SAFE_RELEASE(gstBtnSetting);
		SAFE_RELEASE(gstBtnExit);
	}
}