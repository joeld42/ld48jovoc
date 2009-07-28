#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <allegro.h>
#include <vector>

#include <game_obj.h>

#define MAX_SIZE 300


#ifndef M_PI
#define M_PI (3.14159265)
#endif


// TILE CODES
enum {    
	Tile_LAND,
    Tile_OBSTACLE
};

// TILE GENERATORS
enum {
	Gen_NONE,

	Gen_BUBBLE,
	Gen_CRITTER,
	Gen_NPC
};

struct Tile
{
	Tile( const char *filename, unsigned char code );
	
	BITMAP *m_bitmap;
	BITMAP *m_mask;

	unsigned char m_code;	
};

struct MapCell
{
    MapCell();    

    unsigned char m_hite;    

    // remember our screen position
    int sx, sy;
	
	// selection
    bool m_selected;    
	unsigned char m_gen;

	// Tile for this cell
	Tile *m_tile;	
};





class TileMap
{
public:
    TileMap( int size );
	~TileMap();

    void draw( BITMAP *buf, int root_x, int root_y, 
			   const std::vector<GameObj*> &objs );

    MapCell *m_map;
    int m_size;
	int m_wave_offs;

	// for "bubble" landscapes	
	BITMAP *bub_bmp;
	float bub_age;
	bool do_water;	
    
    void reset();
	void paste( TileMap *other, int x0, int y0 );

	MapCell &map( int i, int j );

    bool screenToMap( int sx, int sy, int &mx, int &my );
    void mapToScreen( int mx, int my, int &sx, int &sy );    
    
    void init();
    void enableSelect( BITMAP *buf );	

	int encodeTilePos( int x, int y );
	void decodeTilePos( int col, int &x, int &y );
    
    static BITMAP *m_cursorTile;
	static BITMAP *m_waterMaskTile;

	static std::vector<Tile*> m_tileset;

	void clearSelected();
    
    // cheat and use a selection bitmap to make selection 
    // i am lazy
    BITMAP *m_selectMap;    
};

#endif