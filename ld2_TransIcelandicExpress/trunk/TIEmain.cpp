#include "TIE.h"
#include "Level.h"
#include "SceneObject.h"
#include "Player.h"
#include "IceFloe.h"
#include "Game.h"

// the global font stuff
fntRenderer *fnt;
fntTexFont *fntHelv;


//--------------------------------------------------------------------


int main( int argc, char *argv[]) {

	TransIcelandicExpress *tieGame;

	if ( SDL_Init(SDL_INIT_NOPARACHUTE | SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

	if( SDL_SetVideoMode( 800, 600, 16, SDL_OPENGL /*| SDL_FULLSCREEN */ ) == 0 ) {
		fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
	}

	SDL_WM_SetCaption( "Trans-Icelandic Express", NULL );

#ifdef TAKE_MOUSE
	SDL_WM_GrabInput( SDL_GRAB_ON );
	SDL_ShowCursor( 0 );
#endif

	// Init SDL_Mixer
	if (Mix_OpenAudio( 22050, AUDIO_S16, 2,  4096 )) {
		fprintf(stderr,	"Unable to open audio\n");
        exit(1);
	}

	// Setup libFnt
	fnt = new fntRenderer();
	fntHelv = new fntTexFont( "helvetica_medium.txf");
	
	fnt->setFont( fntHelv ) ;
	fnt->setPointSize ( 20 ) ;
	
	// initialize DevIL
	ilInit();

	tieGame = new TransIcelandicExpress();
	tieGame->initialize();
	
	tieGame->eventLoop();
		
	return 0;
}