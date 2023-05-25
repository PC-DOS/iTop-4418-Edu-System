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

#include "CNX_TextBox.h"
#include "NX_GuiUtils.h"

#define NX_DTAG "[CNX_TextBox]"
#include "NX_DbgMsg.h"

CNX_TextBox::CNX_TextBox()
{

}

CNX_TextBox::~CNX_TextBox()
{
	Destroy();
}

void CNX_TextBox::Create( OBJ_ATTRIBUTE *pAttr )
{
	OBJ_ATTRIBUTE *pObjAttr = (OBJ_ATTRIBUTE *)malloc( sizeof(OBJ_ATTRIBUTE) );

	memset( pObjAttr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	memcpy( pObjAttr, pAttr, sizeof(OBJ_ATTRIBUTE) );

	SetAttrList( pObjAttr );
}

void CNX_TextBox::Destroy( void )
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

void CNX_TextBox::Draw( OBJ_STATUS status )
{
	CNX_BaseObject 	*pChild		= GetChild();
	OBJ_ATTRIBUTE	*pAttr		= GetAttrList(); 

	if( !pAttr )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		return;
	}

	if( pAttr->pFont && pAttr->pString )
		DrawString( pAttr->pSurface, pAttr->rect, pAttr->pFont, pAttr->pString, pAttr->fontColor, 1);

	UpdateRect( pAttr->pSurface, pAttr->rect );

	if( pChild )
		pChild->Draw( OBJ_STATUS_NORMAL );
}

int32_t CNX_TextBox::EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent )
{
	int32_t ret = 0;
	CNX_BaseObject	*pChild	= GetChild();

	if( pChild )
		ret = pChild->EventLoop( pParent, pSdlEvent );

	return ret;
}

int32_t CNX_TextBox::UserAction( CNX_BaseObject *pParent )
{
	return true;
}

void CNX_TextBox::SetText( char *str,  int32_t r, int32_t g, int32_t b )
{
	OBJ_ATTRIBUTE	*pAttr		= GetAttrList();

	if( !pAttr )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		return ;
	}

	if( str )
		pAttr->pString = str;
	
	DrawRect( pAttr->pSurface, pAttr->rect, SDL_MapRGB( pAttr->pSurface->format, r, g, b ), SDL_MapRGB( pAttr->pSurface->format, r, g, b ), 1 );

	if(pAttr->pFont && pAttr->pString)
		DrawString( pAttr->pSurface, pAttr->rect, pAttr->pFont, pAttr->pString, pAttr->fontColor, 1 );

	UpdateRect( pAttr->pSurface, pAttr->rect );
}