#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <luddite/GLee.h>
#include <GL/gl.h>
#include <GL/glu.h>

// Luddite tools
#include <luddite/luddite.h>
#include <luddite/debug.h>
#include <luddite/atom.h>
#include <luddite/texture.h>
#include <luddite/font.h>

#include <SDL.h>
#include <SDL_endian.h>



// Stupid X11 uses 'Font' too
using namespace Luddite;

// 30 ticks per sim frame
#define STEPTIME (33)

#ifndef WIN32
#define _stricmp strcasecmp
#endif

// ===========================================================================
// Global resources
TextureDB g_texDB;

HTexture hFontTexture;

// Could use a HandleMgr and HFont for fonts too, but since there will
// be just one just do directly
Luddite::Font *g_font20 = NULL;
Luddite::Font *g_font32 = NULL;

// ===========================================================================
void demo_init()
{
    DBG::info( "Demo init\n" );    
    hFontTexture = g_texDB.getTexture("gamedata/digistrip.png") ;

    GLuint texId = g_texDB.getTextureId( hFontTexture );
    g_font20 = ::makeFont_Digistrip_20( texId );
    g_font32 = ::makeFont_Digistrip_32( texId );
}

// ===========================================================================
void demo_shutdown()
{
    DBG::info( "Demo shutdown\n" );
    
    g_texDB.freeTexture( hFontTexture );    

    delete g_font20;
    delete g_font32;    
}

// ===========================================================================
void demo_redraw()
{
    glClearColor( 0.592f, 0.509f, 0.274f, 1.0f );    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // set up camera
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();    
    glOrtho( 0, 800, 0, 600, -1.0, 1.0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();    

    // do text
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );

    g_font32->setColor( 1.0f, 1.0f, 1.0f, 1.0f );    
    g_font32->drawString( 100, 100, "HELLO" );    

    // actually draw the text
    g_font20->renderAll();
    g_font32->renderAll();

    g_font32->clear();
    g_font20->clear();    
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

	// Test debug stuff
	DBG::info( "Test of dbg info. Hello %s\n", "world" );

	// Test assert
	char *cheese = "gorgonzola";
	// cheese = NULL; // Uncomment this to see this in action
	Assert( cheese != NULL, "Cheese is bad" );
	AssertPtr( cheese ); // shorthand version to check for null-ptrs

	// Test out atoms	
	const char *atomA, *atomB;
	char buff[25];
	atomA = Atom_string( "blarg77" );
	sprintf( buff, "blarg%d", 77 );
	atomB = Atom_string( buff );
	DBG::info("Atom A is %p (%s)\n", atomA, atomA );
	DBG::info("Atom B is %p (%s)\n", atomB, atomB );
	Assert( atomA == atomB, "string atoms don't match" );    
	

	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		DBG::error("Unable to init SDL: %s\n", SDL_GetError() );
		exit(1);
	}

	// cheezy check for fullscreen
	Uint32 mode_flags = SDL_OPENGL;
	for (int i=1; i < argc; i++)
	{
		if (!_stricmp( argv[i], "-fullscreen"))
		{
			mode_flags |= SDL_FULLSCREEN;
		}
	}

	if (SDL_SetVideoMode( 800, 600, 32, mode_flags ) == 0 ) 
	{
		DBG::error( "Unable to set video mode: %s\n", SDL_GetError() ) ;
		exit(1);
	}
		
	SDL_WM_SetCaption( "Luddite Demo", NULL );

    // Initialize resources
    demo_init();    
    atexit( demo_shutdown );  

    // init graphics
    glViewport( 0, 0, 800, 600 );    

	//=====[ Main loop ]======
	bool done = false;
	Uint32 ticks = SDL_GetTicks(), ticks_elapsed, sim_ticks = 0;	
	while(!done)
	{
		SDL_Event event;

		while (SDL_PollEvent( &event ) ) 
		{
			switch (event.type )
			{
				case SDL_KEYDOWN:
					switch( event.key.keysym.sym ) 
					{						
						case SDLK_ESCAPE:
							done = true;
							break;
					}

					// let the game handle it
					//game->keypress( event.key );
					break;

				case SDL_MOUSEMOTION:					
					break;

				case SDL_MOUSEBUTTONDOWN:					
					//game->mouse( event.button );
					break;

				case SDL_MOUSEBUTTONUP:					
					//game->mouse( event.button );
					break;				

				case SDL_QUIT:
					done = true;
					break;
			}
		}
		
		
		// Timing
		ticks_elapsed = SDL_GetTicks() - ticks;
		ticks += ticks_elapsed;

		// fixed sim update
		sim_ticks += ticks_elapsed;
		while (sim_ticks > STEPTIME) 
		{
			sim_ticks -= STEPTIME;						

			//printf("update sim_ticks %d ticks_elapsed %d\n", sim_ticks, ticks_elapsed );
			//game->updateSim( (float)STEPTIME / 1000.0f );			
		}	

		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
				
		//game->update( dtRaw ); 
        demo_redraw();        

		SDL_GL_SwapBuffers();

        //ReloadChangedTweakableValues();        
	}


    return 1;    
}



