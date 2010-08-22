#ifndef IRON_GAME_H
#define IRON_GAME_H

#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"
#include "player.h"
#include "enemy.h"
#include "tilemap.h"

// game resources
#include <luddite/singleton.h>
#include <luddite/texture.h>
#include <luddite/resource.h>
#include <luddite/font.h>

#include <fmod.hpp>


// 'controller' presses (multiple can be pressed)
enum 
{
	BTN_LEFT  = ( 1 << 0 ),
	BTN_RIGHT = ( 1 << 1 ),
	BTN_UP    = ( 1 << 2 ),
	BTN_DOWN  = ( 1 << 3 ),

	BTN_JUMP = ( 1 << 4 ),
	BTN_FIRE = ( 1 << 5 ),
};

struct DbgPoint 
{	
	DbgPoint( int _x=0, int _y=0, float _r=1.0, float _g=0.0, float _b=1.0 ) :
		x( _x ), y(_y), r(_r), g(_g), b(_b)
	{
	}

	int x,  y;
	float r, g, b;
};

// links to another level
class Portal
{
public:
	Portal( const std::string &name, int x, int y ) :
		m_name( name ), m_x(x), m_y(y )
	{
	}
	std::string m_name;
	int m_x, m_y;
};

class IronAndAlchemyGame 
{	
public:
	// game loop stuff
	void initResources();
	void freeResources();
	
	void updateSim( float dtFixed );
	void updateFree( float dt );	

	void render();

	// gameplay stuff
	void updateButtons( unsigned int btnMask );
	
	// key "events"
	void buttonPressed( unsigned int btn );

	// Resources
	//HTexture hFontTexture;

	// Enemies stuff
	Entity *makeEnemy( int type, float x, float y );

	// world stuff
	bool onGround( float x, float y );	
	bool collideWorld( Sprite *spr );
	bool _collideWorldPnt( int x, int y );

	// player stuff
	void playerDie();
	int m_dieTimer; // -1 = alive, 0 = dead, >0 die soon

	// Font used for game
	Luddite::Font *m_font20;
	Luddite::Font *m_font32;

	Luddite::TextureDB *m_texDB;
	
	std::list<SpriteBuff*> m_spriteBuffs;
	SpriteBuff *makeSpriteBuff( const char *filename );
	SpriteBuff *m_sbPlayer;
	SpriteBuff *m_sbEnemies;

	// Tilemaps
	Tilemap *m_mapCurr;

	// Levels
	void loadOgmoFile( const char *filename, bool respawn=false );
	std::string m_currLevel;

	std::vector<Portal> m_portals;

	// Player stuff
	Entity *m_player;
	Player *m_playerCtl;

	int m_respawnX, m_respawnY;
	float m_bgR, m_bgG, m_bgB;

	// Sound	
	FMOD::System *m_fmod;
	FMOD::Sound *sfx_health_small;
	FMOD::Sound *sfx_health_big;
	FMOD::Sound *sfx_pewpew;
	FMOD::Sound *sfx_zap;
	FMOD::Sound *sfx_ouch;
	FMOD::Sound *sfx_jump;
	
	bool playingIntro;
	FMOD::Channel *m_chanMusic;	
	FMOD::Sound *m_musicIntro;
	FMOD::Sound *m_musicGame;

	// DBG
	std::vector<DbgPoint> dbgPoints;

	std::list<Entity*> m_entities;
};


#endif