#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include "luddite/GLee.h"
#include <GL/gl.h>
#include <GL/glu.h>

// Luddite tools
#include <luddite/luddite.h>
#include <luddite/debug.h>
#include <luddite/atom.h>
#include <luddite/texture.h>
#include <luddite/font.h>
#include <luddite/avar.h>

#include <SDL.h>
#include <SDL_endian.h>

// Game stuff
#include "game.h"

// Stupid X11 uses 'Font' too
using namespace Luddite;

// 30 ticks per sim frame
#define STEPTIME (33)

#ifndef WIN32
#define _stricmp strcasecmp
#endif

// ===========================================================================
// Global resources
IronAndAlchemyGame *game = NULL;

USE_AVAR( float );
AnimFloat g_textX;



// ===========================================================================
void demo_init()
{
    DBG::info( "main init\n" );    

	game = new IronAndAlchemyGame();
	game->initResources();

    

    // set up the pulse Avar
	g_textX.pulse( 0, 480 - game->m_font32->calcWidth( "HELLO" ), 30.0, 0.0 );    
}

// ===========================================================================
// update on a fixed sim step, do any updates that may even remotely
// effect gameplay here
void demo_updateSim( float dtFixed )
{
    // Updates all avars
    AnimFloat::updateAvars( dtFixed );    

	// update the game
	game->updateSim( dtFixed );
}

// Free running update, useful for stuff like particles or something
void demo_updateFree( float dt )
{
    game->updateFree( dt );
}


// ===========================================================================
void demo_shutdown()
{
    DBG::info( "Demo shutdown\n" );
    game->freeResources();

    
}

// ===========================================================================
void demo_redraw()
{
	game->render();
}


// ===========================================================================
int main( int argc, char *argv[] )
{	

	// I can't live without my precious printf's
#ifdef WIN32
#  ifndef NDEBUG
	AllocConsole();
	SetConsoleTitle( L"iron and alchemy CONSOLE" );
	freopen("CONOUT$", "w", stdout );
#  endif
#endif

	// Test debug stuff
	//DBG::info( "Test of dbg info. Hello %s\n", "world" );

	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		DBG::error("Unable to init SDL: %s\n", SDL_GetError() );
		exit(1);
	}

	// open a window
	Uint32 mode_flags = SDL_OPENGL;
	if (SDL_SetVideoMode( 960, 640, 32, mode_flags ) == 0 ) 
	{
		DBG::error( "Unable to set video mode: %s\n", SDL_GetError() ) ;
		exit(1);
	}
		
	SDL_WM_SetCaption( " =[+ Iron and Alchemy - LD18 jovoc +]=", NULL );

    // Initialize resources
    demo_init();    
    atexit( demo_shutdown );  

    // init graphics
    glViewport( 0, 0, 960, 640 );    

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
			demo_updateSim( (float)STEPTIME / 1000.0f );			
		}	

		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
				
		demo_updateFree( dtRaw ); 
        demo_redraw();        

		SDL_GL_SwapBuffers();

        //ReloadChangedTweakableValues();        
	}


    return 1;    
}



