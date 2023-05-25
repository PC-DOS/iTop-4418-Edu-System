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

#ifndef __CNX_TOGGLEBUTTON_H__
#define __CNX_TOGGLEBUTTON_H__

#include "CNX_BaseObject.h"

class CNX_ToggleButton
	: public CNX_BaseObject
{
public:
	CNX_ToggleButton();
	virtual ~CNX_ToggleButton();

public:
	virtual void	Create( OBJ_ATTRIBUTE *pAttr );
	virtual void	Destroy( void );

	virtual void	Draw( OBJ_STATUS status );
	
	virtual int32_t	EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent );
	virtual int32_t UserAction( CNX_BaseObject *pParent );

public:
	int32_t			SetCurPos( int32_t status );
	int32_t			GetCurPos( void );

private:
	int32_t			m_nTotalStatus;
	int32_t			m_nCurStatus;
};

#endif	// __CNX_TOGGLEBUTTON_H__