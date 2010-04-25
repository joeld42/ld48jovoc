#ifndef ISLAND_GAME_H
#define ISLAND_GAME_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <prmath/prmath.hpp>

#define MAX_MAP_SIZE (50)

#define foreach_map_max \
	for (int mi = 0; mi < MAX_MAP_SIZE; ++mi ) \
		for (int mj = 0; mj < MAX_MAP_SIZE; ++mj )

#define foreach_map \
	for (int mi = 0; mi < m_mapSizeX; ++mi ) \
		for (int mj = 0; mj < m_mapSizeY; ++mj )

#define MAPITER( _map ) \
	_map[mi][mj]

// "Special" terrains
enum 
{
	TERRAIN_EMPTY = -100,
};

struct MapSquare
{
	int m_elevation; // 0 = water
	int m_terrain; // -1 = nothing
};

struct MapVert
{
	enum
	{
		ATTRIB_VERTEX=0,
		ATTRIB_TEXCOORD
	};

	vec4f pos;
	vec2f st;
};

class IslandGame
{
public:
	IslandGame();

	void updateSim( float dtFixed );

	void update( float dt );
	void redraw( );

	void loadLevel( const char *filename );
	void clearLevel();

	void move( int dx, int dy );
	void pass();

	// Player
	int m_px, m_py;

	// Map
	int m_mapSizeX;
	int m_mapSizeY;
	MapSquare m_map[MAX_MAP_SIZE][MAX_MAP_SIZE];

	// Map builder
	void buildMap();
	void buildSideQuad( int ii, int jj, int mi, int mj );
	
	// Adds four mapverts to the map
	MapVert *addQuad();

	MapVert *m_islandDrawBuf;
	int m_quadSize;
	int m_quadCapacity;

	GLuint m_islandVBO;

	// Graphics
	GLuint loadTexture( const char *filename, int upRes=1 );
	void initGraphics();
	GLuint m_playerSprite;

	GLuint m_terrainTilesTexId;
	GLuint m_waterTexId;

	ILuint m_ilFontId;
	GLuint m_glFontTexId, m_fntFontId;
	
};

void errorMessage( const char *msg, ... );

#endif