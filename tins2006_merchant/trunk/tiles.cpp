#include <allegro.h>
#include <math.h>
#include <assert.h>
#include <alfont.h>
#include <stdio.h>
#include <vector>

#include "globals.h"
#include "tiles.h"
#include "shader.h"

Map *playMap = NULL;

Map *g_map;

bool mapOverlay;

Map::Map() {
	mapdata = NULL;
	background = NULL;
	tilebmp = NULL;
	minimap = NULL;
	mapSizeX = 0;
	mapSizeY = 0;

	clr_sky       = makecol16( 180,217, 231 );
	clr_organic   = makecol16( 89, 209, 100 );
	clr_inorganic = makecol16( 197, 159, 76 );
	clr_accent    = makecol16( 250, 229, 158 );
}

int x_offs, y_offs;
int currTile = 0;

Tile *Map::getTile( int i, int j )
{
	assert( i>=0 );
	assert( j>=0 );
	assert( i<mapSizeX );
	assert( j<mapSizeY );

	return &(mapdata[j*mapSizeX+i]);
}

// doesn't apply to CUSTOM types
int Map::getNbr( int i, int j, int type )
{
	assert( i>=0 );
	assert( j>=0 );
	assert( i<mapSizeX );
	assert( j<mapSizeY );

	if (type==BACKGROUND) 
		return mapdata[j*mapSizeX+i].bg_tile;
	else if (type==STRUCTURE) 
		return mapdata[j*mapSizeX+i].s_tile;
	else // (type==FOREGROUND) 
		return mapdata[j*mapSizeX+i].fg_tile;
}

void Map::setNbr( int i, int j, int type, int nbr )
{
	assert( i>=0 );
	assert( j>=0 );
	assert( i<mapSizeX );
	assert( j<mapSizeY );

	if (type==BACKGROUND) 
		mapdata[j*mapSizeX+i].bg_tile = nbr;
	else if (type==STRUCTURE) 
		mapdata[j*mapSizeX+i].s_tile = nbr;
	else // (type==FOREGROUND) 
		mapdata[j*mapSizeX+i].fg_tile = nbr;
}

// takes map with only empty or full tiles and
// adds "corners"
void Map::add_nbr( int type )
{
	int f;
	if (type==BACKGROUND) f = FLAG_BGTILE;
	else if (type==STRUCTURE) f = FLAG_STILE;
	else if (type==FOREGROUND) f = FLAG_FILLED;

	for (int j=0; j < mapSizeY; j++) {
		for (int i=0; i < mapSizeX; i++) {
			int nbr;
			
			// filled
			if (getNbr(i,j, type )!=0) continue;

			nbr = 0;
			if ((i>0)&&(getNbr(i-1,j,type) == 15)) nbr |= WEST;
			if ((i<mapSizeX-1)&&(getNbr(i+1,j,type) == 15)) nbr |= EAST;
			if ((j>0)&&(getNbr(i,j-1,type) == 15)) nbr |= NORTH;
			if ((j<mapSizeY-1)&&(getNbr(i,j+1,type) == 15)) nbr |= SOUTH;

			setNbr( i, j, type, nbr );
			getTile( i, j )->flags |= f;
		}
	}
}

// takes map with only empty or full tiles and
// sets nbr flags based on what's there
void Map::calc_nbr( int type )
{
	int f;
	if (type==BACKGROUND) f = FLAG_BGTILE;
	else if (type==STRUCTURE) f = FLAG_STILE;
	else if (type==FOREGROUND) f = FLAG_FILLED;

	for (int j=0; j < mapSizeY; j++) {
		for (int i=0; i < mapSizeX; i++) {
			int nbr;
			
			// filled
			Tile *t = getTile(i,j);
			if ((t->flags&f)==0) continue;			

			nbr = 0;
			if ((i>0)&&(getTile(i-1,j)->flags&f)) nbr |= WEST;
			if ((i<mapSizeX-1)&&(getTile(i+1,j)->flags&f)) nbr |= EAST;
			if ((j>0)&&(getTile(i,j-1)->flags&f)) nbr |= NORTH;
			if ((j<mapSizeY-1)&&(getTile(i,j+1)->flags&f)) nbr |= SOUTH;					

			setNbr( i, j, type, nbr );
		}
	}
}

void Map::gen_map( int seed ) {
	srand( seed );
	if (mapdata) free( mapdata );
	mapSizeX = irand( 32, 80 );
	mapSizeY = irand( 20, 80 );
	mapdata = (Tile*)malloc( sizeof(Tile) * mapSizeX * mapSizeY );

	g_trains.erase( g_trains.begin(), g_trains.end() );

	for (int j=0; j < mapSizeY; j++) {
		for (int i=0; i < mapSizeX; i++) {
			Tile *t = &(mapdata[ (mapSizeX*j)+i ]);
			t->bg_tile = 0;
			t->fg_tile = 0;
			t->s_tile = 0;
			t->c_tile = 0;

			// always enclose edges
			if ( (i==0)||(j==0)||(i==mapSizeX-1)||(j==mapSizeY-1)) {
				t->flags = FLAG_FILLED | FLAG_BLOCKED;				
			} else {
				t->flags = 0;
			}
		}
	}

	// make some random background tiles
	int numcrap = irand( 0, (mapSizeX*mapSizeY)/2 );
	for (int i=0; i < numcrap; i++) {
		int ndx = irand( 0, mapSizeX*mapSizeY );
		Tile *t = &(mapdata[ndx]);
		t->bg_tile = 15;
		t->flags |= FLAG_BGTILE;
	}
	add_nbr( BACKGROUND );

	// make some random structure tiles
	for (int i=0; i < mapSizeX*mapSizeY/10; i++) {
		int ndx = irand( 0, mapSizeX*mapSizeY );
		Tile *t = &(mapdata[ndx]);
		t->s_tile = 0;
		t->flags |= FLAG_STILE;
	}
	
	// Map types
	enum {
		MAP_STEPPES,		
		MAP_PLATFORMS,
		MAP_PLATFORMS_AND_STEPPES,

		MAP_NUMTYPES
	};	

	
	// Generate platform tiles
	int mapType = irand( 0, MAP_NUMTYPES );
	
	// clear fg tiles (may not be needed any more)
	for (int j=0; j < mapSizeY; j++) {
		for (int i=0; i < mapSizeX; i++) {
			Tile *t = &(mapdata[ (mapSizeX*j)+i ]);
			t->fg_tile = 15;
		}
	}

	// add trestles
	std::vector<int> tresloc;
	int numTrestles = irand( 2, 4 );
	for (int tr = 0; tr < 3; tr++) 
	{
	
		int tresY =0;
		int clear=0;
		while (!clear) {
			tresY = irand( 3, mapSizeY-1 );
			clear = 1;
			for (size_t i=0; i < tresloc.size(); i++ ) {
				if (abs(tresloc[i]-tresY)<2) {
					clear = 0; 
					break;
				}
			}
		}

		tresloc.push_back( tresY );		

		// add two stations on each trestle
		TrainFSM train;
		for (int pi=0; pi < 2; pi++) {
			int x1 = irand( 0, mapSizeX );
			int x2 = x1 + irand( 5, 10 );
			int y = tresY;			

			if (x2 > mapSizeX-1) x2 = mapSizeX-1;

			// add train FSM
			if (pi==0) {
				train.s1x = ((x2+x1)/2) * TILE_SIZE;
				train.y = tresY * TILE_SIZE;
			} else { // pi == 2				

				train.s2x = ((x2+x1)/2) * TILE_SIZE;
				train.x = train.s1x;
				train.stime = irand( 5, 30 );
				train.stime *= 280;
				train.timeleft = train.stime;
				train.state = STATE_S1;

				// Initialize the store

				for (int si=0; si <2; si++) {
					Store *s;
					if (si==0) s = &(train.store1);
						  else s = &(train.store2);

				    // Init store

				    // common
				    s->price[ ITEM_MUSHROOM ] = irand(1, 3); // 1 max margin
					s->price[ ITEM_APPLE ]    = irand(2, 6); // 3
					s->price[ ITEM_BATTERY]   = irand(4, 7); // 2

					// uncommon
					s->price[ ITEM_BULB ]   = irand(50, 100); // 50
					s->price[ ITEM_FISH ]   = irand(80, 115); // 30
					s->price[ ITEM_SAX ]    = irand(100, 175); // 75
					s->price[ ITEM_CHEESE ] = irand(110, 160); // 50

					// rare
					s->price[ ITEM_TILE ]   = irand(500, 600); // 100
					s->price[ ITEM_HAMMER ] = irand(600, 880); // 280
					s->price[ ITEM_LATTE ]  = irand(1000, 1500); // 500

					for (int jj=0; jj < 10; jj++) s->stock[jj] = 0;
				}

				g_trains.push_back( train );				
			}

			// trestle support
			int xx1 = x1, xx2 = x2;
			int min_support = irand( 3, (x2-x1)-4 );
			if (min_support < 3) min_support = 3;

			for (int j=tresY+1; j < mapSizeY-1; j++) {

				if (xx2-xx1 > min_support) xx2--;
				if (xx2-xx1 > min_support) xx1++;
				for (int i=xx1; i < xx2; i++ ) {
					Tile *t = &(mapdata[ (mapSizeX*j)+i ]);					
					t->flags |= FLAG_STILE;					
				}
			}


			// trestle platform
			for (int i=x1; i < x2; i++) {
				int y2 = y + irand( 1, 3 );
				for (int j=y; j < y2; j++ ) {
					
					Tile *t = &(mapdata[ (mapSizeX*j)+i ]);
					t->flags |= FLAG_FILLED | FLAG_BLOCKED;
					if (j==y) {
						t->flags |= FLAG_GROUND;
					}
				}
			}

			//  add train station between x2, x1
			int sloc = (x2+x1)/2;
			Tile *t = &(mapdata[ (mapSizeX*(tresY-1))+sloc ]);
			t->flags |= FLAG_CTILE;
			t->c_tile = CUSTOM_STATION1;
			t = &(mapdata[ (mapSizeX*(tresY-1))+sloc+1 ]);
			t->flags |= FLAG_CTILE;
			t->c_tile = CUSTOM_STATION2;

		}

	}

	calc_nbr( STRUCTURE );

	// "tracks" for trestle"
	for (size_t yi=0; yi < tresloc.size(); yi++) {
		for (int i=1; i < mapSizeX-1; i++) {
			Tile *t = &(mapdata[ (mapSizeX*tresloc[yi])+i ]);

			// Todo: replace with custom "trestle" tile
			t->flags |= FLAG_STILE | FLAG_CTILE;
			t->s_tile = WEST | EAST;
			t->c_tile = CUSTOM_TRACK;
		}
	}

	// add extra platforms
	if ((mapType==MAP_PLATFORMS) || (mapType==MAP_PLATFORMS_AND_STEPPES)) {

		int numPlats = irand( 2, 10 );

		for (int s=0; s < numPlats; s++) {

			int x1 = irand( 0, mapSizeX );
			int x2 = x1 + irand( 5, 10 );
			int y = irand( 0, mapSizeY-3 );

			if (x2 > mapSizeX-1) x2 = mapSizeX-1;

			for (int i=x1; i < x2; i++) {
				int y2 = y + irand( 1, 3 );
				for (int j=y; j < y2; j++ ) {
					// TODO: Make "top" tiles
					Tile *t = &(mapdata[ (mapSizeX*j)+i ]);
					t->flags |= FLAG_FILLED | FLAG_BLOCKED;
					if (j==y) {
						t->flags |= FLAG_GROUND;
					}
				}
			}
		}
	}

	calc_nbr( FOREGROUND );
	
	// Do steppes
	if ((mapType==MAP_STEPPES) || (mapType==MAP_PLATFORMS_AND_STEPPES))  {
		// Good for wide, short maps	
		int numSteppes = irand( 5, 20 );

		for (int s=0; s < numSteppes; s++) {

			int x1 = irand( 1, mapSizeX );
			int x2 = x1 + irand( 5, 10 );
			int y = irand( mapSizeY/2, mapSizeY );

			if (x2 > mapSizeX) x2 = mapSizeX-1;

			for (int i=x1; i < x2; i++) {
				for (int j=y; j < mapSizeY-1; j++ ) {
					// TODO: Make "top" tiles
					Tile *t = getTile(i,j);
					if (t->flags & FLAG_FILLED) {
						// already filled.. unblock & keep going
						t->flags &= ~FLAG_BLOCKED;
						continue;
					}
					t->flags |= FLAG_FILLED;					
					t->flags &= ~FLAG_BLOCKED;

					int nbr = t->fg_tile;
					if (j==y) {
						t->flags |= FLAG_GROUND;
						nbr &= ~NORTH;
					}
					
					if (i==x1) nbr &= ~WEST;
					if (i==x2-1) nbr &= ~EAST;					
					t->fg_tile = nbr;
				}
			}
		}
	}

	// make minimap
	if (minimap) release_bitmap( minimap );
	minimap = create_bitmap( mapSizeX, mapSizeY );
	for (int j=0; j < mapSizeY; j++) {
		for (int i=0; i < mapSizeX; i++) {
			Tile *t = &(mapdata[ (mapSizeX*j)+i ]);
			if (t->flags & FLAG_GROUND) {
				putpixel( minimap, i, j, clr_accent );
			} else if (t->flags & FLAG_FILLED) {
				putpixel( minimap, i, j, clr_organic );
			} else {
				putpixel( minimap, i, j, clr_sky );
			}
		}
	}

	// place rocket (start/exit point)
	std::vector<int> goodLoc;
	for (int j=1; j < mapSizeY-1; j++) {
		for (int i=0; i < mapSizeX; i++) {
			Tile *t = getTile( i, j );
			Tile *t2 = getTile( i, j+1 );
			Tile *t3 = getTile( i, j-1 );
			if ( (!(t->flags & FLAG_CTILE)) &&  // nothing there already
				  (t2->flags & FLAG_GROUND) &&  // ground to walk on
				 (!(t3->flags & FLAG_BLOCKED)) && // space above not blocked
				 (!(t->flags & FLAG_BLOCKED)) ) // not blocked
			{
				goodLoc.push_back( j*mapSizeX + i );
			}
		}
	}
	int loc = goodLoc[ irand(0, static_cast<int>(goodLoc.size()) ) ];
	mapdata[loc].flags |= FLAG_CTILE;
	mapdata[loc].c_tile = CUSTOM_ROCKET;

	// reset offset
	x_offs = 0;
	y_offs = 0;
}

void Map::gen_background( int seed )
{
	srand( seed );	
	Color a, b;

	//float h, s, v;
	float amt;

	if (background) {
		release_bitmap( background );
	}

	//int ir, ig, ib;
	//ir = getr16( clr_sky );
	//ig = getg16( clr_sky );
	//ib = getb16( clr_sky );

	a = b = Color(	getr16( clr_sky ) / 255.0f, 
					getg16( clr_sky ) / 255.0f, 
					getb16( clr_sky ) / 255.0f );	

	//a = rgb2hsv( a );
	//a = hsv2rgb( a );			


	switch( irand(0, 3) ) {
		case 0:
			// shift value			
			amt = frand( 0.05, 0.5 );
			b = a = rgb2hsv( a );
			
			a.b = clamp(a.b+amt); 
			a = hsv2rgb( a );			
			
			b.b = clamp(b.b-amt); 
			b = hsv2rgb( b );
			break;
		case 1:
			// shift hue
			amt = frand( 0.05, 0.1 );
			b = a = rgb2hsv( a );
			a.r /= 360.0f; b.r /= 360.0f;
			a.r = clamp(a.r+amt)*360.0f; a = hsv2rgb( a );	
			b.r = clamp(b.r-amt)*360.0f; b = hsv2rgb( b );			
			break;
		case 2:
			// shift both	
			amt = frand( 0.05, 0.1 );
			b = a = rgb2hsv( a );
			a.r /= 360.0f; b.r /= 360.0f;
			a.r = clamp(a.r+amt)*360.0f; 
			a.b = clamp(a.b+amt);
			a = hsv2rgb( a );
			
			b.r = clamp(b.r-amt)*360.0f; 
			b.b = clamp(b.b+amt);
			b = hsv2rgb( b );			
			break;
	}

	background = create_bitmap( BG_SIZE, BG_SIZE );

#if 0
	//DBG
	rectfill( background, 0, 0, BG_SIZE, BG_SIZE, 
		makecol16( a.r*255, a.g*255, a.b*255 ) );

	rectfill( background, 100, 100, BG_SIZE-100, BG_SIZE-100, 
		makecol16( b.r*255, b.g*255, b.b*255 ) );

	return;
#endif

	bool dither = irand(0,2)==0?true:false;
	float rr,gg,bb, t2, nz;
	nz = 1.0f/255.0f * frand( 1.0f, 10.0f );
	for (int j=0; j < BG_SIZE; j++) {
		float t = (float)j / (float)BG_SIZE;
		if (!dither) {				
			rr = lerp( a.r, b.r, t );
			gg = lerp( a.g, b.g, t );
			bb = lerp( a.b, b.b, t );
			hline( background, 0, j, BG_SIZE, makecol16( rr*255, gg*255, bb*255 ) );
		} else {
			for (int i=0; i < BG_SIZE; i++) {
				t2 = clamp( t + frand( -nz, nz ) );
				rr = lerp( a.r, b.r, t2 );
				gg = lerp( a.g, b.g, t2 );
				bb = lerp( a.b, b.b, t2 );
				putpixel( background, i, j, makecol16( rr*255, gg*255, bb*255 ) );
			}			
		}
	}

	// TODO: Clouds, horizon, other detail stuff...

	// Horizon
	enum {
		HORIZ_NONE,
		HORIZ_RAGGED_MTNS,
		
		HORIZ_COUNT
	};
	int horiz = irand( 0, HORIZ_COUNT );
	float step = frand( 0.1, 3.0 );
	if (horiz==HORIZ_RAGGED_MTNS) {

		float yv = frand(BG_SIZE/2, BG_SIZE);
		for (int i=0; i < BG_SIZE; i++) {
			if (yv > BG_SIZE) yv = BG_SIZE;
			if (yv < 0) yv = 0;
			vline( background, i, (int)yv, BG_SIZE, makecol16( b.r * 255, b.g*255, b.b*255 ) );

			switch( irand(0,3) ) {
				case 0:
					yv += step;
					break;
				case 1:
					yv -= step;
					break;
				default:
					break;
			}
		}
	}
}

void Map::gen_tile( BITMAP *targ, int type, int nbr, int color, int pat )
{
	// fill with clear color
	rectfill( targ, 0, 0, tilebmp->w, tilebmp->h, makecol( 255,0,255 ) );

	static BITMAP *pattern = NULL;
	if (!pattern) {
		pattern = load_bitmap( "gamedata/patterns.bmp", NULL );
	}

	// DBG: just make simple tiles
	if (type==BACKGROUND) {
		// diamondy
		int pnts[8];
		if (nbr & NORTH) {
			pnts[0] = 0; pnts[1] = 0;
			pnts[2] = 64; pnts[3] = 0;
			pnts[4] = 32; pnts[5] = 32;
			polygon( targ, 3, pnts, color );
		}
		if (nbr & SOUTH) {
			pnts[0] = 0; pnts[1] = 64;
			pnts[2] = 64; pnts[3] = 64;
			pnts[4] = 32; pnts[5] = 32;
			polygon( targ, 3, pnts, color );
		}
		if (nbr & WEST) {
			pnts[0] = 0; pnts[1] = 0;
			pnts[2] = 0; pnts[3] = 64;
			pnts[4] = 32; pnts[5] = 32;
			polygon( targ, 3, pnts, color );
		}
		if (nbr & EAST) {
			pnts[0] = 64; pnts[1] = 0;
			pnts[2] = 64; pnts[3] = 64;
			pnts[4] = 32; pnts[5] = 32;
			polygon( targ, 3, pnts, color );
		}
		if (nbr==0) {
			// no-neighbors
			pnts[0] = 32; pnts[1] = 0;
			pnts[2] = 64; pnts[3] = 32;
			pnts[4] = 32; pnts[5] = 64;
			pnts[6] = 0; pnts[7] = 32;
			polygon( targ, 4, pnts, color );
		}
	} else {
		int edgecolor;

		if (type==STRUCTURE) {
			int r, g, b;
			r = getr16(color);
			g = getg16(color);
			b = getb16(color);
			edgecolor = makecol16( iclamp(r-25), iclamp(g-25), iclamp(b-25) );
		} else {
			edgecolor = clr_accent;
		}

		// squarey		
		rectfill( targ, 0, 0, TILE_SIZE, TILE_SIZE, color );
		if (!(nbr & NORTH)) {
			rectfill( targ, 0, 0, TILE_SIZE, 5, edgecolor );
		}
		if (!(nbr & SOUTH)) {
			rectfill( targ, 0, TILE_SIZE-5, TILE_SIZE, TILE_SIZE, edgecolor );				
		}
		if (!(nbr & WEST)) {
			rectfill( targ, 0, 0, 5, TILE_SIZE, edgecolor );
		}
		if (!(nbr & EAST)) {
			rectfill( targ, TILE_SIZE-5, 0, TILE_SIZE, TILE_SIZE, edgecolor );
		}		
	}

	// apply pattern
	if (pat >= 0) {		
		for (int j = 0; j < 64; j++ ) {
			for (int i=0; i < 64; i++) {
				int p = getpixel( targ, i, j );
				if (p==makecol16(255,0,255)) continue;

				Color c( getr16(p)/255.0f, getg16(p)/255.0f, getb16(p)/255.0f );
				c = rgb2hsv( c );
				int p2 = getpixel( pattern, pat*TILE_SIZE + i, j );
				c.g = lerp( c.g, getr16( p2 ) / 255.0f, 0.75 );
				c = hsv2rgb( c );
				putpixel( targ, i, j, makecol16( c.r*255, c.g*255, c.b*255 ) );
			}
		}
	}

}

void Map::gen_tileset( int seed ) 
{
	if (!tilebmp) {
		tilebmp = create_bitmap( TILE_SIZE * 16 * 4, TILE_SIZE );
	}

	// Load custom tiles
	static BITMAP *customTiles = NULL;
	if (!customTiles) {
		customTiles = load_bitmap( "gamedata/customtiles.bmp", NULL );
	}
	

	// dbg: just number the tiles
#if 0		
	// fill with colorkey color
	rectfill( tilebmp, 0, 0, tilebmp->w, tilebmp->h, makecol( 255,0,255 ) );

	alfont_set_font_size( g_fontTextPDA, TILE_SIZE );
	for (int i=0; i < 16*4; i++) {
		char buff[100];
		sprintf( buff, "%d", i );
		alfont_textout_centre( tilebmp, g_fontTextPDA, buff, 
			i*TILE_SIZE + TILE_SIZE/2, 5, makecol(255,255,255) );		

		rect( tilebmp, i*TILE_SIZE, 0, (i+1)*TILE_SIZE-1, TILE_SIZE-1, makecol( 255,255,255 ) );
		rect( tilebmp, (i+1)*TILE_SIZE, 1, (i+1)*TILE_SIZE-2, TILE_SIZE-2, makecol( 255,255,255 ) );
	}
#endif

	// Copy custom tiles in
	Color c( getr16(clr_inorganic)/255.0f, getg16(clr_inorganic)/255.0f, getb16(clr_inorganic)/255.0f );
	c = rgb2hsv( c );
	for (int i=0; i < 4; i++) {
		blit( customTiles, tilebmp,
			  i*TILE_SIZE, 0,
			  i*TILE_SIZE, 0,
			  TILE_SIZE, TILE_SIZE );

		// Tint the custom tiles
		for (int jj=0; jj < TILE_SIZE; jj++) {
			for (int ii=0; ii < TILE_SIZE; ii++) {
				int p = getpixel( tilebmp, i*TILE_SIZE + ii, jj );
				Color pc( getr16(p)/255.0f, getg16(p)/255.0f, getb16(p)/255.0f );
				
				// is pixel is greyscale (saturation==0)
				pc = rgb2hsv( pc );
				if (pc.g < 0.2 ) {
					pc.r = c.r; // replace hue
					pc.g = c.g; // replace sat
					pc = hsv2rgb( pc );
					putpixel( tilebmp, i *TILE_SIZE + ii, jj, 
								makecol16( pc.r * 255, pc.g*255, pc.b * 255 ) );
				}
			}
		}
		
	}

	BITMAP *tile = create_bitmap( TILE_SIZE, TILE_SIZE );
	int color[4];
	color[0] = makecol16( 128, 128, 128 ); // custom
	color[1] = clr_sky; // background
	color[2] = clr_inorganic; // structure
	color[3] = clr_organic; // foreground
	int pat[4];
	pat[0] = irand( -1, 10 );
	pat[1] = irand( -1, 10 );
	pat[2] = irand( -1, 10 );
	pat[3] = irand( -1, 10 );
	for (int i=16; i < 16*4; i++) {
		
		gen_tile( tile, i/16, i&0xf, color[i/16], pat[i/16] );
		blit( tile, tilebmp, 0, 0, i*TILE_SIZE, 0, TILE_SIZE, TILE_SIZE );
	}
	release_bitmap( tile );
}

void Map::gen_scheme( int seed ) 
{
	srand( seed );
	Color accent, organic, inorganic, sky;

	switch( irand(0,4) ) {
		case 0:
			{
				// Monochromatic
				accent.r = frand(0.0f, 360.0f); // hue
				accent.g = frand(0.3f, 1.0f); // sat				

				inorganic = accent;				
				organic = inorganic;				
				sky = organic;
				if (frand(0.0, 1.0) > 0.5) {
					// decrease value
					accent.b = 1.0f; 
					organic.b = 0.3f; 
					inorganic.b = 0.6f; 
					sky.b = 0.0f; 
				} else {
					// increase value
					accent.b = 0.3f; 
					organic.b = 0.5f; 
					inorganic.b = 0.7f; 
					sky.b = 1.0f; 
				}

			}
			break;
		case 1:		
			{
				float amt = frand( 10.0f, 50.0f );				
				
				// Analagous
				accent.r = frand(0.0f, 360.0f); // hue
				accent.g = frand(0.3f, 1.0f); // sat
				accent.b = 1.0f; // value;
		
				organic = accent;
				organic.r += amt;
				if (organic.r > 360.0f) organic.r -= 360.0f;

				inorganic = organic;
				inorganic.r += amt;
				if (inorganic.r > 360.0f) inorganic.r -= 360.0f;

				sky = inorganic;
				sky.r += amt;
				if (sky.r > 360.0f) sky.r -= 360.0f;

				accent = hsv2rgb( accent );
				organic = hsv2rgb( organic );
				inorganic = hsv2rgb( inorganic );

				// desaturate the sky
				sky.g = frand( 0.1, 0.4 );
				sky = hsv2rgb( sky );
			}
			break;

		case 2:		
			{
				// Split complementary
				float amt = frand( 10.0f, 50.0f );				
							
				accent.r = frand(0.0f, 360.0f); // hue
				accent.g = frand(0.3f, 1.0f); // sat
				accent.b = frand(0.5f, 1.0f );
		
				organic = accent;
				organic.r += 180.0f + amt;
				if (organic.r > 360.0f) organic.r -= 360.0f;
				if (organic.r < 0.0f) organic.r += 360.0f;

				inorganic = accent;
				inorganic.r += 180.0f - amt;
				if (inorganic.r > 360.0f) inorganic.r -= 360.0f;
				if (inorganic.r < 0.0f) inorganic.r += 360.0f;

				sky = inorganic;
				sky.r += 180.0f;
				if (sky.r > 360.0f) sky.r -= 360.0f;
				sky.g = frand( 0.1, 0.4 ); // desaturate				
			}
			break;

		case 3:
			{
				// Triad (+ dark accent );			
				sky.r = frand(0.0f, 360.0f); // hue
				sky.g = frand(0.1f, 0.6f); // keep sat low
				sky.b  = frand(0.5f, 1.0f );
		
				organic = sky;
				organic.g = frand(0.1f, 0.6f); // keep sat low
				organic.r += 120.0f;
				if (organic.r > 360.0f) organic.r -= 360.0f;				

				inorganic = organic;
				inorganic.g = frand(0.1f, 0.6f); // keep sat low
				inorganic.r += 120.0f;
				if (inorganic.r > 360.0f) inorganic.r -= 360.0f;				

				accent.r = frand(0.0f, 360.0f); // hue
				accent.g = frand(0.3f, 1.0f); // sat
				accent.b  = frand(0.0f, 0.1f );				
			}

	}

	accent = hsv2rgb( accent );
	organic = hsv2rgb( organic );
	inorganic = hsv2rgb( inorganic );				
	sky = hsv2rgb( sky );

	clr_accent = makecol16( (float)accent.r * 255.0f, (float)accent.g * 255.0f, (float)accent.b * 255.0f );
	clr_organic = makecol16( (float)organic.r * 255.0f, (float)organic.g * 255.0f, (float)organic.b * 255.0f );
	clr_inorganic = makecol16( (float)inorganic.r * 255.0f, (float)inorganic.g * 255.0f, (float)inorganic.b * 255.0f );
	clr_sky = makecol16( (float)sky.r * 255.0f, (float)sky.g * 255.0f, (float)sky.b * 255.0f );
}

void tiles_draw( BITMAP *targ )
{
	char buff[100];

	if (!playMap) {
		playMap = new Map();		
	}

	alfont_set_font_size( g_fontTextPDA, 20 );	

	// draw background
	if (playMap->background) {
		int x_max = (playMap->mapSizeX * TILE_SIZE) - 800;
		int y_max = (playMap->mapSizeY * TILE_SIZE) - 600;
		float fx, fy;
		fx = (float)x_offs / (float) x_max;
		fy = (float)y_offs / (float) y_max;

		blit( playMap->background, targ, fx*(BG_SIZE-800), fy*(BG_SIZE-600), 0, 0, 800, 600 );
	} else {
		// clear/bg
		rectfill( targ, 0, 0, 800, 600, makecol16( 0,0,0 ) );
	}

	// draw map
	if (!playMap->mapdata) {
		alfont_textout_centre( targ, g_fontTextPDA, "No Map Data", 
								400, 300,  makecol16( 255, 100, 100 ) );
	} else {
	
		for (int j=0; j < playMap->mapSizeY; j++) {
			for (int i=0; i < playMap->mapSizeX; i++) {
				Tile *t = &(playMap->mapdata[ (playMap->mapSizeX*j)+i ]);

				// draw map
				if (playMap->tilebmp) {

					// bg tile
					if (t->bg_tile != 0 ) {
						masked_blit(playMap->tilebmp, targ, tilenum( BACKGROUND, t->bg_tile)*TILE_SIZE, 
									0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
									TILE_SIZE, TILE_SIZE );
					}

					// structure tile
					if (t->flags & FLAG_STILE ) {
						masked_blit(playMap->tilebmp, targ, tilenum( STRUCTURE, t->s_tile)*TILE_SIZE, 
									0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
									TILE_SIZE, TILE_SIZE );
					}

					// foreground tile
					if (t->flags & FLAG_FILLED ) {
						masked_blit(playMap->tilebmp, targ, tilenum( FOREGROUND, t->fg_tile)*TILE_SIZE, 
									0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
									TILE_SIZE, TILE_SIZE );
					}

					// custom tile
					if (t->flags & FLAG_CTILE ) {
						masked_blit(playMap->tilebmp, targ, tilenum( CUSTOM, t->c_tile)*TILE_SIZE, 
									0, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
									TILE_SIZE, TILE_SIZE );
					}
				}

				// annotate map
				if (mapOverlay) {
					if (t->flags & FLAG_FILLED) {
						rect( targ, 
							i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
							(i+1)*TILE_SIZE-x_offs-1, (j+1)*TILE_SIZE-y_offs-1, makecol(255,0,0));
					} else {
						rect( targ, i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
							      (i+1)*TILE_SIZE-x_offs-1, (j+1)*TILE_SIZE-y_offs-1, makecol(255,255,255));
					}

					if (t->flags & FLAG_BLOCKED) {
						line( targ, 						
							i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
							(i+1)*TILE_SIZE-x_offs-1, (j+1)*TILE_SIZE-y_offs-1, makecol(255,0,0));

						line( targ, 						
							(i+1)*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
							i*TILE_SIZE-x_offs-1, (j+1)*TILE_SIZE-y_offs-1, makecol(255,0,0));
					}

					if (t->flags & FLAG_GROUND) {
						rectfill( targ, 
							i*TILE_SIZE-x_offs, j*TILE_SIZE-y_offs, 
							(i+1)*TILE_SIZE-x_offs, (j*TILE_SIZE-y_offs) + 5, makecol(255,255,0) );
					}
				}

#if 0
				sprintf( buff, "%c%c%c", 
					(t->flags&FLAG_BGTILE)?'B':'-',
					(t->flags&FLAG_STILE)?'S':'-',
					(t->flags&FLAG_FILLED)?'F':'-'
					);

				alfont_textout( targ, g_fontTextPDA, buff, 
					(i*TILE_SIZE-x_offs)+10, (j*TILE_SIZE-y_offs)+ 10, makecol(255,255,255) );

				int f = t->fg_tile;
				sprintf( buff, "%c%c%c%c", 
					(f&NORTH)?'N':'-',
					(f&SOUTH)?'S':'-',
					(f&EAST)?'E':'-',
					(f&WEST)?'W':'-'
					);

				alfont_textout( targ, g_fontTextPDA, buff, 
					(i*TILE_SIZE-x_offs)+10, (j*TILE_SIZE-y_offs)+ 25, makecol(255,255,255) );
#endif
			}
		}
		
		sprintf( buff, "Map Size %d x %d", playMap->mapSizeX, playMap->mapSizeY );
		alfont_textout( targ, g_fontTextPDA, buff, 
							600, 10,  makecol16( 255, 255, 125500 ) );

		if( playMap->minimap) {
			blit( playMap->minimap, targ, 0, 0, 750-playMap->mapSizeX, 30, playMap->mapSizeX, playMap->mapSizeY );
		}
	}


	// menu bar	
	alfont_textout( targ, g_fontTextPDA, "F1-Map   F2-Background    F3-Colorscheme   F4-Tileset", 
								10, 10,  makecol16( 255, 255, 255 ) );

	// color scheme
	circlefill( targ, 30, 570, 10, playMap->clr_sky );
	circlefill( targ, 55, 570, 10, playMap->clr_organic );
	circlefill( targ, 80, 570, 10, playMap->clr_inorganic );
	circlefill( targ, 105, 570, 10, playMap->clr_accent );

	// Tileset
	if (playMap->tilebmp) {
		int yval = 500;
		int setsize = TILE_SIZE*16;
		alfont_textout( targ, g_fontTextPDA, "Custom", 420, yval, makecol16( 255,255,255 ) );
		stretch_blit( playMap->tilebmp, targ, setsize*0, 0, TILE_SIZE*16, TILE_SIZE, 500, yval, TILE_SIZE*4, TILE_SIZE/4 );

		yval += 20;
		alfont_textout( targ, g_fontTextPDA, "Background", 420, yval, makecol16( 255,255,255 ) );
		stretch_blit( playMap->tilebmp, targ, setsize*1, 0, TILE_SIZE*16, TILE_SIZE, 500, yval, TILE_SIZE*4, TILE_SIZE/4 );

		yval += 20;
		alfont_textout( targ, g_fontTextPDA, "Structure", 420, yval, makecol16( 255,255,255 ) );
		stretch_blit( playMap->tilebmp, targ, setsize*2, 0, TILE_SIZE*16, TILE_SIZE, 500, yval, TILE_SIZE*4, TILE_SIZE/4 );

		yval += 20;
		alfont_textout( targ, g_fontTextPDA, "Foreground", 420, yval, makecol16( 255,255,255 ) );
		stretch_blit( playMap->tilebmp, targ, setsize*3, 0, TILE_SIZE*16, TILE_SIZE, 500, yval, TILE_SIZE*4, TILE_SIZE/4 );

		// tile cursor
		rect( targ, 500+(currTile%16)*(TILE_SIZE/4), 500+(currTile/16)*20,
			        500+(currTile%16)*(TILE_SIZE/4) + (TILE_SIZE/4), 500+(currTile/16)*20  + (TILE_SIZE/4),
					makecol( 255, 0, 0 ) );
		
		// normal size tile
		blit( playMap->tilebmp, targ, currTile*TILE_SIZE, 0, 500, 430, TILE_SIZE, TILE_SIZE );
		
		// stretched tile
		stretch_blit( playMap->tilebmp, targ, currTile*TILE_SIZE, 0, TILE_SIZE, TILE_SIZE,
										570, 300, TILE_SIZE*3, TILE_SIZE*3 );			
	}

}

void tiles_keypress( int k )
{
	switch (k >> 8) {
		case KEY_TAB:
			mapOverlay = !mapOverlay;
			break;

		case KEY_F1:
			playMap->gen_map( rand() );
			break;		

		case KEY_F2:
			playMap->gen_background( rand() );
			break;		

		case KEY_F3:
			playMap->gen_scheme( rand() );
			break;		

		case KEY_F4:
			playMap->gen_tileset( rand() );
			break;

		case KEY_UP:
			currTile -= 16;		
			if (currTile < 0) currTile += (16*4);
			break;	
	
		case KEY_DOWN:
			currTile += 16;		
			if (currTile >= 16*4) currTile -= (16*4);
			break;

		case KEY_LEFT:
			currTile -= 1;
			if (currTile < 0) currTile = (16*4)-1;
			break;
	
		case KEY_RIGHT:
			currTile += 1;
			if (currTile >= (16*4)) currTile = 0;
			break;	
	}
}

void tiles_update() 
{
	if (!playMap) return;
	int step = 5;
	//if (key[KEY_SHIFT]) step = 20;

	if (key[KEY_W]) y_offs -= step;		
	if (key[KEY_S]) y_offs += step;	
	if (key[KEY_A]) x_offs -= step;
	if (key[KEY_D]) x_offs += step;
	

	if (y_offs < 0) y_offs = 0;
	if (x_offs < 0) x_offs = 0;
	if (x_offs > (playMap->mapSizeX * TILE_SIZE) - 800) {
		x_offs = (playMap->mapSizeX * TILE_SIZE) - 800;
	}
	if (y_offs > (playMap->mapSizeY * TILE_SIZE) - 600) {
		y_offs = (playMap->mapSizeY * TILE_SIZE) - 600;
	}
}