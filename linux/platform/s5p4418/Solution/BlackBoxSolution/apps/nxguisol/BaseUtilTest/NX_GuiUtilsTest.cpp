#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <term.h>
#include <curses.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "../BaseComponent/NX_GuiUtils.h"

// kbhit
static struct termios oldTerm, newTerm;
static int chRead;

void SetStdin(void)
{
	tcgetattr(0,&oldTerm);
	newTerm = oldTerm;
	newTerm.c_lflag &= ~(ICANON | ECHO | ISIG);
	newTerm.c_cc[VMIN] = 1;
	newTerm.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &newTerm);

	chRead = -1;
}

void ResetStdin(void)
{
	tcsetattr(0, TCSANOW, &oldTerm);
}

int kbhit( void )
{
	char ch;
	int nRead;

	newTerm.c_cc[VMIN] = 0;
	newTerm.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &newTerm);
	nRead = read(0, &ch, 1);
	newTerm.c_cc[VMIN] = 1;
	newTerm.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &newTerm);
	if( nRead >= 1 )
	{
		chRead = (int) ch;
		return chRead;
	}

	return -1;
}

int main( void )
{
	SDL_Surface *pSurface, *pImgSurface = NULL;
	SDL_Rect	drawRect;
	SDL_Color	fgColor = { 0xFF, 0xFF, 0xFF, 0 };

	TTF_Font *pFont = NULL;

	const SDL_VideoInfo *pInfo;

	TTF_Init();
	pFont = TTF_OpenFont( "/root/DejaVuSansMono.ttf", 18);

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError() );
		exit(1);
	}

	SDL_ShowCursor( false );

	SetStdin();

	pInfo = SDL_GetVideoInfo();
	pSurface = SDL_SetVideoMode( pInfo->current_w, pInfo->current_h, pInfo->vfmt->BitsPerPixel, SDL_SWSURFACE );

	// Draw Image Test
	drawRect.x = 0;
	drawRect.y = 0;
	drawRect.w = 320;
	drawRect.h = 240;

	LoadImage( &pImgSurface, "/root/ray_320x240.jpg" );
	DrawImage( pSurface, drawRect, pImgSurface );
	
	// Draw Circle Test
	DrawCircle( pSurface, 300, 300, 100, SDL_MapRGB(pSurface->format, 0xFF, 0xFF, 0xFF), SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF), 1);

	// Draw Rect Test
	drawRect.x = 300;
	drawRect.y = 600;
	drawRect.w = 300;
	drawRect.h = 50;
	DrawRect( pSurface, drawRect, SDL_MapRGB(pSurface->format, 0xFF, 0xFF, 0xFF), SDL_MapRGB(pSurface->format, 0x00, 0x00, 0xFF), 1);

	// Draw String Test
	drawRect.x = 300;
	drawRect.y = 600;
	drawRect.w = 300;
	drawRect.h = 50;
	DrawString( pSurface, drawRect, pFont, "TEST FONT", fgColor, 1);
	
	UpdateSurface( pSurface );

	while(1)
	{
		char ch = kbhit();
		if( 'q' == ch )
		{
			break;
		}

		usleep(100000);
	}


	ResetStdin();

	SDL_FreeSurface( pImgSurface );
	SDL_FreeSurface( pSurface );
	SDL_Quit();
	
	return 0;
}
