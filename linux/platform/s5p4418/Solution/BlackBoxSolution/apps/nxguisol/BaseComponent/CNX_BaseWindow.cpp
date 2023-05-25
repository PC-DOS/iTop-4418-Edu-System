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

#include "CNX_BaseWindow.h"
#include "NX_GuiUtils.h"

#define NX_DTAG "[CNX_BaseWindow]"
#include "NX_DbgMsg.h"

CNX_BaseWindow::CNX_BaseWindow()
{

}

CNX_BaseWindow::~CNX_BaseWindow()
{
	Destroy();
}

void CNX_BaseWindow::Create( OBJ_ATTRIBUTE *pAttr )
{
	OBJ_ATTRIBUTE *pObjAttr = (OBJ_ATTRIBUTE *)malloc( sizeof(OBJ_ATTRIBUTE) );

	memset( pObjAttr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	memcpy( pObjAttr, pAttr, sizeof(OBJ_ATTRIBUTE) );

	SetAttrList( pObjAttr );
}

void CNX_BaseWindow::Destroy( void )
{
	OBJ_ATTRIBUTE *pAttr = GetAttrList();
	
	if( pAttr )
	{
		OBJ_ATTRIBUTE *pNextAttr = pAttr->pAttrList;
		while( pAttr )
		{
			NxDbgMsg( NX_DBG_VBS, (TEXT("[%s()] Delete Component. ( %p )\n"), __func__, pAttr) );
			free( pAttr );
			pAttr = pNextAttr;
			if( pAttr ) pNextAttr = pAttr->pAttrList;
		}
	}
}

void CNX_BaseWindow::Draw( OBJ_STATUS status )
{
	CNX_BaseObject 	*pChild		= GetChild();
	OBJ_ATTRIBUTE	*pAttr		= GetAttrList(); 

	if( !pAttr )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		return;
	}

	if( status == OBJ_STATUS_NORMAL )
	{
		if( pAttr->pNormalBgSurface )
			DrawImage( pAttr->pSurface, pAttr->rect, pAttr->pNormalBgSurface );
		else
			DrawRect( pAttr->pSurface, pAttr->rect, pAttr->normalLineColor, pAttr->normalBgColor, 1 );
	}
	// else if( status == OBJ_STATUS_FOCUS )
	// {
	// 	if( pAttr->pFocusBgSurface )
	// 		DrawImage( pAttr->pSurface, pAttr->rect, pAttr->pFocusBgSurface  );
	// 	else
	// 		DrawRect( pAttr->pSurface, pAttr->rect, pAttr->focusLineColor, pAttr->focusBgColor, 1 );
	// }

	UpdateSurface( pAttr->pSurface );

	if( pChild )
		pChild->Draw( OBJ_STATUS_NORMAL );
}

int32_t CNX_BaseWindow::EventLoop( void )
{
	static int32_t nLoopDepth = 0;
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()[%02d]++\n"), __func__, ++nLoopDepth) );

	SDL_Event sdlEvent;
	CNX_BaseObject *pChild = GetChild();

	int32_t bExit = false;
	int32_t ret = 0;

	OBJ_ACTION action;
	OBJ_ACTION_FUNC cbActionFunc;

	Draw( OBJ_STATUS_NORMAL );

	while( !bExit )
	{
		SDL_WaitEvent( &sdlEvent );

		if( pChild ) {
			ret = pChild->EventLoop( this, &sdlEvent );
		}
		else {
			ret = OBJ_ACTION_NONE;
		}

		if( ret != OBJ_ACTION_NONE )
		{
			NxDbgMsg( NX_DBG_VBS, (TEXT("[%s()] event type is \"%s\"\n"), __func__,
				(ret == OBJ_ACTION_NONE) ? "none" : 
				(ret == OBJ_ACTION_WINDOW) ? "window" :
				(ret == OBJ_ACTION_EXEC) ? "execute" :
				(ret == OBJ_ACTION_EXIT) ? "exit" : "unknown") );
		}

		switch( ret )
		{
			case OBJ_ACTION_EXEC:
				GetAction( &action, &cbActionFunc );
				cbActionFunc();
				break;
			
			case OBJ_ACTION_NONE:
				break;

			case OBJ_ACTION_WINDOW:	
			case OBJ_ACTION_EXIT:
			default :
				bExit = true;
				break;
		}
	}

	if( ret == OBJ_ACTION_WINDOW )
	{
		GetAction( &action, &cbActionFunc );
		cbActionFunc();
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT( "%s()[%02d]--\n" ), __func__, --nLoopDepth) );
	
	return ret;
}

int32_t CNX_BaseWindow::UserAction( CNX_BaseObject *pParent )
{
	return 0;
}