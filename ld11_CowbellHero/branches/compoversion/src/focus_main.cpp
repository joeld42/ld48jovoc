#ifdef WIN32
	#include <windows.h>
#endif

#include <string>
#include <map>
#include <algorithm>

#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include "focus.h"
#include "beat.h"
#include "world.h"
#include "musicdb.h"

#include "gamefontgl.h"

using namespace std;

// game state
enum 
{
	MODE_TITLE,
	MODE_GAME,
	MODE_PICKSONG
};

enum
{
	PICK_ARTIST,
	PICK_TITLE,
};

const char *menuItem[] = { "Start Game",
							"Difficulty",
						  "Choose Song",
						  "Random Song",
							"Quit",
						  NULL };
int menuNdx = 0;
int pickMode = PICK_ARTIST;

// The game world
World *world;
int mode = MODE_TITLE;

// stuff for title screen
ILuint ilTitleCard;
GLuint glTexId_TitleImg;

// Game font
ILuint ilFontId;
GLuint glFontTexId, fntFontId;

// Song list
SongList songs;
std::vector<std::string> artistNames;
std::vector<std::string> songNames;
std::vector<std::string> songPath;

size_t currArtistNdx  = 0;
size_t currSongNdx = 0;

string currSong   = "RANDOM";
string currArtist = "RANDOM";
string currFullPath;

// cowbell
FSOUND_SAMPLE *sfx_cowbell = NULL;
FSOUND_SAMPLE *sfx_doh = NULL;

int irand( int min, int max )
{
	return min + (int)( ((float)rand() / (float)RAND_MAX) * (float)(max-min) );
}

float frand( float min, float max )
{	
	return min + ((float)rand() / (float)RAND_MAX) * (max-min);
}


float clamp( float n )
{
	if (n < 0.0f) return 0.0f;
	if (n > 1.0f) return 1.0f;
	return n;
}

int iclamp( int n ) 
{
	if (n < 0) return 0;
	if (n > 0xff) return 0xff;
	return n;
}

void GetSongNamesForCurrentArtist();

void ChooseRandomSong()
{
	printf("Choose Random\n" );

	currArtistNdx = irand( 0, artistNames.size() );
	currArtist = artistNames[currArtistNdx ];

	std::vector<Song> &asongs = songs[ currArtist ];
	currSongNdx = irand( 0, asongs.size() );
	currSong = asongs[ currSongNdx ].title;
	currFullPath = asongs[ currSongNdx ].filename;

	GetSongNamesForCurrentArtist();
}

void GetSongNamesForCurrentArtist()
{
	string aname = artistNames[currArtistNdx];
	SongList::iterator ai = songs.find( aname );
	if (ai == songs.end() )
	{
		printf("Could not find %s\n", aname.c_str() );
		return;
	}

	std::vector<Song> &asongs = (*ai).second;

	printf("GetSongNamesForCurrentArtist---\n" );
	printf("%d songs... \n", asongs.size() );

	songNames.clear();
	songPath.clear();
	for ( std::vector<Song>::iterator si = asongs.begin();
		  si != asongs.end(); ++si )
	{		
		songNames.push_back( (*si).title );
		songPath.push_back( (*si).filename );
	}	
}

void DrawMenuList( const std::vector<string> &items, 
				  size_t &currNdx, float xpos, Beatronome &nome,
				  bool active )
{
	int numItems = items.size();

	if (numItems > 15) numItems = 15;

	gfEnableFont( fntFontId, 15 );
	gfBeginText();
	for (int ndx =0; ndx < numItems; ++ndx)
	{
		size_t andx = (currNdx - (numItems/2) + ndx) % items.size();
		glPushMatrix();

		if (ndx == (numItems/2) )
		{
			if (active)
			{
				glColor3f( 1.0f, nome.beat_pulse2, 0.0f );
			}
			else
			{
				glColor3f( 1.0f, 0.5, 0.0f );
			}
		}
		else
		{
			glColor3f( 1.0f, 1.0f, 1.0f );
		}

		glTranslated( xpos, 250 - (ndx*15), 0 );
		gfDrawString( items[andx].c_str() );
		glPopMatrix();
	}
	gfEndText();
}

void moarCowbell( Beatronome &nome )
{
	int beat_ndx = (nome.beat_count * 4) + nome.beat_subcount;

	//float xtra = nome.beat_pulse * 4.0;
	//xtra = xtra - floor(xtra);
	//if (xtra > 0.8)
	//{
		// look at the next beat
	//	beat_ndx++;
	//}

	// Is the user supposed to hit the cowbell at this point
	// in time?
	if (world->gemtrack[beat_ndx] == GEM_NOTE)
	{
		printf("---> Hit!\n" );
		// Yay... moar cowbell!
		FSOUND_PlaySound(FSOUND_FREE, sfx_cowbell );
		world->gemtrack[beat_ndx] = GEM_NOTE_HIT;
	}
	else
	{	
		printf("---> Miss!\n" );

		// Nope. You suck. Get off the stage.
		FSOUND_PlaySound(FSOUND_FREE, sfx_doh );
		//world->gemtrack[beat_ndx] = GEM_NOTE_MISSED;
	}
}

void redraw( Beatronome &nome )
{
	glClearColor( 0.2f, 0.2f, 0.4f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// 2d part
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	if ( ( mode == MODE_TITLE ) ||
		 ( mode == MODE_PICKSONG ) )
	{
#if 0
		//test
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );

		glColor3f( 1.0f, 1.0f, 1.0f );
		glBegin( GL_LINES );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( 800, 600, 0.0f );

		glVertex3f( 800.0f, 0.0f, 0.0f );
		glVertex3f( 0.0f, 600.0f, 0.0f );
		glEnd();
#endif
		static bool initTitle = false;
		if (!initTitle)
		{
			initTitle = true;			

			glEnable( GL_TEXTURE_2D );
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
			
			ilGenImages( 1, &ilTitleCard );
			if (!ilLoadImage( (ILstring)"gamedata/title.png" ))
			{
				printf("Loading title image failed.\n" );
			}

			glTexId_TitleImg = ilutGLBindTexImage();

			// Load the font image
			ilGenImages( 1, &ilFontId );
			ilBindImage( ilFontId );		
	
			if (!ilLoadImage( (ILstring)"gamedata/focusfont.png" )) {
				printf("Loading font image failed\n");
			}
	
			// Make a GL texture for it
			glFontTexId = ilutGLBindTexImage();
			fntFontId = gfCreateFont( glFontTexId );

			// A .finfo file contains the metrics for a font. These
			// are generated by the Fontpack utility.
			gfLoadFontMetrics( fntFontId, "gamedata/focusfont.finfo");

			printf("font has %d chars\n", 
				gfGetFontMetric( fntFontId, GF_FONT_NUMCHARS ) );					

		}

		// draw the title card
		glBindTexture( GL_TEXTURE_2D, glTexId_TitleImg );

		glEnable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		

		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_QUADS );
		glTexCoord2d( 0.0, 0.0 ); glVertex3f( 0.0, 600.0, 0.0 );
		glTexCoord2d( 0.0, 1.0 ); glVertex3f( 0.0,  400.0, 0.0 );
		glTexCoord2d( 1.0, 1.0 ); glVertex3f(  800.0,  400.0, 0.0 );
		glTexCoord2d( 1.0, 0.0 ); glVertex3f(  800.0, 600.0, 0.0 );
		glEnd();

		// Bottom text
		gfEnableFont( fntFontId, 15 );	
		gfBeginText();
		glTranslated( 200, 10, 0 );
		gfDrawString( "Ludumdare 11 - Joel Davis (joeld42@yahoo.com)" );
		gfEndText();

		if (artistNames.size() > 0 )
		{
			// Draw Menu
			if ( mode == MODE_TITLE )
			{
				gfEnableFont( fntFontId, 32 );
				gfBeginText();
		
				for (int ndx =0; menuItem[ndx]; ++ndx)
				{
					glPushMatrix();
					glTranslated( 100, 250 - (ndx*40), 0 );

					if (ndx == menuNdx)
					{
						glColor3f( 1.0f, nome.beat_pulse2, 0.0f );
					}
					else
					{
						glColor3f( 1.0f, 1.0f, 1.0f );
					}
					gfDrawString( menuItem[ndx] );

					glPopMatrix();			
				}					
				gfEndText();

				gfEnableFont( fntFontId, 15 );
				gfBeginText();
	
				glColor3f( 1.0f, 1.0f, 1.0f );

				glPushMatrix();
				glTranslated( 400, 300, 0 );
				gfDrawStringFmt( " Artist: %s",  currArtist.c_str() );
				glPopMatrix();			

				glPushMatrix();
				glTranslated( 415, 280, 0 );
				gfDrawStringFmt( " Song: %s",  currSong.c_str() );
				glPopMatrix();												

				glPushMatrix();
				glTranslated( 380, 260, 0 );
				gfDrawStringFmt( "Difficulty: %s",  nome.easy_mode?"Normal":"Hard" );
				glPopMatrix();												
					
				gfEndText();
			}
			else
			{

				//gfEnableFont( fntFontId, 32 );
				//gfBeginText();
				//gfEndText();
			
				// artist menu				
				DrawMenuList( artistNames, currArtistNdx, 
					100, nome, pickMode == PICK_ARTIST);			

				// song menu
				DrawMenuList( songNames, currSongNdx, 
					500, nome, pickMode == PICK_TITLE );	


			}
		}
		else
		{
			gfEnableFont( fntFontId, 32 );
			gfBeginText();
			glPushMatrix();
			glTranslated( 20, 280, 0 );
			gfDrawString( "Missing or Empty Song DB. See README.txt for\n" 
						  "instructions on how generate one from your music\n" 
						  "library using the included musicdb.py script."
				);
			glPopMatrix();
			gfEndText();

		}
		
	}

	else if (mode == MODE_GAME )
	{
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );

		// redraw the world
		world->redraw( nome);

		// draw metronome
		nome.redraw( 25, 25 );

		gfEnableFont( fntFontId, 15 );
		gfBeginText();
	
		glColor3f( 1.0f, 1.0f, 1.0f );

		glPushMatrix();
		glTranslated( 10, 585, 0 );
		gfDrawStringFmt( " Artist: %s",  currArtist.c_str() );
		glPopMatrix();			

		glPushMatrix();
		glTranslated( 25, 570, 0 );
		gfDrawStringFmt( " Song: %s",  currSong.c_str() );
		glPopMatrix();									
				
		gfEndText();
	}
}



int main( int argc, char *argv[] )
{
	// Init fmod
	if (!FSOUND_Init( 44100, 32, 0 ))
	{
		printf("FSound init failed\n" );
	}

	//FSOUND_SetOutput( FSOUND_OUTPUT_WINMM );
	sfx_cowbell = FSOUND_Sample_Load( FSOUND_UNMANAGED, "gamedata/cowbell.wav", FSOUND_NORMAL, 0, 0 );
	sfx_doh = FSOUND_Sample_Load( FSOUND_UNMANAGED, "gamedata/sfxr_doh.wav", FSOUND_NORMAL, 0, 0 );

	srand( time(NULL) );

	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		fprintf( stderr, "Unable to init SDL: %s\n", SDL_GetError() );
	}

	if (SDL_SetVideoMode( 800, 600, 32, SDL_OPENGL /*| SDL_FULLSCREEN */ ) == 0 ) 
	{
		fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
	}

	SDL_WM_SetCaption( "LD11 Focus", NULL );

	// initialize DevIL
	ilInit();
	ilutRenderer( ILUT_OPENGL );

	// Load the song DB
	LoadSongList( songs );
	
	for (SongList::iterator ca = songs.begin(); ca != songs.end(); ++ca )
	{
		printf( ">> %s\n", (*ca).first.c_str() );	
		artistNames.push_back( (*ca).first );
	}	

	// Main Loop
	Uint32 ticks = SDL_GetTicks(), ticks_elapsed, sim_ticks = 0;
	bool beat_sample = false;
	bool done = false;

	world = new World();

	Beatronome nome;

	while(!done)
	{
		SDL_Event event;

		while (SDL_PollEvent( &event ) ) 
		{
			switch (event.type )
			{
				case SDL_KEYDOWN:

					// global keys
					//if (event.key.keysym.sym == SDLK_ESCAPE)
					//{
					//	done = true;
					//}

					if (mode == MODE_TITLE)
					{
						switch( event.key.keysym.sym ) 
						{						
							case SDLK_DOWN:
								menuNdx++;
								if (menuItem[menuNdx] == NULL)
								{
									menuNdx = 0;
								}
								break;

							case SDLK_UP:
								if (menuNdx==0)
								{
									while (menuItem[menuNdx+1]) menuNdx++;
								}
								else
								{
									menuNdx--;
								}
								break;

							case SDLK_RETURN:
							case SDLK_SPACE:

								// bail 
								if (artistNames.size() == 0)
								{
									done = 1;
									break;
								}

								if (!strcmp( menuItem[menuNdx], "Quit" ))
								{
									done = true;
								}								
								else if (!strcmp( menuItem[menuNdx], "Start Game" ))
								{
									if ( (currSong == "RANDOM") || 
										  (currArtist == "RANDOM" ) )
									{
										ChooseRandomSong();
									}
									mode = MODE_GAME;

									// Start playing									
									world->setMusic( currFullPath.c_str() );
										//(string("\"") + currFullPath.c_str() + "\"").c_str() );
									world->playMusic();
									nome.beat_count = 0;
									nome.beat_subcount = 0;
								}
								else if (!strcmp( menuItem[menuNdx], "Choose Song" ))
								{
									mode = MODE_PICKSONG;
								}
								else if (!strcmp( menuItem[menuNdx], "Random Song" ))
								{
									ChooseRandomSong();								
								}
								else if (!strcmp( menuItem[menuNdx], "Difficulty" ))
								{
									nome.easy_mode = !nome.easy_mode;
								}
								break;
							case SDLK_c:
								printf("test cowbell\n" );
								
								FSOUND_PlaySound(FSOUND_FREE, sfx_cowbell );

								break;
							
							case SDLK_ESCAPE:
								done = true;
								break;
							}
						}					
						else if ( mode == MODE_GAME )
						{
							switch( event.key.keysym.sym ) 
							{		
								case SDLK_ESCAPE:
									world->stopMusic();
									mode = MODE_TITLE;
									break;

								case SDLK_LSHIFT:
								case SDLK_RSHIFT:								
									beat_sample = true;
									break;

								case SDLK_c:
								case SDLK_SPACE:
								case SDLK_LALT:
								case SDLK_RALT:
									printf("cowbell\n" );						
									moarCowbell(nome);
									//FSOUND_PlaySound(FSOUND_FREE, sfx_cowbell );
									break;

								case SDLK_r:
									printf("test restart\n" );		
									nome.beat_count = 0;
									nome.beat_subcount = 0;
									break;
							}
						}
						else if (mode == MODE_PICKSONG)
						{
							switch( event.key.keysym.sym ) 
							{
								case SDLK_LEFT:
								case SDLK_RIGHT:
									pickMode = !pickMode;
									break;

								case SDLK_RETURN:
									mode = MODE_TITLE;
									currSong = songNames[currSongNdx];
									currFullPath = songPath[currSongNdx];
									currArtist = artistNames[currArtistNdx];									
									break;								
							}


							if (pickMode == PICK_ARTIST)
							{
								switch( event.key.keysym.sym ) 
								{								
									case SDLK_DOWN:
										currArtistNdx++;
										if (	currArtistNdx == artistNames.size())
										{
											currArtistNdx = 0;
										}
										GetSongNamesForCurrentArtist();
										break;

									case SDLK_UP:
										if (currArtistNdx == 0)
										{
											currArtistNdx = artistNames.size() - 1 ;
										}
										else currArtistNdx--;

										GetSongNamesForCurrentArtist();
										break;
									case SDLK_PAGEUP:
										currArtistNdx = (currArtistNdx - 15) % (artistNames.size()-1);
										GetSongNamesForCurrentArtist();
										break;
									case SDLK_PAGEDOWN:
										currArtistNdx = (currArtistNdx + 15) % (artistNames.size()-1);
										GetSongNamesForCurrentArtist();
										break;									
								}							
							}
							else if (songNames.size() > 0 )
							{

								switch( event.key.keysym.sym ) 
								{								
									case SDLK_DOWN:
										currSongNdx++;
										if (currSongNdx == songNames.size())
										{
											currSongNdx = 0;
										}										
										break;

									case SDLK_UP:
										if (currSongNdx == 0)
										{
											currSongNdx = songNames.size() - 1 ;
										}
										else currSongNdx--;
										break;
									case SDLK_PAGEUP:
										currSongNdx = (currSongNdx - 15) % (songNames.size()-1);
										GetSongNamesForCurrentArtist();
										break;
									case SDLK_PAGEDOWN:
										currSongNdx = (currSongNdx + 15) % (songNames.size()-1);
										GetSongNamesForCurrentArtist();
										break;
									
								}							
							}
						}
						break;
				case SDL_QUIT:
					done = true;
					break;
			}
		}

		// Timing
		ticks_elapsed = SDL_GetTicks() - ticks;
		ticks += ticks_elapsed;

		// update beat counter
		if (beat_sample) 
		{
			nome.beat_sample( ticks );
			beat_sample = false;
		}

		// update beat
		nome.beat_update( ticks_elapsed );

		// fixed sim update
		sim_ticks += ticks_elapsed;
		while (sim_ticks > STEPTIME) 
		{
			sim_ticks -= STEPTIME;			
			//update();

//			printf("update sim_ticks %d ticks_elapsed %d\n", sim_ticks, ticks_elapsed );

			nome.beat_sim_update();
			world->update( nome );

			//if (beat_trigger) beat_trigger = false;
		}	

		// update the graphics as often as possible
		redraw( nome );
		SDL_GL_SwapBuffers();
	}

	SDL_Quit();

	return 0;
}