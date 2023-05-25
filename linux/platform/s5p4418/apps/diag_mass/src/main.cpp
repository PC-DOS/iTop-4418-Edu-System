#include <DiagWindow.h>

int main( int argc, char *argv[] )
{

	if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 )
	{
		printf("Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	TTF_Init();

	CDiagnosticWnd *pDiag = new CDiagnosticWnd();
	pDiag->EventLoop( 0 );
	delete pDiag;

	return 0;
}


