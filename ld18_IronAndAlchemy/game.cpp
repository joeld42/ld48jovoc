#include "game.h"
#include "player.h"
#include "physics.h"

#include <luddite/resource.h>
#include <luddite/texture.h>
#include <luddite/random.h>


//#include <luddite/foreach.h>
//#define foreach BOOST_FOREACH
#define foreach(type, it, L) \
	for (type::iterator (it) = (L).begin(); (it) != (L).end(); ++(it))

using namespace Luddite;

void IronAndAlchemyGame::initResources()
{
	// Init the texture db
	TextureDB *db = new TextureDB();
	TextureDB &texDB = TextureDB::singleton();

	// Init the fonts
	HTexture hFontTexture = texDB.getTexture("gamedata/digistrip.png") ;

    GLuint texId = texDB.getTextureId( hFontTexture );
    m_font20 = ::makeFont_Digistrip_20( texId );
    m_font32 = ::makeFont_Digistrip_32( texId );

	// Init the sprite textures & sprites	
	m_sbPlayer = makeSpriteBuff( "gamedata/player_bad.png") ;
	
	Sprite *spr = m_sbPlayer->makeSprite( 0.0, 0.0, 0.25, 0.5 );
    spr->sx = 16; spr->sy = 16;
	spr->x = 120;
	spr->y = 80;
	spr->update();

	// Create the player
	m_player = new Entity( spr );
	m_player->addBehavior( new Physics( m_player ) );
	m_playerCtl = new Player( m_player );
	m_player->addBehavior( m_playerCtl );

	m_entities.push_back( m_player );

	// Init enemies
	m_sbEnemies = makeSpriteBuff( "gamedata/enemies_bad.png");
}

void IronAndAlchemyGame::freeResources()
{
	// meh... 
	//m_texDB->singleton().freeTexture( hFontTexture );    

    delete m_font20;
    delete m_font32;  
}

SpriteBuff *IronAndAlchemyGame::makeSpriteBuff( const char *filename )
{
	SpriteBuff *sbNew;

	TextureDB &texDB = TextureDB::singleton();
	HTexture hSpriteTex = texDB.getTexture( filename );
	GLuint texId = texDB.getTextureId( hSpriteTex );

	sbNew = new SpriteBuff( texId );
	m_spriteBuffs.push_back( sbNew );

	return sbNew;
}

void IronAndAlchemyGame::updateSim( float dtFixed )
{
	// update entities
	for (std::list<Entity*>::iterator ei = m_entities.begin();
		ei != m_entities.end(); ++ei )
	{
		(*ei)->updateSim( this, dtFixed );		
	}
}

void IronAndAlchemyGame::updateFree( float dt )
{
}

bool IronAndAlchemyGame::onGround( float x, float y )
{
	// TODO: do real
	if (y < 9) return true;
	return false;
}

bool IronAndAlchemyGame::collideWorld( Sprite *spr )
{
	// TODO: actual world
	if ((spr->x < 8) || (spr->x > 232)) return true;
	return false;

}

Entity *IronAndAlchemyGame::makeEnemy( EnemyType type, float x, float y )
{
	// Make a sprite
	Sprite *spr = m_sbEnemies->makeSprite( 0.0, 0.0, 1.0, 1.0 ); // fixme
    spr->sx = 16; spr->sy = 16;
	spr->x = x; spr->y = y;
	spr->update();

	// Create the enemy entity
	Entity *ent = new Entity( spr );
	ent->addBehavior( new Physics( ent ) );
	Enemy *beh_enemy = new Enemy( ent );
	ent->addBehavior( beh_enemy );

	// add to our entities list
	m_entities.push_back( ent );

	return ent;
}

void IronAndAlchemyGame::render()
{
    glClearColor( 0.592f, 0.509f, 0.274f, 1.0f );    
    glClear( GL_COLOR_BUFFER_BIT );

    // set up camera
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();    
    glOrtho( 0, 240, 0, 160, -1.0, 1.0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();    

    // do text
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

    m_font32->setColor( 1.0f, 1.0f, 1.0f, 1.0f );    
    m_font32->drawString( 10, 100, "HELLO" );    

	// draw the sprites
	foreach( std::list<SpriteBuff*>, sbi, m_spriteBuffs )
	{
		(*sbi)->renderAll();
	}

    // actually draw the text
    m_font20->renderAll();
    m_font32->renderAll();

    m_font32->clear();
    m_font20->clear();    
}


void IronAndAlchemyGame::updateButtons( unsigned int btnMask )
{
	// Movment buttons
	if ((btnMask & BTN_LEFT) && (!(btnMask & BTN_RIGHT)) )
	{
		m_playerCtl->ix = -1;
	}
	else if ((btnMask & BTN_RIGHT) && (!(btnMask & BTN_LEFT)) )
	{
		m_playerCtl->ix = 1;
	}
	else
	{
		m_playerCtl->ix = 0;
	}

	// other buttons
	m_playerCtl->m_jumpPressed = ((bool)(btnMask & BTN_JUMP));
	m_playerCtl->m_firePressed = ((bool)(btnMask & BTN_FIRE));
}
	
// key "events"
void IronAndAlchemyGame::buttonPressed( unsigned int btn )
{
	switch (btn)
	{
		case BTN_JUMP:
			m_playerCtl->jump( this );
			break;

		case BTN_FIRE:
			// todo
			// TMP for testing
			{
				Entity *baddy = makeEnemy( Enemy_REDBUG, randUniform( 10, 100 ), randUniform( 30, 50 ) );
			}
			break;
	}
}