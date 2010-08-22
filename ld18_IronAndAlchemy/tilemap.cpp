#include <luddite/debug.h>
#include <luddite/resource.h>
#include <luddite/texture.h>

#include <tinyxml.h>

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

Tilemap *loadOgmoFile( const char *filename )
{
	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );

	if (!xmlDoc->LoadFile() ) 
	{
		DBG::error("ERR! Can't load %s\n", filename );
	}

	TiXmlElement *xLevel;
	TiXmlNode *xText;

	xLevel = xmlDoc->FirstChildElement( "level" );
	assert( xLevel );
	const char *tileset = xLevel->Attribute( "tileset" );	
	char tilesetFilename[512];
	sprintf( tilesetFilename, "gamedata/tiles_%s.png", tileset );
	DBG::info( "Using tileset: %s\n", tilesetFilename );

	// Load the texture	
	TextureDB &texDB = TextureDB::singleton();
	HTexture hTilesTex = texDB.getTexture( tilesetFilename );
	GLuint texId = texDB.getTextureId( hTilesTex );

	//m_mapText = xLevel->Attribute( "intro" );

	// Get width and height
	int mapSizeX, mapSizeY;
	TiXmlElement *xTag;
	xTag = xLevel->FirstChildElement( "width" );
	xText = xTag->FirstChild();
	mapSizeX = atoi( xText->Value() ) / 8;

	xTag = xLevel->FirstChildElement( "height" );
	xText = xTag->FirstChild();
	mapSizeY = atoi( xText->Value() ) / 8;

	// Make the tilemap
	DBG::info( "map size is %d x %d\n", mapSizeX, mapSizeY );
	Tilemap *map = new Tilemap( texId, mapSizeX, mapSizeY );

	// Fill in floors	
	TiXmlElement *xTileRect;
	TiXmlElement *xFloorTiles = xLevel->FirstChildElement( "floors" );
	
	xTileRect = xFloorTiles->FirstChildElement( "rect" );	
	while (xTileRect) 
	{
		int xpos, ypos, w, h;		
		xpos = atoi( xTileRect->Attribute( "x" ) ) / 8;
		ypos = atoi( xTileRect->Attribute( "y" ) ) / 8;

		// y is flipped vs. ogmo coords
		ypos = mapSizeY - (ypos+1);	

		w = atoi( xTileRect->Attribute( "w" ) ) / 8;
		h = atoi( xTileRect->Attribute( "h" ) ) / 8;
		for (int j=0; j < h; ++j)
		{
			for (int i=0; i < w; ++i) 
			{				
				Assert( xpos+i < mapSizeX, "map range x" );
				Assert( ypos-j < mapSizeY, "map range y" );
				Assert( xpos+i >= 0, "map range x" );
				Assert( ypos-j >= 0, "map range y" );

				map->setSolid( xpos+i, ypos-j, true );
			}
		}

		xTileRect = xTileRect->NextSiblingElement( "rect" );
	}

	// Load map tiles
	TiXmlElement *xTile;
	TiXmlElement *xTerrainTiles = xLevel->FirstChildElement( "terrainTiles" );
	AssertPtr( xTerrainTiles );
	
	xTile = xTerrainTiles->FirstChildElement( "tile" );	
	while (xTile) 
	{
		int id = atoi( xTile->Attribute( "id" ) );
		int x = atoi( xTile->Attribute( "x" ) ) / 8;
		int y = atoi( xTile->Attribute( "y" ) ) / 8;
		
		// flip y vs. ogmo
		y = mapSizeY - (y+1);	

		// Set tile
		map->setTileId( x, y, id );

		xTile = xTile->NextSiblingElement( "tile" );
	};

	// build map polys
	map->build();

	return map;
}