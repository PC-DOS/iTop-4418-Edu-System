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

#ifndef __CNX_TEXTBOX_H__
#define __CNX_TEXTBOX_H__

#include "CNX_BaseObject.h"

class CNX_TextBox
	: public CNX_BaseObject
{
public:
	CNX_TextBox();
	virtual ~CNX_TextBox();

public:
	virtual void	Create( OBJ_ATTRIBUTE *pAttr );
	virtual void	Destroy( void );
	
	virtual void	Draw( OBJ_STATUS status );
	
	virtual int32_t	EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent );
	virtual int32_t	UserAction( CNX_BaseObject *pParent );	

public:
	void SetText( char *str, int32_t r, int32_t g, int32_t b );
};

#endif	// __CNX_TEXTBOX_H__