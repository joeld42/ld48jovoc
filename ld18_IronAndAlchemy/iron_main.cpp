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
#include <luddite/tweakval.h>

#include <SDL.h>
#include <SDL_endian.h>

// Game stuff
#include "game.h"

// Stupid X11 uses 'Font' too
using namespace Luddite;

// 30 ticks per sim frame
//#define STEPTIME (33)

// 100 ticks per sim frame
#define STEPTIME (10)

#ifndef WIN32
#define _stricmp strcasecmp
#endif

// ===========================================================================
// Global resources
IronAndAlchemyGame *game = NULL;

USE_AVAR( float );
AnimFloat g_textX;

// ===========================================================================
// Error Checkings
int checkForGLErrors( const char *s, const char * file, int line )
 {
    int errors = 0 ;
    int counter = 0 ;

    while ( counter < 1000 )
    {
      GLenum x = glGetError() ;

      if ( x == GL_NO_ERROR )
        return errors ;

	  DBG::error( "%s:%d OpenGL error: %s [%08x]\n", s ? s : "", file, line, gluErrorString ( x ) ) ;
      errors++ ;
      counter++ ;
    }
	return 0;
}

void checkFBO()
{
	GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT )
	{
		const char *errStr;
		switch (status)
		{
		case GL_FRAMEBUFFER_UNDEFINED: errStr = "GL_FRAMEBUFFER_UNDEFINED"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: errStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: errStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: errStr = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: errStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;		
		default: errStr = "UNKNOWN_ERROR"; break;
		}
		DBG::error( "Bad framebuffer status: [0x%08X] %s\n", status, errStr );
	}
	else
	{
		DBG::info( "glCheckFramebufferStatus good: GL_FRAMEBUFFER_COMPLETE" );
	}
}


#define CHECKGL( msg ) checkForGLErrors( msg, __FILE__, __LINE__ );

// ===========================================================================
void demo_init( FMOD::System *fmod )
{
    DBG::info( "main init\n" );    

	game = new IronAndAlchemyGame();
	game->m_fmod = fmod;
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

	// Init fmod
	FMOD::System *fmod = NULL;
	FMOD::System_Create( &fmod );
	unsigned int version;
	fmod->getVersion( &version );
	DBG::info("FMOD version %d\n", version );
	
	FMOD_RESULT result = fmod->init(32, FMOD_INIT_NORMAL, 0);
    //ERRCHECK(result);
	DBG::info("FMOD init OK\n" );

    // Initialize resources
    demo_init( fmod );    
    atexit( demo_shutdown );  

    // init graphics
	GLuint fboGame;    
	
	// check the unbound fbo
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	checkFBO( );
	DBG::info( "doing fbo stuff\n" );

	// easier than making one from scratch XD
	TextureDB &texDB = TextureDB::singleton();
	HTexture hFBOTex = texDB.getTexture( "gamedata/blank256.png" );
	GLuint texIdFbo = texDB.getTextureId( hFBOTex );

	// make a small framebuffer to draw the game into
	glGenFramebuffersEXT( 1, &fboGame );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboGame );
	DBG::info( "fboGame is %d\n", fboGame );	

	// attach a texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
				GL_TEXTURE_2D, texIdFbo, 0 );

	checkFBO( );
	CHECKGL( "attach fbo" );

	DBG::error( "error test" );


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

						case SDLK_z:
							game->buttonPressed( BTN_JUMP );
							break;

						case SDLK_x:
							game->buttonPressed( BTN_FIRE );
							break;

						// DBG: test levels
						case SDLK_1:
							game->loadOgmoFile( "gamedata/test1.oel" );
							break;

					}
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
		
		// Do continuous keys
		Uint8 *keyState = SDL_GetKeyState( NULL );

		// convert to btn mask
		Uint32 btnMask = 0;
		btnMask |= keyState[SDLK_LEFT]?BTN_LEFT:0;
		btnMask |= keyState[SDLK_RIGHT]?BTN_RIGHT:0;
		btnMask |= keyState[SDLK_UP]?BTN_UP:0;
		btnMask |= keyState[SDLK_DOWN]?BTN_DOWN:0;

		btnMask |= keyState[SDLK_z]?BTN_JUMP:0;
		btnMask |= keyState[SDLK_x]?BTN_FIRE:0;

		game->updateButtons( btnMask );
		
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
		
		// draw into the offscreen framebuffer
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboGame );
		glViewport( 0, 0, 240, 160 );
        demo_redraw();
		
		// now draw the offscreen buffer to the screen
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
		glViewport( 0, 0, 960, 640 );

		// set up camera
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();    
		glOrtho( 0, 960, 0, 640, -1.0, 1.0 );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();    

		glBindTexture( GL_TEXTURE_2D, texIdFbo );

		glBegin( GL_QUADS );
		glTexCoord2f( 0.0, 0.0f ); glVertex2f( 0.0f, 0.0f );
		glTexCoord2f( 240.0/256.0, 0.0f ); glVertex2f( 960.0f, 0.0f );
		glTexCoord2f( 240.0/256.0, 160.0/256.0 ); glVertex2f( 960.0f, 640.0f );
		glTexCoord2f( 0.0, 160.0/256.0 ); glVertex2f( 0.0f, 640.0f );
		glEnd();

		SDL_GL_SwapBuffers();

		// Arrrrr... ya
        ReloadChangedTweakableValues();        
	}


    return 1;    
}



