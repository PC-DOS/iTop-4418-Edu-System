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

#include "CNX_ToggleButton.h"
#include "NX_GuiUtils.h"

#define NX_DTAG	"[CNX_ToggleButton]"
#include "NX_DbgMsg.h"

CNX_ToggleButton::CNX_ToggleButton()
	: m_nTotalStatus( 0 )
	, m_nCurStatus( 0 )
{

}

CNX_ToggleButton::~CNX_ToggleButton()
{
	Destroy();
}

void CNX_ToggleButton::Create( OBJ_ATTRIBUTE *pAttr )
{
	OBJ_ATTRIBUTE *pObjAttr = (OBJ_ATTRIBUTE *)malloc( sizeof(OBJ_ATTRIBUTE) );

	memset( pObjAttr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	memcpy( pObjAttr, pAttr, sizeof(OBJ_ATTRIBUTE) );
	
	if( pAttr->status == OBJ_STATUS_NORMAL )
		m_nTotalStatus++;

	SetAttrList( pObjAttr );	
}

void CNX_ToggleButton::Destroy( void )
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

void CNX_ToggleButton::Draw( OBJ_STATUS status )
{
	CNX_BaseObject	*pChild 	= GetChild();
	OBJ_ATTRIBUTE 	*pAttr		= GetAttrList(); 

	for( int32_t i = 0; i < m_nCurStatus; i++)
	{
		pAttr = pAttr->pAttrList;
	}

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

int32_t CNX_ToggleButton::EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent )
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
			if( bHit ) {
				m_nCurStatus++;
				m_nCurStatus %= m_nTotalStatus;
				ret = UserAction( pParent );
				Draw( OBJ_STATUS_NORMAL );
				return ret;
			}
			Draw( OBJ_STATUS_NORMAL );
			
			break;
		
		default:
			break;
	}

	if( pChild )
		ret = pChild->EventLoop( pParent, pSdlEvent );

	return ret;
}

int32_t CNX_ToggleButton::UserAction( CNX_BaseObject *pParent )
{
	OBJ_ACTION action;
	OBJ_ACTION_FUNC cbActionFunc;

	GetAction( &action, &cbActionFunc );
	pParent->SetAction( action, cbActionFunc );
	
	return action;
}

int32_t CNX_ToggleButton::SetCurPos( int32_t curStatus )
{
	if( curStatus > m_nTotalStatus )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Status Over-range.(curStatus = %d, totalStatus = %d)\n"), __func__, curStatus, m_nTotalStatus) );
		return -1;
	}

	m_nCurStatus = curStatus;

	return 0;
}

int32_t CNX_ToggleButton::GetCurPos( void )
{
	return m_nCurStatus;
}