#ifndef ISLAND_GAME_H
#define ISLAND_GAME_H

#include <string>
#include <map>
#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <fmod.hpp>

#include <prmath/prmath.hpp>


#define MAX_MAP_SIZE (100)

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
	const char *m_portal; 
};

struct MapVert
{
	enum
	{
		ATTRIB_VERTEX=0,
		ATTRIB_TEXCOORD,
		ATTRIB_NORMAL
	};

	vec4f pos;
	vec2f st;
	vec3f norm;
};


enum
{
	DIR_NORTH,
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST
};

// behaviors
enum 
{
	BEHAVIOR_STATIC,
	BEHAVIOR_RANDOM,
	BEHAVIOR_SEEK_PLAYER,	
};

struct Critter
{	
	std::string m_displayName;
	std::string m_name;
	GLuint m_critterTex;

	int m_x, m_y;	
	int m_level;
	int m_hp;
	int m_dir;	
	int m_behavior;
	bool m_friendly;
};

struct Npc
{
	std::string m_displayName;
	std::string m_name;
	std::string m_speech;
	GLuint m_personTex;
	int m_x, m_y;
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

	float getRotForDir( int dir );
	float getRotLookAt( int x, int y );

	// Player
	int m_px, m_py;
	int m_pdir;
	vec3f m_camPos, m_camTarg;
	GLuint m_personMesh;
	GLuint m_playerTex;

	// Critters
	GLuint m_critterMesh;
	//GLuint m_critterPigTex;	
	std::map<std::string, Critter*> m_masterCritter;
	std::vector<Critter*> m_critters;
	void updateCritters();

	// NPCs
	std::vector<Npc*> m_npcs;

	// Map
	int m_mapSizeX;
	int m_mapSizeY;
	MapSquare m_map[MAX_MAP_SIZE][MAX_MAP_SIZE];
	std::string m_mapName;
	std::string m_mapText;

	// HUD text
	void showHudText( const std::string &title,
					  const std::string &text );
	
	bool m_showHudText;
	std::string m_hudTitle;
	std::string m_hudText;

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
	std::map<std::string, GLuint> m_textureCache;
	GLuint loadTexture( const char *filename, int upRes=1 );
	void initGraphics();
	GLuint m_playerSprite;

	GLuint m_terrainTilesTexId;
	GLuint m_waterTexId;

	ILuint m_ilFontId;
	GLuint m_glFontTexId, m_fntFontId;

	// SFX
	FMOD::System *m_fmod;
	FMOD::Sound *sfx_wall;
	FMOD::Sound *sfx_blahblah;
	
	FMOD::Sound *m_music;
	
};

void errorMessage( const char *msg, ... );

#endif