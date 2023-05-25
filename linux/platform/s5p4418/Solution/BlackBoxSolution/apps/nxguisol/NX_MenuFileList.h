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

#ifndef __NX_MENUFILELIST_H__
#define	__NX_MENUFILELIST_H__

#include <CNX_BaseObject.h>

CNX_BaseObject	*GetMenuFileListHandle( SDL_Surface *pSurface, TTF_Font *pFont );
void 			ReleaseMenuFileListHandle( CNX_BaseObject *pBaseWindow );

void 			FileListNormal(void);
void			FileListEvent(void);

#endif	// __NX_MENUFILELIST_H__

