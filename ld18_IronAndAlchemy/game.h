#ifndef IRON_GAME_H
#define IRON_GAME_H

#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"

// game resources
#include <luddite/singleton.h>
#include <luddite/texture.h>
#include <luddite/resource.h>
#include <luddite/font.h>

class IronAndAlchemyGame 
{	
public:
	// game loop stuff
	void initResources();
	void freeResources();
	
	void updateSim( float dtFixed );
	void updateFree( float dt );	

	void render();

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

	std::list<Entity*> m_entities;
};


#endif