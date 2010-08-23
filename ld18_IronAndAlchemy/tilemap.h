#ifndef TILEMAP_H
#define TILEMAP_H

#include <luddite/quadbuff.h>

struct TilemapVert
{
	GLfloat pos[2];
	GLfloat st[2];	
};

struct MapCell
{
	MapCell() :
		m_solid( false ),
		m_tileId( -1 )
	{		
	}

	bool m_solid;
	int m_tileId;
};

class Tilemap : public QuadBuff<TilemapVert>
{
public:
	Tilemap( GLuint texId, int width, int height );
	virtual ~Tilemap();

	// edit the map
	void setSolid( int x, int y, bool solid );
	void setTileId( int x, int y, int tileId );

	bool solid( int x, int y );
	int tileId( int x, int y );
	
	void build();
	void _buildTile( int i, int j, int id );

	// Draw all the text in this buff
	void renderAll();
	MapCell *m_map;

	// size of the map in tiles
	int m_width;
	int m_height;	

	// gl stuff
	GLuint m_texId;
};

#endif