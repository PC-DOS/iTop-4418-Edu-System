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

#ifndef __CNX_BASEOBJECT_H__
#define __CNX_BASEOBJECT_H__

#include <stdint.h>
#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

typedef enum {
	OBJ_STATUS_NORMAL,
	OBJ_STATUS_FOCUS,
	OBJ_STATUS_SELECT,
	OBJ_STATUS_DISABLE,
} OBJ_STATUS;

typedef enum {
	OBJ_ACTION_NONE,
	OBJ_ACTION_WINDOW,
	OBJ_ACTION_EXEC,
	OBJ_ACTION_EXIT,
} OBJ_ACTION;

typedef void (*OBJ_ACTION_FUNC)( void );

typedef struct tag_OBJ_ATTRIBUTE {
	SDL_Surface			*pSurface;
	SDL_Rect			rect;
	OBJ_STATUS			status;

	SDL_Surface			*pNormalBgSurface;
	uint32_t 			normalBgColor;
	uint32_t			normalLineColor;

	SDL_Surface			*pFocusBgSurface;
	uint32_t			focusBgColor;
	uint32_t			focusLineColor;
	
	SDL_Surface			*pSelectBgSurface;
	uint32_t			selectBgColor;
	uint32_t			selectLineColor;

	TTF_Font			*pFont;
	const char			*pString;
	SDL_Color 			fontColor;

	tag_OBJ_ATTRIBUTE	*pAttrList;
} OBJ_ATTRIBUTE;


class CNX_BaseObject
{
public:
	CNX_BaseObject()
		: m_pChild( NULL )
		, m_pAttrList( NULL )
		, m_nAttributeNum( 0 )
		, m_Action( OBJ_ACTION_NONE )
		, m_cbActionFunc( NULL )
	{
	}
	virtual ~CNX_BaseObject(){}

public:
	virtual void Create( OBJ_ATTRIBUTE *pAttr ){}
	virtual void Destroy( void ){}

	virtual void Draw( OBJ_STATUS status ){}
	virtual int32_t EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent ){ return 0; }
	virtual int32_t UserAction( CNX_BaseObject *pParent  ){ return 0; }

	virtual void SetChild( CNX_BaseObject *pObj )
	{
		m_pChild = pObj;
	}
	virtual CNX_BaseObject *GetChild( void  )
	{
		return m_pChild;
	}
	virtual void SetAttrList( OBJ_ATTRIBUTE *pAttr )
	{
		if( !m_nAttributeNum ) {
			m_pAttrList = pAttr;	
		}
		else {
			OBJ_ATTRIBUTE *pList = m_pAttrList;	
			for(int32_t i = 0; i < m_nAttributeNum - 1; i++)
			{
				pList = pList->pAttrList;
			}
			pList->pAttrList = pAttr;
		}
		m_nAttributeNum++;
	}
	virtual OBJ_ATTRIBUTE *GetAttrList( void )
	{
		return m_pAttrList;
	}

	virtual void SetAction( OBJ_ACTION action, OBJ_ACTION_FUNC func)
	{
		m_Action = action;
		m_cbActionFunc = func;
	}
	virtual void GetAction( OBJ_ACTION *pAction, OBJ_ACTION_FUNC *func )
	{
		*pAction = m_Action;
		*func = m_cbActionFunc;
	}

private:
	// CNX_BaseObject		*m_pParent;
	CNX_BaseObject		*m_pChild;

	OBJ_ATTRIBUTE		*m_pAttrList;
	int32_t				m_nAttributeNum;
	
	OBJ_ACTION			m_Action;
	OBJ_ACTION_FUNC		m_cbActionFunc;
};

#endif	// __CNX_BASEOBJECT_H__