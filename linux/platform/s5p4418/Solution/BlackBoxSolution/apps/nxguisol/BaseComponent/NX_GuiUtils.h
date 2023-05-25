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

#ifndef __NX_GUIUTILS_H__
#define __NX_GUIUTILS_H__

#include <stdint.h>
#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

// Drawing Shape
void DrawLine	( SDL_Surface *pSurface, int32_t startX, int32_t startY, int32_t endX, int32_t endY, uint32_t lineColor );
void DrawRect	( SDL_Surface *pSurface, SDL_Rect rect, uint32_t lineColor, uint32_t bg_color = 0, uint32_t bFill = 0 );
void DrawCircle	( SDL_Surface *pSurface, int32_t x, int32_t y, int32_t radius, uint32_t lineColor, uint32_t bgColor = 0, uint32_t bFill = 0 );

// Drawing Image
void LoadImage	( SDL_Surface **ppImgSurface, const char *imgPath );
void DrawImage	( SDL_Surface *pSurface, SDL_Rect rect, SDL_Surface *pImgSurface );

void DrawString( SDL_Surface *pSurface, SDL_Rect rect, TTF_Font *pFont, const char *pString, SDL_Color fgColor, uint32_t bCenter = 0);

// Surface Refresh
void UpdateSurface	( SDL_Surface *pSurface );
void UpdateRect		( SDL_Surface *pSurface, SDL_Rect rect );

#endif // __NX_GUIUTILS_H__