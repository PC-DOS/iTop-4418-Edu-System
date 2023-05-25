#include <pthread.h>
#include <base_app_window.h>
#include <nx_diag_type.h>
#include <utils.h>
#include "NX_CWaveOut.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#if 0	//	800x480 UI
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 410, 385,  60 };
static SDL_Rect st_NOKRect = { 405, 410, 385,  60 };
#endif

#if 1	//	1024x600 UI
static SDL_Rect st_BGRect  = {   0,   0, SCREEN_WIDTH, SCREEN_HEIGHT };
static SDL_Rect st_OKRect  = {  10, 480, 490, 110 };
static SDL_Rect st_NOKRect = { 520, 480, 490, 110 };
#endif


#define AUDIO_FILE "/usr/local/diag/data/test_wave.wav"

//	Wave Header
struct WAVEHDR{
    unsigned long   ChunkID;			// RIFF
    unsigned long   ChunkSize;			// filelength
    unsigned long   Format;				// format
    unsigned long   SubChunk1ID;		// WAVEfmt_
    unsigned long   SubChunk1Size;		// format lenght
    unsigned short  AudioFormat;		// format Tag
    unsigned short  NumChannels;		// Mono/Stereo
    unsigned long   SampleRate;
    unsigned long   ByteRate;
    unsigned short  BlockAlign;
    unsigned short  BitsPerSample;
    unsigned long   SubChunk2ID;		// Data Chunk
    unsigned long   SubChunk2Size;		// Data Chunk Size
};

class AudioTestWnd: public BaseAppWnd
{
public:
	AudioTestWnd();
	~AudioTestWnd();
	virtual void Initialize();
	virtual int EventLoop( int lastResult );
	virtual void UpdateWindow();

	int PlayFile(const char *fileName);
	static void *WavePlay( void *pObject );

private:
	int		Action( SDL_UserEvent *usrEvent );
	int		m_TestResult;
	char	m_LastMessage[1024];
	char	*m_FileName;
	pthread_t m_Thread;
	int		m_ExitPlay;
	int		m_bThreadRunning;
};

AudioTestWnd::AudioTestWnd()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	m_Font = TTF_OpenFont(m_DiagCfg.GetFont(), m_DiagCfg.GetFontSize());
	m_Surface = SDL_SetVideoMode( info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_SWSURFACE );
	m_BtnList = NULL;
	memset(m_LastMessage, 0, sizeof(m_LastMessage) );
	m_FileName = NULL;
	m_ExitPlay = 0;
	m_bThreadRunning = 0;
}

AudioTestWnd::~AudioTestWnd()
{
	if( m_FileName )
		free( m_FileName );
	if( m_Font )
		TTF_CloseFont( m_Font );
	if( m_Surface )
		SDL_FreeSurface( m_Surface );
}

void AudioTestWnd::Initialize()
{
	NXDiagButton *btn;
	m_BtnList = CreateButton( (SDL_Rect*)&st_BGRect  , BUTTON_DISABLED, BTN_ACT_NONE, NULL );	btn = m_BtnList;
	btn->next = CreateButton( (SDL_Rect*)&st_OKRect  , BUTTON_NORMAL  , BTN_ACT_OK  , NULL );	btn = btn->next;
	btn->next = CreateButton( (SDL_Rect*)&st_NOKRect , BUTTON_NORMAL  , BTN_ACT_NOK , NULL );	btn = btn->next;
}

int AudioTestWnd::EventLoop( int lastResult )
{
	NXDiagButton *btn;
	SDL_Event sdl_event;
	bool bExit = false;
	int hit_test = 0;
	int processed = 0;
	int update = 0;
	BUTTON_STATE next_state;
	m_TestResult = lastResult;
	while( !bExit )
	{
		update = 0;
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
							//event.user.data2= ;
							SDL_PushEvent(&evt);
						}
					}
					break;
				case SDL_KEYDOWN:
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
			Action( &sdl_event.user );
			bExit = true;
			update = 1;
		}

		if( update ){
			UpdateWindow();
		}

		if ( bExit )	break;
	}

	m_ExitPlay = 1;

	if( m_bThreadRunning ){
		pthread_join( m_Thread, NULL );
	}

	return m_TestResult;
}

void AudioTestWnd::UpdateWindow()
{
	NXDiagButton *btn = m_BtnList;
	SDL_FillRect( m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0xff, 0xff, 0xff ) );

	if( strlen(m_LastMessage) > 0 ){
		DrawString ( m_Surface, m_Font, m_LastMessage, btn->rect );
	}

	btn = btn->next;
	DrawButton ( m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f ) );
	DrawString ( m_Surface, m_Font, "OK", btn->rect );
	btn = btn->next;
	DrawButton  ( m_Surface, &btn->rect, SDL_MapRGB(m_Surface->format, 0x6f, 0x6f, 0x6f ) );
	DrawString ( m_Surface, m_Font, "NOK", btn->rect );
	SDL_Flip( m_Surface );
}

int	AudioTestWnd::Action( SDL_UserEvent *usrEvent )
{
	switch(usrEvent->code)
	{
		case BTN_ACT_OK:
			m_TestResult = TEST_PASSED;
			break;
		case BTN_ACT_NOK:
			m_TestResult = TEST_FAILED;
			break;
	}
	return 0;
}


void *AudioTestWnd::WavePlay( void *pObject )
{
	AudioTestWnd *pAudWnd = (AudioTestWnd *)pObject;
	//	fd
	FILE *fd = fopen( pAudWnd->m_FileName, "rb" );
	if( fd == NULL )
	{
		goto error_exit;
	}

	pAudWnd->m_bThreadRunning = 1;
	struct WAVEHDR header;

	if( sizeof(header) != fread( &header, 1, sizeof(header), fd ) ){
		printf("Cannot open file\n");
		goto error_exit;
	}
/*
	printf("header.ChunkID       =0x%08x\n", header.ChunkID       );
	printf("header.Format        =0x%08x\n", header.Format        );
	printf("header.SubChunk1ID   =0x%08x\n", header.SubChunk1ID   );
	printf("header.SubChunk2ID   =0x%08x\n", header.SubChunk2ID   );
	printf("header.AudioFormat   =0x%04x\n", header.AudioFormat   );
	printf("header.BitsPerSample =0x%04x\n", header.BitsPerSample );
*/

	if( header.ChunkID       != 0x46464952	||	//	check "RIFF"
		header.Format        != 0x45564157	||	//	check "WAVE"
		header.SubChunk1ID   != 0x20746d66	||	//	check "fmt "
		header.SubChunk2ID   != 0x61746164	||	//	check "data"
		header.AudioFormat   != 0x0001		||	//	check pcm format
		header.BitsPerSample != 0x0010			//	check bit per sample( must be 16 )
		)
	{
		goto error_exit;
	}

	{
		NX_CWaveOut *waveOut = new NX_CWaveOut();
		unsigned char *pBuffer = (unsigned char *)malloc(64*1024);
		size_t readSize;
		int error;
		waveOut->WaveOutOpen( header.SampleRate, header.NumChannels, 16, 1024, 1 );
		do{
			readSize = fread( pBuffer, 1, 64*1024, fd );
			if( readSize > 0 ){
				waveOut->WaveOutPlayBuffer( pBuffer, readSize, error );
			}
		}while(readSize > 0 && !pAudWnd->m_ExitPlay);

		delete waveOut;
		free( pBuffer );
		fclose( fd );
	}
	return (void*)0xDEADDEAD;

error_exit:
	if( fd ){
		fclose( fd );
	}
	sprintf( pAudWnd->m_LastMessage, "Cannot play file : %s", pAudWnd->m_FileName );
	pAudWnd->UpdateWindow();
	return (void*)0xDEADDEAD;
}

int AudioTestWnd::PlayFile( const char *fileName )
{
	m_FileName = strdup( fileName );
	if( pthread_create(&m_Thread, NULL, WavePlay, (void*)this ) < 0 )
	{
		sprintf( m_LastMessage, "Cannot create wave out thread" );
		return -1;
	}
	return 0;
}

extern "C" int test_audio( int last_result )
{
	int result;
	AudioTestWnd *audWnd = new AudioTestWnd();
	audWnd->Initialize();
#if 0
	int tmp;
	struct stat st;
	tmp=stat("/mnt/mmc/test_wave.wav",&st);
	printf("tmp = %d \n\n",tmp);
	if(tmp==0)
	{
		printf("mmc test_wave.wav");
		audWnd->PlayFile( "/mnt/mmc/test_wave.wav" );
	}
	else	
	{	
		audWnd->PlayFile( AUDIO_FILE );
	}
#endif
	audWnd->UpdateWindow();
	result = audWnd->EventLoop( last_result );
	delete audWnd;
	return result;
}


//
static NXDiagPluginInfo AudioTestPluginInfo = {
	"AUDIO PLAY TEST",
	"AUDIO Play Diagnostic Application",
	"",
	test_audio,
	0,				//	Disable Auto Test
	0,				//	Start Result
};

extern "C" NXDiagPluginInfo* NXGetPluginInfo(void)
{
	return &AudioTestPluginInfo;
}
