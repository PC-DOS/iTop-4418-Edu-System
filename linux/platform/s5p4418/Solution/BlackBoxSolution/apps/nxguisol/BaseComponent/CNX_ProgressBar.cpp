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

#include "CNX_ProgressBar.h"
#include "NX_GuiUtils.h"

#define NX_DTAG	"[CNX_ProgressBar]"
#include "NX_DbgMsg.h"

CNX_ProgressBar::CNX_ProgressBar()
	: m_nCurPos( 0 )
	, m_nPosRatio( 0 )
	, m_bHitStatus( false )
{

}

CNX_ProgressBar::~CNX_ProgressBar()
{
	Destroy();
}

void CNX_ProgressBar::Create( OBJ_ATTRIBUTE *pAttr )
{
	OBJ_ATTRIBUTE *pObjAttr = (OBJ_ATTRIBUTE *)malloc( sizeof(OBJ_ATTRIBUTE) );

	memset( pObjAttr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	memcpy( pObjAttr, pAttr, sizeof(OBJ_ATTRIBUTE) );
	
	SetAttrList( pObjAttr );	
}

void CNX_ProgressBar::Destroy( void )
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

void CNX_ProgressBar::Draw( OBJ_STATUS status )
{
	CNX_BaseObject	*pChild 	= GetChild();

	Update();

	if( pChild )
		pChild->Draw( OBJ_STATUS_NORMAL );
}

void CNX_ProgressBar::Update( void )
{
	OBJ_ATTRIBUTE	*pAttr		= GetAttrList(); 
	OBJ_ATTRIBUTE	*pAttrBar	= NULL;

	SDL_Rect 		barRect;

	if( pAttr )
		pAttrBar = pAttr->pAttrList;

	if( !pAttr || !pAttrBar ) {
		return;
	}
	
	barRect.x = pAttr->rect.x + 1;
	barRect.y = pAttr->rect.y + 1;
	barRect.h = pAttr->rect.h - 2;

	if( m_nCurPos <= 0)
	{
		barRect.w = 0;
	}
	else if( m_nCurPos >= 100 )
	{
		barRect.w = pAttr->rect.w - 2;
	}
	else 
	{
		barRect.w = (pAttr->rect.w * m_nCurPos / 100) - 2;
	}

	DrawRect( pAttr->pSurface, pAttr->rect, pAttr->normalLineColor, pAttr->normalBgColor, 1 );
	if( barRect.w > 0)
	{
		DrawRect( pAttrBar->pSurface, barRect, pAttrBar->normalLineColor, pAttrBar->normalBgColor, 1);
	}
	
	UpdateRect( pAttr->pSurface, pAttr->rect );	
}

int32_t CNX_ProgressBar::EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent )
{
	int32_t ret = 0, bHit = 0;

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
			if( bHit ) m_bHitStatus = true;
			break;
		
		case SDL_MOUSEBUTTONUP:
			if( bHit && !m_bHitStatus)
			{
				CalHitPos( pEvent );
				ret = UserAction( pParent );
				return ret;
			}
					
			if( m_bHitStatus )
			{
				ret = UserAction( pParent );
				CalHitPos( pEvent );
				m_nCurPos = GetHitPos( );
				Update( );
				m_bHitStatus = false;
				return ret;
			}

			break;

		//case SDL_MOUSEMOTION:
		//	if( m_bHitStatus )
		//	{
		//		CalHitPos( pEvent );
		//		m_nCurPos = GetHitPos( );
		//		Update( );
		//	}
		//	break;		

		default:
			break;
	}

	if( pChild )
		ret = pChild->EventLoop( pParent, pSdlEvent );

	return ret;
}

int32_t CNX_ProgressBar::UserAction( CNX_BaseObject *pParent )
{
	OBJ_ACTION action;
	OBJ_ACTION_FUNC cbActionFunc;

	GetAction( &action, &cbActionFunc );
	pParent->SetAction( action, cbActionFunc );

	return action;
}

int32_t CNX_ProgressBar::SetPos( int32_t pos )
{
	m_nCurPos = pos;
	Update();

	return 0;
}

int32_t CNX_ProgressBar::GetPos( void )
{
	return m_nCurPos;
}

int32_t CNX_ProgressBar::CalHitPos( SDL_MouseButtonEvent *pEvent )
{
	OBJ_ATTRIBUTE	*pAttr = GetAttrList(); 

	if( !pAttr ) {
		return -1;
	}
	
	m_nPosRatio = (int32_t)( (float)(pEvent->x - pAttr->rect.x) / (float)(pAttr->rect.w) * 100. + .5);

	return 0;
}

int32_t CNX_ProgressBar::GetHitPos( void )
{
	return m_nPosRatio;
}
