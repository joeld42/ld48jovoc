#include <assert.h>


#include "ScriptConsole.h"
#include "map.h"

Map *Map::g_activeMap = NULL;

Map::Map( int height, char *tileset )
{
	m_maphite = height;
	assert( m_maphite <= MAP_MAXHITE );

	for (int j=0; j < MAP_MAXHITE; j++) {
		for (int i=0; i < MAP_WIDTH; i++) {
			m_mapdata[i][j] = 0;
			if ((i==0)||(j==0)) {
				m_mapdata[i][j] = 1;
			}
		}
	}

	load_tileset( tileset );

	m_scroll = 0;
}

Map::~Map()
{
	for (std::vector<BITMAP*>::iterator ti = m_tileset.begin();
	ti != m_tileset.end(); ti++ ) {
		
		destroy_bitmap( *ti );
	}
}

void Map::load_tileset( char *tilesetName ) 
{
	char fn[200];
	sprintf( fn, "./Graphics/tileset_%s.bmp", tilesetName );

	BITMAP *tileset;
	int w, h;

	tileset = load_bitmap( fn, NULL );

	w = tileset->w / 32;
	h = tileset->h / 32;
	
	for (int j =0; j < h; j++) {
		for (int i=0; i < w; i++) {
			BITMAP *tile = create_bitmap( 32, 32 );
			blit( tileset,tile, i*32, j*32, 0,0, 32, 32 );
			m_tileset.push_back( tile );
		}
	}

	conoutf( "Loaded [%d x %d] %d tiles...\n", w, h, w*h );

	destroy_bitmap( tileset );

}


void Map::draw( BITMAP *targ )
{
	int yval;
		
		
	for (int j=0; j < m_maphite; j++) {
			
		yval = (j*32)- (((m_maphite-15)*32)-m_scroll);
		if ((yval >= -32) && (yval <= 480) ) {
			
			for (int i=0; i < MAP_WIDTH; i++) {
				blit( m_tileset[m_mapdata[i][j]], targ, 
					  0,0, i*32, yval, 32, 32 );
			}
		}
	}
}


int Map::getScroll() 
{ 
	return g_activeMap?g_activeMap->m_scroll:0; 
}

int Map::mapYtoScreenY( int map_y ) 
{ 
	if (g_activeMap) {
		int mapYsz = ((g_activeMap->m_maphite-15)*32);
		int mapScroll = g_activeMap->m_scroll;
		return 480-(map_y - mapScroll) ;
	} else return (480-map_y);	
}