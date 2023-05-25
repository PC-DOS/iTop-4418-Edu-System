#ifndef __debug_window_h__
#define	__debug_window_h__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

//	nexell diagnostic configuration interface
#include <diag_config.h>
#include <utils.h>

#define	MAX_LINE_BUFFER		255
#define	MAX_LINES			128

typedef struct LINE_BUFFER{
	char msg[MAX_LINE_BUFFER+1];
	char size;
}LINE_BUFFER;

class DebugWindow
{
public:
	DebugWindow(SDL_Rect *rect, int fontSize)
	: m_Surface(NULL)
	, m_Font(NULL)
	, m_FontSize(fontSize)
	, m_PosX( rect->x )
	, m_PosY( rect->y )
	, m_Lines( 0 ), m_Head( 0 ), m_Tail( 0 )
	{
		const SDL_VideoInfo *info = SDL_GetVideoInfo();
		m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
		m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), fontSize);

		m_Rect.x = rect->x;
		m_Rect.y = rect->y;
		m_Rect.w = rect->w;
		m_Rect.h = rect->h;

		m_FontHeight = TTF_FontHeight( m_Font );
		m_MaxDspLines = (m_Rect.h-m_Rect.y) / m_FontHeight;

		Clear();
	}

	virtual ~DebugWindow()
	{
		if( m_Font )
			TTF_CloseFont( m_Font );
		if( m_Surface )
			SDL_FreeSurface( m_Surface );
	}

	void DrawMessage( const char *msg )
	{
		int len = strlen(msg);

		LINE_BUFFER *lineBuf = &m_MsgBuffer[m_Tail];
		if( len > MAX_LINE_BUFFER ){
			len = MAX_LINE_BUFFER;
			strncpy( lineBuf->msg, msg, len );
		}else
			strcpy( lineBuf->msg, msg );
		lineBuf->size = len;
		m_Lines ++;
		m_Tail = (m_Tail+1)%m_MaxDspLines;

		if( m_Lines > m_MaxDspLines ){
			m_Lines = m_MaxDspLines;
			m_Head = (m_Head+1)%m_MaxDspLines;
			SDL_FillRect( m_Surface, &m_Rect, SDL_MapRGB(m_Surface->format, 0x00, 0x00, 0x00 ) );
		}
		UpdateScreen();
	}

	void Clear()
	{
		m_Tail = m_Head = 0;
		m_Lines = 0;
		SDL_FillRect( m_Surface, &m_Rect, SDL_MapRGB(m_Surface->format, 0x00, 0x00, 0x00 ) );
	}


private:
	void UpdateScreen()
	{
		int i;
		int tmp = m_Head;
		LINE_BUFFER *lineBuf;
		SDL_Rect rect;
		//	Draw Lines & Update Screen
		SDL_Color fgColor = { 0xFF, 0xFF, 0xFF, 0 };

		rect.x = m_Rect.x;
		rect.w = m_Rect.w;
		rect.h = m_FontHeight;
		for( i=0 ; i<m_Lines ; i++ )
		{
			rect.y = i* m_FontHeight + m_Rect.y;
			lineBuf = &m_MsgBuffer[tmp];
			tmp = (tmp+1)%m_MaxDspLines;
			DrawString2( m_Surface, m_Font, lineBuf->msg, rect, fgColor, 0, 0 );		//	Don't Need Alignment
		}
		SDL_Flip( m_Surface );
	}


private:
	SDL_Surface	*m_Surface;
	TTF_Font	*m_Font;
	int			m_FontSize;
	SDL_Rect	m_Rect;
	int			m_FontHeight;
	int			m_PosX, m_PosY;

	NX_DiagConfig		m_DiagCfg;					//	Configuration Util

	//	Message Buffer
	int					m_MaxDspLines;				//	Max Displable Lines
	int					m_Lines;
	struct LINE_BUFFER	m_MsgBuffer[MAX_LINES];
	int					m_Head, m_Tail;
};

#endif	//	 __debug_window_h__
