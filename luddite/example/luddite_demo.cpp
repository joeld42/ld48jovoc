#include <stdio.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

//============================================================================
void errorMessage( std::string msg )
{
	// on linux or other OS, just printf
#ifdef WIN32
	MessageBoxA( NULL, msg.c_str(), "ld15_cavern ERROR", MB_OK | MB_ICONSTOP );
#else
    printf("ERROR: %s\n", msg.c_str() );    
#endif
}


// ===========================================================================
int main( int argc, char *argv[] )
{	

	// I can't live without my precious printf's
#ifdef WIN32
#  ifndef NDEBUG
	AllocConsole();
	SetConsoleTitle( L"luddite demo CONSOLE" );
	freopen("CONOUT$", "w", stdout );
#  endif
#endif
	
	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		errorMessage( string("Unable to init SDL:") + SDL_GetError() );
		exit(1);
	}

	// cheezy check for fullscreen
	Uint32 mode_flags = SDL_OPENGL;
	for (int i=1; i < argc; i++)
	{
		if (!stricmp( argv[i], "-fullscreen"))
		{
			mode_flags |= SDL_FULLSCREEN;
		}
	}

	if (SDL_SetVideoMode( 800, 600, 32, mode_flags ) == 0 ) 
	{
		errorMessage( string("Unable to set video mode: ") +  SDL_GetError() ) ;
		exit(1);
	}
		

	SDL_WM_SetCaption( "Luddite Demo", NULL );

    return 1;    
}

