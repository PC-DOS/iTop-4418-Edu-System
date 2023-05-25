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

#ifndef __CNX_PROGRESSBAR_H__
#define __CNX_PROGRESSBAR_H__

#include "CNX_BaseObject.h"

class CNX_ProgressBar
	: public CNX_BaseObject
{
public:
	CNX_ProgressBar();
	virtual ~CNX_ProgressBar();

public:
	virtual void	Create( OBJ_ATTRIBUTE *pAttr );
	virtual void	Destroy( void );

	virtual void	Draw( OBJ_STATUS status );
	
	virtual int32_t	EventLoop( CNX_BaseObject *pParent, SDL_Event *pSdlEvent );
	virtual int32_t UserAction( CNX_BaseObject *pParent );

public:
	void			Update( void );
	
	int32_t			SetPos( int32_t pos );
	int32_t 		GetPos( void );

	int32_t 		CalHitPos( SDL_MouseButtonEvent *pEvent );
	int32_t 		GetHitPos( void );

private:
	int32_t			m_nCurPos;
	int32_t 		m_nPosRatio;
	int32_t			m_bHitStatus;
};

#endif	// __CNX_PROGRESSBAR_H__
