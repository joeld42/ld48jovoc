#include <luddite/debug.h>
#include <luddite/resource.h>
#include <luddite/texture.h>

#include "tilemap.h"

using namespace Luddite;

Tilemap::Tilemap( GLuint texId, int w, int h ) :
	QuadBuff<TilemapVert>( 0, true, GL_STATIC_DRAW ),
	m_texId( texId ),
	m_width( w ), m_height( h )	
{
	m_map = new MapCell[ w * h ];
}

Tilemap::~Tilemap()
{
	delete [] m_map;
}

void Tilemap::setSolid( int x, int y, bool solid )
{
	m_map[ m_width * y + x ].m_solid = solid;
}

void Tilemap::setTileId( int x, int y, int tileId )
{
	m_map[ m_width * y + x ].m_tileId = tileId;
}

bool Tilemap::solid( int x, int y )
{
	return m_map[ m_width * y + x ].m_solid;
}

int Tilemap::tileId( int x, int y )
{
	return m_map[ m_width * y + x ].m_tileId;
}
	
void Tilemap::build()
{
	// clear any existing quads
	clear();

	// Loop over all the tiles
	for (int j=0; j < m_height; ++j)
	{
		for (int i=0; i < m_width; ++i)
		{
			int id = m_map[ m_width * j + i].m_tileId;
			if (id >= 0)
			{
				_buildTile( i, j, id );
			}
		}
	}
}

void Tilemap::_buildTile( int i, int j, int id )
{
	// Get next vert
	TilemapVert *newVert;
	newVert = this->addQuad();

	// TODO: un-hardcode this
	const float stSize = 8.0/256.0;
	const int tileNum = 256/8;

	float s0 = (id % tileNum) * stSize;
	float t0 = (id / tileNum) * stSize;
	float s1 = s0 + stSize;
	float t1 = t0 + stSize;
	
	// Upper triangle
	newVert[0].st[0] = s0;  newVert[0].st[1] = t0;
	newVert[0].pos[0] = i*8; newVert[0].pos[1] = (j+1)*8;
	
	newVert[1].st[0] = s0; newVert[1].st[1] = t1;
	newVert[1].pos[0] = i*8;   newVert[1].pos[1] = j*8;
	
	newVert[2].st[0] = s1;     newVert[2].st[1] = t0;
	newVert[2].pos[0] = (i+1)*8; newVert[2].pos[1] = (j+1)*8;	
	
	// Lower triangle
	newVert[3].st[0] = s1;     newVert[3].st[1] = t0;
	newVert[3].pos[0] = (i+1)*8; newVert[3].pos[1] = (j+1)*8;
	
	newVert[4].st[0] = s0; newVert[4].st[1] = t1;
	newVert[4].pos[0] = i*8;   newVert[4].pos[1] = j*8;
	
	newVert[5].st[0] = s1; newVert[5].st[1] = t1;
	newVert[5].pos[0] = (i+1)*8;   newVert[5].pos[1] = j*8;
}

// Draw all the tiles in this buff
void Tilemap::renderAll()
{
	glBindTexture( GL_TEXTURE_2D, m_texId );
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo() );
	updateBuffer();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, sizeof(TilemapVert), 0 );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );	
	glTexCoordPointer( 2, GL_FLOAT, sizeof(TilemapVert), (void*)(2*sizeof(GLfloat)) );
	
	//glEnableClientState( GL_COLOR_ARRAY );
	//glColorPointer( 4, GL_FLOAT, sizeof(TilemapVert), (void*)(4*sizeof(GLfloat)) );
	
	glDrawArrays( GL_TRIANGLES, 0, this->size() );
	
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	//glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
}
