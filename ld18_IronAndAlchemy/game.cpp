#include "game.h"
#include "player.h"
#include "physics.h"

#include <algorithm>

#include <luddite/resource.h>
#include <luddite/texture.h>
#include <luddite/random.h>
#include <luddite/avar.h>

#include <luddite/tweakval.h>
#include <tinyxml.h>

#include <fmod.h>
#include <fmod_errors.h>

USE_AVAR(float);

//#include <luddite/foreach.h>
//#define foreach BOOST_FOREACH
#define foreach(type, it, L) \
	for (type::iterator (it) = (L).begin(); (it) != (L).end(); ++(it))

using namespace Luddite;

void IronAndAlchemyGame::initResources()
{

	// Init sounds
	FMOD_RESULT res;
	res = m_fmod->createSound(  "gamedata/health_big.wav", FMOD_HARDWARE, 0, &sfx_health_big );
	res = m_fmod->createSound(  "gamedata/health_small.wav", FMOD_HARDWARE, 0, &sfx_health_small );
	res = m_fmod->createSound(  "gamedata/jump.wav", FMOD_HARDWARE, 0, &sfx_jump );
	res = m_fmod->createSound(  "gamedata/ouch.wav", FMOD_HARDWARE, 0, &sfx_ouch );
	res = m_fmod->createSound(  "gamedata/pewpew.wav", FMOD_HARDWARE, 0, &sfx_pewpew );
	res = m_fmod->createSound(  "gamedata/zap.wav", FMOD_HARDWARE, 0, &sfx_zap );

	res = m_fmod->createStream( "gamedata/intro.mp3", 
		FMOD_HARDWARE |FMOD_LOOP_NORMAL | FMOD_2D, 0, &m_musicIntro );

	res = m_fmod->createStream( "gamedata/irongame.mp3", 
		FMOD_HARDWARE |FMOD_LOOP_NORMAL | FMOD_2D, 0, &m_musicGame );

	playingIntro = false;

	// Init the texture db
	TextureDB *db = new TextureDB();
	TextureDB &texDB = TextureDB::singleton();

	// HACK Load the blank image 
	//HTexture hBlankTex = texDB.getTexture( "gamedata/blank256.png" );

	// Init the fonts
	HTexture hFontTexture = texDB.getTexture("gamedata/digistrip.png") ;

    GLuint texId = texDB.getTextureId( hFontTexture );
    m_font20 = ::makeFont_Digistrip_20( texId );
    m_font32 = ::makeFont_Digistrip_32( texId );

	// Init particles
	HTexture hParticleTex = texDB.getTexture("gamedata/particles.png") ;
    texId = texDB.getTextureId( hParticleTex );
	m_particles = new ParticleBuff( texId );

	// Init the sprite textures & sprites	
	m_sbPlayer = makeSpriteBuff( "gamedata/player.png") ;
	
	Sprite *spr = m_sbPlayer->makeSprite( 0.0, 0.0, 0.25, 0.5 );
    spr->sx = 16; spr->sy = 16;
	spr->x = 120;
	spr->y = 80;
	spr->update();

	// not dead
	m_dieTimer = -1;

	// Create the player
	m_player = new Entity( spr );
	m_player->addBehavior( new Physics( m_player ) );
	m_playerCtl = new Player( m_player );
	m_player->addBehavior( m_playerCtl );

	m_entities.push_back( m_player );

	// Init enemies
	m_sbEnemies = makeSpriteBuff( "gamedata/enemies_bad.png");

	// Start with the starting map
	m_mapCurr = NULL;
	//loadOgmoFile( "gamedata/noc_test1.oel" );
	loadOgmoFile( "gamedata/intro.oel" );

	DBG::info("initResources done." );
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
	//dbgPoints.clear();	

	// update entities	
	if (m_dieTimer == -1 )
	{
		for (std::list<Entity*>::iterator ei = m_entities.begin();
			ei != m_entities.end(); ++ei )
		{
			(*ei)->updateSim( this, dtFixed );		
		}

		// Check if the player hit a portal
		for (std::vector<Portal>::iterator pi = m_portals.begin();
				pi != m_portals.end(); ++pi )
		{
			if (m_player->m_sprite->testHit( (*pi).m_x, (*pi).m_y ))
			{
				DBG::info("Hit portal to map %s\n", (*pi).m_name.c_str() );
				char buff[200];
				sprintf( buff, "gamedata/%s.oel",  (*pi).m_name.c_str()  );
				loadOgmoFile( buff );
				break;
			}
		}
	}

	// are we dying
	if (m_dieTimer > 0)
	{
		m_dieTimer--;
	}
	
	// are we dead?
	if (m_dieTimer==0)
	{
		// breathe new life
		m_playerCtl->m_hitPoints = MAX_LIFE;

		// respawn
		loadOgmoFile( m_currLevel.c_str(), true );

		m_dieTimer = -1;
	}


	// Everything's updated... clean up ents
	for (std::list<Entity*>::iterator ei = m_entsToRemove.begin();
		ei != m_entsToRemove.end(); ++ei )
	{
		Entity *ent = (*ei);
		m_entities.remove( ent );
		DBG::info( "Going to delete entity %p\n", ent );
		delete ent;
	}
	m_entsToRemove.clear();

}

void IronAndAlchemyGame::updateFree( float dt )
{
	// update avars
	AnimFloat::updateAvars( dt );

	// maybe should be in the updatefixed 'cause they
	// do contribute to gameplay but whatever.
	m_particles->updateParts( dt );
}

// busted don't use it
bool IronAndAlchemyGame::onGround( float x, float y )
{
#if 0
	// TODO: do for real
	int ix, iy;
	ix = (int)x; 
	iy = (int)y+1; // 1px below

	if (m_mapCurr->solid( ix/8, iy/8 ))
	{
		return true;
	}
#endif

	if (y < 9) return true;
	return false;
}

void IronAndAlchemyGame::removeEntity( Entity *ent )
{
	// Check if it's already scheduled for removal
	std::list<Entity*>::iterator ei= std::find( m_entsToRemove.begin(), 
									m_entsToRemove.end(), ent );

	if (ei == m_entsToRemove.end())
	{
		// not there yet, add it
		m_entsToRemove.push_back( ent );
	}
}

bool IronAndAlchemyGame::collideWorld( Sprite *spr )
{
	// TODO: use actual world
	//if ((spr->x < 8) || (spr->x > 232)) return true;
	//if (spr->y < 16) return true;
	//return false;

	int ix, iy; 
	ix = (int)(spr->x); iy = (int)(spr->y );

	if (_collideWorldPnt( ix, iy - (spr->sy/2)) )
	{
		return true;
	}
	else return false;

#if 0
	// cheat a bit, just check the corner pixels of the sprite
	int ix, iy; 
	ix = (int)(spr->x); iy = (int)(spr->y);

	if (_collideWorldPnt( ix - (spr->sx/2), iy - (spr->sy/2)) ||
		_collideWorldPnt( ix + (spr->sx/2), iy - (spr->sy/2)) ||
		_collideWorldPnt( ix + (spr->sx/2), iy + (spr->sy/2) ) ||
		_collideWorldPnt( ix - (spr->sx/2), iy + (spr->sy/2) ) )
	{
		return true;
	}
	else return false;
#endif
}

bool IronAndAlchemyGame::_collideWorldPnt( int x, int y )
{
	// DBG
	// ground is always solid
	if (y < 0) return true;
	if ((x < 0) || (x>=m_mapCurr->m_width * 8)) return true;
	
	// check against map
	bool result = m_mapCurr->solid( x/8, y/8 );	
	//dbgPoints.push_back( DbgPoint(x, y, result?1.0:0.0, result?0.0:0.0, 0.0) );	

	return result;
}

Entity *IronAndAlchemyGame::makeEnemy( int type, float x, float y )
{
	// Make a sprite

	// fixme -- handle more rows
	Sprite *spr = m_sbEnemies->makeSprite( type*(16.0/256.0), 0.0, (type+1)*(16.0/256.0), (16.0/128.0) ); 
    spr->sx = 16; spr->sy = 16;
	spr->x = x; spr->y = y;
	spr->update();

	// Create the enemy entity
	Entity *ent = new Entity( spr );	

	// Add some physics
	if (type <= Enemy_SNOUTY)
	{
		ent->addBehavior( new Physics( ent ) );
	}
	
	// 'enemy' should be called 'walking' behavor
	if ( (type==Enemy_REDBUG) ||
		 (type==Enemy_GREENBUG) ||
		 (type==Enemy_BLUEBUG) )
	{		
		Enemy *beh_enemy = new Enemy( ent );
		ent->addBehavior( beh_enemy );
	}
	

	// shooting things
	if ((type==Enemy_SNOUTY) ||
		(type==Enemy_SHOOTY_N) ||
		(type==Enemy_SHOOTY_E) ||
		(type==Enemy_SHOOTY_W) )
	{
		spr->canShoot = true;
		Shoot *beh_shoot = new Shoot( ent );

		if (type == Enemy_SNOUTY)
		{
			beh_shoot->m_shootAng = 180.0;
			beh_shoot->m_spreadAng = 0.0;
			beh_shoot->m_numShoot = 1;
		}
		else
		{
			beh_shoot->m_shootAng = 90.0;
			beh_shoot->m_spreadAng = 45.0;
			beh_shoot->m_numShoot = 3;
			switch( type )
			{
			case Enemy_SHOOTY_N:
				beh_shoot->m_shootAng = 90.0;
				break;
			case Enemy_SHOOTY_S:
				beh_shoot->m_shootAng = 270.0;
				break;
			case Enemy_SHOOTY_W:
				beh_shoot->m_shootAng = 180.0;
				break;
			case Enemy_SHOOTY_E:
				beh_shoot->m_shootAng = 0.0;
				break;
			}			
		}
		
		ent->addBehavior( beh_shoot );
	}	

	// add to our entities list
	m_entities.push_back( ent );

	return ent;
}

void IronAndAlchemyGame::render()
{
    //glClearColor( 0.592f, 0.509f, 0.274f, 1.0f );    
	glClearColor( m_bgR, m_bgG, m_bgB, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    // set up camera
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();    
    glOrtho( 0, 240, 0, 160, -1.0, 1.0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();    

	// translate view
	float view_x = m_playerCtl->m_physics->x - 120;
	float view_y = m_playerCtl->m_physics->y - 60;

	if (view_x < 0) view_x = 0;
	if (view_y < 0) view_y = 0;
	float mapW  = (m_mapCurr->m_width*8) - 240;
	float mapH  = (m_mapCurr->m_height*8) - _TV(160);
	if (view_x > mapW) view_x = mapW;
	if (view_y > mapH) view_y = mapH;

	glPushMatrix();
	glTranslated( -view_x, -view_y, 0.0 );

    // do text
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

	// draw the map
	m_mapCurr->renderAll();

#if 0
	if ( onGround( m_playerCtl->m_physics->x,
				   m_playerCtl->m_physics->y ) )
	{
		m_font20->setColor( 0.0f, 1.0f, 0.0f, 1.0f );    
		m_font20->drawString( 10, 130, "Ground" );    
	}
	else
	{
		m_font20->setColor( 1.0f, 1.0f, 0.0f, 1.0f );    
		m_font20->drawString( 10, 130, "NO GRND" );    
	}
#endif

	// draw the sprites
	foreach( std::list<SpriteBuff*>, sbi, m_spriteBuffs )
	{
		// render the sprites (unless it's the player and we're dead)
		if ((m_dieTimer == -1) || ((*sbi) != m_sbPlayer))
		{
			(*sbi)->renderAll();
		}
	}

#if 0
	// draw the dbg points
	glDisable( GL_TEXTURE );
	glDisable( GL_BLEND );
	glBegin( GL_POINTS );
	//DBG::info("%d debug points\n", dbgPoints.size() );
	for (int i=0; i < dbgPoints.size(); i++)
	{
		DbgPoint &p = dbgPoints[i];
		glColor3f( p.r, p.g, p.b );
		glVertex3f( p.x, p.y, 0.01 );
	}
	glEnd();
	glColor3f( 1.0, 1.0, 1.0 );
#endif

	// draw the particles
	glBlendFunc( GL_ONE, GL_ONE );    
	m_particles->renderAll();
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    

    // actually draw the text
    m_font20->renderAll();
    m_font32->renderAll();

	glPopMatrix();

	// draw player stuff
	m_playerCtl->drawLifeMeter();

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
			m_playerCtl->shoot( this );
			break;
	}
}


void IronAndAlchemyGame::loadOgmoFile( const char *filename, bool respawn )
{
	// remember the level
	m_currLevel = filename;

	// Load the level
	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );

	// do we need to change music?
	if ( !strcmp(filename, "gamedata/intro.oel" ))
	{
		// if already playing intro music, ignore
		if (!playingIntro)
		{
			if (m_chanMusic)
			{
				m_chanMusic->stop();
			}

			m_fmod->playSound(FMOD_CHANNEL_FREE, 
								m_musicIntro, false, 
								   &m_chanMusic);
			playingIntro = true;
		}
	}
	else
	{
		// Play the game music if not already
		if (playingIntro)
		{
			if (m_chanMusic)
			{
				m_chanMusic->stop();
			}

			m_fmod->playSound(FMOD_CHANNEL_FREE, 
								m_musicGame, false, 
								   &m_chanMusic);

			playingIntro = false;
		}
	}

	// clear any particles
	m_particles->clearParticles();

	// TODO: free old map
	if (m_mapCurr)
	{
		delete m_mapCurr;
	}

	// clear entities
	for (std::list<Entity*>::iterator ei = m_entities.begin(); 
		 ei != m_entities.end(); ++ei )
	{
		// delete everything but the player
		if ((*ei) != m_player)
		{
			delete (*ei);
		}
	}

	// reset list to just the player
	m_entities.clear();	
	m_entsToRemove.clear();

	DBG::warn( "after delete, %d entities\n", m_entities.size() );

	// put the player back
	m_entities.push_back( m_player );

	// clear portal list
	m_portals.clear();

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

	// get bg color
	int r, g, b;
	const char *bgcolor = xLevel->Attribute( "bgcolor" );		
	sscanf( bgcolor, "#%02X%02X%02X", &r, &g, &b );
	m_bgR = (float)r / 255.0;
	m_bgG = (float)g / 255.0;
	m_bgB = (float)b / 255.0;

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
		y = mapSizeY - (y+1);	// flip y vs. ogmo

		// Set tile
		map->setTileId( x, y, id );

		xTile = xTile->NextSiblingElement( "tile" );
	};

	// build map polys
	map->build();

	// set this to be current map
	m_mapCurr = map;

	// Load actors
	TiXmlElement *xActor;
	TiXmlElement *xActors = xLevel->FirstChildElement( "actors" );
	AssertPtr( xActors );
	
	xActor = xActors->FirstChildElement();	
	while (xActor) 
	{
		//DBG::info("Actor is: %s\n", xActor->Value() );
		
		// get enemy type from name		
		const char *actorStr = xActor->Value();

		// get location
		int x = atoi( xActor->Attribute( "x" ) );
		int y = atoi( xActor->Attribute( "y" ) );
		y = ((mapSizeY*8) - (y+1)) - 23;	// flip y vs. ogmo

		// What kind of object is it?
		if (!strcmp( actorStr, "pstart" ))
		{
			// Player Start
			if (!respawn)
			{
				m_respawnX = x;
				m_respawnY = y + 9;
			}

			m_playerCtl->m_physics->x = m_respawnX;
			m_playerCtl->m_physics->y = m_respawnY;			
		}
		else if (!strcmp( actorStr, "portal" ))
		{
			y += (23-8);
			x += 8;
			DBG::info( "load portal: %d %d\n", x, y );
			m_portals.push_back( Portal( xActor->Attribute("map"), x, y ) );
		}
		else
		{
			// Some type of enemy
			int enemyType = Enemy_REDBUG;
			if (!strcmp( actorStr, "redbug" )) enemyType = Enemy_REDBUG;
			else if (!strcmp( actorStr, "bluebug" )) enemyType = Enemy_BLUEBUG;
			else if (!strcmp( actorStr, "greenbug" )) enemyType = Enemy_GREENBUG;
			else if (!strcmp( actorStr, "snooty" )) enemyType = Enemy_SNOUTY;
			else if (!strcmp( actorStr, "shooty" )) 
			{
				const char *dir = xActor->Attribute( "dir" );
				switch( dir[0] )
				{
				case 'w':
					enemyType = Enemy_SHOOTY_W;				
					break;
				case 'e':
					enemyType = Enemy_SHOOTY_E;
					break;
				case 's':
					enemyType = Enemy_SHOOTY_S;
					break;
				case 'n':
				default:
					enemyType = Enemy_SHOOTY_N;
					break;
				}
				
			}

			// make enemy
			Entity *baddy = makeEnemy( enemyType, x, y );
		}
		

		xActor = xActor->NextSiblingElement( );
	}	
}

void IronAndAlchemyGame::playerDie()
{
	// set dead soon
	m_dieTimer = 200;

	// cool particle effects
	for (int i=0; i < 150; i++)
	{
		m_particles->emitRadial( Particle_DOT, 
			m_player->m_sprite->x,
			m_player->m_sprite->y,
			8.0 );
	}
}