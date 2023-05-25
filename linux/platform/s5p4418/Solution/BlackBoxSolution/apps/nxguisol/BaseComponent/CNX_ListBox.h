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

#ifndef __CNX_LISTBOX_H__
#define __CNX_LISTBOX_H__

#include "CNX_BaseObject.h"

enum {
	SCROLL_NONE		= -1,
	SCROLL_STANDBY	= 0,
	SCROLL_FOCUS	= 1,
	SCROLL_LOWER 	= 2,
	SCROLL_UPPER 	= 3,
};

typedef struct tagListItem {
	uint8_t			*item;
	tagListItem		*pNextItem;
} ListItem;

class CNX_ListBox
	: public CNX_BaseObject
{
public:
	CNX_ListBox();
	virtual ~CNX_ListBox();

public:
	virtual void	Create( OBJ_ATTRIBUTE *pAttr );
	virtual void	Destroy( void );

	virtual void	Draw( OBJ_STATUS status );
	
	virtual int32_t	EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent );
	virtual int32_t UserAction( CNX_BaseObject *pParent );	

public:
	int32_t			AddString( uint8_t *str );
	uint8_t			*GetCurItemString(void);
	uint8_t			*GetPrevItemString(void);
	uint8_t			*GetNextItemString(void);
	
	int32_t			RemoveAll( void );

	void			SetCurItem( int32_t itemNum );
	int32_t			GetCurItem( void );

	void			SetPageItemNum( int32_t pageNum );

	int32_t			GetItemNum( void );
	void			Update( void );
	
private:
	int32_t			CheckScrollHit( SDL_MouseButtonEvent *pEvent );
	int32_t 		CheckItemHit( SDL_MouseButtonEvent *pEvent );
	void			UpdateInfo( void );
	void 			UpdateList( OBJ_STATUS status );

	int32_t			m_nCurItem;
	int32_t			m_nCurPage;
	int32_t			m_nPageItemNum;
	
	int32_t			m_nTotalItemNum;
	int32_t			m_nTotalPageNum;

	int32_t			m_bScroll;

	// drawing
	int32_t			m_nCellWidth;
	int32_t			m_nCellHeight;
	int32_t			m_nScrollWidth;
	int32_t			m_nScrollHeight;
	int32_t			m_nScrollStep;

	int32_t 		m_bHitStatus;

	SDL_Rect		m_ScrollRect;
	int32_t			m_ScrollPrevY;

	ListItem		*m_pListItem;
};

#endif	// __CNX_LISTBOX_H__