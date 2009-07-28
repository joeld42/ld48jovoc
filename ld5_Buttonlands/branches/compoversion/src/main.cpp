#include <stdio.h>

#define USE_CONSOLE

#include <allegro.h>
#include <alfont.h>

#include <jpegdecoder.h>

#include "hero.h"
#include "combat.h"
#include "map.h"
#include "random.h"
#include "UIbutton.h"

ALFONT_FONT *title_font, *text_font;

// game data
std::vector<BMHero*> g_allHero, g_allEnemy;

BITMAP *blank;

BMMap *map;
BITMAP *cursorX, *cursorNorm;

enum {
	MODE_TITLE,
	MODE_MAP,
	MODE_COMBAT,
	MODE_INFO, 
	MODE_FIGHTOVER
};

// info screens
enum {
	INFO_PICKHERO,
	INFO_EQUIP,
};

float pulse=0,pulse2=0;
float scrolly=0;
int scrollndx;
std::vector <BMHero*> scrollbtn;

int menuItem = 0;
char *menuText[] = {
	"Small Map",
	"Large Map",
	"Huge Map",
	"Quit",
	NULL
};

extern BMHero *hero, *enemy;

int allSides[] = { 4, 6, 8, 10, 12, 20, 0 };

int mode, infoMode;
int gameWon;

BITMAP *load_jpeg( char *filename, int alpha_thresh ) 
{
	int w,h;
	BITMAP *img;

	Pjpeg_decoder_file_stream inputStream = new jpeg_decoder_file_stream();	
	inputStream->open(filename);
	Pjpeg_decoder pd = new jpeg_decoder(inputStream, 0);
	w = pd->get_width();
	h = pd->get_height();
	assert(pd->get_num_components() == 3);
	unsigned char *pbuf = (unsigned char *)malloc(pd->get_width() * 3);
	
	img = create_bitmap( w, h );

	pd->begin();

	int lines_decoded = 0;
	unsigned y = 0;	
	for ( ; ; y++)
	{
		void *Pscan_line_ofs;
		uint scan_line_len;

		if (pd->decode(&Pscan_line_ofs, &scan_line_len)) {			
		  break;
		}		

		lines_decoded++;

		int srcBpp = pd->get_bytes_per_pixel();
		assert(srcBpp == 4 || srcBpp == 3);
				
		unsigned char* scanLine = (unsigned char*)Pscan_line_ofs;
		for (int x = 0; x < pd->get_width(); x ++)
		{

			// cleanup transparency on jpgs
			if ( (scanLine[x*srcBpp] > (255-alpha_thresh) ) &&
				 (scanLine[x*srcBpp+1] < alpha_thresh) &&
				 (scanLine[x*srcBpp+2] > (255-alpha_thresh) ) ) {

				putpixel( img, x, y, makecol( 255,0,255) );

			} else {
				putpixel( img, x, y, makecol( scanLine[x*srcBpp],
											scanLine[x*srcBpp+1],
											scanLine[x*srcBpp+2] ) );			
			}

		}
	}

	free(pbuf);
	delete pd;
	delete inputStream;

	return img;

}

void textout_in_rect( BITMAP *targ, ALFONT_FONT *the_font, const char *text,
				 int x1, int y1, int x2, int y2, int col ) 
{
	//rect( targ, x1, y1, x2, y2, makecol( 255,0,0) ); //dbg

	char buff[1024], textcopy[1024], *ch;
	buff[0] = 0;
	int y = y1;


	strcpy( textcopy, text );

	ch = strtok( textcopy, " " );
	while (ch) {
		strcat( buff, ch );
		strcat( buff, " " );		

		ch = strtok( NULL, " " );

		if ( (!ch) || (alfont_text_length( the_font, buff) > x2-x1) ) {
			alfont_textout_aa( targ, the_font, buff, x1, y, col );
			buff[0] = 0;
			y += alfont_text_height( the_font );
		}
	}


}

int main( int argc, char *argv[] )
{
	BITMAP *backbuf;
	BITMAP *currCursor;

	BITMAP *hero_icon;
	std::vector<BMHero*> myHeros;

	int heroNdx, result;
	std::string resultStr;

	int i,j;
	int last_mouse_b;

	// map stuff
	BMLoc *currLoc = NULL, *heroLoc, *travelLoc;

	BMHero *newHero;

	// interface buttons
	BMButton *btnMapQuit, *btnInfoNext, *btnInfoPrev, *btnInfoFight;

	allegro_init();
	set_window_title( "Buttonlands" );



	set_color_depth( 32 );
	set_gfx_mode( GFX_AUTODETECT_WINDOWED, 800,600, 0,0 );
	//set_gfx_mode( GFX_AUTODETECT_FULLSCREEN, 800,600, 0,0 );

	backbuf = create_bitmap( 800, 600 );

	install_keyboard();
	install_mouse();


	alfont_init();
	alfont_text_mode( -1 );

	srand( time(0) );

	// load resources
	title_font = alfont_load_font( "gamedata/Brinkmann.ttf" );
	text_font  = alfont_load_font( "gamedata/GenR1.ttf" );

	// load game data
	load_buttons( "gamedata/heros.xml", g_allHero );
	load_buttons( "gamedata/enemy.xml", g_allEnemy );
	

	for (i = 0; i < g_allHero.size(); i++) {
		scrollbtn.push_back( (g_allHero[i]) );
	}

	for (i = 0; i < g_allEnemy.size(); i++) {
		scrollbtn.push_back( (g_allEnemy[i]) );
	}

	printf("scrollbuttn %d\n", scrollbtn.size() );

	// shuffle
	BMHero *tmp;
	for (i=0; i < scrollbtn.size(); i++) {		
		j = random( scrollbtn.size() );
		tmp = scrollbtn[i];
		scrollbtn[i] = scrollbtn[j];
		scrollbtn[j] = tmp;
	}

	cursorNorm = load_bitmap( "gamedata/cursor.bmp", NULL );
	cursorX = load_bitmap( "gamedata/cursorX.bmp", NULL );
	currCursor = cursorNorm;

	blank = load_jpeg( "gamedata/blank.jpg" );
	
	mode = MODE_TITLE;
	map = new BMMap();

	//map->make_map( 12 );

	init_combat();

	btnMapQuit = new BMButton( "Quit", 150, 20, 100, 25, makecol( 100, 220, 250 ) );

	btnInfoNext = new BMButton( "Next", 600, 550, 100, 25, makecol( 230, 230, 230 ) );
	btnInfoPrev = new BMButton( "Prev", 340, 550, 100, 25, makecol( 230, 230, 230 ) );
	btnInfoFight = new BMButton( "Done", 470, 570, 100, 35, makecol( 255, 200, 50 ) );


	hero_icon = create_bitmap( 50, 50 );
	newHero = NULL;

	heroLoc = &(map->loc[0]);

	while (1) {
		

		if (mode==MODE_TITLE) {
			// draw title screen
			
			//rectfill( backbuf, 0, 0, 800, 600, makecol( 32, 20,200 ) );
			blit( blank, backbuf, 0,0, 0,0, 800, 600 );

			alfont_set_font_size( title_font, 120 );
			alfont_textout_centre_aa( backbuf, title_font, "ButtonLands", 405, 45, makecol( 30, 30, 60 ) );
			alfont_textout_centre_aa( backbuf, title_font, "ButtonLands", 400, 40, makecol( 255, 200, 50 ) );

			alfont_set_font_size( text_font, 40 );
			alfont_textout_centre_aa( backbuf, text_font, "A game by Joel Davis for Ludumdare 48 Hour contest #5",
														400, 550, makecol( 255, 200, 50 ) );

			// draw menu
			alfont_set_font_size( title_font, 30 );
			for (i=0; menuText[i]; i++) {



				alfont_textout_centre_aa( backbuf, title_font, menuText[i], 
					402, 202 + i*alfont_text_height( title_font), makecol( 30, 30, 60 ) );

				alfont_textout_centre_aa( backbuf, title_font, menuText[i], 
					400, 200 + i*alfont_text_height( title_font), 
					(i==menuItem)?makecol( 255, 200, 50):makecol( 200, 150, 50) );
			}


			// draw scrolly things
			int xpos = -250 + scrolly, ndx = scrollndx;
			while (xpos < 800) {
				masked_blit( scrollbtn[ndx]->photo, backbuf, 0, 0, xpos, 340, 200, 200 );
				
				xpos += 250;
				ndx++;
				if (ndx >= scrollbtn.size() ) ndx = 0;

				
			}



		} else if (mode==MODE_COMBAT) {
			combat_draw( backbuf );			

		} else if (mode==MODE_INFO) {
			
			blit( blank, backbuf, 0,0, 0,0, 800, 600 );

			std::string infoTitle;
			switch (infoMode) {
				case INFO_PICKHERO:
					infoTitle = "Choose A  Hero";
					break;
				case INFO_EQUIP:
					infoTitle = "Equip Hero";
					break;
			};
			alfont_set_font_size( title_font, 50 );
			alfont_textout_centre_aa( backbuf, title_font, infoTitle.c_str(), 403, 33, makecol( 30, 30, 60 ) );
			alfont_textout_centre_aa( backbuf, title_font, infoTitle.c_str(), 400, 30, makecol( 255, 200, 50 ) );

			// draw opponedt
			draw_portrait( backbuf, 30, 120, enemy, true );
			
			alfont_textout_centre_aa( backbuf, title_font, "Opponent", 500, 120, makecol( 255, 200, 50 ) );
			alfont_set_font_size( text_font, 30 );
			textout_in_rect( backbuf, text_font, enemy->bio.c_str(), 
						350, 150, 650, 600, makecol( 255,255,255 ) );	

			rectfill( backbuf, 250, 340, 750, 345, makecol( 255, 200, 50 ) );
			
			draw_portrait( backbuf, 30, 370, hero, true, infoMode==INFO_EQUIP );
			
			alfont_textout_centre_aa( backbuf, title_font, "Hero", 500, 350, makecol( 255, 200, 50 ) );
			
			if (infoMode==INFO_PICKHERO) {
				alfont_set_font_size( text_font, 30 );
				textout_in_rect( backbuf, text_font, hero->bio.c_str(), 
						350, 380, 650, 600, makecol( 255,255,255 ) );	
			
			
				btnInfoNext->draw( backbuf, text_font );
				btnInfoPrev->draw( backbuf, text_font );
			}

			alfont_set_font_size( text_font, 25 );
			btnInfoFight->draw( backbuf, text_font );
			
		
		} else if (mode==MODE_FIGHTOVER) {

			blit( blank, backbuf, 0,0, 0,0, 800, 600 );


			std::string title;
			switch (result) {
				case HERO_WON:
					title = "Victory!";
					break;
				case ENEMY_WON:
					title = "Defeat!";
					break;
				case DRAW:
					title = "It's a Draw!";
					break;
			};
			alfont_set_font_size( title_font, 50 );
			alfont_textout_centre_aa( backbuf, title_font, title.c_str(), 403, 33, makecol( 30, 30, 60 ) );
			alfont_textout_centre_aa( backbuf, title_font, title.c_str(), 400, 30, makecol( 255, 200, 50 ) );

			alfont_set_font_size( text_font, 30 );
			textout_in_rect( backbuf, text_font, resultStr.c_str(), 
						     200, 200,600, 600, makecol( 255,255,255 ) );

			if ((result==HERO_WON) && (newHero)) {
				draw_portrait( backbuf, 300, 360, newHero, true );
			} else if (result==ENEMY_WON) {
				draw_portrait( backbuf, 300, 360, enemy, true );
			}

		} else if (mode==MODE_MAP) {
			blit( map->bmpMap, backbuf, 0,0, 0,0, 800, 600 );

			if (currLoc) {
				alfont_set_font_size( text_font, 30 );
				for (i=0; i < 5; i++) {
					for (j=0; j < 5; j++) {
						alfont_textout_centre( backbuf, text_font, currLoc->name.c_str(),
												400+i, 480+j, makecol( 60, 40, 2 ) );
					}
					alfont_textout_centre_aa( backbuf, text_font, currLoc->name.c_str(),
												400+2, 480+2, makecol( 255,255,255 ) );

				}
			
						
				alfont_set_font_size( text_font, 20 );
				textout_in_rect( backbuf, text_font, currLoc->desc.c_str(), 
					200, 530,600, 600, makecol( 255,255,255 ) );						

			}

			// hero icon
			masked_blit( hero_icon, backbuf, 0,0, heroLoc->xpos - 32 + (1.0-pulse)*10, heroLoc->ypos - 32 + pulse2*10, 50,50 );			

		
			alfont_set_font_size( text_font, 30 );
			btnMapQuit->draw( backbuf, text_font );

		}

		// add cursor
		masked_blit( currCursor, backbuf, 0, 0, mouse_x, mouse_y, 16, 16 );

		// flip
		vsync();
		acquire_screen();
		blit( backbuf, screen, 0, 0, 0, 0, 800, 600 );
		release_screen();

		yield_timeslice();


		if (mode == MODE_TITLE ) {
			if (keypressed()) {

				int k = readkey();
				switch (k>>8) {
				
					case KEY_DOWN:
						menuItem++;
						if (menuText[menuItem]==NULL) menuItem = 0;
						break;

					case KEY_UP:
						menuItem--;
						if (menuItem==-1) {
							while(menuText[menuItem+1]) menuItem++;
						}
						break;

					case KEY_ENTER:
					case KEY_SPACE:
						if (!strcmp( menuText[menuItem], "Quit" ) ) {
							exit(0);
						} else {
							int mapSz = 10;
							float mapSpc = 50.0;
							if (!strcmp( menuText[menuItem], "Small Map" ) ) { mapSz = 8; mapSpc = 150.0; }
							if (!strcmp( menuText[menuItem], "Large Map" ) ) { mapSz = 15; mapSpc = 100.0; }
							if (!strcmp( menuText[menuItem], "Huge Map" ) ) { mapSz = 20; mapSpc = 75.0; }

							map->make_map( mapSz, mapSpc );
							mode = MODE_MAP;
							
							gameWon =  0;

							heroLoc = &(map->loc[0]);
							heroLoc->heroVisited = 1;

							// for now, give all the heros
							myHeros.erase( myHeros.begin(), myHeros.end() );
							//for ( i=0; i < g_allHero.size(); i++) {
							//	myHeros.push_back( g_allHero[i] );
							//}
							heroNdx = 0;
							myHeros.push_back( g_allHero[random(g_allHero.size() ) ] );

							stretch_blit( myHeros[0]->photo, hero_icon,
								0,0, 200,200, 
								0,0, 50, 50 );

						}
						break;

				}

				

		
			}
		} else if (mode==MODE_FIGHTOVER) {			

			if (keypressed()) {

				if ((myHeros.size()) && (!gameWon)) {
					mode = MODE_MAP;
				} else {
					mode = MODE_TITLE;
				}
				

			}

		} else if (mode==MODE_COMBAT) {			
			
			result = combat_update(); 
			if (result != STILL_FIGHTING) {

				mode = MODE_FIGHTOVER;

				if (result == HERO_WON) {
					printf("Player wins!\n" );
					heroLoc = travelLoc;
					heroLoc->heroVisited = 1;
					
					// HERE CHECK
					gameWon = 1;
					for (i=0; i < map->nloc; i++) {
						if (!map->loc[i].heroVisited) {
							gameWon = 0;
							break;
						}
					}

					resultStr = hero->name + std::string(" defeats the pathetic ")+
						enemy->name + std::string(", woot.  ");

					if (gameWon) {
						
						resultStr = resultStr + std::string(" You've explored the entire map! "
										"Congratulations, you've won the game!\n" );

					} else {
						newHero = g_allHero[ random( g_allHero.size() ) ];
						myHeros.push_back( newHero);

						resultStr = resultStr + std::string("Impressed with your prowess, ")+
								 newHero->name + std::string(" offers to join your party!" );
					}

				} else if (result == ENEMY_WON) {

					printf( "Enemy wins\n" );

					resultStr = hero->name + std::string(" is no match for ")+
						enemy->name;
					

					// kill him!!
					myHeros[ heroNdx ] = myHeros[myHeros.size()-1];
					myHeros.pop_back();

										
					if (myHeros.size()!=0) {
						resultStr = resultStr + std::string(". You have one less hero in your party, loser.");
						heroNdx = 0;
						hero = myHeros[heroNdx];

						printf("Hero is %s\n", hero->name.c_str() );

					} else {
						resultStr = resultStr + std::string(". You're all out of heros. You lost the damn game!");
					}

				} else if (result == DRAW) {
					printf( "Draw!\n" );

					resultStr = "That was a waste of time.";

				}


			}

							
		} else if (mode==MODE_MAP) {
			
			// check keys
			if (keypressed()) {			

				int k = readkey();
				switch (k>>8) {
				
					case KEY_N:
						map->make_map( 15, 75.0 );
						break;

				}
			}			

			// update location
			currLoc = NULL;
			currCursor = cursorNorm;
			for (int i=0; i < map->nloc; i++) {
				if ( (mouse_x > map->loc[i].xpos - 32) &&
					 (mouse_x < map->loc[i].xpos + 32) &&
					 (mouse_y > map->loc[i].ypos - 32) &&
					 (mouse_y < map->loc[i].ypos + 32) ) {
					
					currLoc = &(map->loc[i]);

					if (!map->adj[ heroLoc->index ][ i ].pass ) {
						currCursor = cursorX;						
					}

				}
			}

			// check button
			if (mouse_b & 1) {

				if (btnMapQuit->mouseIn) {
					mode = MODE_TITLE;
				}

				if (currLoc) {

					if (map->adj[ heroLoc->index ][ currLoc->index ].pass ) {
						
						set_enemy( g_allEnemy[ random( g_allEnemy.size() ) ] );						

						set_hero( myHeros[ heroNdx ] );

						travelLoc = currLoc;

						mode = MODE_INFO;	
						infoMode = INFO_PICKHERO;
						//start_combat();
	
					}
				}

			}



		} else if (mode==MODE_INFO) {	

			if (keypressed()) {
			
				mode = MODE_COMBAT;
				start_combat();
			}

			
			if ( (mouse_b & 1)  && !(last_mouse_b & 1)) {

				if ((infoMode ==INFO_PICKHERO) && (btnInfoNext->mouseIn)) {
					heroNdx++;
					if (heroNdx == myHeros.size()) {
						heroNdx = 0;
					}
					hero = myHeros[heroNdx];
				} else if ((infoMode ==INFO_PICKHERO) && (btnInfoPrev->mouseIn)) {
					heroNdx--;
					if (heroNdx == -1) {
						heroNdx = myHeros.size() - 1;
					}
					hero = myHeros[heroNdx];
				} else if (btnInfoFight->mouseIn) {

					if (infoMode == INFO_PICKHERO ) {
						
						infoMode = INFO_EQUIP;

					} else {

						mode = MODE_COMBAT;
						start_combat();
					}
				}


				// equip
				if (infoMode==INFO_EQUIP) {
					int dienum, choosenum;
					if ((mouse_x > 300) && (mouse_y > 350)) {
						dienum = (mouse_y - 350) / 50;
						choosenum = (mouse_x - 310) / 60;

						//printf("CHOICE: dice %d choice %d\n", dienum, choosenum );
						if (hero->die[dienum].nrmsides==SIDES_X) {
							if (choosenum < 6) {
								hero->die[dienum].sides = allSides[choosenum];
							}
						} else if (hero->die[dienum].altsides) {
							if (choosenum <2) {
								hero->die[dienum].sides = choosenum?hero->die[dienum].altsides:hero->die[dienum].nrmsides;
							}
						}

					}
				}
			}
		}

		
		
		// update all buttons
		for (i=0; i < BMButton::allUIBtns.size(); i++) {
			BMButton::allUIBtns[i]->mouse_check();
		}


		// keys for any mode
		if (keypressed()) {
			int k = readkey();
			
			switch(k>>8) {			
				case KEY_A:					
					break;
				}
			}

		/* bail */
		if (key[KEY_ESC]) {
			exit(0);
			break;
		}

		last_mouse_b = mouse_b;


		static int last_retrace = 0;
		if (last_retrace) {
			int dt = retrace_count - last_retrace;				

			pulse = sin( (float)retrace_count * 0.05 );
			pulse2 = cos( (float)retrace_count * 0.05 );

			scrolly += dt*0.6;
			if (scrolly > 250) {
				scrollndx--;
				scrolly = 0;

				if (scrollndx < 0) {
					scrollndx = scrollbtn.size()-1;					
				}

				//if (scrollndx >= scrollbtn.size()) {
				//	scrollndx = 0;
				//	scrolly = 0;
				//}
			}			

		}
		last_retrace = retrace_count;
	}



	return 0;
}
END_OF_MAIN();