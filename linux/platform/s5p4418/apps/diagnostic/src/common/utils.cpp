#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "nx_diag_type.h"

//////////////////////////////////////////////////////////////////////////////
//
//							Draw Functions
//

static void Draw_pixel(SDL_Surface *surface, Uint32 x, Uint32 y, Uint32 color)
{
	Uint32 bpp, ofs;

	bpp = surface->format->BytesPerPixel;
	ofs = surface->pitch*y + x*bpp;

	SDL_LockSurface(surface);
	memcpy((void*)((int)surface->pixels + ofs), &color, bpp);
	SDL_UnlockSurface(surface);
}

#define SGN(x) ((x)>0 ? 1 : ((x)==0 ? 0:(-1)))
#define ABS(x) ((x)>0 ? (x) : (-x))


/* Basic unantialiased Bresenham line algorithm */
static void bresenham_line(SDL_Surface *surface, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2, Uint32 color)
{
	int lg_delta, sh_delta, cycle, lg_step, sh_step;

	lg_delta = x2 - x1;
	sh_delta = y2 - y1;
	lg_step = SGN(lg_delta);
	lg_delta = ABS(lg_delta);
	sh_step = SGN(sh_delta);
	sh_delta = ABS(sh_delta);
	if (sh_delta < lg_delta) {
		cycle = lg_delta >> 1;
		while (x1 != x2) {
			Draw_pixel(surface, x1, y1, color);
			cycle += sh_delta;
			if (cycle > lg_delta) {
				cycle -= lg_delta;
				y1 += sh_step;
			}
			x1 += lg_step;
		}
		Draw_pixel(surface, x1, y1, color);
	}
	cycle = sh_delta >> 1;
	while (y1 != y2) {
		Draw_pixel(surface, x1, y1, color);
		cycle += lg_delta;
		if (cycle > sh_delta) {
			cycle -= sh_delta;
			x1 += lg_step;
		}
		y1 += sh_step;
	}
	Draw_pixel(surface, x1, y1, color);
}

void DrawButton( SDL_Surface *surface, SDL_Rect *rect, Uint32 color )
{
	Uint32 line_color = SDL_MapRGB(surface->format, 0x20, 0x20, 0x20);
	SDL_FillRect( surface, rect, color );
	bresenham_line( surface, rect->x          , rect->y          , rect->x+rect->w-1, rect->y          , line_color );
	bresenham_line( surface, rect->x          , rect->y+rect->h-1, rect->x+rect->w-1, rect->y+rect->h-1, line_color );
	bresenham_line( surface, rect->x          , rect->y          , rect->x          , rect->y+rect->h-1, line_color );
	bresenham_line( surface, rect->x+rect->w-1, rect->y          , rect->x+rect->w-1, rect->y+rect->h-1, line_color );
}

void DrawLine( SDL_Surface *surface, int startX, int startY, int endX, int endY, Uint32 color )
{
	int x, y, w, h;
	if( startX > endX ){
		x = endX;
		w = startX - endX;
	}else{
		x = startX;
		w = endX - startX;
	}

	if( startY > endY ){
		y = endY;
		h = startY - endY;
	}else{
		y = startY;
		h = endY - startY;
	}

	bresenham_line( surface, startX, startY, endX, endY, color );
	SDL_UpdateRect( surface, x, y, w+1, h+1 );
	SDL_Flip( surface );
}

void SetLine( SDL_Surface *surface, int startX, int startY, int endX, int endY, Uint32 color )
{
	bresenham_line( surface, startX, startY, endX, endY, color );
}

void DrawString( SDL_Surface *surface, TTF_Font *font, const char *string, SDL_Rect rect )
{
	//	SDL_Color foregroundColor = { 0x00, 0x00, 0x00, 0 };
	SDL_Color foregroundColor = { 0xff, 0xff, 0xff, 0 };
	SDL_Surface* text =  TTF_RenderText_Blended(font, string, foregroundColor);

	//	Draw Center
	{
		int off_x=0, off_y=0;
		if( rect.w > text->w )	off_x = (rect.w-text->w)>>1;
		if( rect.h > text->h )	off_y = (rect.h-text->h)>>1;
		rect.x += off_x;
		rect.y += off_y;
	}

	SDL_BlitSurface(text, NULL, surface, &rect);
	SDL_FreeSurface(text);
}

void DrawString2( SDL_Surface *surface, TTF_Font *font, const char *string, SDL_Rect rect, SDL_Color fgColor, int x_center, int y_center )
{
	SDL_Surface* text =  TTF_RenderText_Blended(font, string, fgColor);

	//	Draw Center
	if (x_center)
	{
		int off_x=0;
		if( rect.w > text->w )	off_x = (rect.w-text->w)>>1;
		rect.x += off_x;
	}

	if( y_center )
	{
		int off_y=0;
		if( rect.h > text->h )	off_y = (rect.h-text->h)>>1;
		rect.y += off_y;
	}

	SDL_BlitSurface(text, NULL, surface, &rect);
	SDL_FreeSurface(text);
}


//
//
//

NXDiagButton *CreateButton( SDL_Rect *rect, BUTTON_STATE state, int action, void *priv )
{
	NXDiagButton *btn = (NXDiagButton *)malloc(sizeof(NXDiagButton));

	btn->state = state;
	btn->action = action;
	btn->rect.x = rect->x;
	btn->rect.y = rect->y; 
	btn->rect.w = rect->w;
	btn->rect.h = rect->h;
	btn->priv = priv;
	btn->plugin = NULL;
	btn->next = NULL;
	return btn;
}

void DestroyButton( NXDiagButton *btn )
{
	if( btn ){
		free( btn );
	}
}
