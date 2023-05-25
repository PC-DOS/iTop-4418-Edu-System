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

#include "CNX_ListBox.h"
#include "NX_GuiUtils.h"

#define NX_DTAG "[CNX_ListBox]"
#include "NX_DbgMsg.h"

CNX_ListBox::CNX_ListBox()
	: m_nCurItem( 0 )
	, m_nPageItemNum( 1 )
	, m_nTotalItemNum( 0 )
	, m_nTotalPageNum( 0 )
	, m_bScroll( false )
	, m_nCellWidth( 0 )
	, m_nCellHeight( 0 )
	, m_nScrollWidth( 10 )
	, m_nScrollHeight( 0 )
	, m_ScrollPrevY( -1 )
	, m_pListItem( NULL )
{

}

CNX_ListBox::~CNX_ListBox()
{
	RemoveAll();
	Destroy();
}

void CNX_ListBox::Create( OBJ_ATTRIBUTE *pAttr )
{
	OBJ_ATTRIBUTE *pObjAttr = (OBJ_ATTRIBUTE *)malloc( sizeof(OBJ_ATTRIBUTE) );

	memset( pObjAttr, 0x00, sizeof(OBJ_ATTRIBUTE) );
	memcpy( pObjAttr, pAttr, sizeof(OBJ_ATTRIBUTE) );
	
	SetAttrList( pObjAttr );
}

void CNX_ListBox::Destroy( void )
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

void CNX_ListBox::Draw( OBJ_STATUS status )
{
	CNX_BaseObject *pChild 		= GetChild();

	UpdateList( OBJ_STATUS_NORMAL );

	if( pChild )
		pChild->Draw( OBJ_STATUS_NORMAL );
}

void CNX_ListBox::Update(void)
{
	UpdateList( OBJ_STATUS_NORMAL );
}

void CNX_ListBox::UpdateList( OBJ_STATUS status )
{
	OBJ_ATTRIBUTE	*pAttr		= GetAttrList(); 

	if( !pAttr )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		return;
	}

	// Background 
	if( pAttr->pNormalBgSurface )
		DrawImage( pAttr->pSurface, pAttr->rect, pAttr->pNormalBgSurface );
	else
		DrawRect( pAttr->pSurface, pAttr->rect, pAttr->normalLineColor, pAttr->normalBgColor, 1 );

	if( pAttr->pFont )
	{
		ListItem *pCurItem = m_pListItem;
		
		for(int32_t i = 0; i < m_nCurPage * m_nPageItemNum; i++)
		{
			if( pCurItem )
				pCurItem = pCurItem->pNextItem;
		}
		
		for(int32_t i = 0; i < m_nPageItemNum; i++)
		{
			SDL_Rect itemRect;
			itemRect.x = pAttr->rect.x;
			itemRect.y = pAttr->rect.y + (m_nCellHeight * i) - i;
			itemRect.w = pAttr->rect.w;
			itemRect.h = m_nCellHeight;

			if( i == (m_nCurItem % m_nPageItemNum) )
			{
				DrawRect( pAttr->pSurface, itemRect, pAttr->focusLineColor, pAttr->focusBgColor, 1);
			}
			else
			{
				if( i != m_nPageItemNum - 1)
					DrawRect( pAttr->pSurface, itemRect, pAttr->normalLineColor, pAttr->normalBgColor, 1);
			}
			if( pCurItem )
			{
				DrawString( pAttr->pSurface, itemRect, pAttr->pFont, (const char*)pCurItem->item, pAttr->fontColor, 1);
				pCurItem = pCurItem->pNextItem;
			}
		}
	}
	
	if( m_bScroll )
	{
		DrawRect( pAttr->pSurface, m_ScrollRect, pAttr->focusLineColor, pAttr->focusBgColor, 1);
	}

	UpdateRect( pAttr->pSurface, pAttr->rect );	
}

int32_t CNX_ListBox::EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent )
{
	int32_t ret = 0, nItemHit = 0, nScrollHit = false;
	
	CNX_BaseObject	*pChild	= GetChild();
	OBJ_ATTRIBUTE	*pAttr	= GetAttrList();
	
	SDL_MouseButtonEvent *pEvent = &(pSdlEvent->button);

	if( !pAttr )
	{
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		ret = -1;
		return ret;
	}

	if( SCROLL_NONE == (nScrollHit = CheckScrollHit(pEvent)) )
	{
		nItemHit = CheckItemHit( pEvent );
	}

	switch( pSdlEvent->type )
	{
		case SDL_MOUSEBUTTONDOWN:
			break;

		case SDL_MOUSEBUTTONUP:
			m_ScrollPrevY = -1;

			if( nScrollHit == SCROLL_NONE )
			{
				if( 0 <= nItemHit ) 
				{
					m_nCurItem = m_nCurPage * m_nPageItemNum + nItemHit;
					SetCurItem( m_nCurItem );
					Draw( OBJ_STATUS_NORMAL );
					ret = UserAction( pParent );
					return ret;
				}
			}
			break;

		case SDL_MOUSEMOTION:
			if( nScrollHit == SCROLL_UPPER )
			{
				m_nCurItem -= m_nPageItemNum;
				if( m_nCurItem < 0 )	m_nCurItem = 0;
				SetCurItem( m_nCurItem );
				Draw( OBJ_STATUS_NORMAL );
			}
			else if( nScrollHit == SCROLL_LOWER )
			{
				m_nCurItem += m_nPageItemNum;
				if( m_nCurItem > m_nTotalItemNum - 1 ) m_nCurItem = m_nTotalItemNum - 1;
				SetCurItem( m_nCurItem );
				Draw( OBJ_STATUS_NORMAL );
			}
			break;
		
		default:
			break;
	}

	if( pChild )
		ret = pChild->EventLoop( pParent, pSdlEvent );

	return ret;
}

#define SCROLL_MARGIN_W			10
#define SCROLL_SENSITIVITY_Y	20

int32_t CNX_ListBox::CheckScrollHit( SDL_MouseButtonEvent *pEvent )
{
	if( !m_bScroll ) 
		return SCROLL_NONE;

	if( (m_ScrollRect.x - SCROLL_MARGIN_W <= pEvent->x) && 
		(m_ScrollRect.y <= pEvent->y) && 
		((m_ScrollRect.x + m_ScrollRect.w + SCROLL_MARGIN_W) > pEvent->x) && 
		((m_ScrollRect.y + m_ScrollRect.h) > pEvent->y) )
	{

		if( m_ScrollPrevY < 0 )
		{
			m_ScrollPrevY = pEvent->y;
			return SCROLL_FOCUS;
		}
		else
		{
			if( m_ScrollPrevY + SCROLL_SENSITIVITY_Y < pEvent->y )
			{
				m_ScrollPrevY = pEvent->y;
				return SCROLL_LOWER;
			}
			else if( m_ScrollPrevY - SCROLL_SENSITIVITY_Y > pEvent->y )
			{
				m_ScrollPrevY = pEvent->y;
				return SCROLL_UPPER;
			}
			return SCROLL_STANDBY;
		}
	}

	return SCROLL_NONE;
}

int32_t CNX_ListBox::CheckItemHit( SDL_MouseButtonEvent *pEvent )
{
	OBJ_ATTRIBUTE	*pAttr	= GetAttrList();

	for(int32_t i = 0; i < m_nPageItemNum; i++)
	{
		SDL_Rect itemRect;

		itemRect.x = pAttr->rect.x;
		itemRect.y = pAttr->rect.y + (m_nCellHeight * i) - i;
		itemRect.w = pAttr->rect.w;
		itemRect.h = m_nCellHeight;

		if( (itemRect.x <= pEvent->x) && 
			(itemRect.y <= pEvent->y) && 
			((itemRect.x + itemRect.w) > pEvent->x) && 
			((itemRect.y + itemRect.h) > pEvent->y) )
		{

			if( m_nCurPage * m_nPageItemNum + i > m_nTotalItemNum - 1 )
				return -1;
			else
				return i;
		}
	}

	return -1;
}

int32_t CNX_ListBox::UserAction( CNX_BaseObject *pParent )
{
	OBJ_ACTION action;
	OBJ_ACTION_FUNC cbActionFunc;

	GetAction( &action, &cbActionFunc );
	pParent->SetAction( action, cbActionFunc );
	
	return action;
}

int32_t	CNX_ListBox::AddString( uint8_t *str )
{
	ListItem *pItem = NULL;
	ListItem *pLastItem = NULL;
	
	pItem = (ListItem*)malloc( sizeof(ListItem) );
	pItem->item = str;
	pItem->pNextItem = NULL;

	if( !m_nTotalItemNum )
	{
		m_pListItem = pItem;
	}
	else
	{
		pLastItem = m_pListItem;
		for(int32_t i = 0; i < m_nTotalItemNum - 1; i++)
		{
			pLastItem = pLastItem->pNextItem;
		}

		pLastItem->pNextItem = pItem;
	}
	
	m_nTotalItemNum++;
	//printf("%s():: m_pListItem[%p], pItem[%p], pItem.item = %s\n", __func__, m_pListItem, pItem, pItem->item);

	return 0;
}

int32_t	CNX_ListBox::RemoveAll( void )
{
	ListItem *pCurItem = NULL, *pNextItem = NULL;

	if( !m_pListItem )
		return -1;

	pCurItem = m_pListItem;
	pNextItem = m_pListItem->pNextItem;

	for(int32_t i = 0; i < m_nTotalItemNum; i++)
	{
		free( pCurItem );

		if( pNextItem )
		{
			pCurItem = pNextItem;
			pNextItem = pCurItem->pNextItem;
		}
	}

	m_pListItem = NULL;
	m_nTotalItemNum = 0;

	return 0;
}

uint8_t *CNX_ListBox::GetCurItemString(void)
{
	ListItem *pCurItem = NULL, *pNextItem = NULL;

	if (!m_pListItem)
		return NULL;

	pCurItem = m_pListItem;
	pNextItem = m_pListItem->pNextItem;

	for (int32_t i = 0; i < m_nCurItem; i++)
	{
		if (pNextItem)
		{
			pCurItem = pNextItem;
			pNextItem = pCurItem->pNextItem;
		}
	}
	return pCurItem->item;
}

uint8_t *CNX_ListBox::GetPrevItemString(void)
{
	m_nCurItem--;
	if (m_nCurItem < 0) {
		m_nCurItem = m_nTotalItemNum;
	}
	return GetCurItemString();
}

uint8_t *CNX_ListBox::GetNextItemString(void)
{
	m_nCurItem++;
	if (m_nCurItem > m_nTotalItemNum) {
		m_nCurItem = 0;
	}
	return GetCurItemString();
}

void	CNX_ListBox::SetCurItem( int32_t itemNum )
{
	m_nCurItem = itemNum;
	UpdateInfo();
	UpdateList( OBJ_STATUS_NORMAL );
}

int32_t	CNX_ListBox::GetCurItem( void )
{
	return m_nCurItem;
}

void	CNX_ListBox::SetPageItemNum( int32_t pageNum )
{
	m_nPageItemNum = pageNum;
	UpdateInfo();
	UpdateList( OBJ_STATUS_NORMAL );
}

int32_t CNX_ListBox::GetItemNum( void )
{
	return m_nTotalItemNum;
}

void	CNX_ListBox::UpdateInfo( void )
{
	OBJ_ATTRIBUTE	*pAttr = GetAttrList(); 

	if( !pAttr ){
		NxDbgMsg( NX_DBG_ERR, (TEXT("[%s()] Not matched attribute.\n"), __func__) );
		return;
	}

	m_nCurPage 		= m_nCurItem / m_nPageItemNum;
	m_nTotalPageNum = m_nTotalItemNum / m_nPageItemNum + 1;

	pAttr->rect.h = pAttr->rect.h - (pAttr->rect.h % m_nPageItemNum);
	m_nCellWidth	= pAttr->rect.w;

	if( m_nPageItemNum ) {
		m_nCellHeight	= pAttr->rect.h / m_nPageItemNum;
	}
	else {
		m_nCellHeight 	= 1;
	}

	m_nScrollWidth	= 20;
	m_nScrollHeight	= pAttr->rect.h - 10;

	if( m_nTotalPageNum > 1 ) {
		m_bScroll = true;
	}
	else {
		m_bScroll = false;
		return ;
	}

	int32_t scrollStartX 	= (pAttr->rect.x + pAttr->rect.w) - m_nScrollWidth - (m_nScrollWidth / 2);
	int32_t scrollStartY 	= (pAttr->rect.y + 5);
	int32_t scrollEndX		= (pAttr->rect.x + pAttr->rect.w) - m_nScrollWidth + (m_nScrollWidth / 2);
	int32_t scrollEndY		= (pAttr->rect.y + pAttr->rect.h) - 10;
	int32_t scrollStep 		= ((scrollEndY - scrollStartY) / 3) / (m_nTotalPageNum - ((m_nTotalItemNum % m_nPageItemNum) ? 1 : 2));

	int32_t scrollX			= scrollStartX;
	int32_t scrollY			= scrollStartY + (scrollStep * m_nCurPage);

	int32_t scrollW 		= scrollEndX - scrollStartX;
	int32_t scrollH 		= (scrollEndY - scrollStartY) * 2 / 3;

	m_ScrollRect.x = scrollX;
	m_ScrollRect.y = scrollY;

	m_ScrollRect.w = scrollW;
	m_ScrollRect.h = scrollH;
}