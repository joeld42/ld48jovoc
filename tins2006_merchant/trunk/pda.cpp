#include <allegro.h>
#include <alfont.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"
#include "pda.h"
#include "tiles.h"

#define NSTARS (100)

int g_pdaMode = PDA_TITLESCREEN;
int g_pdaBGColor = makecol16( 5, 15, 5 );
char planetName[1000] = "_";

int g_genPercent = 50;
int genStep = 0;

int currStoreItem=0;

std::vector<char *> g_planetsWon;

const char *ascii_shade[10] = {
	" ", ".", ",", ":", "=", "+", "*", "&", "@", "#"
};

// menu
const char *g_menuItems[] = {
		"New Game", 
		"Tile Playground",
		"Quit", NULL
	};
int g_currMenuItem = 0;

// tabs
const char *g_tabItems[] = {
	"Main Menu",
	"Map",
	"Inventory",	
	"Help"
};
int g_currTabItem = 0;
#define NUM_TABS (sizeof(g_tabItems) / sizeof(const char *) )

// Convert a bitmap to ascii art
// maskcol is what color to REPLACE mask (0xff00ff) with
BITMAP *ascii_art( BITMAP *src, int charx, int chary, int maskcol=makecol16(0,0,0) ) 
{
	BITMAP *targ;		
	
	targ = create_bitmap( src->w * charx, src->h * chary );
	rectfill( targ, 0, 0, targ->w, targ->h, g_pdaBGColor );
	for (int j=0; j <src->h; j++) {
		for (int i=0; i < src->w; i++) {
			int p = getpixel( src, i, j );

			if (p==makecol16(0xff,0,0xff)) {
				p = maskcol;
			}

			int val = (getr16( p ) + getg16( p ) + getb16( p )) / 3;
			
			// dither
			val += rand() % 15;
			if (val > 255) val = 255;

			// Use allegro font for actual ascii 
			textout_centre( targ, font, ascii_shade[ val/26 ], 
				i*charx, j*chary, p );
		}
	}

	return targ;
}

void draw_pda_help( BITMAP *targ )
{
	static BITMAP *train = NULL;
	rectfill( targ, 0, 0, 800, 600, g_pdaBGColor );

	if (!train) {
		if (!bmpTrain) {
			bmpTrain = load_bitmap( "gamedata/train.bmp", NULL );
		}
		train = ascii_art( bmpTrain, 9, 8 );
	}
	
	static int tx = 400;
	blit( train, targ, 0,0, tx , 580-train->h, train->w, train->h );

	// help text
	alfont_set_font_size( g_fontTextPDA, 20 );	
	int yval = 100, leading = 24;
	alfont_textout( targ, g_fontTextPDA, 
		"Arriving on new planet with no money for a ticket on the rocket ship, "
		"you must collect items, and then", 
		60, yval, makecol( 0, 255, 0 ) );
	yval += leading;

	alfont_textout( targ, g_fontTextPDA, 
		"sell them to passing trains until you have enough to purchase "
		"passage on the rocket to the next planet.", 
		60, yval, makecol( 0, 255, 0 ) );
	yval += leading*2;	

	alfont_textout( targ, g_fontTextPDA, 
		"Move with Arrow keys. Use TAB to bring up your PDA. You can use "
		"your PDA to buy and sell if",
		60, yval, makecol( 0, 255, 0 ) );
	yval += leading;

	alfont_textout( targ, g_fontTextPDA, 
		"you are in front of a train station, or to check "	
		"the map of the current planet, and to",
		60, yval, makecol( 0, 255, 0 ) );
	yval += leading;

	alfont_textout( targ, g_fontTextPDA, 		
		"buy passage to the next planet.",
		60, yval, makecol( 0, 255, 0 ) );
	yval += leading*2;

	alfont_textout( targ, g_fontTextPDA, 		
		"Time is important, watch the train schedule, there are more goods"
		"in stock when a train is at the station.", 
		60, yval, makecol( 0, 255, 0 ) );
	yval += leading;

	

	static int lastRetrace=0;

	if (retrace_count - lastRetrace > 0) {
		lastRetrace = retrace_count;
		tx -= 10;
		if (tx <= -1000-train->w) tx = 1000;
	}

}

void draw_pda_map( BITMAP *targ )
{
	static BITMAP *map = NULL;
	rectfill( targ, 0, 0, 800, 600, g_pdaBGColor );
	if (!g_map) {
		alfont_set_font_size( g_fontTitlePDA, 30 );
		alfont_textout_centre( targ, g_fontTitlePDA, "Not on a Planet.", 400, 300, makecol16( 0, 150, 170 ) );	
	} else {
		if (map==NULL) {
			map = ascii_art( g_map->minimap, 9, 8 );
		}
		blit( map, targ, 0, 0, 400-map->w/2, 300-map->h/2, map->w, map->h );
	}
}

void draw_pda_inventory( BITMAP *targ )
{	
	static const char *itemName[] = {
		"Mushroom", "Apple", "Battery", "Bulb", "Fish",
		"Saxophone", "Cheese", "Tile", "Hammer", "Latte"
	};
	rectfill( targ, 0, 0, 800, 600, g_pdaBGColor );

	if (!gameOn) {
		alfont_set_font_size( g_fontTitlePDA, 30 );
		alfont_textout_centre( targ, g_fontTitlePDA, "Inventory Mode not active in Orbit", 400, 300, makecol16( 0, 150, 170 ) );	
		return;
	}


	// ascii item pics
	static BITMAP *asciiPic[10] = {0};
	if (!asciiPic[0]) {
		for (int j=0; j < 10; j++) {
			asciiPic[j] = ascii_art( bmpItems[j], 9, 8, makecol16( 31, 97, 28 ) );
		}
	}	

	alfont_set_font_size( g_fontTextPDA, 20 );

	if (g_atStore) {
		alfont_textout( targ, g_fontTextPDA, "Press +  to buy and - to sell", 450, 160, makecol( 0, 255, 0 ) );


		alfont_textout( targ, g_fontTextPDA, "OWN  PRICE STOCK", 242, 150, makecol( 0, 255, 0 ) );
	} else {
		alfont_textout( targ, g_fontTextPDA, "OWN", 242, 150, makecol( 0, 255, 0 ) );
	}

	alfont_set_font_size( g_fontTextPDA, 30 );

	char buff[300];
	for (int i=0; i < 10; i++) {		

		// Inventory list
		
		if (i==currStoreItem) {
			rectfill( targ, 100, 168+i*32, 350, 202+i*32, makecol( 0, 100, 0 ) );
		}
		draw_sprite( targ, bmpItems[i], 100, 168+i*32 );
		alfont_textout( targ, g_fontTextPDA, itemName[i], 140, 170+i*32, makecol( 0, 255, 0 ) );

		// inv
		sprintf( buff, "%d", g_player.inv[i] );
		alfont_textout( targ, g_fontTextPDA, buff, 250, 170+i*32, makecol( 0, 255, 0 ) );

		if (g_atStore) {
			sprintf( buff, "%d", g_atStore->price[i] );
			alfont_textout( targ, g_fontTextPDA, buff, 290, 170+i*32, makecol( 0, 255, 0 ) );
			
			sprintf( buff, "%d", g_atStore->stock[i] );
			alfont_textout( targ, g_fontTextPDA, buff, 330, 170+i*32, makecol( 0, 255, 0 ) );
		}

	}

	alfont_set_font_size( g_fontTitlePDA, 30 );
	sprintf( buff, "Cash on hand: %d", g_player.cash );
	alfont_textout( targ, g_fontTitlePDA, buff, 100, 500, makecol( 0, 255, 0 ) );	


	// draw item portrait	
	blit( asciiPic[currStoreItem], targ, 0,0,
			550 - asciiPic[currStoreItem]->w/2, 
			325 - asciiPic[currStoreItem]->h/2,
			asciiPic[currStoreItem]->w, asciiPic[currStoreItem]->h );
	

}

void draw_pda_title( BITMAP *targ )
{
	rectfill( targ, 0, 0, 800, 600, g_pdaBGColor );
	rect( targ, 49, 49, 751, 551, makecol16( 200, 200, 200 ) );

	static BITMAP *pic=NULL;
	if (!pic) {
		BITMAP *srcPic;
		srcPic = load_bitmap( "gamedata/title.bmp", NULL );
		
		pic = ascii_art( srcPic, 9, 8 );
	}

	blit( pic, targ, 0, 0, 50, 80, pic->w-100, pic->h );

	textout_centre( targ, font, "...in Space!", 400, 340, makecol16( 50, 200, 255) );
	
	// If game is underway.. no menu
	if (gameOn) {
		if ((atRocket) && (g_player.cash >= TICKET_PRICE)) {
			alfont_textout_centre( targ, g_fontTitlePDA, 
							 "Level Complete!", 
							 398, 358+40, makecol16( 0, 250, 250 ) );				
			char buff[1000];
			sprintf( buff, "Press ENTER to purchase ticket for $%d!", TICKET_PRICE );
			alfont_textout_centre( targ, g_fontTitlePDA, 
							 buff, 398, 358+60, makecol16( 0, 150, 150 ) );	
		} else {
			if (g_player.cash < TICKET_PRICE) {
				alfont_textout_centre( targ, g_fontTitlePDA, 
							 "You can't afford a ticket yet!", 
							 398, 358+40, makecol16( 150, 0, 0 ) );	
			} else {
				alfont_textout_centre( targ, g_fontTitlePDA, 
							 "Return to rocket to purchase ticket!", 
							 398, 358+40, makecol16( 150, 0, 0 ) );	
			}
		}
		return;
	}

	if (g_pdaMode == PDA_TITLESCREEN) {
		textout_centre( targ, font, "Written for TINS2006 by Joel Davis (joeld42@yahoo.com)", 400, 520, makecol16( 50, 160, 200) );

		// Use alfont for "nicer" menu items
		alfont_set_font_size( g_fontTitlePDA, 30 );
		for (int i = 0; g_menuItems[i]; i++) {

			if (i==g_currMenuItem) {
				// selected
				alfont_textout_centre( targ, g_fontTitlePDA, g_menuItems[i], 398, 358+40*i, makecol16( 0, 150, 150 ) );	
				alfont_textout_centre( targ, g_fontTitlePDA, g_menuItems[i], 402, 362+40*i, makecol16( 0, 150, 150 ) );				
				alfont_textout_centre( targ, g_fontTitlePDA, g_menuItems[i], 398, 362+40*i, makecol16( 0, 150, 150 ) );	
				alfont_textout_centre( targ, g_fontTitlePDA, g_menuItems[i], 402, 358+40*i, makecol16( 0, 200, 200 ) );	

				alfont_textout_centre( targ, g_fontTitlePDA, g_menuItems[i], 400, 360+40*i, makecol16( 250, 250, 0 ) );	
			} else {
				// not selected
				alfont_textout_centre( targ, g_fontTitlePDA, g_menuItems[i], 400, 360+40*i, makecol16( 0, 150, 170 ) );	
			}
		}
	} else if (g_pdaMode == PDA_STARTGAME) {
		// start new game
		alfont_set_font_size( g_fontTextPDA, 30 );
		rect( targ, 55, 355, 745, 545, makecol( 0, 255, 0 ) );

		if (g_planetsWon.size()==0) {
			alfont_textout_centre( targ, g_fontTextPDA, "You are prepared to embark on your Journey!", 
				400, 360, makecol( 0, 255, 0 ) );		
		} else {
			char buff[100];
			sprintf( buff, "Prepare for level %d", g_planetsWon.size() + 1 );
			alfont_textout_centre( targ, g_fontTextPDA, buff,  400, 360, makecol( 0, 255, 0 ) );		
			
			alfont_set_font_size( g_fontTextPDA, 20 );
			alfont_textout( targ, g_fontTextPDA, "Planets visited:",  60, 380, makecol( 0, 255, 255 ) );		
			for (size_t i=0; i < g_planetsWon.size(); i++) {
				alfont_textout( targ, g_fontTextPDA, g_planetsWon[i],  
					60 + (i%6)*100, 400 + (i/6)*20, 
					makecol( 0, 255, 255 ) );		
			}			
		}


		int yval = 485;
		alfont_textout( targ, g_fontTextPDA, "Enter the name of the planet to visit:", 60, yval+10, makecol( 0, 255, 0 ) );
		
		rect( targ, 450, yval, 740, yval + 50, makecol( 0, 255, 0 ) );
		alfont_textout( targ, g_fontTextPDA, planetName, 455, yval+10, makecol( 0, 255, 255 ) );
	} else if (g_pdaMode == PDA_GENERATE) {
		
		static BITMAP *planet=NULL;
		if (!planet) {
			BITMAP *srcPic;
			srcPic = load_bitmap( "gamedata/planet.bmp", NULL );
		
			planet = ascii_art( srcPic, 9, 8 );
		}

		blit( planet, targ, 0, 0, 75, 360, pic->w-100, pic->h );

		rect( targ, 55, 355, 745, 545, makecol( 0, 255, 0 ) );

		alfont_set_font_size( g_fontTitlePDA, 45 );
		alfont_textout_centre( targ, g_fontTitlePDA, planetName, 470, 370, makecol( 0, 255, 255 ) );

		// progress bar
		char buff[40];
		alfont_set_font_size( g_fontTextPDA, 18 );
		alfont_textout( targ, g_fontTextPDA, "Generating Planet...", 300, 450, makecol( 0, 255, 0 ) );
		sprintf( buff, "%d%%", g_genPercent );
		alfont_textout( targ, g_fontTextPDA, buff, 600-40, 450, makecol( 0, 255, 0 ) );
		rect( targ, 300, 470, 600, 490, makecol( 0, 255, 0 ) );
		rectfill( targ, 302, 472, 302 + (600-302) * ((float)g_genPercent / 100.0f), 
							488, makecol( 0, 255, 0 ) );

	}
}

void draw_pda_tabs( BITMAP *targ )
{
	alfont_set_font_size( g_fontTitlePDA, 20 );

	int tabsize = (700 / NUM_TABS);
	for (int i=0; i < NUM_TABS; i++) {
		
		// draw the box
		int x;
		x = 50 + tabsize*i;
		if (g_currTabItem==i) {
			rectfill( targ, x, 55, x+tabsize-5, 85, makecol16( 0, 180, 0 ) );
			alfont_textout_centre( targ, g_fontTitlePDA, g_tabItems[i], 
							x+(tabsize/2), 60, makecol16( 250, 250, 250 ) );	
		} else {
			rectfill( targ, x, 55, x+tabsize-5, 85, makecol16( 0, 80, 0 ) );
			alfont_textout_centre( targ, g_fontTitlePDA, g_tabItems[i], 
							x+(tabsize/2), 60, makecol16( 0, 0, 0 ) );	
		}
	}
}

void pda_keypress( int k )
{
	if (g_pdaMode==PDA_STARTGAME) {
		int c = (k>>8);
		if ((c >= KEY_A) && (c <= KEY_Z)) {
			if (strlen(planetName) < 18) {
				planetName[strlen(planetName)-1] = 0;
				char buff[100];
				sprintf(buff, "%s%c_", planetName, c-KEY_A + (planetName[0]?'a':'A') );
				strcpy( planetName, buff );
			}
		} else if (c == KEY_BACKSPACE) {
			if (strlen(planetName) > 1) {
				planetName[strlen(planetName)-1] = 0;
				planetName[strlen(planetName)-1] = '_';
			}
		} else if (c == KEY_ENTER) {
			if (strlen(planetName) > 1) {
				planetName[strlen(planetName)-1] = 0;
				g_pdaMode = PDA_GENERATE;
				genStep = 0;
				g_genPercent = 1;
			}
		}
		return;
	}

	switch( k>>8 ) {
		case KEY_DOWN:
			if (g_currTabItem==0) { 
				g_currMenuItem++;
				if (!g_menuItems[g_currMenuItem]) g_currMenuItem=NULL;
			} else if (g_currTabItem==2) { 
				currStoreItem++;
				if (currStoreItem>=10) currStoreItem=0;
			}
			break;			
		case KEY_UP:
			if (g_currTabItem==0) { 
				g_currMenuItem--;
				if (g_currMenuItem<0) {
					g_currMenuItem = (sizeof(g_menuItems) / sizeof(const char *)) - 2;
				}	
			} else if (g_currTabItem==2) { 
				currStoreItem--;
				if (currStoreItem<0) currStoreItem=9;
			}
			break;
		case KEY_LEFT:
			g_currTabItem--;
			if (g_currTabItem<0) g_currTabItem = NUM_TABS-1;
			break;
		case KEY_RIGHT:
			g_currTabItem++;
			if (g_currTabItem==NUM_TABS) g_currTabItem = 0;
			break;
		case KEY_SPACE:
		case KEY_ENTER:
			if (!gameOn) {
				if (g_currMenuItem==0) {
					g_pdaMode = PDA_STARTGAME;
				} else if (g_currMenuItem==1) {
					gamestate = GAMESTATE_TILETEST;
				} else if (g_currMenuItem==2) {
					// quit
					exit(1);
				}
			} else {
				if ((g_currMenuItem==0) && (g_player.cash>=TICKET_PRICE)) {
					g_player.cash -= TICKET_PRICE;
					g_pdaMode = PDA_STARTGAME;
					g_planetsWon.push_back( strdup( planetName ) );

					TICKET_PRICE = static_cast<int>(g_planetsWon.size()) * 1000;

					strcpy( planetName, "_" );
					gameOn = 0;

					

					// TODO: Leak, map needs to really clean up
					delete g_map;
				}
			}
			break;
		case KEY_TAB:
			if (gameOn) {
				gamestate = GAMESTATE_GAME;
				g_simResume = 1;
			}
			break;
		case KEY_EQUALS: // plus
			if ((g_currTabItem==2) && (g_atStore)) { 
				if ((g_player.cash >= g_atStore->price[currStoreItem]) &&
				     (g_atStore->stock[currStoreItem] > 0) ) {
					g_atStore->stock[currStoreItem]--;
					g_player.cash -= g_atStore->price[currStoreItem];
					g_player.inv[currStoreItem]++;
				}
			}
			break;
		case KEY_MINUS: // plus
			if ((g_currTabItem==2) && (g_atStore)) { 
				if ((g_player.inv[currStoreItem] > 0)) {
					g_atStore->stock[currStoreItem]++;
					g_player.cash += g_atStore->price[currStoreItem];
					g_player.inv[currStoreItem]--;
				}
			}
			break;
	}
}

void pda_draw( BITMAP *targ )
{
	if (g_currTabItem == 0) {	// titlescreen
		draw_pda_title( targ );
	} else if (g_currTabItem == 1) {	// map
		draw_pda_map( targ );
	} else if (g_currTabItem == 2) {	// inventory
		draw_pda_inventory( targ );
	} else if (g_currTabItem == 3) { //help
		draw_pda_help( targ );
	} else {
		rectfill( targ, 0, 0, 800, 600, makecol16( 0,0,0 ) );
	}
	draw_pda_tabs( targ );
}

void pda_update()
{
	static int seed_planet;
	static int mapseed;

	if (g_pdaMode==PDA_GENERATE) {

		if (genStep==0) {
			// generate seeds
			seed_planet = 1;
			for (char *ch=planetName; *ch; ch++) {
				// crappyhash
				seed_planet *= (*ch);
			}			

			srand( seed_planet );
			
			mapseed = rand();
			
			genStep = 1;

		} else if (genStep==12) {
			// Done, switch to game mode
			genStep  = 0;
			g_pdaMode = PDA_TITLESCREEN;
			gamestate = GAMESTATE_GAME;	
			gameOn = 1;
			initLevel();
		} else if (genStep==6) {

			// Generate the planet
			int scheme_seed, map_seed, tileset_seed, bg_seed;
			srand( mapseed );
			scheme_seed = rand();
			map_seed = rand();
			tileset_seed = rand();
			bg_seed = rand();

			// DBG
			//rest(200);

			// Generate map
			int mapNdx = genStep - 1;
			Map *map = new Map();
			map->gen_scheme( scheme_seed );
			map->gen_tileset( tileset_seed );
			map->gen_background( bg_seed );
			map->gen_map( map_seed );
			g_map = map;

		} else {
			// just a rest to make generating tiles seem harder
			rest(50);
		}

		// Next
		genStep++;
		g_genPercent = (genStep-1)*10;
		if (g_genPercent > 100) g_genPercent = 100;
	}	
}