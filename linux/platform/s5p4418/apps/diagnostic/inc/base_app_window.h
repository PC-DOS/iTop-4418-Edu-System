#ifndef __BASE_APP_WINDOW_H__
#define	__BASE_APP_WINDOW_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <linux/fb.h>

#include <nx_diag_type.h>
#include <diag_config.h>

class BaseAppWnd
{
public:
	BaseAppWnd()
		: m_Surface(NULL)
		, m_Font(NULL)
		, m_BtnList(NULL)
	{
	}
	virtual ~BaseAppWnd(){}

	virtual void Initialize() = 0;
	virtual int EventLoop( int lastResult ) = 0;
	virtual void UpdateWindow() = 0;

	SDL_Surface		*m_Surface;
	TTF_Font		*m_Font;
	NXDiagButton	*m_BtnList;

	NX_DiagConfig	m_DiagCfg;
};

#endif //	__BASE_APP_WINDOW_H__
