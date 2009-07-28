#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include "boost/program_options.hpp"

#include "GlacierGame.h"
#include "GameView.h"

#include "ld14wall.h"

#define DEFAULT_SCREEN_X (1280)
#define DEFAULT_SCREEN_Y (1024)

using namespace std;


int g_state = STATE_MENU;
	


void errorMessage( std::string msg )
{
	// todo: on linux or other real OS, just printf
	MessageBoxA( NULL, msg.c_str(), "ld14walls ERROR", MB_OK | MB_ICONSTOP );
}

bool IsScreenSizeValid( int sx, int sy, bool fullscreen )
{
	// Anything is ok for a window
	if (!fullscreen) return true;

	// Check if the mode is supported
	SDL_Rect **modes;
	modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

	if (!modes) return false; // no fullscreen modes supported

	if (modes==(SDL_Rect**)-1) return true; // no mode restrictions

	// look for res in list of screen modes
	for (int i=0; modes[i]; i++) 
	{
		if ((sx==modes[i]->w) && (sy==modes[i]->h)) return true;
	}

	return false;
}

// 30 ticks per sim frame
#define STEPTIME (33)

//============================================================================
int main( int argc, char *argv[] )
{	

	
	// Command line only arguments (not stored in cfg file)
	boost::program_options::options_description arg_desc_generic("Command Line Arguments");
	arg_desc_generic.add_options()
		("help,h", "produce help message")
		;

	// Common options (config file and command line)
	boost::program_options::options_description arg_desc_common("Configuration");
	arg_desc_common.add_options()
		("fullscreen,f", 
				boost::program_options::value<int>()->default_value( 0 ), 
				"run fullscreen" )
		("screenw,W", 
			boost::program_options::value<int>()->default_value( 1280 ), 
			"screen width" )
		("screenh,H", 
			boost::program_options::value<int>()->default_value( 1024 ), 
			"screen height" )
		("vsync,v", 
			boost::program_options::value<int>()->default_value( 1 ), 
			"enable vsync" )
		;	

	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		fprintf( stderr, "Unable to init SDL: %s\n", SDL_GetError() );
	}

	// get options from command line
	boost::program_options::options_description arg_desc_cmdline;
	arg_desc_cmdline.add( arg_desc_generic).add(arg_desc_common );
	boost::program_options::variables_map arg_map;
	try 
	{
		boost::program_options::store( 
			boost::program_options::parse_command_line( argc, argv, arg_desc_cmdline), arg_map );
		boost::program_options::notify( arg_map );
	} 
	catch (boost::program_options::error err_opt) 
	{
		cout << "Invalid argument: " << err_opt.what() << endl;
		cout << arg_desc_cmdline << "\n";
		return 1;
	}

	// Print usage
	if (arg_map.count("help")) 
	{
		cout << arg_desc_cmdline << "\n";
		return 1;
	}

	// Gameplay Options (hidden on command line)
	boost::program_options::options_description arg_desc_gameplay("Gameplay");
	arg_desc_gameplay.add_options()
			("gameplay.travel_speed",  boost::program_options::value<float>()->default_value( 0 ), "travel speed" )
			;	

	// This is annoying -- should do this in a way that we don't have 
	// to repeat these.. 
	arg_desc_gameplay.add_options()
			("mammoth.delay",  boost::program_options::value<int>()->default_value( 20 ), "mammoth delay" )			
			;

	arg_desc_common.add( arg_desc_gameplay );	

	// get options from config file	
	try
	{
		ifstream ifs_config("settings.txt");
		if (ifs_config.fail() ) {
			printf("DBG: Can't find config file.\n" );
		}
			
		boost::program_options::store(					
			boost::program_options::parse_config_file( ifs_config, arg_desc_common ),
			arg_map );
		boost::program_options::notify( arg_map );
	}
	catch (boost::program_options::error err_opt) 
	{
		errorMessage( string("Error reading config file: ") + err_opt.what() );

		cout << "Error reading config file:" << err_opt.what() << endl;
		cout << "Valid config entries are:" << endl;
		cout << arg_desc_common << "\n";
		return 1;
	}

	// Screen size
	Uint32 sx = DEFAULT_SCREEN_X, 
		   sy = DEFAULT_SCREEN_Y;
	bool fullscreen = false;
	
	if (arg_map.count("screenw"))
	{
		sx = arg_map["screenw"].as<int>();
	}

	if (arg_map.count("screenh"))
	{
		sy = arg_map["screenh"].as<int>();
	}	

	if (arg_map.count("fullscreen"))
	{
		if(arg_map["fullscreen"].as<int>())
		{
			fullscreen = true;	
		}
	}
	
	if (!IsScreenSizeValid(sx, sy, fullscreen ))
	{
		char buff[1024];
		sprintf(buff, "Invalid screen size %d x %d", sx, sy );
		errorMessage( buff );
		exit(1);
	}	
	else 
	{		
	
		Uint32 mode_flags = SDL_OPENGL;
		if (fullscreen) 
		{
			mode_flags |= SDL_FULLSCREEN;
		}
		if (SDL_SetVideoMode( sx, sy, 32, mode_flags ) == 0 ) 
		{
			errorMessage( string("Unable to set video mode: ") +  SDL_GetError() ) ;
			exit(1);
		}
	}	

	SDL_WM_SetCaption( "LD14 Glacier", NULL );

	// seed rand
	srand( time(0) );

	// initialize DevIL
	ilInit();
	ilutRenderer( ILUT_OPENGL );

	// Game Stuff
	GlacierGame *game = new GlacierGame();
	game->m_gameplayOpts.add( arg_desc_common );

	// Apply any gameplay options
	game->applyOptions( arg_map );	

	GameView *gameView = new GameView( game );
	

	gameView->m_screenX = sx;
	gameView->m_screenY = sy;

	// fill list of levels
	WIN32_FIND_DATAA ffd;
	HANDLE hFind;   
   
	hFind = FindFirstFileA( "levels/*.txt", &ffd);
	do
	{
			//_tprintf (TEXT("The first file found is %s\n"), 
			//		ffd.cFileName);
		_RPT1( _CRT_WARN, "Found Level: %s\n", ffd.cFileName );
		if (ffd.cFileName[0] != '_')
		{
			gameView->m_levelNames.push_back( string( ffd.cFileName ) );
		}
		
	} while (FindNextFileA(hFind, &ffd) != 0);
	FindClose(hFind);

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
							done = true;
							break;
						
						case SDLK_F8:
							game->m_editMode = !game->m_editMode;
							break;		

						case SDLK_UP:
							if (g_state==STATE_GAME)
							{
								if (game->m_editMode)
								{								
									if (gameView->m_editHite < 3) 
										gameView->m_editHite++;
								}
							}
							else
							{
								gameView->m_currMenuItem--;
								if (gameView->m_currMenuItem < 0)
								{
									gameView->m_currMenuItem = 2;
								}
							}
							break;

						case SDLK_DOWN:
							if (g_state==STATE_GAME)
							{
								if (game->m_editMode)
								{
									if (gameView->m_editHite > 1) 
										gameView->m_editHite--;								
								}
							}
							else
							{
								gameView->m_currMenuItem++;
								if (gameView->m_currMenuItem > 2)
								{
									gameView->m_currMenuItem = 0;
								}
							}
							break;

						case SDLK_LEFT:
							if (g_state==STATE_MENU)
							{
								if (gameView->m_levelNdx>0)
								{
									gameView->m_levelNdx--;
								}
								else
								{
									gameView->m_levelNdx  = gameView->m_levelNames.size() -1;
								}
							}
							break;

						case SDLK_RIGHT:
							if (g_state==STATE_MENU)
							{
								gameView->m_levelNdx++;
								if(gameView->m_levelNdx>=gameView->m_levelNames.size() )
								{
									gameView->m_levelNdx = 0;
								}								
							}
							break;

						case SDLK_RETURN:
							if (g_state==STATE_GAME)
							{
								if (game->m_editMode)
								{
									// commit current block
									gameView->m_editBlock = NULL;
								}
							}
							else if (g_state==STATE_MENU)
							{
								
								if (gameView->m_currMenuItem==2)
								{
									done = true;
								}
								else // play or edit
								{
									// TODO: Choose level to load
									if (gameView->m_currMenuItem==0)
									{
										string levelName = string("levels/") + gameView->m_levelNames[ gameView->m_levelNdx ] ;
										game->loadLevel( levelName.c_str() );
									}
									else
									{
										game->loadLevel( "levels/_editor.txt" );
									}

									if (gameView->m_currMenuItem==1) 
										game->m_editMode=1;
									g_state = STATE_GAME;
								}
							}
							else if (g_state==STATE_RESULTS)
							{
								g_state = STATE_MENU;
							}
							break;

						case SDLK_BACKSPACE:
							if (game->m_editMode)
							{
								if (game->m_landBlocks.size())
								{
									// delete current block
									gameView->m_editBlock = NULL;

									// TODO: delete
									game->m_landBlocks.pop_back();
								}
							}
							break;

						case SDLK_z:
							if (game->m_editMode)
							{
								gameView->m_zoomOut = !gameView->m_zoomOut;
							}
							break;

						case SDLK_s:
							if (game->m_editMode)
							{
								game->saveLevel( "levels/_editor.txt");
							}
							break;

						case SDLK_c:
							if (game->m_editMode)
							{
								game->clearBlocks();
							}
							break;

						case SDLK_h:
							if (game->m_editMode)
							{
								game->calcHeights();
								game->updateHite();
							}
							break;

						// DBG: cheat - get a random new piece
						case SDLK_1:
							game->m_currTet = game->randTet();
							break;

						case SDLK_F5:
							// Toggle debug draw
							gameView->dbgDrawHeight = !gameView->dbgDrawHeight;
							break;
					}
					break;
				case SDL_MOUSEMOTION:
					gameView->mouseMove( event.motion.x, event.motion.y );					
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{					
						gameView->mouseDown();
					}
					else
					{
						game->m_tetRot += 1;
						if (game->m_tetRot == 4) game->m_tetRot = 0;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					gameView->mouseUp();
					break;				

				case SDL_QUIT:
					done = true;
					break;
			}
		}

		
		// Continuous (key state) keys
		Uint8 *keyState = SDL_GetKeyState( NULL );
		if (game->m_editMode)
		{
			if (keyState[ SDLK_LEFT])
			{
				game->m_latitude -= 1.0;
				if (game->m_latitude < 0.0) game->m_latitude = 0.0;
			}
			else if (keyState[SDLK_RIGHT])
			{
				game->m_latitude += 1.0;
				if (game->m_latitude > 1000.0) game->m_latitude = 1000.0;
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
			if (g_state==STATE_GAME)
			{
				game->update( (float)STEPTIME / 1000.0f );
			}
		}	

		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
		
		
		gameView->update( dtRaw ); 
		gameView->redraw( g_state );
		

		SDL_GL_SwapBuffers();
	}

	// oh yeah fix the crash the easy way...dont clean up!!
	//delete gameView;
	//delete game;

	SDL_Quit();	

	return 0;
}