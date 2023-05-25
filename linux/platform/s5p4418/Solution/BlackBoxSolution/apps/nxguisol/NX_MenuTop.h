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

#ifndef __NX_MENUTOP_H__
#define	__NX_MENUTOP_H__

#include <CNX_BaseObject.h>

CNX_BaseObject	*GetMenuTopHandle( SDL_Surface *pSurface, TTF_Font *pFont );
void 			ReleaseMenuTopHandle( CNX_BaseObject *pBaseObject );

#endif	// __NX_MENUTOP_H__
