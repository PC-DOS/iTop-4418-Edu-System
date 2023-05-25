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

#include "NX_GuiUtils.h"

static void DrawPixel( SDL_Surface *pSurface, uint32_t x, uint32_t y, uint32_t color )
{
	uint32_t bpp, offset;

	bpp		= pSurface->format->BytesPerPixel;
	offset	= pSurface->pitch * y + x * bpp;

	SDL_LockSurface( pSurface );
	memcpy( (void*)((int)pSurface->pixels + offset), &color, bpp );
	SDL_UnlockSurface( pSurface );
}

#define SGN(x) ((x)>0 ? 1 : ((x)==0 ? 0:(-1)))
#define ABS(x) ((x)>0 ? (x) : (-x))

// Basic unantialiased Bresenham line algorithm
static void BresenhamLine( SDL_Surface *pSurface, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color )
{
	int32_t  lg_delta, sh_delta, cycle, lg_step, sh_step;

	lg_delta	= x2 - x1;
	sh_delta	= y2 - y1;
	lg_step		= SGN( lg_delta );
	lg_delta	= ABS( lg_delta );
	sh_step		= SGN( sh_delta );
	sh_delta	= ABS( sh_delta );
	if( sh_delta < lg_delta )
	{
		cycle = lg_delta >> 1;
		while( x1 != x2 )
		{
			DrawPixel( pSurface, x1, y1, color );
			cycle += sh_delta;
			if (cycle > lg_delta)
			{
				cycle -= lg_delta;
				y1 += sh_step;
			}
			x1 += lg_step;
		}
		DrawPixel( pSurface, x1, y1, color );
	}
	cycle = sh_delta >> 1;
	while( y1 != y2 )
	{
		DrawPixel( pSurface, x1, y1, color );
		cycle += lg_delta;
		if( cycle > sh_delta )
		{
			cycle -= sh_delta;
			x1 += lg_step;
		}
		y1 += sh_step;
	}
	DrawPixel( pSurface, x1, y1, color );
}

// Basic unantialiased Bresenham circle algorithm
static void BresenhamCircle( SDL_Surface *pSurface, uint32_t x, uint32_t y, uint32_t radius, uint32_t lineColor, uint32_t bgColor, uint32_t bFill )
{
	int32_t start_x, start_y;
	int32_t finder;

	start_x	= 0;
	start_y	= radius;
	finder	= 3 - 2 * radius;

	while( start_x <= start_y )
	{
		if( bFill )
		{
			BresenhamLine( pSurface, x + start_x - 1, y + start_y - 1, x + start_x - 1, y - start_y + 1, bgColor );
			BresenhamLine( pSurface, x - start_x + 1, y + start_y - 1, x - start_x + 1, y - start_y + 1, bgColor );
			BresenhamLine( pSurface, x + start_y - 1, y + start_x - 1, x + start_y - 1, y - start_x + 1, bgColor );
			BresenhamLine( pSurface, x - start_y + 1, y + start_x - 1, x - start_y + 1, y - start_x + 1, bgColor );
		}

		DrawPixel( pSurface, x + start_x, y + start_y, lineColor);
		DrawPixel( pSurface, x + start_x, y - start_y, lineColor);
		DrawPixel( pSurface, x - start_x, y + start_y, lineColor);
		DrawPixel( pSurface, x - start_x, y - start_y, lineColor);
		DrawPixel( pSurface, x + start_y, y + start_x, lineColor);
		DrawPixel( pSurface, x + start_y, y - start_x, lineColor);
		DrawPixel( pSurface, x - start_y, y + start_x, lineColor);
		DrawPixel( pSurface, x - start_y, y - start_x, lineColor);

        if(finder < 0)
        {
            finder += 4 * start_x + 6;
            start_x++;
        }
        else
        {
            finder += 4 * (start_x - start_y) + 10;
            start_x++;
            start_y--;
        }
	}
}

void DrawLine( SDL_Surface *pSurface, int32_t startX, int32_t startY, int32_t endX, int32_t endY, uint32_t lineColor )
{
	SDL_Rect rect;

	if( startX < endX )
	{
		rect.x = startX;
		rect.w = endX - startX;
	}
	else
	{
		rect.x = endX;
		rect.w = startX - endX;
	}
	if( startY < endY )
	{
		rect.y = startY;
		rect.h = endY - startY;
	}
	else
	{
		rect.y = endY;
		rect.h = startY - endY;
	}

	BresenhamLine( pSurface, rect.x, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1, lineColor );
}

void DrawRect( SDL_Surface *pSurface, SDL_Rect rect, uint32_t lineColor, uint32_t bgColor, uint32_t bFill )
{
	if( bFill )
		SDL_FillRect( pSurface, &rect, bgColor );

	BresenhamLine( pSurface, rect.x             , rect.y             , rect.x + rect.w - 1, rect.y             , lineColor );
	BresenhamLine( pSurface, rect.x             , rect.y + rect.h - 1, rect.x + rect.w - 1, rect.y + rect.h - 1, lineColor );
	BresenhamLine( pSurface, rect.x             , rect.y             , rect.x             , rect.y + rect.h - 1, lineColor );
	BresenhamLine( pSurface, rect.x + rect.w - 1, rect.y             , rect.x + rect.w - 1, rect.y + rect.h - 1, lineColor );
}

void DrawCircle( SDL_Surface *pSurface, int32_t x, int32_t y, int32_t radius, uint32_t lineColor, uint32_t bgColor, uint32_t bFill )
{
	BresenhamCircle( pSurface, x, y, radius, lineColor, bgColor, bFill );
}

void LoadImage( SDL_Surface **ppImgSurface, const char *imgPath )
{
	SDL_Surface *pImgSurface = IMG_Load( imgPath );
	*ppImgSurface = SDL_DisplayFormat( pImgSurface );
	SDL_FreeSurface( pImgSurface );
}

void DrawImage( SDL_Surface *pSurface, SDL_Rect rect, SDL_Surface *pImgSurface )
{
	rect.w = pImgSurface->w;
	rect.h = pImgSurface->h;

	SDL_BlitSurface( pImgSurface, NULL, pSurface, &rect);
}

void DrawString( SDL_Surface *pSurface, SDL_Rect rect, TTF_Font *pFont, const char *pString, SDL_Color fgColor, uint32_t bCenter )
{
	SDL_Surface *pTextSurface =  TTF_RenderText_Blended( pFont, pString, fgColor );
	
	if( bCenter )
	{
		int off_x = 0, off_y = 0;
		if( rect.w > pTextSurface->w )	off_x = (rect.w - pTextSurface->w) >> 1;
		if( rect.h > pTextSurface->h )	off_y = (rect.h - pTextSurface->h) >> 1;
		rect.x += off_x;
		rect.y += off_y;
	}

	SDL_BlitSurface( pTextSurface, NULL, pSurface, &rect );
}

void UpdateSurface( SDL_Surface *pSurface )
{
	SDL_Flip( pSurface );
}

void UpdateRect( SDL_Surface *pSurface, SDL_Rect rect )
{
	SDL_UpdateRect( pSurface, rect.x, rect.y, rect.w + 1, rect.h + 1 );
}
