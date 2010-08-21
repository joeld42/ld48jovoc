#include "game.h"
#include "player.h"

#include <luddite/resource.h>
#include <luddite/texture.h>



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
    spr->sx = 32; spr->sy = 32;
	spr->x = 240;
	spr->y = 160;
	spr->update();

	m_player = new Entity( spr );
	m_player->addBehavior( new Player( m_player ) );

	m_entities.push_back( m_player );
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

void IronAndAlchemyGame::render()
{
    glClearColor( 0.592f, 0.509f, 0.274f, 1.0f );    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // set up camera
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();    
    glOrtho( 0, 480, 0, 320, -1.0, 1.0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();    

    // do text
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

    m_font32->setColor( 1.0f, 1.0f, 1.0f, 1.0f );    
    m_font32->drawString( 10, 250, "HELLO" );    

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