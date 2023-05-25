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

#include <NX_DvrConfig.h>
#include "NX_MenuSetting.h"

static CNX_BaseWindow 	*gstWnd				= NULL;
static CNX_TextBox		*gstTxtTitle		= NULL;
static CNX_TextBox		*gstTxtChannel		= NULL;
static CNX_TextBox		*gstTxtPreview		= NULL;
static CNX_TextBox		*gstTxtQuality		= NULL;
static CNX_TextBox		*gstTxtEnableHls	= NULL;

CNX_ToggleButton	*gstBtnChannel		= NULL;
CNX_ToggleButton	*gstBtnPreview		= NULL;
CNX_ToggleButton	*gstBtnQuality		= NULL;
CNX_ToggleButton	*gstBtnEnableHls	= NULL;

static CNX_PushButton 	*gstBtnExit			= NULL;

static SDL_Surface		*gstSurface		= NULL;
static TTF_Font			*gstFont		= NULL;

extern CNX_BaseObject *GetMenuTopHandle( SDL_Surface *pSurface, TTF_Font *pFont );

static void Move2MenuTop( void )
{
	CNX_BaseWindow *pWnd = (CNX_BaseWindow*)GetMenuTopHandle( gstSurface, gstFont );

	// save configuration
	gDvrConfig.nChannel		= gstBtnChannel->GetCurPos();
	gDvrConfig.nPreview		= gstBtnPreview->GetCurPos();
	gDvrConfig.nEncQuality	= gstBtnQuality->GetCurPos();
	gDvrConfig.bHls			= gstBtnEnableHls->GetCurPos();

	DvrConfigWrite( "/mnt/mmc/config.dat" );

	pWnd->EventLoop();
}

static void BuildMenuSetting( void )
{
	SDL_Rect	rtWnd			= {   0,   0, 800, 480 };
	SDL_Rect	rtTxtTitle		= {  10,  10, 800,  50 };
	
	SDL_Rect	rtTxtChannel	= { 50, 120, 400, 40 };
	SDL_Rect	rtTxtPreview	= { 50, 180, 400, 40 };
	SDL_Rect	rtTxtQuality	= { 50, 240, 400, 40 };
	SDL_Rect	rtTxtEnableHls	= { 50, 300, 400, 40 };
	
	SDL_Rect	rtBtnChannel	= { 500, 120, 200, 40 };
	SDL_Rect	rtBtnPreview	= { 500, 180, 200, 40 };
	SDL_Rect	rtBtnQuality	= { 500, 240, 200, 40 };
	SDL_Rect	rtBtnEnableHls	= { 500, 300, 200, 40 };

	SDL_Rect	rtBtnExit		= { 500, 360, 200, 40 };

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
	attr.pString			= "- BlackBox Simple Setting (800 x 480) -";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	gstTxtTitle->Create( &attr );
	gstTxtTitle->SetChild( (CNX_BaseObject*)gstTxtChannel );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtTxtChannel;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Encoding Channel";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	gstTxtChannel->Create( &attr );
	gstTxtChannel->SetChild( (CNX_BaseObject*)gstTxtPreview );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtTxtPreview;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Preview Channel";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	gstTxtPreview->Create( &attr );
	gstTxtPreview->SetChild( (CNX_BaseObject*)gstTxtQuality );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtTxtQuality;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Encoding Quality";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	gstTxtQuality->Create( &attr );
	gstTxtQuality->SetChild( (CNX_BaseObject*)gstTxtEnableHls );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface			= gstSurface;
	attr.rect				= rtTxtEnableHls;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Support HLS";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	gstTxtEnableHls->Create( &attr );
	gstTxtEnableHls->SetChild( (CNX_BaseObject*)gstBtnChannel );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnChannel;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "1 Channel";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnChannel->Create( &attr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnChannel;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "2 Channel";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnChannel->Create( &attr );
	//gstBtnChannel->SetAction( OBJ_ACTION_EXEC, &BlackBoxCapture );	
	gstBtnChannel->SetChild( (CNX_BaseObject*)gstBtnPreview );	

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnPreview;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "1 Channel";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnPreview->Create( &attr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnPreview;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "2 Channel";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnPreview->Create( &attr );
	gstBtnPreview->SetChild( (CNX_BaseObject*)gstBtnQuality );	

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnQuality;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Normal";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnQuality->Create( &attr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnQuality;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Fine";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnQuality->Create( &attr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnQuality;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "Super Fine";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnQuality->Create( &attr );		
	gstBtnQuality->SetChild( (CNX_BaseObject*)gstBtnEnableHls );	

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnEnableHls;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "NO";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnEnableHls->Create( &attr );

	memset( &attr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	attr.pSurface 			= gstSurface;
	attr.rect				= rtBtnEnableHls;
	attr.status				= OBJ_STATUS_NORMAL;
	attr.pFont				= gstFont;
	attr.pString			= "YES";
	attr.fontColor			= (SDL_Color){ 0x00, 0x00, 0x00, 0 };
	attr.normalBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0xFF, 0x00);
	attr.normalLineColor	= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	attr.focusBgColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0xFF);
	attr.focusLineColor		= SDL_MapRGB(gstSurface->format, 0x00, 0x00, 0x00);
	gstBtnEnableHls->Create( &attr );
	gstBtnEnableHls->SetChild( (CNX_BaseObject*)gstBtnExit );	

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


CNX_BaseObject *GetMenuSettingHandle( SDL_Surface *pSurface, TTF_Font *pFont )
{
	if( !gstWnd )
	{
		gstSurface	= pSurface;
		gstFont		= pFont;

		gstWnd 				= new CNX_BaseWindow();
		gstTxtTitle			= new CNX_TextBox();
		gstTxtChannel		= new CNX_TextBox();
		gstTxtPreview		= new CNX_TextBox();
		gstTxtQuality		= new CNX_TextBox();
		gstTxtEnableHls		= new CNX_TextBox();

		gstBtnChannel		= new CNX_ToggleButton();
		gstBtnPreview		= new CNX_ToggleButton();
		gstBtnQuality		= new CNX_ToggleButton();
		gstBtnEnableHls		= new CNX_ToggleButton();

		gstBtnExit			= new CNX_PushButton();

		BuildMenuSetting();
	}
	
	return gstWnd;
}

#define SAFE_RELEASE(A)		if(A){delete A; A = NULL;}

void ReleaseMenuSettingHandle( CNX_BaseObject *pBaseWindow )
{
	if( pBaseWindow )
	{
		SAFE_RELEASE( gstWnd );
		SAFE_RELEASE( gstTxtTitle );
		SAFE_RELEASE( gstTxtChannel );
		SAFE_RELEASE( gstTxtPreview );
		SAFE_RELEASE( gstTxtQuality );
		SAFE_RELEASE( gstTxtEnableHls );
		SAFE_RELEASE( gstBtnChannel );
		SAFE_RELEASE( gstBtnPreview );
		SAFE_RELEASE( gstBtnQuality );
		SAFE_RELEASE( gstBtnEnableHls );
		SAFE_RELEASE( gstBtnExit );
	}
}
