#ifndef TILES_H
#define TILES_H

#define TILE_SIZE (64)
#define BG_SIZE (1024)

// Neighbors
enum {
	NORTH = 1 << 0,
	SOUTH = 1 << 1,
	EAST  = 1 << 2,
	WEST  = 1 << 3
};

// Tile types
enum {
	CUSTOM,
	BACKGROUND, 
	STRUCTURE,
	FOREGROUND,	 
};

// Tile flags
enum {
	FLAG_FILLED = 1 << 0,
	FLAG_STILE  = 1 << 1,
	FLAG_BGTILE = 1 << 2,
	FLAG_CTILE  = 1 << 3,
	
	// other meanings
	FLAG_GROUND  = 1 << 4,
	FLAG_BLOCKED = 1 << 5,
};

// Custom tiles
enum {
	CUSTOM_ROCKET,
	CUSTOM_STATION1,
	CUSTOM_STATION2,
	CUSTOM_TRACK
};

// encode tiles
#define tilenum( type, neighbors ) ( (type<<4) | neighbors )
#define tiletype( num ) ( num >> 4 )

struct Tile {
	unsigned char bg_tile, s_tile, fg_tile, c_tile;
	unsigned char flags;
};

struct Map {
	BITMAP *tilebmp;
	BITMAP *minimap;
	BITMAP *background;
	void draw_map( BITMAP *targ );

	int mapSizeX, mapSizeY;
	Tile *mapdata;
	unsigned char *collide;

	// The palette
	int clr_sky, 
		clr_organic, 
		clr_inorganic, 
		clr_accent;

	// constructor
	Map();

	// generators
	void gen_background( int seed );
	void gen_map( int seed );
	void gen_scheme( int seed );
	void gen_tileset( int seed );

	// helpers for tile gen
	void gen_tile( BITMAP *targ, int type, int nbr, int color, int pat );
	
	// helpers for map gen
	void add_nbr( int type );
	void calc_nbr( int type );
	Tile *getTile( int i, int j );
	int getNbr( int i, int j, int type );
	void setNbr( int i, int j, int type, int nbr );

};

void tiles_draw( BITMAP *targ );
void tiles_keypress( int k );
void tiles_update();

#endif