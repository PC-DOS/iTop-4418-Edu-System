#include <dlfcn.h>
#include <top_window.h>
#include <utils.h>
#include <base_app_window.h>

#include <build_info_s5p4418_diag.h>

//	Designed Diag Pannel
#if 0	//	800x480 UI
static DiagMenuTable st_Diag800x480Menu = {
	{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT},								//	Back ground
	{
		{10,  80, 385, 80}, {405,  80, 385, 80},
		{10, 180, 385, 80}, {405, 180, 385, 80},
		{10, 280, 385, 80}, {405, 280, 385, 80}
	},												//	Apps
	{10, 380, 385, 90},	{405, 380, 385, 90}			//	Next
};
#endif

#if 1	//	1024x600 UI
static DiagMenuTable st_Diag1024x600Menu = {
	{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT},								//	Back ground
	{
		{10, 100, 490, 100}, {520, 100, 490, 100},
		{10, 220, 490, 100}, {520, 220, 490, 100},
		{10, 340, 490, 100}, {520, 340, 490, 100}
	},												//	Apps
	{10, 480, 490, 110},	{520, 480, 490, 110}			//	Next
};
#endif

#define TITLE_FONT_SIZE_OFFSET	6
#define	EN_TITLE_FONT_BOLD		0


TopWindow::TopWindow( int numPlugin, NXDiagPluginInfo **puginList )
	: m_NumPlugin(numPlugin)
	, m_CurPage(1)
	, m_PluginList(puginList)
	, m_Font(NULL)
	, m_TitleFont(NULL)
	, m_Surface(NULL)
	, m_BtnList(NULL)
{
	//	Initialize Total Application Pages
	m_TotalPage = (numPlugin+NUM_APP_IN_SCREEN-1)/NUM_APP_IN_SCREEN;
	
	TTF_Init();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_TitleFont = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize()+TITLE_FONT_SIZE_OFFSET);
	if( EN_TITLE_FONT_BOLD )
		TTF_SetFontStyle( m_TitleFont, TTF_GetFontStyle(m_TitleFont)|TTF_STYLE_BOLD );

	//	Initialize SDL
	if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 )
	{
		printf("Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}

	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );

	InitializeButtons();
	UpdateWindow();
}

TopWindow::~TopWindow()
{
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_TitleFont )
		TTF_CloseFont( m_TitleFont );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
	if( m_BtnList )
	{
		//	delete all buttons
		NXDiagButton *btn = m_BtnList;
		NXDiagButton *next = NULL;
		while( btn )
		{
			next = btn->next;
			DestroyButton( btn );
			btn = next;
		}
	}
	SDL_Quit();
}


void TopWindow::EventLoop()
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed  = 0;
	int update = 0;
	BUTTON_STATE next_state;
	while( !bExit )
	{
		hit_test = 0;
		processed   = 0;

		SDL_MouseButtonEvent *event_btn = &sdl_event.button;
		SDL_WaitEvent( &sdl_event );
		for( btn = m_BtnList; 0!=btn&&0==processed; btn = btn->next )
		{
			if( btn->state == BUTTON_DISABLED ) continue;
			hit_test =	(btn->rect.x<=event_btn->x) &&
						(btn->rect.y<=event_btn->y) &&
						((btn->rect.x+btn->rect.w)>event_btn->x) &&
						((btn->rect.y+btn->rect.h)>event_btn->y) ;

			next_state = btn->state;
			switch( sdl_event.type )
			{
				case SDL_MOUSEBUTTONDOWN:
					if( BUTTON_NORMAL == btn->state && hit_test ){
						processed = 1;
						next_state = BUTTON_FOCUS_IN;
					}
					break;
				case SDL_MOUSEMOTION:
					if( BUTTON_FOCUS_OUT == btn->state || BUTTON_FOCUS_IN == btn->state )
					{
						if( hit_test ) next_state = BUTTON_FOCUS_IN;
						else           next_state = BUTTON_FOCUS_OUT;
						processed = 1;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if( BUTTON_FOCUS_OUT == btn->state || BUTTON_FOCUS_IN == btn->state ){
						next_state = BUTTON_NORMAL;
						processed = 1;
						//	send event
						if( hit_test && BTN_ACT_NONE != btn->action )
						{
							// send message
							SDL_Event evt;
							evt.type = SDL_USEREVENT;
							evt.user.type = SDL_USEREVENT;
							evt.user.code = btn->action;
							evt.user.data1 = btn;
							//evt.user.data2 = btn->owner;
							SDL_PushEvent(&evt);
						}
					}
					break;
				case SDL_KEYDOWN:
//					printf("SDL_KEYdowni %d",sdl_event.key.keysym.sym);
					if( SDLK_HOME == sdl_event.key.keysym.sym ){
						bExit = true;
						processed = 1;
					}
					break;
			}

			if( btn->state != next_state )	update = 1;
			btn->state = next_state;
		}

		if( sdl_event.type == SDL_USEREVENT ){
			UserAction( &sdl_event.user );
			update = 1;
		}

		if( update ){
			UpdateWindow( );
		}

		if ( bExit )
		{
			ExitWindow();
			break;
		}
	}
}


static NXDiagAppType *GetStartApp( NXDiagAppType *app, int index )
{
	for( int i=0 ; i<index ; i++ ){
		app = app->next;
		if( app == NULL )
			return NULL;
	}
	return app;
}

static NXDiagButton *GetStartBtn( NXDiagButton *btn, int index )
{
	for( int i=0 ; i<index ; i++ ){
		btn = btn->next;
		if( btn == NULL )
			return NULL;
	}
	return btn;
}

#ifndef MIN
#define	MIN(A,B)	((A<B)?A:B)
#endif

void TopWindow::ChangeApplicationMap()
{
	int startIdx, endIdx;
	//	Application Button Start
	NXDiagButton *btn = GetStartBtn( m_BtnList, 1 );

	startIdx = (m_CurPage-1) * NUM_APP_IN_SCREEN;
	endIdx = MIN((m_CurPage)*NUM_APP_IN_SCREEN, m_NumPlugin );

	for( int i=startIdx ; i<startIdx+NUM_APP_IN_SCREEN ; i++ )
	{
		if( i < endIdx )
		{
			btn->plugin = m_PluginList[i];
			btn->state = BUTTON_NORMAL;
		}
		else
		{
			btn->plugin = NULL;
			btn->state = BUTTON_DISABLED;
		}
		btn = btn->next;
	}
}

void TopWindow::DrawVersion()
{
	int off_x=0, off_y=0;
	SDL_Color foregroundColor = { 0x00, 0x00, 0x00, 0 };
	char msg[128] = {0};

	SDL_Surface* text;
	SDL_Rect rect = { 0, 10, SCREEN_WIDTH, 70 };

	//	draw first line
	sprintf( msg, "%s Diagnostic ver.%d.%d (%d/%d)", diagnostic_DEVICE, diagnostic_MAJOR, diagnostic_MINOR, m_CurPage, m_TotalPage );
	text = TTF_RenderText_Blended( m_TitleFont, msg, foregroundColor );
	if( SCREEN_WIDTH > text->w )	off_x = (SCREEN_WIDTH-text->w)>>1;
	rect.x = off_x;
	off_y = text->h;
	SDL_BlitSurface( text, NULL, m_Surface, &rect );
	SDL_FreeSurface( text );

	sprintf( msg, "(build%d %s @%s)", diagnostic_BUILD, diagnostic_DATE, diagnostic_USER );
	text = TTF_RenderText_Blended( m_TitleFont, msg, foregroundColor );
	if( SCREEN_WIDTH > text->w )	off_x = (SCREEN_WIDTH-text->w)>>1;
	rect.y += off_y;
	rect.x = off_x;
	SDL_BlitSurface( text, NULL, m_Surface, &rect );
	SDL_FreeSurface( text );
}

void TopWindow::ExitMessage()
{
	int off_x=0, off_y=0;
	SDL_Color foregroundColor = { 0x00, 0x00, 0x00, 0 };
	char msg[128] = {0};

	SDL_Surface* text;
	SDL_Rect rect = { 0, 100, 800, 50 };

	//	draw first line
	sprintf( msg, "EXIT Diagnostics");
	text = TTF_RenderText_Blended( m_Font, msg, foregroundColor );
	if( 800 > text->w )	off_x = (800-text->w)>>1;
	rect.x = off_x;
	off_y = text->h;
	SDL_BlitSurface( text, NULL, m_Surface, &rect );
	SDL_FreeSurface( text );

//	sprintf( msg, "Version : 1.6.3(June 21 2011)" );
	sprintf( msg, "Rebooting......" );
	text = TTF_RenderText_Blended( m_Font, msg, foregroundColor );
	if( 800 > text->w )	off_x = (800-text->w)>>1;
	rect.y += off_y;
	rect.x = off_x;
	SDL_BlitSurface( text, NULL, m_Surface, &rect );
	SDL_FreeSurface( text );
}

void TopWindow::DrawAppButton( NXDiagButton *btn )
{
	Uint32 color;
	if( BUTTON_NORMAL==btn->state || BUTTON_FOCUS_OUT==btn->state )
	{
		color = SDL_MapRGB(m_Surface->format, 0x10, 0x10, 0x80);
	}else{
		color = SDL_MapRGB(m_Surface->format, 0x10, 0x10, 0x60);
	}

	if( btn->plugin ){
		if ( btn->plugin->result == TEST_PASSED ){
			color = SDL_MapRGB( m_Surface->format, 0x10, 0x80, 0x10 );
		}else if( btn->plugin->result == TEST_FAILED ){
			color = SDL_MapRGB( m_Surface->format, 0x80, 0x10, 0x10 );
		}
	}

	DrawButton( m_Surface, &btn->rect, color );
	if( btn->state != BUTTON_DISABLED ){
		DrawString( m_Surface, m_Font, btn->plugin->name, btn->rect );
	}
}

void TopWindow::DrawPageSelButton( NXDiagButton *btn, const char *string )
{
	Uint32 color;
	if( btn->state == BUTTON_NORMAL || btn->state == BUTTON_FOCUS_OUT ){
		color = SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f);
	}else{
		color = SDL_MapRGB(m_Surface->format, 0x4f, 0x4f, 0x4f);
	}
	DrawButton( m_Surface, &btn->rect, color );
	DrawString( m_Surface, m_Font, string, btn->rect );
}

void TopWindow::UpdateWindow()
{
	int i=0;
	NXDiagButton *btn = m_BtnList;

	//	Draw BG
	if( btn != NULL ){
//		SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0x85, 0x85, 0x80 ) );
		//SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xff, 0xa5, 0x00 ) );//ORANGE
		//SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xee, 0x82, 0xee ) );//violet
		SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff ) );//white
		btn = btn->next;
	}

	DrawVersion();

	for( i=0 ; i<NUM_APP_IN_SCREEN ; i++ ){
		DrawAppButton( btn );
		btn = btn->next;
	}

	if( btn != NULL ){
		DrawPageSelButton( btn, "Previous" );
		btn = btn->next;
		DrawPageSelButton( btn, "Next" );
		btn = btn->next;
	}
	SDL_Flip( m_Surface );
}

void TopWindow::ExitWindow()
{
	NXDiagButton *btn = m_BtnList;

	//	Draw BG
	if( btn != NULL ){
		//SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0x85, 0x85, 0x80 ) );
		//SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xff, 0xa5, 0x00 ) );//ORANGE
		//SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xee, 0x82, 0xee ) );//violet
		SDL_FillRect(m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff ) );//white
	}
	ExitMessage();
	SDL_Flip( m_Surface );
}

void TopWindow::InitializeButtons()
{
	NXDiagButton *btn;
	DiagMenuTable *table = (DiagMenuTable *)&st_Diag1024x600Menu;

	m_BtnList = CreateButton( &table->bg    , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( &table->app[0], BUTTON_NORMAL  , BTN_ACT_EXEC, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->app[1], BUTTON_NORMAL  , BTN_ACT_EXEC, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->app[2], BUTTON_NORMAL  , BTN_ACT_EXEC, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->app[3], BUTTON_NORMAL  , BTN_ACT_EXEC, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->app[4], BUTTON_NORMAL  , BTN_ACT_EXEC, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->app[5], BUTTON_NORMAL  , BTN_ACT_EXEC, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->prev  , BUTTON_NORMAL  , BTN_ACT_PREV, NULL );	btn = btn->next;
	btn->next = CreateButton( &table->next  , BUTTON_NORMAL  , BTN_ACT_NEXT, NULL );	btn = btn->next;

	ChangeApplicationMap();
}

void TopWindow::UserAction( SDL_UserEvent *usrEvent )
{
	NXDiagButton *btn;
	switch( usrEvent->code )
	{
		case BTN_ACT_NEXT:
			if( m_CurPage < m_TotalPage )
			{
				m_CurPage++;
			}
			ChangeApplicationMap();
			break;
		case BTN_ACT_PREV:
			if( m_CurPage > 1 )
			{
				m_CurPage--;
			}
			ChangeApplicationMap();
			break;
		case BTN_ACT_EXEC:
			btn = (NXDiagButton *)usrEvent->data1;
			if( NULL != btn )
			{
				void *handle;
				printf("Excution: %s, %s(page=%d)\n", btn->plugin->name, btn->plugin->fileName, m_CurPage);

				handle = dlopen( btn->plugin->fileName, RTLD_LAZY );
				if( handle )
				{
					NXDiagPluginInfo* (*getInfo)(void) = (GetPluginInfo)dlsym( handle, PLUGIN_FUNC_SYMBOL );
					if( getInfo )
					{
						NXDiagPluginInfo *pInfo = getInfo();
						btn->plugin->result = pInfo->testDiag( btn->plugin->result );
					}
					else
					{
						printf("Error : cannot find symbol (name:%s, so:%s)\n", btn->plugin->name, btn->plugin->fileName );
					}
					dlclose(handle);
				}
				else
				{
					printf("Error : cannot find so file(name:%s, so:%s)\n", btn->plugin->name, btn->plugin->fileName );
				}
			}
			else
			{
				printf("Cannot execute data.(btn==NULL)\n");
			}
			break;
	}
}

