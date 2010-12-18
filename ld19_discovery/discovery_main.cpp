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
#include <luddite/texture.h>
#include <luddite/font.h>
#include <luddite/avar.h>

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

USE_AVAR( float );
AnimFloat g_textX;

// ===========================================================================
void game_init()
{
    DBG::info( "Game init\n" );    
    hFontTexture = g_texDB.getTexture("gamedata/digistrip.png") ;

    GLuint texId = g_texDB.getTextureId( hFontTexture );
    g_font20 = ::makeFont_Digistrip_20( texId );
    g_font32 = ::makeFont_Digistrip_32( texId );

    // set up the pulse Avar
    g_textX.pulse( 0, 800 - g_font32->calcWidth( "HELLO" ), 10.0, 0.0 );    
}

// ===========================================================================
// update on a fixed sim step, do any updates that may even remotely
// effect gameplay here
void game_updateSim( float dtFixed )
{
    // Updates all avars
    AnimFloat::updateAvars( dtFixed );    
}

// Free running update, useful for stuff like particles or something
void game_updateFree( float dt )
{
    // do nothing...
}


// ===========================================================================
void game_shutdown()
{
    DBG::info( "Game shutdown\n" );
    
    g_texDB.freeTexture( hFontTexture );    

    delete g_font20;
    delete g_font32;    
}

// ===========================================================================
void game_redraw()
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
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

    g_font32->setColor( 1.0f, 1.0f, 1.0f, 1.0f );    
    g_font32->drawString( g_textX.animValue(), 300, "HELLO" );    

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
	SetConsoleTitle( L"ld19 Discovery CONSOLE" );
	freopen("CONOUT$", "w", stdout );
#  endif
#endif

	// Test debug stuff
	DBG::info( "Test of dbg info. Hello %s\n", "world" );
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
		
	SDL_WM_SetCaption( "LD19 Jovoc - Discovery", NULL );

    // Initialize resources
    game_init();    
    atexit( game_shutdown );  

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

						case SDLK_F6:
							g_texDB.reportUsage();
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
			game_updateSim( (float)STEPTIME / 1000.0f );			
		}	

		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
				
		game_updateFree( dtRaw ); 
        game_redraw();        

		SDL_GL_SwapBuffers();

		// Call this once a frame if using tweakables
        //ReloadChangedTweakableValues();        
	}


    return 1;    
}



