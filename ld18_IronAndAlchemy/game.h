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
	Entity *makeEnemy( EnemyType type, float x, float y );

	// world stuff
	bool onGround( float x, float y );	
	bool collideWorld( Sprite *spr );
	bool _collideWorldPnt( int x, int y );

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

	Entity *m_player;
	Player *m_playerCtl;

	// DBG
	std::vector<DbgPoint> dbgPoints;

	std::list<Entity*> m_entities;
};


#endif