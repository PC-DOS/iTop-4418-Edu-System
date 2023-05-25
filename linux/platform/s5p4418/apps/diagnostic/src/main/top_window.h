#ifndef __TOP_WINDOW_H__
#define	__TOP_WINDOW_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <nx_diag_type.h>
#include <diag_config.h>

//
typedef struct tagDiagMenuTable {
	SDL_Rect	bg;
	SDL_Rect	app[NUM_APP_IN_SCREEN];
	SDL_Rect	prev;
	SDL_Rect	next;
}DiagMenuTable;

class TopWindow{
public:
	TopWindow( int numPlugin, NXDiagPluginInfo **plguinList );
	virtual ~TopWindow();

public:
	void ShowMemu();
	void EventLoop();
	void UpdateWindow();
	void ExitWindow();
	TTF_Font *GetFont() {	return m_Font;	}
	
private:
	void InitializeButtons();
void ChangeApplicationMap();
	void DrawVersion();
	void ExitMessage();
	void DrawAppButton( NXDiagButton *btn );
	void DrawPageSelButton( NXDiagButton *btn, const char *string );

	void UserAction( SDL_UserEvent *usrEvent );

	//	member variables
private:
	int					m_NumPlugin;
	int					m_CurPage;			//	1 ~ 
	int					m_TotalPage;		//	1 ~ 

	NXDiagPluginInfo	**m_PluginList;
	TTF_Font			*m_Font;
	TTF_Font			*m_TitleFont;
	SDL_Surface			*m_Surface;
	NXDiagButton		*m_BtnList;
	NX_DiagConfig		m_DiagCfg;
};


#endif //__TOP_WINDOW_H__
