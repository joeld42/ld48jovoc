#include <allegro.h>
#include <math.h>
#include <assert.h>
#include <alfont.h>
#include <stdio.h>

#include <vector>

#include "globals.h"
#include "pda.h"
#include "tiles.h"


// fonts
ALFONT_FONT *g_fontTextPDA;
ALFONT_FONT *g_fontTitlePDA;

// game state
int gamestate;
Player g_player;
std::string g_debugString;

int gameOn = 0, atRocket = 0;
int g_simResume = 0;

// was a define..
int TICKET_PRICE = 1000;

Store *g_atStore = NULL;

BITMAP *bmpTrain = NULL;

BITMAP *bmpItems[10] = {0};

struct Item {
    Item() {life = 0;}
    int life;
	int type;
	int x, y;
};
std::vector<Item> g_items;

std::vector<TrainFSM> g_trains;

int irand( int min, int max )
{
	return min + ((float)rand() / (float)RAND_MAX) * (max-min);
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

int iclamp( int n ) {
	if (n < 0) return 0;
	if (n > 0xff) return 0xff;
	return n;
}

void spawn_item()
{
	static float pTable[] = {

		// common items
		200.0f, // mushroom
		100.0f, // apple
		80.0f, // battery

		// uncommon items
		3.0f,  // bulb
		2.0f,  // fish
		2.0f,  // sax
		2.0f,  // cheese

		// rare items
		0.3f,  // tile
		0.2f,  // hammer
		0.1f,  // latte
	};
	
	static int init=0;
	if (!init) {
		init = 1;
		float ptot = 0.0f;
		// normalize items
		for (int i=0; i < 10; i++) {
			ptot += pTable[i];
		}
		for (int i=0; i < 10; i++) {
			pTable[i] /= ptot;
		}
	}

	// drop an item
	Item item;

	// weighted pick from pTable
	float p = frand( 0.0, 1.0 );
	for (int i=0; i < 10; i++) {
		item.type = i;
		p -= pTable[i];
		if (p < 0.0f) break;
	}

	// now choose a location
	Tile *t;
	do {
		item.x = irand( TILE_SIZE, (g_map->mapSizeX-2) * TILE_SIZE  );
		item.y = irand( TILE_SIZE, (g_map->mapSizeY-2) * TILE_SIZE  );
		t = g_map->getTile( item.x / TILE_SIZE, item.y/TILE_SIZE );
	} while (t->flags & FLAG_BLOCKED );

	g_items.push_back( item );
}

void initLevel() 
{
	// find the rocket and start the player there
	for (int i=0; i < g_map->mapSizeX*g_map->mapSizeY; i++) {
		if ( (g_map->mapdata[i].flags & FLAG_CTILE) &&
			 (g_map->mapdata[i].c_tile==CUSTOM_ROCKET) ) {
                 
				 g_player.setY( (((i/g_map->mapSizeX)+1) * TILE_SIZE) - 3 );
				 g_player.setX( (((i%g_map->mapSizeX)+1) * TILE_SIZE) - TILE_SIZE/2 );
		}
	}

	// clear inv
	for (int i = 0; i < 10; i++) {
		g_player.inv[i] = 0;
	}

	// clear cash
	g_player.cash = 0;

	// init the items
	g_items.erase( g_items.begin(), g_items.end() );

	for (int i=0; i < 50; i++) {
		spawn_item();
	}

}

void game_draw( BITMAP *targ ) 
{
	Map *map = g_map;	

	int x_offs = g_player.x() - 400;
	int y_offs = g_player.y() - 300;

	if (x_offs < 0) x_offs = 0;
	if (y_offs < 0) y_offs = 0;
	if (x_offs > (map->mapSizeX * TILE_SIZE) - 800) {
		x_offs = (map->mapSizeX * TILE_SIZE) - 800;
	}
	if (y_offs > (map->mapSizeY * TILE_SIZE) - 600) {
		y_offs = (map->mapSizeY * TILE_SIZE) - 600;
	}

	// draw background
	if (map->background) {
		int x_max = (map->mapSizeX * TILE_SIZE) - 800;
		int y_max = (map->mapSizeY * TILE_SIZE) - 600;
		float fx, fy;
		fx = (float)x_offs / (float) x_max;
		fy = (float)y_offs / (float) y_max;

		blit( map->background, targ, fx*(BG_SIZE-800), fy*(BG_SIZE-600), 0, 0, 800, 600 );
	} else {
		// clear/bg
		rectfill( targ, 0, 0, 800, 600, makecol16( 0,0,0 ) );
	}

	// draw map
	for (int j=0; j < map->mapSizeY; j++) {
		for (int i=0; i < map->mapSizeX; i++) {

			Tile *t = &(map->mapdata[ (map->mapSizeX*j)+i ]);

			if (map->tilebmp) {
				// bg tile
				if (t->bg_tile != 0 ) {
					masked_blit(map->tilebmp, targ, tilenum( BACKGROUND, t->bg_tile)*TILE_SIZE, 
						0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, TILE_SIZE, TILE_SIZE );
				}
				
				// structure tile
				if (t->flags & FLAG_STILE ) {
					masked_blit(map->tilebmp, targ, tilenum( STRUCTURE, t->s_tile)*TILE_SIZE, 
						0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, TILE_SIZE, TILE_SIZE );
				}

				// foreground tile
				if (t->flags & FLAG_FILLED ) {
					masked_blit(map->tilebmp, targ, tilenum( FOREGROUND, t->fg_tile)*TILE_SIZE, 
						0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, TILE_SIZE, TILE_SIZE );
				}

				// custom tile
				if (t->flags & FLAG_CTILE ) {
					masked_blit(map->tilebmp, targ, tilenum( CUSTOM, t->c_tile)*TILE_SIZE, 
								0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
								TILE_SIZE, TILE_SIZE );
				}
			}
		}
	}

	// draw trains	
	if (!bmpTrain) {
		bmpTrain = load_bitmap( "gamedata/train.bmp", NULL );

		// TODO: maybe tint the train with level color??
	}
	for (unsigned ti=0; ti < g_trains.size(); ti++) {
		draw_sprite( targ, bmpTrain, g_trains[ti].x-x_offs-TILE_SIZE/2, g_trains[ti].y-y_offs-64 );

		if ( (g_trains[ti].state == STATE_S1) ||
			 (g_trains[ti].state == STATE_S2) ) {
		
				char buf[30];
				sprintf(buf,"Departs in %d", (g_trains[ti].timeleft/280)+1 );
				alfont_set_font_size( g_fontTitlePDA, 20 );

				alfont_textout_centre( targ, g_fontTitlePDA, buf, g_trains[ti].x-x_offs+63, g_trains[ti].y-y_offs-82, makecol(0,0,0) );
				alfont_textout_centre( targ, g_fontTitlePDA, buf, g_trains[ti].x-x_offs+65, g_trains[ti].y-y_offs-82, makecol(0,0,0) );
				alfont_textout_centre( targ, g_fontTitlePDA, buf, g_trains[ti].x-x_offs+63, g_trains[ti].y-y_offs-84, makecol(0,0,0) );
				alfont_textout_centre( targ, g_fontTitlePDA, buf, g_trains[ti].x-x_offs+65, g_trains[ti].y-y_offs-84, makecol(0,0,0) );

				alfont_textout_centre( targ, g_fontTitlePDA, buf, g_trains[ti].x-x_offs+64, 
										    g_trains[ti].y-y_offs-83, makecol(255,255,255) );

		}

	}

	// draw items	
	if (!bmpItems[0]) {
		BITMAP *srcBmp = load_bitmap( "gamedata/items.bmp", NULL );				
		for (int i=0; i < 10; i++) {
			bmpItems[i]=create_bitmap( 32, 32 );
			blit( srcBmp, bmpItems[i], i*32, 0, 0,0,32, 32 );			
		}
		release_bitmap( srcBmp );
	}

	for (unsigned i=0; i < g_items.size(); i++) 
    {
        static const float fadeLife = 200;

        int x = g_items[i].x - x_offs;
        int y = g_items[i].y - y_offs;
        if (g_items[i].life < fadeLife)
        {
            float alpha = static_cast<float>(g_items[i].life) / static_cast<float>(fadeLife);
            int   val   = static_cast<int>(256.0f * alpha);
            set_trans_blender(val, val, val, val);

            draw_trans_sprite(targ, bmpItems[ g_items[i].type ], x, y);                
        }
        else
        {
    		draw_sprite( targ, bmpItems[ g_items[i].type ], x, y );
        }
	}	


	// draw player
	static BITMAP *bmpPlayer[6];
	static int playerW=0;	
	if (!playerW) {
		BITMAP *srcBmp = load_bitmap( "gamedata/testdude2.bmp", NULL );		

		// clean up bad mask pixels :(
		for (int jj=0; jj < srcBmp->h; jj++) {
			for (int ii=0; ii < srcBmp->w; ii++) {
				int c = getpixel( srcBmp, ii, jj );
				if ( (getr16(c) > 200) && (getg16(c) < 10) && (getb16(c)>200) ) {
					putpixel( srcBmp, ii, jj, makecol16( 255,0,255 ) );
				}
			}
		}

		playerW =srcBmp->w / 6;
		for (int i=0; i < 6; i++) {
			bmpPlayer[i]=create_bitmap( playerW, srcBmp->h );
			blit( srcBmp, bmpPlayer[i], i*playerW, 0, 0,0,playerW, srcBmp->h );			
		}

		release_bitmap( srcBmp );
	}
	int f;
	if (g_player.jetpack) {
		f = 5;
	} else if (g_player.airborne) {
		f = 4;
	} else {
		f = g_player.f;
	}
	if (g_player.walkdir) {		
		draw_sprite( targ, bmpPlayer[f], 
				 (g_player.x()-x_offs) - playerW/2,
				 (g_player.y()-y_offs) - bmpPlayer[0]->h );			
	} else {
		draw_sprite_h_flip( targ, bmpPlayer[f], 
				 (g_player.x()-x_offs) - playerW/2,
				 (g_player.y()-y_offs) - bmpPlayer[0]->h );			

	}

	// HUD
	char buf[30];
	alfont_set_font_size( g_fontTitlePDA, 30 );
	sprintf ( buf, "Cash: %d", g_player.cash );
	alfont_textout( targ, g_fontTitlePDA, buf, 10, 10, makecol(255,255,255) );
    if (g_debugString.size())
    {
    	alfont_textout( targ, g_fontTitlePDA, g_debugString.c_str(), 
                        10, 30, makecol(255,255,255) );
    }

	// Pop up message
	char *s= NULL;
	char buff[1000];
	if (g_player.cash >= TICKET_PRICE) {		
		if (atRocket) {
			s = "Level Complete! Press TAB to enter rocket.";
		} else {
			s = "Level Complete! Return to rocket to buy ticket to next level";
		}
	} else if (g_atStore) {
			s = "Press TAB to enter train station.";		
	} else if (atRocket) {
		sprintf( buff, "Return to rocket with $%d cash to go to next level.", TICKET_PRICE );
		s = buff;
	}

	if (s) {
		alfont_set_font_size( g_fontTitlePDA, 20 );

		alfont_textout_centre( targ, g_fontTitlePDA, s , 399, 549, makecol(0,0,0) );
		alfont_textout_centre( targ, g_fontTitlePDA, s , 399, 551, makecol(0,0,0) );
		alfont_textout_centre( targ, g_fontTitlePDA, s , 401, 549, makecol(0,0,0) );
		alfont_textout_centre( targ, g_fontTitlePDA, s , 401, 551, makecol(0,0,0) );

		alfont_textout_centre( targ, g_fontTitlePDA, s , 400, 550, makecol(150,255,255) );
	}

	
	
	//sprintf(buf,"At Store %p", g_atStore  );
	//textout_centre( targ, font, buf, g_player.x()-x_offs, g_player.y()-y_offs, makecol(255,255,255) );
	
			
}

bool checkLoc( int x, int y ) {
	static int offsTable[] = {
		-20, 0,
		-20, -80,
		 20, 0,
		 20, -80,
		-20, -40,
		 20, -40
	};
	Tile *t;
	for (int i=0; i < 6; i++) {
		t = g_map->getTile( (x+offsTable[i*2]) / TILE_SIZE, (y+offsTable[i*2+1]) / TILE_SIZE );
		if (t->flags & FLAG_BLOCKED) return false;
    }
	return true;	
}


static bool
integratePlayer(Player& player, const float dt)
{
    // store old state
    float old_fx = player.xF();
    int   old_x  = player.x();
    float old_fy = player.yF();
    int   old_y  = player.y();

    // 'integrate'
    player.setY( player.yF() + player.vy * dt );
    player.setX( player.xF() + player.vx * dt );

    // if we've moved off a valid tile, just reset
    bool yCollision = false;
    bool xCollision = false;
    if ( (!checkLoc( player.x(), player.y() )) )
    {
		// can we move just in x?
	    if (checkLoc( player.x(), old_y )) {
            yCollision = true;
        }
        else if (checkLoc( old_x, player.y() )) {
            xCollision = true;
	    }
        else {
            xCollision = true;
            yCollision = true;
	    }
    }

    Tile *t = g_map->getTile( player.x() / TILE_SIZE,
						      player.y() / TILE_SIZE );
    if ( (t->flags & FLAG_GROUND) &&
         ((player.y()%TILE_SIZE) < 1) &&
         (player.vy > 0.0f) )
    {
        yCollision = true;
    }

    if (yCollision)
    {
        player.setY(old_fy);
        player.vy = 0.0f;
    }
    if (xCollision)
    {
        player.setX(old_fx);
        player.vx = 0.0f;
    }

    return (player.vy != 0.0f) || (player.vx != 0.0f);
}

static void
iteratePlayer(Player& player)
{
    static const float accel    = 0.04;
    static const float termVel  = 3.5f;
    static const float maxUpVel = 1.7f;


    // retrieve the tile one pixel below the player
    int yTile = (player.y()+1) / TILE_SIZE;
    Tile *t = g_map->getTile( player.x() / TILE_SIZE,
						      yTile);

    // calculate whether the feet are on the ground
    bool onBottomTile = yTile == (g_map->mapSizeY-1);
    bool feetOnGround = onBottomTile;
    if (t->flags & FLAG_GROUND)
    {
        static const char maxDistancePerIter =
            static_cast<int>(ceilf(termVel + 2.0f));
        if (((player.y()+1)%TILE_SIZE) < 1)
        {
            feetOnGround = true;
        }
    }

    if (feetOnGround)
    {
        // ensure we have no negative velocity when the 
        // feet are on the ground
        player.vy = std::min(player.vy, 0.0f);
	    player.airborne = 0;
    }
    else 
    {
	    // Flying or falling
	    player.airborne = 1;					
	    if (!player.jetpack) {
            player.vy = std::min(player.vy + accel, termVel);
	    }
    }

    if (player.jetpack) {								
        player.vy = std::max(player.vy - accel, -maxUpVel);
    }

	player.vx = 0.0f;
    if (player.iswalking)
    {
	    player.vx = static_cast<float>(g_player.iswalking);
    }

    // integrate, but ensure we don't go over
    const float maxComponentSpeed = fabsf(std::max(player.vy, player.vx));
    if ((maxComponentSpeed+0.01) <= 1.0f)
    {
        integratePlayer(player, 1.0f);
    }
    else
    {
        const float denom = ceilf(maxComponentSpeed + 0.01);
        const float dt = 1.0f / denom;
        const int   numIter = static_cast<int>(denom);
        assert(fabsf(player.vy * dt) <= 1.0f);
        for (int i = 0; i < numIter; i ++)
        {
            if (!integratePlayer(player, dt))
                break;
        }
        return;
    }
}



void game_update()
{
	static int last_retrace = 0;
	int time_left;

	if ((last_retrace==0)||(g_simResume)) {
		last_retrace = retrace_count;
		time_left = 0;
		g_simResume = 0;
	} else {
		time_left = retrace_count - last_retrace;
		last_retrace = retrace_count;
	}

	// a few ticks to each retrace
	time_left *= 4;

	while ( time_left > 0 )
    {
        iteratePlayer(g_player);

        // check trains
		for (unsigned ti=0; ti < g_trains.size(); ti++) {
			TrainFSM &tr = g_trains[ti];

			if ((tr.state == STATE_S1) || (tr.state==STATE_S2)) {
				tr.timeleft--;
				if (tr.timeleft <= 0 ) {
					if (tr.state==STATE_S1) {
						tr.state = STATE_MOVING_S1_TO_S2;

						// departure from s1, reduce stock
						for (int ii=0; ii < 10; ii++) {
							if (ii <= ITEM_BATTERY) {
								// reduce stock a lot for items that get spawned often
								tr.store1.stock[ii] -= irand(1,10);
							} else {
								tr.store1.stock[ii] -= irand(1,4);
							}
							if (tr.store1.stock[ii] < 0) tr.store1.stock[ii] = 0;
						}

					} else {
						tr.state = STATE_MOVING_S2_TO_S1;

						// departure from s2, reduce stock
						for (int ii=0; ii < 10; ii++) {
							if (ii <= ITEM_BATTERY) {
								// reduce stock a lot for items that get spawned often
								tr.store2.stock[ii] -= irand(1,10);
							} else {
								tr.store2.stock[ii] -= irand(1,4);
							}
							if (tr.store2.stock[ii] < 0) tr.store2.stock[ii] = 0;
						}

					}
				}
			} else if (tr.state == STATE_MOVING_S2_TO_S1) {
				tr.x++;
				if (tr.x > g_map->mapSizeX * TILE_SIZE + 100) {
					tr.x = -100;
				}
				if (abs(tr.x-tr.s1x) < 3) {
					tr.state = STATE_S1;
					tr.timeleft = tr.stime;

					// arrival at s1, restock
					for (int ii=0; ii < 10; ii++) {
						tr.store1.stock[ii] += irand(1,4);
					}

				}
			} else { // moving s1 to s2
				tr.x++;
				if (tr.x > g_map->mapSizeX * TILE_SIZE + 100) {
					tr.x = -100;
				}
				if (abs(tr.x-tr.s2x) < 3 ) {
					tr.state = STATE_S2;
					tr.timeleft = tr.stime;

					// arrival at s2, restock
					for (int ii=0; ii < 10; ii++) {
						tr.store2.stock[ii] += irand(1,4);
					}
				}				
			}			
		}

        // increase lifespan of items
        for (unsigned i = 0; i < g_items.size(); i ++)
        {
            g_items[i].life ++;
        }

		time_left -= 1; 
	}

	// non-ticked update	
    static const int maxNumItems = 50;
	if (g_items.size() < maxNumItems) {
		spawn_item();
	}

	// check if player is at rocket
	Tile *t = g_map->getTile( g_player.x() / TILE_SIZE, (g_player.y() - 5) / TILE_SIZE );
	if ( (t->flags & FLAG_CTILE) &&
		(t->c_tile == CUSTOM_ROCKET) ) {
		atRocket = 1;
	} else {
		atRocket = 0;
	}


	// check if player is at a station
	g_atStore = NULL;
	for (unsigned ti=0; ti < g_trains.size(); ti++) {
		TrainFSM &tr = g_trains[ti];
		if ( (g_player.y()-4 < tr.y) &&
			 (tr.y - g_player.y() < 64)) {

			if ((g_player.x() > tr.s1x )&& 
			    (g_player.x() - tr.s1x < 128 )) {
				g_atStore = &(tr.store1);
				break;
			} else if ((g_player.x() > tr.s2x )&& 
				       (g_player.x() - tr.s2x < 128 ) ) {
				g_atStore = &(tr.store2);
				break;
			}
		}

	}

	// check for item pickups
	std::vector<Item> items2;
	for (unsigned i=0; i < g_items.size(); i++) {
		Item &item = g_items[i];
		if (( abs(item.x - g_player.x()) < 32 ) &&
			( g_player.y() - item.y  > 0 ) &&
			( g_player.y() - item.y  < 80 ) ) {

			// pick up item
			g_player.inv[item.type]++;

		} else {
			items2.push_back( item );
		}
	}
	g_items = items2;

	// Animate walking frame	
	if (g_player.iswalking) {
		g_player.ff++;
	}
	if (g_player.ff>=10) {
		g_player.ff=0;
		g_player.f++;		
	}
	if (g_player.f>=4) g_player.f = 0;		
}

int main( int argc, char *argv[] )
{	
	BITMAP *backbuf;			
	
	allegro_init();
	alfont_init();

	install_keyboard();
	install_mouse();
	
	set_color_depth( 16 );
	//if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,800,600, 0, 0) != 0) {
	if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 800,600, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set 800x600 graphic mode\n%s\n", allegro_error);
      return 1;
   }

	// Initialize game
	gamestate = GAMESTATE_PDA;

	// Set up the fonts
	g_fontTextPDA = alfont_load_font( "gamedata/bobcaygr.ttf" );
	g_fontTitlePDA = alfont_load_font( "gamedata/negtiv12.TTF" );
	
	// init player
	g_player.setX(300.0f);
	g_player.setY(300.0f);
	g_player.f = 0;
	g_player.ff = 0;
	g_player.cash = 0;

	text_mode( -1 );
	alfont_text_mode( -1 );
	backbuf = create_bitmap( 800, 600 );
	while (1) {

		// redraw
		if (gamestate==GAMESTATE_PDA) {
			pda_draw( backbuf );
		} else if (gamestate==GAMESTATE_TILETEST) {
			tiles_draw( backbuf );
		} else {
			// game draw
			game_draw( backbuf );
			//rectfill( backbuf, 0, 0, 800, 600, makecol(0,0,0) );
		}

		// swapbuf
		vsync();
		acquire_screen();
		blit( backbuf, screen, 0, 0, 0, 0, 800, 600 );
		release_screen();

		// update .. check for keys
		if (keypressed()) {
			int k = readkey();
			
			if (gamestate==GAMESTATE_PDA) {
				pda_keypress( k );
			} else if (gamestate==GAMESTATE_TILETEST) {
				tiles_keypress( k );
			} else if (gamestate==GAMESTATE_GAME){
				switch(k>>8) {			
					case KEY_TAB:				
						{							
							if (atRocket) {
								// if we're on the rocket tile, go to 
								// the "main menu" tab.
								g_currTabItem = 0;
							}

							// if at a store, we probably want to go to inv screen
							else if (g_atStore) {
								g_currTabItem = 2;
							}
							gamestate=GAMESTATE_PDA;
						}
						break;
				}	
			}

			// global keypresses
			switch(k>>8) {			
				case KEY_9:
				// cheat
				g_player.cash += 1000;
				break;
			}	
		}

		// check for quit
		if (key[KEY_ESC]) {
			exit(0);
			break;
		}


		// player movement		
		int step = 5;
		g_player.iswalking = 0;
		g_player.jetpack = 0;
		if ((key[KEY_UP])&&(!key[KEY_DOWN])) { g_player.jetpack = 1; }		
		//if ((key[KEY_S])) { g_player.y() += step; }	
		if ((key[KEY_LEFT])&&(!key[KEY_RIGHT])) { g_player.iswalking = -1; g_player.walkdir = 1; }
		if ((key[KEY_RIGHT])&&(!key[KEY_LEFT])) { g_player.iswalking = 1; g_player.walkdir = 0; }
	
		// do update
		// TODO: replace with real timing loop
		if (gamestate==GAMESTATE_PDA) {
			pda_update();
		} else if (gamestate==GAMESTATE_TILETEST) {
			tiles_update( );
		} else if (gamestate==GAMESTATE_GAME) {
			game_update();
		}
		
		yield_timeslice();
	}
		
}
END_OF_MAIN();