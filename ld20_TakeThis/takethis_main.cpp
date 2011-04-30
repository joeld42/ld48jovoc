#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include "GLee.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL.h>
#include <SDL_endian.h>

#ifndef WIN32
#define _stricmp strcasecmp
#endif

#include "Take5Game.h"

// 30 ticks per sim frame
#define STEPTIME (33)


int main( int argc, char *argv[] )
{
  printf("It's dangerous to go alone! Take this.\n" );
    
	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		printf("Unable to init SDL: %s\n", SDL_GetError() );
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
		printf( "Unable to set video mode: %s\n", SDL_GetError() ) ;
		exit(1);
	}
    
	SDL_WM_SetCaption( "Take Five", NULL );

    // Initialize resources
    Take5Game *game = new Take5Game();
    game->init();   
    
    atexit( Take5Game::shutdown );  
    
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
			game->updateSim( (float)STEPTIME / 1000.0f );			
		}	
        
		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
        
		game->updateFree( dtRaw ); 
        game->redraw();        
        
		SDL_GL_SwapBuffers();
        
		// Call this once a frame if using tweakables
        //ReloadChangedTweakableValues();        
	}
    
    
    
    return 1;
    
};
