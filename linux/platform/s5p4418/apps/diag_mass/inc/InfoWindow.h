#ifndef __INFOWINDOW_H__
#define	__INFOWINDOW_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <pthread.h>

#include <utils.h>
//	nexell diagnostic configuration interface
#include <diag_config.h>

#define	MAX_LINE_BUFFER		255
#define	MAX_LINES			128

typedef struct LINE_BUFFER{
	char *str;
	int size;
	SDL_Color color;
}LINE_BUFFER;

class CInformationWindow
{
public:
	CInformationWindow(SDL_Rect *rect, int fontSize, unsigned int bgColor, SDL_Color fgColor)
	: m_Surface(NULL)
	, m_Font(NULL)
	, m_FontSize(fontSize)
	, m_BackgroundColor(bgColor)
	, m_Lines( 0 )
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

		pthread_mutex_init(&m_hMutex, NULL);

		for(int i=0; i<MAX_LINES ; i++)
		{
			m_MsgBuffer[i].str = NULL;
		}

		Clear();
	}

	virtual ~CInformationWindow()
	{
		Clear();
		if( m_Font )
			TTF_CloseFont( m_Font );
		if( m_Surface )
			SDL_FreeSurface( m_Surface );
		pthread_mutex_destroy(&m_hMutex);
	}

	void Update()
	{
		CNX_AutoLock lock(&m_hMutex);
		int i;
		LINE_BUFFER *lineBuf;
		SDL_Rect rect;

		rect.x = m_Rect.x;
		rect.w = m_Rect.w;
		rect.h = m_FontHeight;
		for( i=0 ; i<m_Lines ; i++ )
		{
			rect.y = i* m_FontHeight + m_Rect.y;
			lineBuf = &m_MsgBuffer[i];
			SDL_FillRect( m_Surface, &rect, m_BackgroundColor );
			DrawString2( m_Surface, m_Font, lineBuf->str, rect, lineBuf->color, 0, 0 );		//	Don't Need Alignment
		}
		SDL_UpdateRect( m_Surface, m_Rect.x, m_Rect.y, m_Rect.w, m_Rect.h);
	}

	bool AddString( const char *str, SDL_Color color )
	{
		CNX_AutoLock lock(&m_hMutex);
		if( m_Lines >= MAX_LINES )
		{
			return false;
		}
		LINE_BUFFER *lineBuf = &m_MsgBuffer[m_Lines];
		if( lineBuf->str )
			free(lineBuf->str);
		lineBuf->str = strdup(str);
		lineBuf->size = strlen(str);
		lineBuf->color = color;
		m_Lines ++;
		return true;
	}

	void Clear()
	{
		CNX_AutoLock lock(&m_hMutex);
		for( int i=0 ; i<m_Lines ;i++ )
		{
			if( m_MsgBuffer[i].str )
			{
				free(m_MsgBuffer[i].str);
				m_MsgBuffer[i].str = NULL;
			}
		}
		SDL_FillRect( m_Surface, &m_Rect, m_BackgroundColor );
		m_Lines = 0;
	}

private:
	SDL_Surface			*m_Surface;
	TTF_Font			*m_Font;
	int					m_FontSize;
	SDL_Rect			m_Rect;
	int					m_FontHeight;
	unsigned int		m_BackgroundColor;
	SDL_Color			m_PenColor;
	NX_DiagConfig		m_DiagCfg;					//	Configuration Util

	//	Message Buffer
	int					m_MaxDspLines;				//	Max Displable Lines
	int					m_Lines;
	struct LINE_BUFFER	m_MsgBuffer[MAX_LINES];

	pthread_mutex_t		m_hMutex;
};

#endif	//	 __INFOWINDOW_H__
