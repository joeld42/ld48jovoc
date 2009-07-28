
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <wininet.h>

#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <prmath/prmath.hpp>

#include <SDL_mixer.h>

#include "gamefontgl.h"


#include "GamePad.h"
#include "GameState.h"
#include "GameView.h"
#include "ViewSimple2D.h"

#include "SoundFX.h"

// The game itself
GameState *g_game = NULL;

// The game view
GameView *g_view = NULL;

// the music
Mix_Music *music_title = NULL, *music_gameover = NULL, *music_ingame = NULL;
Mix_Chunk *sfx[10];
int sfx_chan[10];
bool musicOn = true;

// paused
bool paused = false;

// the font stuff
ILuint ilFontId;
GLuint glFontTexId, fntFontId;

#define HISCORE_PROMPT "ENTER NAME"

GamePadState gamepad;

bool bDrawGamepad = false;

enum {
	STATE_TITLE,
	STATE_GAMEOVER,
	STATE_PLAYING
};
int g_state = STATE_TITLE;


// Menu stuff
enum {
	MENU_MAINMENU,
	MENU_HELP,
	MENU_PICK_LEVEL,
	MENU_DESCRIBE_LEVEL,
};
int g_menuState = MENU_MAINMENU;
int g_levelNdx = 0;
int g_menuItem = 0;

const char *mainMenu[] = {
	"Play Game",
	"High Scores",
	"Help",
	"Quit",
	NULL
};

void backToTitleScreen();

char *help_string = 
"You are making pies out of the ingredients provided.\n"
"Recipies are listed on the right hand side of\n"
"the screen. There may be secret combos, so stay alert.\n\n"
"Use left/right arrow or gamepad to switch move the cursor.\n"
"Press space or gamepad button to activate action.\n\n"
"Actions are:\n"
"    SWAP  - Swap cursor with center.\n"
"    FREEZE  - Put an item in the freezer.\n"
"    TORCH  - Burn an item (costs 10 points).\n\n"
"Addional keys are:\n"
"    P - Pause Game\n"
"    S - Take Screenshot\n"
"    D - Toggle Gamepad diagram\n"
"    M - Toggle Music\n\n"
"Have Fun, and HAPPY HALLOWEEN";

// The main picture texture
ILuint ImageName;
unsigned int gl_tex_id;

// should really be somewhere else
struct HighScoreEntry {	
	std::string m_name;
	int m_score;

	bool operator< ( const HighScoreEntry &other ) const {
		return m_score > other.m_score;
	}
};

// read a name into the high score table
int ndxGetName = -1;

std::vector<HighScoreEntry> g_highScoreTable;

/// The cleanup 
void do_quit()
{
	Mix_CloseAudio();

	if (SDL_JoystickOpened(0)) {
		SDL_JoystickClose( gamepad.sdlinfo );
	}

	SDL_Quit();

	
	exit(0);
}

//////////////////////////////////////////////////////////////////////////
// Init graphics
//////////////////////////////////////////////////////////////////////////

void initGraphics() {
		ilGenImages( 1, &ImageName );

		ilBindImage( ImageName );
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
		if (!ilLoadImage( "gamedata/bg-art.png" )) {
			printf("Loading image bg-art failed\n");
		}

		
		gl_tex_id = ilutGLBindTexImage();
		//ilutGLTexImage( gl_tex_id );

		initGamepadDiagram();				
}


//////////////////////////////////////////////////////////////////////////
// Redraw
//////////////////////////////////////////////////////////////////////////
void redraw( void ) {	
	static Uint32 last_tick=0;
	static float angle = 0.0;
	static int tex_init = 0;	

	Uint32 tick;
	tick = SDL_GetTicks();
	float deltaT;

	if (last_tick==0) {		
		deltaT = 0.001f;
	} else {
		deltaT = (float)(tick - last_tick) / 1000.0f;
	}
	last_tick = tick;

	// step the simulation
	if (!paused) {
		g_view->simulate( deltaT );
	}

	if (!tex_init) {		
		initGraphics();
		g_view->initGraphics( fntFontId );
		tex_init = 1;
	} 

	

	glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );	

	// 2d part
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	gluOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// draw the background
	vec2f st0, st1;
	if (g_state == STATE_PLAYING) {		

		st0 = vec2f( 560, 1024-680 );
		st1 = vec2f( 1024, 0);

		// TODO: Modify based on level color
		glColor3f( 1.0, 1.0, 1.0 );

	} else if (g_state == STATE_GAMEOVER) {		
		st0 = vec2f( 0, 1024-606 );
		st1 = vec2f( 555, 0 );

		glColor3f( 1.0, 1.0, 1.0 );

	} else if (g_state == STATE_TITLE ) {		

		st0 = vec2f( 0, 1024 );
		st1 = vec2f( 800, 1024-600 );

		if (g_menuState==MENU_HELP) {
			glColor3f( 0.3, 0.3, 0.5 );
		} else {
			glColor3f( 1.0, 1.0, 1.0 );
		}
	}

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();
	glScalef( 1.0/1024.0, 1.0/1024.0, 1.0 );

	
	glBindTexture( GL_TEXTURE_2D, gl_tex_id );		
	

	glBegin( GL_QUADS );

	glTexCoord2d( st0[0], st1[1] );  glVertex3f( 0, 0, 0.0 );
	glTexCoord2d( st0[0], st0[1] );  glVertex3f( 0, 600, 0.0 );
	glTexCoord2d( st1[0], st0[1] );  glVertex3f( 800, 600, 0.0 );
	glTexCoord2d( st1[0], st1[1] );  glVertex3f( 800, 0, 0.0 );
	glEnd();
		
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );

	
	

	// draw the view
	if (g_state == STATE_PLAYING ) {
		g_view->redraw( paused);		
	}

	if (paused) {
		gfEnableFont( fntFontId, 72 );
		glColor4f ( 1.0f, 0.2f, 1.0f, 0.7f ) ;
		gfBeginText(); 
		glTranslated ( 220, 270, 0 ) ;
		gfDrawString("PAUSED" );
		gfEndText();
	}

	if (g_game->isGameOver()) {
		
		if (g_state==STATE_PLAYING) {
			g_state = STATE_GAMEOVER;
			g_menuState = MENU_MAINMENU;

			if (musicOn) {
				Mix_PlayMusic( music_gameover, 1 );
			}

			// Need to update the highscore??
			if (g_game->getLevelScore() > g_highScoreTable[9].m_score) {
				HighScoreEntry e;
				e.m_name = HISCORE_PROMPT;
				e.m_score = g_game->getLevelScore();

				g_highScoreTable.push_back( e );

				std::sort( g_highScoreTable.begin(), g_highScoreTable.end() );
				g_highScoreTable.erase( g_highScoreTable.end()-1 );

				for (int i=0; i < g_highScoreTable.size(); i++) {
					if (!strcmp( g_highScoreTable[i].m_name.c_str(), HISCORE_PROMPT)) {
						ndxGetName = i;
						break;
					}
				}

			}
		}

		if (g_state==STATE_TITLE) {

			if (g_menuState == MENU_MAINMENU) {				

				gfEnableFont( fntFontId, 72 );
				glColor4f ( 1.0f, 0.8f, 0.2f, 0.7f ) ;
				gfBeginText(); 
				glTranslated ( 10, 400, 0 ) ;
				gfDrawString("The Halloween Machine" );
				gfEndText();


				gfEnableFont( fntFontId, 15 );
				glColor4f ( 1.0f, 1.0f, 0.2f, 0.9f ) ;
				gfBeginText(); 
				glTranslated ( 10, 10, 0 ) ;
				gfDrawString("Copyright 2003 Joel Davis -- Part of the LudumDare Halloween Hack!" );
				gfEndText();


				int y = 290;
				gfEnableFont( fntFontId, 25 );
				gfBeginText();
				for (int i=0; mainMenu[i]; i++) {

					if (g_menuItem==i) {
						glColor4f ( 0.6f, 0.8f, 0.9f, 0.7f ) ;
					} else {
						glColor4f ( 1.0f, 0.8f, 0.2f, 0.7f ) ;
					}
					glPushMatrix();
					glTranslated( 400-gfGetStringWidth( mainMenu[i] )/2, y, 0.0 );
					gfDrawString( mainMenu[i] );
					glPopMatrix();

					y -= 30;
				}
				gfEndText();

			} else if (g_menuState == MENU_PICK_LEVEL ) {
			
				int y = 400;
				gfEnableFont( fntFontId, 25 );
				gfBeginText();
				for (int i=0; i < g_game->getNumLevels(); i++) {

					if (g_levelNdx==i) {
						glColor4f ( 0.6f, 0.8f, 0.9f, 0.7f ) ;
					} else {
						glColor4f ( 1.0f, 0.8f, 0.2f, 0.7f ) ;
					}
					glPushMatrix();
					glTranslated( 100, y, 0.0 );
					
					gfDrawStringFmt( "%s -- %s", 
								g_game->getLevelGroup(i).c_str(),
								g_game->getLevelTitle(i).c_str() );

					//char buff[1000];
					//sprintf( buff, "%s -- %s",  g_game->getLevelGroup(i).c_str(),
					//			g_game->getLevelTitle(i).c_str() );
					//gfDrawString( buff );					
					glPopMatrix();

					y -= 30;
				} 
				gfEndText();

			} else if (g_menuState == MENU_DESCRIBE_LEVEL ) {

				gfEnableFont( fntFontId, 72 );
				glColor4f ( 1.0f, 0.8f, 0.2f, 0.7f ) ;
				gfBeginText(); 
				glTranslated ( 400 - gfGetStringWidth( g_game->getLevelTitle(g_levelNdx).c_str() ) /2, 
					400, 0 ) ;
				gfDrawString( g_game->getLevelTitle(g_levelNdx).c_str() );
				gfEndText();

				gfEnableFont( fntFontId, 25 );				
				gfBeginText(); 
				glTranslated ( 100, 300, 0 ) ;
				gfDrawString( g_game->getLevelDesc(g_levelNdx).c_str() );
				gfEndText();

			} else if (g_menuState == MENU_HELP ) {
				glColor3f( 1.0, 1.0, 1.0 );
				gfEnableFont( fntFontId, 25 );				
				gfBeginText(); 
				glTranslated ( 50, 550, 0 ) ;
				gfDrawString( help_string );
				gfEndText();
			}

			//gfEnableFont( fntFontId, 25 );
			//glColor4f ( 1.0f, 0.8f, 0.2f, 0.7f ) ;
			//gfBeginText(); 
			//glTranslated ( 170, 360, 0 ) ;
			//gfDrawString("Press Spacebar or Start Button." );
			//gfEndText();


		} else if (g_state==STATE_GAMEOVER) {
			gfEnableFont( fntFontId, 25 );
			glColor4f ( 1.0f, 0.2f, 0.2f, 0.7f ) ;
			gfBeginText(); 
			glTranslated ( 320, 350, 0 ) ;
			gfDrawString("High Scores" );
			gfEndText();

			gfBeginText();
			for (int i=0; i < 10; i++) {
				if (i==ndxGetName) {
					glColor4f ( 1.0f, 1.0, 1.0f, 0.7f ) ;
				} else {
					glColor4f ( 1.0f, 0.2f, 0.2f, 0.7f ) ;
				}
				glPushMatrix();
				glTranslated( 400 - gfGetStringWidth(  g_highScoreTable[i].m_name.c_str() ), 
					300-i*30, 0 );
				gfDrawString( g_highScoreTable[i].m_name.c_str() );
				glPopMatrix();

				glPushMatrix();
				glTranslated( 430, 300-i*30, 0 );
				gfDrawStringFmt( "%d", g_highScoreTable[i].m_score );
				glPopMatrix();
			}
			gfEndText();

			
		}

	}

	// score
	if (g_state != STATE_TITLE) {
		gfEnableFont( fntFontId, 25 );
		glColor4f ( 0.3f, 1.0f, 0.3f, 0.7f ) ;
		gfBeginText(); 
		glTranslated ( 5, 580, 0 ) ;
		gfDrawStringFmt("Score: %d", g_game->getScore());
		gfEndText();
	}
	

	// Draw the gamepad diagram	
	if (bDrawGamepad) {
		drawGamepadDiagram( gamepad, fntFontId );	
	}

	// all done
	SDL_GL_SwapBuffers();
}


//////////////////////////////////////////////////////////////////////////
// High Scores
//////////////////////////////////////////////////////////////////////////
void readHighScores() 
{
	int i;
	FILE *fp;
	fp = fopen( "hiscores.txt", "rt" );
	HighScoreEntry e;
	
	g_highScoreTable.erase( g_highScoreTable.begin(), 
							g_highScoreTable.end() );

	if (!fp) {
		
		// No high scores, make new ones		
		for (i = 0; i < 10; i++) {
			e.m_name = "Nobody";
			e.m_score = (i+1)*100;
			g_highScoreTable.push_back( e );
		}
		
	} else {
		char name[100];
		int value;
		for (i=0; i < 10; i++) {
			fscanf( fp, "%s %d", name, &value );
			e.m_name = name;
			e.m_score = value;

			g_highScoreTable.push_back( e );
		}
		fclose( fp );
	}

	std::sort( g_highScoreTable.begin(), g_highScoreTable.end() );

	for (int j=0; j < 10; j++) {
		printf(" %d %s\n", g_highScoreTable[j].m_score,
						   g_highScoreTable[j].m_name.c_str() );

	}
	
}

void writeHighScores() 
{
	int i;
	FILE *fp;
	fp = fopen( "hiscores.txt", "wt" );
	if (fp ) {
		for( i = 0; i < 10; i++) {
			fprintf( fp, "%s %d\n", 
					 g_highScoreTable[i].m_name.c_str(),
					 g_highScoreTable[i].m_score );
		}
		fclose (fp );
	}
}

void startGame() 
{
	g_game->resetLevel( g_levelNdx );	
	g_state = STATE_PLAYING;

	if (musicOn) {
		//Mix_HaltMusic();
		Mix_PlayMusic( music_ingame, -1 );	
	}


#if 0
	// add some combos.. TODO.. make dependant on level
	Combo c;


// debugging combos
	c = Combo( 10 );
	c.addGroup( PUMPKIN, 3 );
	c.m_name = "Three Pumpkins";
	c.m_desc = "Three matching items";
	g_game->addCombo( c );

	c = Combo( 20 );
	c.addGroup( PUMPKIN, 4 );
	c.m_name = "Four Pumpkins";
	c.m_desc = "Four matching items";
	g_game->addCombo( c );

	c = Combo( 30 );
	c.addGroup( PUMPKIN, 2 );
	c.addGroup( SKULL, 2 );
	c.addGroup( FISH_HEAD, 1 );
	c.m_name = "Mixed Bag";	
	g_game->addCombo( c );

	c = Combo( 40 );
	c.addGroup( PUMPKIN, 1 );
	c.addGroup( PUMPKIN+1, 1 );
	c.addGroup( PUMPKIN+2, 1 );
	c.addGroup( PUMPKIN+3, 1 );
	c.addGroup( SKULL, 1 );
	c.m_name = "One of each";	
	g_game->addCombo( c );

	// real combos
	c = Combo( 10 );
	c.addGroup( ANY_ITEM, 3 );
	c.m_name = "Three of a kind";
	c.m_desc = "Three matching items";
	g_game->addCombo( c );


	c = Combo( 20 );
	c.addGroup( ANY_ITEM, 4 );
	c.m_name = "Four of a kind";
	c.m_desc = "Four matching items";
	g_game->addCombo( c );

	c = Combo( 60 );
	c.addGroup( ANY_ITEM, 5 );
	c.m_name = "Jumbo Pie";
	c.m_desc = "Five matching items";
	g_game->addCombo( c );

	c = Combo( 30 );
	c.addGroup( ANY_ITEM, 3 );
	c.addGroup( ANY_ITEM, 2 );
	c.m_name = "Full House";
	c.m_desc = "A pair and three matching items";
	g_game->addCombo( c );
	
	c = Combo( 15 );
	c.addGroup( ANY_PUMPKIN, 2 );
	c.addGroup( SKULL, 1 );
	c.m_name = "Pumpkin Surprise";
	c.m_desc = "Pair of Pumpkins and a Skull";
	g_game->addCombo( c );

	c = Combo( 15 );
	c.addGroup( ANY_PUMPKIN, 2 );
	c.addGroup( ANY_PUMPKIN, 2 );	
	c.m_name = "Pumpkin Medly";
	c.m_desc = "Two pairs of pumpkins";
	g_game->addCombo( c );

	c = Combo( 50 );
	c.addGroup( PUMPKIN, 1 );
	c.addGroup( PUMPKIN_ORANGE, 1 );
	c.addGroup( PUMPKIN_YELLOW, 1 );
	c.addGroup( PUMPKIN_RED, 1 );
	c.m_name = "Pumpkin Supreme";
	c.m_desc = "One of each pumpkin";
	g_game->addCombo( c );

	c = Combo( 75 );
	c.addGroup( ANY_PUMPKIN, 4 );
	c.addGroup( FISH_HEAD, 1 );
	c.m_name = "Stargazy Pie";
	c.m_desc = "Four Pumpkins and a Fishhead";
	g_game->addCombo( c );
#endif

}

void backToTitleScreen()
{
	if (g_state == STATE_PLAYING) {
		g_game->forceGameOver();		
	} else {
		g_state = STATE_TITLE;
		g_menuState = MENU_MAINMENU;
		g_menuItem = 0;

		if (musicOn) {
			Mix_PlayMusic( music_title, -1 );
		}
	}
}

void doStartButton() 
{
	if (g_state == STATE_PLAYING) {
		paused = !paused;
	} else if (g_state == STATE_GAMEOVER) {
		backToTitleScreen();
	} else if (g_state == STATE_TITLE) {

		if (g_menuState == MENU_DESCRIBE_LEVEL) {
			startGame();						
		} else if (g_menuState == MENU_PICK_LEVEL) {
			g_menuState = MENU_DESCRIBE_LEVEL;
		} else if (g_menuState == MENU_MAINMENU) {
			if (g_menuItem==0) {
				
				// Play
				g_menuState = MENU_PICK_LEVEL;
			} else if (g_menuItem==1) {
				// Hi Scores
				g_state = STATE_GAMEOVER;
			} else if (g_menuItem==2) {			
				// Help
				g_menuState = MENU_HELP;
			} else if (g_menuItem==3) {
				do_quit();
			}
		} else if (g_menuState == MENU_HELP ) {
			g_menuState = MENU_MAINMENU;
		}
	}	
}

void doActivateStation() 
{
	g_view->activateStation();
	switch( g_view->getStation() ) {
	case SWAP_NDX1:
	case SWAP_NDX2:
	case SWAP_NDX3:
		playSound( SFX_WHIISH );
		break;
	case FREEZER_NDX:
		playSound( SFX_FREEZE );
		break;
	case TORCHY_NDX:
		playSound( SFX_BURN );
		break;
	}
}

// TODO: This is really crappy.. need to allocate channels and stuff.
void playSound( int sfxId )
{
	sfx_chan[sfxId] = Mix_PlayChannel(-1, sfx[sfxId], 0);
}

void prevMenuItem()
{
	if (g_menuState == MENU_MAINMENU) {
		g_menuItem--;
		if (g_menuItem<0) g_menuItem=3;
	} else if (g_menuState == MENU_PICK_LEVEL) {
		g_levelNdx--;
		if (g_levelNdx<0) g_levelNdx=g_game->getNumLevels()-1;
	}
}

void nextMenuItem()
{
	if (g_menuState == MENU_MAINMENU) {
		g_menuItem++;
		if (!mainMenu[g_menuItem]) g_menuItem=0;
	} else if (g_menuState == MENU_PICK_LEVEL) {		
		g_levelNdx++;
		if (g_levelNdx==g_game->getNumLevels()) g_levelNdx=0;
	}	
}

//////////////////////////////////////////////////////////////////////////
// Main program
//////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[]) {

// this was a quick test of InetOpenURL
#if 0
	HINTERNET hnet = InternetOpen( "Game", INTERNET_OPEN_TYPE_PRECONFIG, 
									NULL, NULL, 0 );
	HINTERNET hfile = InternetOpenUrl( hnet, "http://www.yahoo.com", 
										NULL, 0, 
										INTERNET_FLAG_DONT_CACHE, 0);
	char buff[5000];
	unsigned long nread;
	InternetReadFile( hfile, buff, 5000, &nread );
	buff[nread] = 0;
	printf( buff );
#endif

	if ( SDL_Init(SDL_INIT_NOPARACHUTE | SDL_INIT_AUDIO|SDL_INIT_VIDEO |SDL_INIT_JOYSTICK) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

	if( SDL_SetVideoMode( 800, 600, 16, SDL_OPENGL /*| SDL_FULLSCREEN */ ) == 0 ) {
		fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
	}

	SDL_WM_SetCaption( "The Halloween Machine", NULL );

	// Init SDL_Mixer
	if (Mix_OpenAudio( 22050, AUDIO_S16, 2,  4096 )) {
		fprintf(stderr,	"Unable to open audio\n");
        exit(1);
	}

	// Initialize Joysticks/Gamepads
	printf("%d Controllers found.\n", SDL_NumJoysticks() );
	for (int i=0; i < SDL_NumJoysticks(); i++) {
		printf( "Controller %d: %s\n", i, SDL_JoystickName(i) );
	}
	gamepad.name = "No gamepad detected.";
	gamepad.sdlinfo = NULL;
	gamepad.stick = vec2f(0.0, 0.0 );
	gamepad.stick2 = vec2f(0.0, 0.0 );

	// Only use the first controller
	if (SDL_NumJoysticks() > 0 ) {
		gamepad.sdlinfo = SDL_JoystickOpen(0);
	}

	// init highscores
	readHighScores();	

	if (gamepad.sdlinfo) {
		gamepad.name = strdup( SDL_JoystickName(0) );
		printf( "Gamepad: Number of axes: %d\n", SDL_JoystickNumAxes(gamepad.sdlinfo) );
		printf( "Gamepad: Number of buttons: %d\n", SDL_JoystickNumButtons(gamepad.sdlinfo) );
		printf( "Gamepad: Number of balls: %d\n", SDL_JoystickNumBalls(gamepad.sdlinfo) );
		printf( "Gamepad: Number of hats: %d\n", SDL_JoystickNumHats(gamepad.sdlinfo) );
	}

	

	// initialize DevIL
	ilInit();
	ilutRenderer( ILUT_OPENGL );

	// Setup glgameFont
	
	// Load the font image
	ilGenImages( 1, &ilFontId );
	ilBindImage( ilFontId );		
	
	if (!ilLoadImage( "gamedata/magic.png" )) {
		printf("Loading font image failed\n");
	}
	
	// Make a GL texture for it
	glFontTexId = ilutGLBindTexImage();

	// Create a font by passing in an opengl texture id
	fntFontId = gfCreateFont( glFontTexId );

	// A .finfo file contains the metrics for a font. These
	// are generated by the Fontpack utility.
	gfLoadFontMetrics( fntFontId, "gamedata/magic.finfo");

	printf("font has %d chars\n", gfGetFontMetric( fntFontId, GF_FONT_NUMCHARS ) );
	gfEnableFont( fntFontId, 25 );		

	sfx[0] = Mix_LoadWAV("gamedata/whiit.wav");
	sfx[1] = Mix_LoadWAV("gamedata/didge.wav");
	sfx[2] = Mix_LoadWAV("gamedata/sfx_boom.wav");
	sfx[3] = Mix_LoadWAV("gamedata/harp.wav");
	sfx[4] = Mix_LoadWAV("gamedata/secret.wav");

	music_title = Mix_LoadMUS( "gamedata/AutumnLeft.ogg" );
	music_gameover = Mix_LoadMUS( "gamedata/YouDiedLoser.ogg" );
	music_ingame = Mix_LoadMUS( "gamedata/ingame.ogg" );

	if (!music_title) {
		printf("Error loading music %s\n", Mix_GetError() );
	} else {
		printf("Music loaded\n");
	}
	Mix_PlayMusic( music_title, -1 );

	
	// Start up the game
	g_game = new GameState();
	g_game->init();

	g_game->loadLevels( "gamedata/levels.txt" );

	g_view = new ViewSimple2D( g_game );
	g_game->setView( g_view );	

	//////////////////////
	// Event Loop
	//////////////////////
	while (1) {		
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {
		
        switch( event.type ) {
        case SDL_KEYDOWN:

			// are we reading highscore??
			if (ndxGetName >=0 ) {
				char buff[10];
				if ( ((event.key.keysym.sym >= SDLK_a) && (event.key.keysym.sym <= SDLK_z)) ||					 
					 ((event.key.keysym.sym >= SDLK_0) && (event.key.keysym.sym <= SDLK_9))) {
					sprintf( buff, "%c", (char)event.key.keysym.sym );

					if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) {
						buff[0] = toupper(buff[0]);
					}

					if (!strcmp(g_highScoreTable[ndxGetName].m_name.c_str(), HISCORE_PROMPT)) {
						g_highScoreTable[ndxGetName].m_name = "";
					}

					g_highScoreTable[ndxGetName].m_name.append( buff);

				} else if ( (event.key.keysym.sym==SDLK_BACKSPACE) ) {
					if (g_highScoreTable[ndxGetName].m_name.length()){ 
						g_highScoreTable[ndxGetName].m_name.erase( 
											g_highScoreTable[ndxGetName].m_name.end()-1 );
					}
				} else if ( (event.key.keysym.sym==SDLK_RETURN) ||
					      (event.key.keysym.sym==SDLK_ESCAPE) ) {

					if (!g_highScoreTable[ndxGetName].m_name.length()) {
						g_highScoreTable[ndxGetName].m_name = "Anonymous";
					}

					ndxGetName = -1;


					writeHighScores();
				}

				break;
			} 

            switch( event.key.keysym.sym ) {

				case SDLK_ESCAPE:				
					if ( (g_state==STATE_TITLE) &&
						(g_menuState == MENU_MAINMENU)) {
						do_quit();
					} else {
						backToTitleScreen();
					}
					break;

				case SDLK_a:
					g_view->toggleAnim();
					break;

				case SDLK_p:
					paused = !paused;
					break;

// Cheat codes.. enable for testing
#if 0
				case SDLK_z:
					if (g_state==STATE_GAMEOVER) {
						ndxGetName = 3;
						break;
					}
	
					

				case SDLK_1:
					g_game->dbgNextPumpkin( PUMPKIN );
					break;
				case SDLK_2:
					g_game->dbgNextPumpkin( PUMPKIN_ORANGE );
					break;
				case SDLK_3:
					g_game->dbgNextPumpkin( PUMPKIN_YELLOW );
					break;
				case SDLK_4:
					g_game->dbgNextPumpkin( PUMPKIN_RED );
					break;
				case SDLK_5:
					g_game->dbgNextPumpkin( SKULL );
					break;
				case SDLK_6:
					g_game->dbgNextPumpkin( FISH_HEAD );
					break;
				case SDLK_7:
					g_game->dbgNextPumpkin( BLACKBIRD );
					break;

				case SDLK_8:
					g_game->dbgClearQueue();
					break;
#endif


				case SDLK_s:
					ilutGLScreenie();
					break; 

				case SDLK_d:
					bDrawGamepad = !bDrawGamepad;
					break;

				case SDLK_m:					

					if (!musicOn) {
						printf("Playing\n");
						if (g_state==STATE_TITLE) {
							Mix_PlayMusic( music_title, -1 );
						} else if (g_state==STATE_PLAYING) {
							Mix_PlayMusic( music_ingame, -1 );
						} else if (g_state==STATE_GAMEOVER) {
							// easter egg.. kindof.. you get to hear the
							// game over music again. woo.
							Mix_PlayMusic( music_gameover, 0 );
						}
						musicOn = 1;
					} else {
						printf("Halting\n");
						Mix_HaltMusic();
						musicOn = 0;
					}


				case SDLK_LEFT:
					g_view->nextStation();
					break;
				case SDLK_RIGHT:
					g_view->prevStation();
					break;

				case SDLK_SPACE:
				case SDLK_RETURN:
					if (g_state == STATE_PLAYING) {
						doActivateStation();
					} else {
						doStartButton();
					}
					
					break;

				case SDLK_UP:
					if (g_state==STATE_PLAYING) {
						//g_view->activateStation();
						doActivateStation();
					} else if (g_state==STATE_TITLE ) {
						prevMenuItem();						
					}
					break;

				case SDLK_DOWN:
					if (g_state==STATE_TITLE ) {
						nextMenuItem();						
					}
					break;

				default:
					break;
				}
			break;

		

		case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
			if ( ( event.jaxis.value < -3200 ) || (event.jaxis.value > 3200 ) ) 
			{				

				switch(event.jaxis.axis) {				
				case 0:
					gamepad.stick[0] = (float)(event.jaxis.value) / 32768.0f;
					break;
				case 1:
					gamepad.stick[1] = (float)(event.jaxis.value) / 32768.0f;
					break;
				case 2:
					gamepad.throttle = (float)(event.jaxis.value) / 32768.0f;
					break;
				case 3:
					gamepad.stick2[0] = (float)(event.jaxis.value) / 32768.0f;
					break;
				case 4:															
					gamepad.stick2[1] = (float)(event.jaxis.value) / 32768.0f;					
					break;						
				}				
				
			} else {
				// prevent jitter near the center positions
				switch(event.jaxis.axis) {
				case 0:
					gamepad.stick[0] = 0.0;
					break;
				case 1:
					gamepad.stick[1] = 0.0;
					break;
				case 2:
					gamepad.throttle = 0.0;
					break;
				case 3:
					gamepad.stick2[0] = 0.0;
					break;
				case 4:
					gamepad.stick2[1] = 0.0;
					break;				
				}
			}
			break;

		case SDL_JOYBUTTONDOWN:  /* Handle Joystick Button Presses */
			gamepad.button[event.jbutton.button] = true;
			
			switch (event.jbutton.button) {
			case 0:				
				//g_view->activateStation();
				if (g_state==STATE_PLAYING) {
					doActivateStation();
				} else {
					doStartButton();
				}
				break;
			//case 1:
			//	sfx_chan[1] = Mix_PlayChannel(-1, sfx[1], 0);
			//	break;
			//case 2:
			//	sfx_chan[2] = Mix_PlayChannel(-1, sfx[2], 0);
			//	break;

			case 8: // start button 
				doStartButton();				
				break;

			}
			break;

		case SDL_JOYBUTTONUP:  /* Handle Joystick Button Release */
			gamepad.button[event.jbutton.button] = false;			
			break;

		case SDL_JOYHATMOTION:  /* Handle Hat Motion */			
			gamepad.hat = event.jhat.value;			

			if (g_state==STATE_PLAYING) {
	
				if (gamepad.hat & SDL_HAT_LEFT) {
				
					g_view->nextStation();
				}

				if (gamepad.hat & SDL_HAT_RIGHT) {
					g_view->prevStation();
				}				
			} else {
				if (gamepad.hat & SDL_HAT_UP) {
				
					prevMenuItem();
				}

				if (gamepad.hat & SDL_HAT_DOWN) {
					nextMenuItem();
				}
			}

			break;
			case SDL_QUIT:
				/* Handle quit requests (like Ctrl-c). */
				if ( (g_state==STATE_TITLE) &&
					(g_menuState == MENU_MAINMENU)) {
					do_quit();
				} else {
					backToTitleScreen();
				}
				break;
			}	
		}
		redraw();
	}
	return 0;
} 