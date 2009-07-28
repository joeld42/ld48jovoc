#include <stdio.h>
#include <math.h>
#include <deque>
#include <vector>
#include <algorithm>

#include <allegro.h>
#ifndef NDEBUG
# include <winalleg.h>
#endif

#include <tilemap.h>
#include <game_obj.h>

enum 
{
	Tool_WALK,
	Tool_FISH
};




std::vector<TileMap*> bubbles;
std::vector<GameObj*> npcs;

std::vector<BITMAP*> critterBmps;

#define MSG_TIME (200)
char messageText[256] = "";
int messageTime = 0;

bool cheatsEnabled = false;

std::deque<std::string> messageQueue;

float lerp( float t, float a, float b );

void message( const char *message )
{
	messageQueue.push_back( message );
}

volatile int ticks = 0;
void ticker()
{
	ticks++;
}
END_OF_FUNCTION( ticker );

const int UPDATES_PER_SEC = 30;

BITMAP *make_pixbot( BITMAP *codeMask );

std::vector<GameObj*> emptyObjs;
TileMap *create_bubble( std::vector<TileMap*> &landChunks )
{
	int ndx = ((float)rand() / (float)RAND_MAX) * landChunks.size();
	TileMap *bub = new TileMap( landChunks[ndx]->m_size );
	bub->bub_age = (float)rand() / (float)RAND_MAX;

	bub->paste( landChunks[ndx], 0, 0 );

	int sz = bub->m_size * 12;
	BITMAP *tmpBmp = create_bitmap( sz, sz );
	rectfill( tmpBmp, 0, 0, sz, sz, makecol( 0xff, 0x00, 0xff) );

	bub->draw( tmpBmp, 0, 0, emptyObjs );

	bub->bub_bmp = create_bitmap( 20, 20 );
	stretch_blit( tmpBmp, bub->bub_bmp, 0, 0, 
	  		  	  tmpBmp->w, tmpBmp->h, 0, 0, 
				bub->bub_bmp->w, bub->bub_bmp->h );
	destroy_bitmap( tmpBmp );

	circle( bub->bub_bmp, 10, 10, 9, makecol( 0x66, 0xee, 0xff ) );
	circlefill( bub->bub_bmp, 6, 6, 2, makecol( 0xff, 0xff, 0xff ) );

	hline( bub->bub_bmp, 2, 17, 18, makecol( 155, 193, 212 ) );
	hline( bub->bub_bmp, 2, 18, 18, makecol( 0xff, 0x00, 0xff ) );
	hline( bub->bub_bmp, 2, 19, 18, makecol( 0xff, 0x00, 0xff ) );

	return bub;
}

void loadLandChunks( const char *mapfile, std::vector<TileMap*> &landChunks )
{
	FILE *fp = fopen( mapfile, "rt" );
	char line[1000];
	while (!feof(fp))
	{
		char tok[256];
		int chunkSize;

		fgets( line, 1000, fp );
		if(sscanf( line, "%s", tok))
		{
			if (!stricmp( tok, "LAND" ))
			{
				sscanf( line, "%*s %d",  &chunkSize );
				TileMap *chk = new TileMap( chunkSize );
				for (int j=0; j < chunkSize; ++j)
				{
					fgets( line, 1000, fp );					
					for (int i=0; i < chunkSize; ++i )
					{
						chk->map( i, j ).m_gen = Gen_NONE;

						// get hite
						char hite = line[i*2];
						if (hite=='.')
						{							
							chk->map( i,j ).m_hite = 0;
						}
						else
						{
							chk->map( i,j ).m_hite = (hite-'1');
						}

						// get terrain type
						Tile *t = NULL;
						switch( line[i*2+1] )
						{
							case '.':
								// empty
								t = NULL;								
								break;
							case 'o':
								t = chk->m_tileset[ 0 ]; // land tile								
								break;
							case 'O':
							case '0':								
								t = chk->m_tileset[ 0 ]; // land tile with bubble																	
								chk->map( i, j ).m_gen = Gen_BUBBLE;								
								break;
							case '*':
								t = chk->m_tileset[ 0 ]; // land tile with critter
								chk->map( i, j ).m_gen = Gen_CRITTER;
								break;
							case '$':
								t = chk->m_tileset[ 0 ]; // land tile with NPC
								chk->map( i, j ).m_gen = Gen_NPC;
								break;
							case 't':
								t = chk->m_tileset[ 1 ]; // palm tree tile								
								break;
							case 'T':
								t = chk->m_tileset[ 2 ]; // tree tile								
								break;
							case 'r':
								t = chk->m_tileset[ 3 ]; // rock tile
								break;
							case 'R':
								t = chk->m_tileset[ 4 ]; // big rock tile
								break;
						}

						chk->map( i, j ).m_tile = t;
					}					
				}
				landChunks.push_back( chk );
			}
		}
	}
}

int main( int argc, char *argv[] )
{
    BITMAP *lobuf;
    BITMAP *backbuf;    

#ifndef NDEBUG
    // Create a win32 console for printfing
	AllocConsole();

	freopen("CONIN$","rb",stdin);   // reopen stdin handle as console window input
	freopen("CONOUT$","wb",stdout);  // reopen stout handle as console window output
	freopen("CONOUT$","wb",stderr); // reopen stderr handle as console window output
#endif

    //----- Game stuff ---------------

	// the map of the world
    TileMap map( 300 );

	// the chunks of land
	std::vector<TileMap*> landChunks;

    set_color_depth( 32 );

    if (allegro_init() != 0) return 1;
    
    install_keyboard();
    install_timer();
	install_mouse();	

	// install ticker
	LOCK_VARIABLE( ticks );
	LOCK_FUNCTION( ticker );
	install_int_ex( ticker, BPS_TO_TIMER( UPDATES_PER_SEC ) );
    
    if (set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0 ) != 0)
    {
        set_gfx_mode( GFX_TEXT, 0, 0, 0, 0 );
        allegro_message( "Unable to set graphics mode:\n%s\n", 
                         allegro_error );
        return 1;
    }

	enable_hardware_cursor();
	select_mouse_cursor( MOUSE_CURSOR_ARROW );
	

    lobuf = create_bitmap( 320, 240 );    
    backbuf = create_bitmap( SCREEN_W, SCREEN_H );
	show_mouse( backbuf );

//    printf("lobuf %p backbuf %p\n", lobuf, backbuf );    
        
	

    // init gameplay stuff
	srand( time(0));

    //printf("init map...\n" );    
    map.init();
    map.reset();    
	map.do_water = true;

	message( "I woke near the ocean. I didn't even know my own name." );
	message( "So I started exploring (drag RMB to scroll)");

	// load chunks
	loadLandChunks( "gamedata/land.txt", landChunks );
	map.paste( landChunks[0], 148, 148 );

    map.enableSelect( lobuf );   

	// make player
	BITMAP *npcCodeMask = load_bitmap( "gamedata/code_npc.bmp", NULL );
	BITMAP *critterCodeMask = load_bitmap( "gamedata/code_critter.bmp", NULL );
	BITMAP *playerBmp = make_pixbot( npcCodeMask );		

	// Init game objects
	std::vector<GameObj*> gameObjs;

	Player *player = new Player();
	player->m_bmp = playerBmp;
	player->m_x = 150;
	player->m_y = 148;
	gameObjs.push_back( player );
	int currTool = Tool_WALK;	

    //printf("--- mainloop\n" );   
    bool done = false;
	bool dbgShowSelects = false;

	// near center on a 300x300 map
	int view_x = -1644, 
		view_y = -800;
	bool dragging = false;
	int drag_x, drag_y; 	
	int view_drag_x, view_drag_y; 
	int mouse_x2, mouse_y2; // in lobuf coords	
	int last_b = 0;
	int px, py;
	bool doGenerate = true;

    while (!done)
    {
		// handle ticks -- give the ticker a chance to start
		while( ticks==0)
		{
			rest(100 / UPDATES_PER_SEC );
		}
		while ( ticks > 0)
		{
			int old_ticks = ticks;			
			
			//=== Update ====
			map.m_wave_offs++;
						
			if (messageTime>0)
			{
				messageTime--;
				if (messageTime==0)
				{
					strcpy( messageText, "" );					
				}
			} else {
				if (!messageQueue.empty())
				{
					strcpy( messageText, messageQueue.front().c_str() );
					messageQueue.pop_front();
					messageTime = MSG_TIME;
				}
			}

			for (int i=0; i < gameObjs.size(); ++i )
			{
				if (gameObjs[i])
				{
					gameObjs[i]->update( map, gameObjs );				
				}
			}

			// update may have NULL'd objects
			std::vector<GameObj*>::iterator removed;
			removed = std::remove( gameObjs.begin(), gameObjs.end(), (GameObj*)NULL );
			gameObjs.erase( removed, gameObjs.end() );
			

			// update bubbles
			for (int i=0; i < bubbles.size(); i++)
			{
				bubbles[i]->bub_age += 1.0 / (float)UPDATES_PER_SEC;
			}

			ticks--;
			if (old_ticks <= ticks) break;
		}

		int dropBubble = -1;

		// keyboard input
        if (keypressed())
        {
			int k = readkey();
			
			if (cheatsEnabled)
			{
				switch(k>>8) 
				{
					//---- debug keys
					case KEY_F5:
						dbgShowSelects = !dbgShowSelects;
						break;
					case KEY_F6:
						playerBmp = make_pixbot( npcCodeMask );	
						player->m_bmp = playerBmp;
						break;
					case KEY_F7:
						bubbles.push_back( create_bubble( landChunks ) );
						break;
				}				
			}

			switch(k>>8) {			            
				
				case KEY_F11:
					cheatsEnabled = true;
					message("Cheats enabled.");
					break;
				
				//---- game keys
				case KEY_1: dropBubble = 0; break;				
				case KEY_2: dropBubble = 1; break;
				case KEY_3: dropBubble = 2; break;
				case KEY_4: dropBubble = 3; break;
				case KEY_5: dropBubble = 4; break;

				// hack -- on keydown, clear walk wait counter
				case KEY_LEFT:
				case KEY_RIGHT:
				case KEY_UP:
				case KEY_DOWN:
					player->walk_c = 0;
					break;

			}
        }

		if ((dropBubble >= 0) && (dropBubble < bubbles.size()) )
		{
			TileMap *bub = bubbles[dropBubble];
			bubbles[dropBubble] = bubbles[ bubbles.size() -1 ];
			bubbles.pop_back();

			map.paste( bub, 
					   player->m_x - bub->m_size/2, 
					   player->m_y - bub->m_size/2 );
			doGenerate = true;
			delete bub;
		}

		// generate entities
		if (doGenerate)
		{
			doGenerate = false;			

			for (int i=0; i < map.m_size; i++)
			{
				for (int j=0; j < map.m_size; j++)
				{
					if (map.map(i,j).m_gen != Gen_NONE )
					{						
						if (map.map(i,j).m_gen == Gen_BUBBLE)
						{							
							BubbleObj *bubObj = new BubbleObj();
							bubObj->m_x = i;
							bubObj->m_y = j;
							bubObj->m_bub = create_bubble( landChunks );
							bubObj->m_bmp = create_bitmap( 15, 15 );
							BITMAP *bubBmp = bubObj->m_bub->bub_bmp;
							stretch_blit( bubBmp, bubObj->m_bmp, 0,0, bubBmp->w, bubBmp->h, 0,0, 15, 15 );

							gameObjs.push_back( bubObj );
						} else if (map.map(i,j).m_gen == Gen_CRITTER) {
							CritterObj *critObj = new CritterObj();
							critObj->m_x = i;
							critObj->m_y = j;				

							BITMAP *critpic;
							int critNdx = rand() % (critterBmps.size()+1);
							if (critNdx == critterBmps.size())
							{
								// yay new critter
								critpic = make_pixbot( critterCodeMask );
								critterBmps.push_back( critpic );
							}
							else
							{
								critpic = critterBmps[ critNdx ];
							}

							critObj->m_bmp = critpic;

							gameObjs.push_back( critObj );

						} else if (map.map(i,j).m_gen == Gen_NPC) {
							NpcObj *npc = new NpcObj();
							npc->m_x = i;
							npc->m_y = j;							
							npc->m_bmp = make_pixbot( npcCodeMask );							

							gameObjs.push_back( npc );
						}

						map.map(i,j).m_gen = Gen_NONE;
					}
				}
			}
		}
		
		// check for quit
		if (key[KEY_ESC]) {
			exit(0);
			break;
		}

		player->walk_x = 0; player->walk_y = 0;
		if (key[KEY_UP] && !key[KEY_DOWN])
		{
			player->walk_x = 0; player->walk_y = 1;
		}
		else if (!key[KEY_UP] && key[KEY_DOWN])
		{
			player->walk_x = 0; player->walk_y = -1;
		}
		else if (!key[KEY_LEFT] && key[KEY_RIGHT])
		{
			player->walk_x = -1; player->walk_y = 0;
		}
		else if (key[KEY_LEFT] && !key[KEY_RIGHT])
		{
			player->walk_x = 1; player->walk_y = 0;
		}
				
        // Map cursor
		char buff[246];
		int mapX, mapY;
		map.clearSelected();

		mouse_x2 = mouse_x/2;
		mouse_y2 = mouse_y/2;
		if (map.screenToMap( mouse_x2, mouse_y2, mapX, mapY ))
		{
			sprintf( buff, "V %d %d x y %d %d [%d]  -- map %d %d", 
					view_x, view_y, mouse_x, mouse_y, mouse_b, mapX, mapY );

			// NOTE: Don't use mouse selection anymore -- just use selection
			// to mark player
			//map.map( mapX, mapY ).m_selected = true;
		}
		else
		{
			sprintf( buff, "V %d %d x y %d %d  NO TILE", 
					view_x, view_y, mouse_x, mouse_y );
		}

		map.map( player->m_x, player->m_y ).m_selected = true;

		

		// Mouse button 2 -- drag
		if (mouse_b & 0x2)
		{
			if (!dragging)
			{
				dragging = true;
				drag_x = mouse_x2;
				drag_y = mouse_y2;
				view_drag_x = view_x;
				view_drag_y = view_y;
			}
			else
			{
				int dx = mouse_x2 - drag_x;
				int dy = mouse_y2 - drag_y;
				view_x = view_drag_x + dx;
				view_y = view_drag_y + dy;

			}
		}
		else
		{
			dragging = false;
		}

		// button 1 -- use tool (or walk)		
		if (mouse_b & 0x1)
		{
			switch( currTool)
			{
			case Tool_WALK:
				{
					MapCell &cell = map.map(player->m_x, player->m_y );					
					px = cell.sx + 4;
					py = cell.sy + 10;
					
					if ((px < mouse_x2) && (py < mouse_y2 ))
					{
						player->walk_x = -1; player->walk_y = 0;
					}
					else if ((px > mouse_x2) && (py > mouse_y2 ))
					{
						player->walk_x = 1; player->walk_y = 0;
					}
					else if ((px > mouse_x2) && (py < mouse_y2 ))
					{
						player->walk_x = 0; player->walk_y = -1;
					}
					else if ((px < mouse_x2) && (py > mouse_y2 ))
					{
						player->walk_x = 0; player->walk_y = 1;
					}
				}
				break;
			}
		}
		else
		{
			switch( currTool )
			{
			case Tool_WALK:
				if (last_b & 0x01)
				{
					player->walk_x = 0;
					player->walk_y = 0;
				}
				break;
			}
		}
		last_b = mouse_b;

		// ==== draw =====
		rectfill( lobuf, 0, 0, 320, 240, makecol( 135, 171, 189 ) );
        //map.draw( lobuf, 10, 30 );
        map.draw( lobuf, view_x, view_y, gameObjs );        

		// draw bubbles
		int bx = 160 - (bubbles.size() * 12);
		char buf[10];
		for (int i=0; i < bubbles.size(); i++)
		{
			TileMap *b = bubbles[i];
			draw_sprite( lobuf, b->bub_bmp, 
				bx,  215 - (int)(sin(b->bub_age * M_PI)*3) );

			sprintf( buf,"%d", i+1 );
			textout_centre_ex( lobuf, font, buf, bx + 13, 231, makecol( 0x44, 0x66, 0x77 ), -1);
			textout_centre_ex( lobuf, font, buf, bx + 12, 230, makecol( 0xff, 0xff, 0xff ), -1);

			bx += 24;
		}

		//DBG draw player
		//masked_stretch_blit( playerBmp, lobuf, 0, 0, playerBmp->w, playerBmp->h,
		//								10, 20, playerBmp->w * 4, playerBmp->h * 4 );				

        // scale buffer to screen
		stretch_blit( dbgShowSelects?map.m_selectMap:lobuf, backbuf, 
                      0, 0, lobuf->w, lobuf->h,
                      0, 0, SCREEN_W, SCREEN_H );        
        
        
		// Draw text and stuff at full res
		//masked_blit( bubbles[0]->bub_bmp, backbuf, 0, 0, 50, 20, 
		//			 bubbles[0]->bub_bmp->w, bubbles[0]->bub_bmp->h );

		//textout( backbuf, font, buff, 10, 10, makecol( 0xff, 0xff, 0xff ) );

		drawing_mode( DRAW_MODE_TRANS, NULL, 0, 0 );
		set_trans_blender( 0, 0, 0, 128 );
		rectfill( backbuf, 0, 7, 640, 75, makecol( 0xff, 0xff, 0xff ) );
		solid_mode();
		hline( backbuf, 0, 7, 640, makecol( 0, 0, 0x77 ) );
		hline( backbuf, 0, 75, 640, makecol( 0, 0, 0x77 ) );

		// player icon
		masked_stretch_blit( playerBmp, backbuf, 0, 0, playerBmp->w, playerBmp->h,
												10, 10, playerBmp->w * 4, playerBmp->h * 4 );

		//party icons
		for (int i=0; i < npcs.size(); i++)
		{
			BITMAP *npcBmp = npcs[i]->m_bmp;
			masked_stretch_blit( npcBmp, backbuf, 0, 0, npcBmp->w, npcBmp->h,
													70 + 45*i, 10, npcBmp->w * 4, npcBmp->h * 4 );
		}

		if (strlen(messageText))
		{
			float t = (float)messageTime / MSG_TIME;			

			//textout_centre_ex( backbuf, font, messageText, 322, 62, makecol( 0x44, 0x66, 0x77 ), -1 );
			textout_centre_ex( backbuf, font, messageText, 320, 60, 
									makecol( lerp( t, 0x00, 0xcc ), 
											 lerp( t, 0x00, 0xcc ), 
											 lerp( t, 0x00, 0xcc ) ), -1 );
				//makecol( 0x44, 0x66, 0x77 ),
				//makecol( 0xff, 0xff, 0xff ), -1 );
		}

        // flip screen
		vsync();
		acquire_screen();		
        blit( backbuf, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H );        
		release_screen();

		yield_timeslice();
    }

    return 0;
    
}
END_OF_MAIN();

