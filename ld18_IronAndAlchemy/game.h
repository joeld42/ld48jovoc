#ifndef IRON_GAME_H
#define IRON_GAME_H

#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"
#include "player.h"

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

	// Font used for game
	Luddite::Font *m_font20;
	Luddite::Font *m_font32;

	Luddite::TextureDB *m_texDB;
	
	std::list<SpriteBuff*> m_spriteBuffs;
	SpriteBuff *makeSpriteBuff( const char *filename );
	SpriteBuff *m_sbPlayer;

	// tmp
	//Sprite *m_playerSprite;
	
	Entity *m_player;
	Player *m_playerCtl;

	std::list<Entity*> m_entities;
};


#endif