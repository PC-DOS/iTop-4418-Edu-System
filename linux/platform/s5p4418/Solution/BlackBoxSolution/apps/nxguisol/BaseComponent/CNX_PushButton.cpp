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

#include "CNX_PushButton.h"
#include "NX_GuiUtils.h"

#define NX_DTAG "[CNX_PushButton]"
#include "NX_DbgMsg.h"

//NxDbgMsg( NX_DBG_VBS, (TEXT("%s--\n"), __func__) );
//NxDbgMsg( NX_DBG_DEBUG, (TEXT("Thread info ( pid:%4d, tid:%4d )\n"), pid, tid) );
//printf("%s():: line %d\n", __func__, __LINE__);

CNX_PushButton::CNX_PushButton()
{
}

CNX_PushButton::~CNX_PushButton()
{
	Destroy();
}

void CNX_PushButton::Create( OBJ_ATTRIBUTE *pAttr )
{
	OBJ_ATTRIBUTE *pObjAttr = (OBJ_ATTRIBUTE *)malloc( sizeof(OBJ_ATTRIBUTE) );

	memset( pObjAttr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	memcpy( pObjAttr, pAttr, sizeof(OBJ_ATTRIBUTE) );
	
	SetAttrList( pObjAttr );
}

void CNX_PushButton::Destroy( void )
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

void CNX_PushButton::Draw( OBJ_STATUS status )
{
	CNX_BaseObject *pChild 		= GetChild();
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
	else if( status == OBJ_STATUS_FOCUS )
	{
		if( pAttr->pFocusBgSurface )
			DrawImage( pAttr->pSurface, pAttr->rect, pAttr->pFocusBgSurface );
		else
			DrawRect( pAttr->pSurface, pAttr->rect, pAttr->focusLineColor, pAttr->focusBgColor, 1 );
	}
	
	if( pAttr->pFont && pAttr->pString )
		DrawString( pAttr->pSurface, pAttr->rect, pAttr->pFont, pAttr->pString, pAttr->fontColor, 1);

	UpdateRect( pAttr->pSurface, pAttr->rect );

	if( pChild )
		pChild->Draw( OBJ_STATUS_NORMAL );
}

int32_t CNX_PushButton::EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent )
{
	int32_t ret = 0, bHit = 0;;

	CNX_BaseObject	*pChild	= GetChild();
	OBJ_ATTRIBUTE	*pAttr	= GetAttrList();
	
	SDL_MouseButtonEvent *pEvent = &(pSdlEvent->button);

	if( !pAttr )
	{
		//NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		ret = -1;
		return ret;
	}

	bHit =	(pAttr->rect.x <= pEvent->x) && 
			(pAttr->rect.y <= pEvent->y) && 
			((pAttr->rect.x + pAttr->rect.w) > pEvent->x) && 
			((pAttr->rect.y + pAttr->rect.h) > pEvent->y);

	switch( pSdlEvent->type )
	{
		case SDL_MOUSEBUTTONDOWN:
			if( bHit ) {
				Draw( OBJ_STATUS_FOCUS );
			}
			break;
		
		case SDL_MOUSEBUTTONUP:
			Draw( OBJ_STATUS_NORMAL );
			if( bHit ) {
				ret = UserAction( pParent );
				return ret;
			}
			break;
		
		default:
			break;
	}

	if( pChild )
		ret = pChild->EventLoop( pParent, pSdlEvent );

	return ret;
}

int32_t CNX_PushButton::UserAction( CNX_BaseObject *pParent )
{
	OBJ_ACTION action;
	OBJ_ACTION_FUNC cbActionFunc;

	GetAction( &action, &cbActionFunc );
	pParent->SetAction( action, cbActionFunc );
	
	return action;
}