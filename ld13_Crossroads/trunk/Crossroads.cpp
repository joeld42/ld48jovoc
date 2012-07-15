#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include "GLee.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "GameState.h"
#include "GameViewGL.h"

#import "ResourceFile.h"

// 30 ticks per sim frame
#define STEPTIME (33)

//============================================================================
int main( int argc, char *argv[] )
{	
    
    printf("Game Data Dir: %s\n", getResourceDir().c_str() );
	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		fprintf( stderr, "Unable to init SDL: %s\n", SDL_GetError() );
	}

	if (SDL_SetVideoMode( SCREEN_RES_X, SCREEN_RES_Y, 32, SDL_OPENGL /* | SDL_FULLSCREEN */  ) == 0 ) 
	{
		fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
	}

	SDL_WM_SetCaption( "LD13 Crossroads", NULL );

	// seed rand
	srand( time(0) );

	// Game
	GameState game;
	game.nextLevel();
	GameViewGL gameView( &game );

	//=====[ Main loop ]======
	Uint32 ticks = SDL_GetTicks(), ticks_elapsed, sim_ticks = 0;
	bool done = false;
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
								game.cancelWord();
								break;

						case SDLK_RETURN:
						case SDLK_SPACE:
								game.commitWord();
								break;	
						case SDLK_r:
							game.restartLevel();
							break;
						case SDLK_n:
							game.nextLevel();
							break;
                        default:
                            break;
					}
					break;
				case SDL_MOUSEMOTION:
					gameView.mouseMove( event.motion.x, event.motion.y );					
					break;
				case SDL_MOUSEBUTTONDOWN:
					gameView.mouseDown();
					break;
				case SDL_MOUSEBUTTONUP:
					gameView.mouseUp();
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

//			printf("update sim_ticks %d ticks_elapsed %d\n", sim_ticks, ticks_elapsed );			
			game.update( (float)STEPTIME / 1000.0f );
		}	

		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
		//_RPT2( _CRT_WARN, "ticks_elapsed %d dtraw %f\n", ticks_elapsed, dtRaw );
		gameView.update( dtRaw ); 
		gameView.redraw( dtRaw );

		SDL_GL_SwapBuffers();
	}

	SDL_Quit();

	return 0;
}
