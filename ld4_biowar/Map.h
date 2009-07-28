#ifndef MAP_H
#define MAP_H

#include <allegro.h>
#include <vector>

// fixed screen size
#define MAP_WIDTH (20)
#define MAP_MAXHITE (200)
#define MAX_TILES (100)

class Map {
public:
	Map( int height, char *tileset );
	~Map();


	void draw( BITMAP *targ );

	int m_maphite;

	char m_mapdata[MAP_WIDTH][MAP_MAXHITE];

	int m_scroll;

protected:
	void load_tileset( char *tileset );
	
	std::vector<BITMAP*> m_tileset;

public:

	static int getScroll();
	static int mapYtoScreenY( int map_y);
	static Map *g_activeMap;
};

#endif