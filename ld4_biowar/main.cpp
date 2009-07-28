#include <allegro.h>
#include <alfont.h>

#include <math.h>
#include <string.h>

#include <assert.h>

#include "ScriptConsole.h"
#include "Map.h"
#include "Sprite.h"
#include "Game.h"

ALFONT_FONT *font_console, *font_title, *font_game;

GameMode *g_mode = NULL;

int main( int argc, char *argv[] ) 
{
	int screenMode = GFX_AUTODETECT_WINDOWED;
	BITMAP *backbuf;
	
	TitleScreen *mode_titlescreen;
	CityMode *mode_city;

	bool showConsole = false;

	//args
	for (int i=1; i < argc; i++) {
		if (!strcmp(argv[i],"-fullscreen") ) {
			screenMode = GFX_AUTODETECT_FULLSCREEN;
		}
	}

	// Init alleg
	allegro_init();
	install_keyboard();
	install_mouse();
	set_color_depth( 16 );
	
	
	

	// Turn on the screeny goodness
	if (set_gfx_mode(screenMode, 640,480, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set 640x480 graphic mode\n%s\n", allegro_error);
      return 1;
   }

	alfont_init();

	//set_color_conversion(COLORCONV_TOTAL);
	//drawing_mode(DRAW_MODE_TRANS, NULL,0,0);


	// fonts
	font_console = alfont_load_font("Graphics/fonts/ProFontWindows.ttf");
	alfont_set_font_size(font_console, 12);

	font_title = alfont_load_font("Graphics/fonts/AbductionIII.ttf");
	alfont_set_font_size( font_title, 24 );

	font_game = alfont_load_font("Graphics/fonts/archtura.ttf" );
	alfont_set_font_size( font_game, 24 );


	// screen 
	backbuf = create_bitmap( 640, 480 );
	rectfill( backbuf, 0,0, 640, 480, 0x0 );	
	
	SpriteGraphic *testgfx = new SpriteGraphic( "testship" );
	Sprite *test = new Sprite( testgfx );
	test->m_x = 10;
	test->m_y = 70;

	// game mode
	mode_titlescreen = new TitleScreen();
	mode_city = new CityMode();
	g_mode = mode_titlescreen;

	// init scripts
	script_init( font_console );

	// The main loop
	while (1) {

		if (g_mode) {
			g_mode->update();
		}

		g_mode->draw( backbuf );

#if 0
		// draw the game
		if (Map::g_activeMap) {
			Map::g_activeMap->draw( backbuf );
			
			// DBG: Map scrolling
			Map::g_activeMap->m_scroll = ((float)mouse_y / 480.0) * ((Map::g_activeMap->m_maphite-15)*32);

		} else {
			rectfill( backbuf, 0,0, 640, 480, 0x0 );
			alfont_textout_centre( backbuf, font_game, "Infection",  320, 200, makecol(200, 0, 0) );
		}

		// draw the sprites
		test->draw( backbuf );

		char buff[200];
		sprintf( buff, "testy %d m2s %d scroll %d", test->m_y, Map::mapYtoScreenY( test->m_y ), Map::getScroll() );
		alfont_textout( backbuf, font_console, buff,  320, 200, makecol(255,255,255) );
#endif

		// draw the console
		if (showConsole) {
			draw_console( backbuf );
		}

		// event handler
		if (keypressed()) {			

			int k = readkey();
			int kk = k >>8;

			// mode specific keypress
			g_mode->key_pressed( k );
			
			// global keypress
			if (kk==KEY_TILDE) {
				showConsole = !showConsole;
			} else if (showConsole) {
				// send key to console
				console_key( k );

			} else {
				switch(k>>8) {			
				
				case KEY_DOWN:
					test->m_y += 10;
					break;
				case KEY_UP:
					test->m_y -= 10;
					break;
				}

			}
		}

		

		if (key[KEY_ESC]) {
			//exit(0);
			break;
		}

		// Blit the screen
		vsync();
		acquire_screen();
		blit( backbuf, screen, 0, 0, 0, 0, 640, 480 );
		release_screen();

		yield_timeslice();
	}
	

	return 0;
}
END_OF_MAIN();