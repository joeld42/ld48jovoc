#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "GLee.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <tinyxml.h>

#include <prmath/prmath.hpp>

#include "gamefontgl.h"
#include "IslandGame.h"

#include "load_obj.h"
#include "tweakval.h"
#include "debug.h"


IslandGame::IslandGame() :
	m_islandDrawBuf( NULL ),
	m_px(0), m_py(0), m_showHudText( false )
{
}

void IslandGame::updateSim( float dtFixed )
{	
	m_camPos += (m_camTarg - m_camPos) * dtFixed;
}

void IslandGame::update( float dt )
{
}

void IslandGame::redraw( )
{
	//static bool gfxInit = false;
	//if (!gfxInit)
	//{
	//	initGraphics();
	//	gfxInit = true;
	//}

	glClearColor( 0.3, 1.0, 1.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// 3d stuff
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	

	int useOrtho = _TV(1);
	if (!useOrtho)
	{
		gluPerspective( _TV(2.0f), 800.0/600.0, 0.1, 1000.0 );
	}
	else
	{
		float aspect = 800.0 / 600.0;
		float hite = _TV(4.0f);
		glOrtho( -aspect * hite, aspect * hite, -hite, hite, 0.1, 1000.0 );
	}

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();	
	
	glDisable( GL_LIGHTING );

	// player
	MapSquare &mp = m_map[m_px][m_py];
	vec3f ppos( m_px + 0.5f, 
				(mp.m_elevation+1.0) * 0.3f, 
				m_py + 0.5f );
	
	//gluLookAt( ppos.x, ppos.y + 1, ppos.z + 1,
	//		   ppos.x, ppos.y, ppos.z,
	//		   0.0, 1.0, 0.0 );	
	glRotatef( _TV(29.7f), 1.0, 0.0, 0.0 );
	glTranslatef( _TV(0), _TV(-5), _TV(-6) );
	glRotatef( _TV(45.0f), 0.0, 1.0, 0.0 );

	glTranslatef( m_camPos.x * _TV(-1.0f), 
				  m_camPos.y * _TV( 0.0f), 
				  m_camPos.z  * _TV(-1.0f));
	

	//static float ang = 0;
	//glRotatef( ang, 0.0, 1.0, 0.0 );
	//ang += 1;	

	glDisable( GL_TEXTURE_2D );
#if 0	
	glLineWidth( 4.0 );
	glBegin( GL_LINES );

	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( ppos.x-1.0, ppos.y + 0.01, ppos.z );
	glVertex3f( ppos.x+1.0, ppos.y + 0.01, ppos.z );

	glColor3f( 0.0, 1.0, 0.0 );
	glVertex3f( ppos.x, ppos.y -1.0, ppos.z );
	glVertex3f( ppos.x, ppos.y +1.0, ppos.z );

	glColor3f( 0.0, 0.0, 1.0 );
	glVertex3f( ppos.x, ppos.y + 0.01, ppos.z - 1.0 );
	glVertex3f( ppos.x, ppos.y + 0.01, ppos.z + 1.0 );

	glEnd();
#endif

	glColor3f( 1.0, 1.0, 1.0 );
	glEnable( GL_TEXTURE_2D );	

	// draw some stuff the easy way
	glBindTexture( GL_TEXTURE_2D, m_waterTexId );	
	glBegin( GL_QUADS );
	
	glTexCoord2f( 0.0, 0.0 );
	glVertex3f( -2000, 0, -2000 );

	glTexCoord2f( 0.0, 1000.0 );
	glVertex3f( -2000, 0, 2000 );

	glTexCoord2f( 1000.0, 1000.0 );
	glVertex3f( 2000, 0, 2000 );

	glTexCoord2f( 1000.0, 0.0 );
	glVertex3f( 2000, 0, -2000 );

	glEnd();

	glEnable( GL_TEXTURE_2D );	
	//glColor3f( 1.0, 0.0, 1.0 );	

	// draw player
	glPushMatrix();	
	glTranslatef( ppos.x, ppos.y, ppos.z );
	glScalef( 0.4f, 0.4f, 0.4f );
	glRotatef( -90.0, 0.0, 1.0, 0.0 );
	glBindTexture( GL_TEXTURE_2D, m_playerTex );
	glCallList( m_personMesh );
	glPopMatrix();

	// draw critters
	for (std::vector<Critter*>::iterator ci = m_critters.begin(); 
		 ci != m_critters.end(); ci++)
	{
		Critter *c = (*ci);
		float elev = (m_map[c->m_x][c->m_y].m_elevation + 1.0) * 0.3;
		glPushMatrix();	
		glTranslatef( c->m_x + 0.5f, elev, c->m_y + 0.5f );
		glScalef( 0.4f, 0.4f, 0.4f );

		if (c->m_behavior == BEHAVIOR_STATIC)
		{
			glRotatef( getRotLookAt( c->m_x, c->m_y ), 0.0, 1.0, 0.0 );
		}
		else
		{
			glRotatef( getRotForDir( c->m_dir ), 0.0, 1.0, 0.0 );
		}

		glBindTexture( GL_TEXTURE_2D, c->m_critterTex );
		glCallList( m_critterMesh );
		glPopMatrix();
	}

	// draw npcs
	for (std::vector<Npc*>::iterator ci = m_npcs.begin(); 
		 ci != m_npcs.end(); ci++)
	{
		Npc *npc = (*ci);
		float elev = (m_map[npc->m_x][npc->m_y].m_elevation + 1.0) * 0.3;
		glPushMatrix();	
		glTranslatef( npc->m_x + 0.5f, elev, npc->m_y + 0.5f );
		glScalef( 0.4f, 0.4f, 0.4f );
		glRotatef( getRotLookAt( npc->m_x, npc->m_y ), 0.0, 1.0, 0.0 );
		glBindTexture( GL_TEXTURE_2D, npc->m_personTex );
		glCallList( m_personMesh );
		glPopMatrix();
	}

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );	

	glBindTexture( GL_TEXTURE_2D, m_terrainTilesTexId );

	// Bind the island VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_islandVBO);

	//glEnableVertexAttribArray( MapVert::ATTRIB_VERTEX);
	//glEnableVertexAttribArray( MapVert::ATTRIB_TEXCOORD );
	//glEnableVertexAttribArray( ATTRIB_NORMAL );
	
	//glVertexAttribPointer( MapVert::ATTRIB_VERTEX,   4, GL_FLOAT, GL_FALSE, sizeof(MapVert), 0);
	//glVertexAttribPointer( MapVert::ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(MapVert), (void*)(3*sizeof(GLfloat)) );
	//glVertexAttribPointer( ATTRIB_NORMAL,   3, GL_FLOAT, GL_FALSE, sizeof(MapVert), (void*)(6*sizeof(GLfloat)) );
	
	//glBufferData( GL_ARRAY_BUFFER, sizeof(MapVert) * m_quadSize, 0, GL_STATIC_DRAW );
	//glVertexPointer( 4, GL_FLOAT, sizeof(MapVert), NULL );
	//

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, sizeof(MapVert), 0 );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );	
	glTexCoordPointer( 2, GL_FLOAT, sizeof(MapVert), (void*)(4*sizeof(GLfloat)) );

	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer( GL_FLOAT, sizeof(MapVert), (void*)(6*sizeof(GLfloat)) );

	glDrawArrays( GL_QUADS, 0, m_quadSize*4 );	

	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

	// 2D text and GUI stuff
	glDisable( GL_DEPTH_TEST );	
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_LIGHTING );		
	

	glColor3f( 1.0, 1.0, 1.0 );
		
	if (m_showHudText)
	{
		gfEnableFont( m_fntFontId, 32 );
		gfBeginText();		

		// draw background box
		glDisable( GL_TEXTURE_2D );
		glColor4f( 0.0, 0.0, 0.0, 0.6 );
		glBegin( GL_QUADS );
		glVertex2f( 0, 0 ); glVertex2f( 0, 210 );
		glVertex2f( 800, 210 ); glVertex2f( 800, 0 );
		glEnd();
		glColor4f( 1.0, 1.0, 0.0, 1.0 );

		glEnable( GL_TEXTURE_2D );

		glTranslated( 20, 170, 0 );		
		gfDrawString( m_hudTitle.c_str() );
		gfEndText();

		glColor4f( 1.0, 1.0, 1.0, 1.0 );
		gfEnableFont( m_fntFontId, 20 );
		gfBeginText();		
		glTranslated( 20, 130, 0 );		
		gfDrawString( m_hudText.c_str() );
		gfEndText();
	}

}

void IslandGame::initGraphics( )
{
	// Load Font	
	ilGenImages( 1, &m_ilFontId );
	ilBindImage( m_ilFontId );		
	
	if (!ilLoadImage( (ILstring)"gamedata/digistrip.png" )) {
		errorMessage("Loading font image failed\n");
	}
	
	// Make a GL texture for it
	m_glFontTexId = ilutGLBindTexImage();
	m_fntFontId = gfCreateFont( m_glFontTexId );

	// A .finfo file contains the metrics for a font. These
	// are generated by the Fontpack utility.
	gfLoadFontMetrics( m_fntFontId, "gamedata/digistrip.finfo");

	_RPT1( _CRT_WARN, "font has %d chars\n", 
		gfGetFontMetric( m_fntFontId, GF_FONT_NUMCHARS ) );					

	// Load the player sprite
	m_playerTex = loadTexture( "gamedata/npc_monk.png", 4 );
	m_personMesh = load_obj( "gamedata/person.obj");

	// Load critters	
	m_critterMesh = load_obj( "gamedata/hemi_critter.obj");

	// Load the tiles
	m_terrainTilesTexId  = loadTexture( "gamedata/crappytiles.png", 4 );

	m_waterTexId  = loadTexture( "gamedata/water.png", 4 );

	// Set up simple lighting	
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_direction[] = { -0.5, 1.0, 0.5, 0.0 };
	GLfloat light_ambient[] = { 2.0, 2.0, 2.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };

	vec3f lightDir( -0.5, 1.0, 0.5 );
	lightDir.Normalize();
	light_direction[0] = lightDir.x;
	light_direction[1] = lightDir.y;
	light_direction[2] = lightDir.z;


	//glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	//glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_direction);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);	

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glEnable(GL_DEPTH_TEST);

}

void IslandGame::loadLevel( const char *filename )
{
	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );
	
	// clear the level
	clearLevel();

	if (!xmlDoc->LoadFile() ) {
		errorMessage("ERR! Can't load %s\n", filename );
	}

	TiXmlElement *xLevel;
	TiXmlNode *xText;

	xLevel = xmlDoc->FirstChildElement( "level" );
	assert( xLevel );
	m_mapName = xLevel->Attribute( "isleName" );	
	m_mapText = xLevel->Attribute( "intro" );

	showHudText( m_mapName, m_mapText );

	const char *waterTex = xLevel->Attribute( "water" );
	if (waterTex)
	{
		char buff[200];
		sprintf( buff, "gamedata/%s.png", waterTex );
		m_waterTexId = loadTexture( buff );
	}
	else
	{
		m_waterTexId  = loadTexture( "gamedata/water.png", 4 );
	}

	// Get width and height
	TiXmlElement *xTag;
	xTag = xLevel->FirstChildElement( "width" );
	xText = xTag->FirstChild();
	m_mapSizeX = atoi( xText->Value() ) / 16;

	xTag = xLevel->FirstChildElement( "height" );
	xText = xTag->FirstChild();
	m_mapSizeY = atoi( xText->Value() ) / 16;

	// Load elevations
	TiXmlElement *xTile;
	TiXmlElement *xElevTiles = xLevel->FirstChildElement( "elevTiles" );
	
	xTile = xElevTiles->FirstChildElement( "tile" );	
	while (xTile) 
	{
		int id, xpos, ypos;
		id = atoi( xTile->Attribute( "id" ) );
		xpos = atoi( xTile->Attribute( "x" ) ) / 16;
		ypos = atoi( xTile->Attribute( "y" ) ) / 16;		

		xTile = xTile->NextSiblingElement( "tile" );

		MapSquare &m = m_map[xpos][ypos];
		m.m_elevation = id;
		m.m_terrain = 0; // TODO get from terrain tileset
	}

	// Load terrain indices
	TiXmlElement *xTerrainTiles = xLevel->FirstChildElement( "terrainTiles" );
	
	xTile = xTerrainTiles->FirstChildElement( "tile" );	
	while (xTile) 
	{
		int id, xpos, ypos;
		id = atoi( xTile->Attribute( "id" ) );
		xpos = atoi( xTile->Attribute( "x" ) ) / 16;
		ypos = atoi( xTile->Attribute( "y" ) ) / 16;

		_RPT3( _CRT_WARN, "loc %d %d id %d\n", xpos, ypos, id );

		xTile = xTile->NextSiblingElement( "tile" );

		MapSquare &m = m_map[xpos][ypos];		
		m.m_terrain = id;
	}

	// Load actors	
	TiXmlElement *xActorLayer = xLevel->FirstChildElement( "actors" );
	
	// Get the player start pos
	TiXmlElement *xActor = xActorLayer->FirstChildElement( "pstart" );
	m_px = atoi( xActor->Attribute( "x" ) ) / 16;
	m_py = atoi( xActor->Attribute( "y" ) ) / 16;

	m_camTarg = vec3f( m_px + 0.5f, 
					(m_map[m_px][m_py].m_elevation+1.0) * 0.3f, 
					m_py + 0.5f );
	m_camPos = m_camTarg;

	// first, make sure all master critters are loaded
	xActor = xActorLayer->FirstChildElement( "critter" );
	while (xActor)
	{
		// If this is the master copy of the critter, load it
		if (!strcmp( xActor->Attribute("master"), "true" ) )
		{
			Critter *mcrit = NULL;
			std::string critKey = xActor->Attribute("displayName" );			
			std::transform( critKey.begin(), critKey.end(), critKey.begin(), toupper );

			// is it in the master list?
			if ( m_masterCritter.find( critKey ) == m_masterCritter.end() )
			{
				// Nope, load it
				mcrit = new Critter();
				mcrit->m_displayName = xActor->Attribute( "displayName" );
				mcrit->m_level = atoi( xActor->Attribute( "level" ) );
				mcrit->m_name = atoi( xActor->Attribute( "map" ) );
				mcrit->m_behavior = atoi( xActor->Attribute( "behave" ) );

				// DBG:
				mcrit->m_behavior = BEHAVIOR_RANDOM;

				char buff[1000];
				sprintf( buff, "gamedata/critter_%s.png", xActor->Attribute( "map" ) );
				mcrit->m_critterTex = loadTexture( buff, 4 );

				// store in master list
				m_masterCritter[ critKey ] = mcrit;
			}
		}
		xActor = xActor->NextSiblingElement( "critter" );
	}

	// Now go through all the critters again and create them
	for (std::vector<Critter*>::iterator ci = m_critters.begin();
		 ci != m_critters.end(); ++ci )
	{
		delete *ci;
	}
	//m_critters.clear( m_critters.begin(), m_critters.end() );
	m_critters.clear();

	xActor = xActorLayer->FirstChildElement( "critter" );
	while (xActor)
	{
		Critter *mcrit = NULL;
		std::string critKey = xActor->Attribute("displayName" );			
		std::transform( critKey.begin(), critKey.end(), critKey.begin(), toupper );
	
		std::map<std::string, Critter*>::iterator mcriti = m_masterCritter.find( critKey );
		if ( mcriti == m_masterCritter.end() )
		{
			DBG::warn( "Couldn't find master critter: %s\n", critKey.c_str() );
		}
		else
		{
			mcrit = (*mcriti).second;
			Critter *crit = new Critter();
			*crit = *mcrit;
			crit->m_x = atoi( xActor->Attribute( "x" ) ) / 16;
			crit->m_y = atoi( xActor->Attribute( "y" ) ) / 16;

			crit->m_dir = rand() % 4;
			m_critters.push_back( crit );
		}

		xActor = xActor->NextSiblingElement( "critter" );
	}

	// Now, load all the NPCs... npcs are simpler than critters
	for (std::vector<Npc*>::iterator ci = m_npcs.begin();
		 ci != m_npcs.end(); ++ci )
	{
		delete *ci;
	}
	m_npcs.clear();
	xActor = xActorLayer->FirstChildElement( "npc" );
	while (xActor)
	{
		Npc *npc = new Npc();
		npc->m_x = atoi( xActor->Attribute( "x" ) ) / 16;
		npc->m_y = atoi( xActor->Attribute( "y" ) ) / 16;
		npc->m_displayName = xActor->Attribute("displayName" );
		npc->m_name = xActor->Attribute( "map" );
		npc->m_speech = xActor->Attribute( "speech" );

		char buff[1000];
		sprintf( buff, "gamedata/npc_%s.png", xActor->Attribute( "map" ) );
		npc->m_personTex = loadTexture( buff, 4 );
		
		m_npcs.push_back( npc );

		xActor = xActor->NextSiblingElement( "npc" );
	}
}

void IslandGame::clearLevel()
{
	foreach_map_max
	{
		MapSquare &tile = MAPITER( m_map );
		tile.m_elevation = -1;
		tile.m_terrain = TERRAIN_EMPTY;
	}
}

void IslandGame::buildMap()
{
	if (m_islandDrawBuf)
	{
		free( m_islandDrawBuf );
		m_islandDrawBuf = 0;
		m_quadSize = 0;
		m_quadCapacity = 0;
	}
	

	//m_islandDrawBuf = (MapVert*)malloc( m_mapSizeX * m_mapSizeY * sizeof(MapVert) 

	m_quadSize = 0;
	m_quadCapacity = 0;

	DBG::info("buildMap: size %d %d\n", m_mapSizeX, m_mapSizeY );
	foreach_map
	{
		MapSquare &m = MAPITER( m_map );

		// Is this a blank or otherwise special tile?
		if (m.m_terrain < 0)
		{
			continue;
		}

		// Add a quad
		MapVert *newQuad = addQuad();
		
		// prevent seams between tiles
		const float subpixel_nudge = 1.0/1024.0;

		// and fill it in
		const float tilesz = 1.0/16.0;
		int smap = m.m_terrain % 16;
		int tmap = m.m_terrain / 16;
		float elevation = (m.m_elevation + 1.0f) * 0.3f;
		vec2f st = vec2f( (smap * tilesz), 
						  (tmap * tilesz) );
		newQuad[0].pos = vec4f( mi, elevation, mj, 1.0f );
		newQuad[0].st  = st + vec2f( subpixel_nudge, subpixel_nudge );

		newQuad[1].pos = vec4f( mi + 1.0f, elevation, mj, 1.0f );
		newQuad[1].st  = st + vec2f( tilesz, 0.0f ) + vec2f( -subpixel_nudge, subpixel_nudge );

		newQuad[2].pos = vec4f( mi + 1.0f, elevation, mj + 1.0f, 1.0f );
		newQuad[2].st  = st + vec2f( tilesz, tilesz ) + vec2f( -subpixel_nudge, -subpixel_nudge );

		newQuad[3].pos = vec4f( mi, elevation, mj + 1.0f, 1.0f );
		newQuad[3].st  = st + vec2f( 0.0f, tilesz ) + vec2f( subpixel_nudge, -subpixel_nudge );

		// normals
		newQuad[0].norm = vec3f( 0.0, 1.0, 0.0 );
		newQuad[1].norm = vec3f( 0.0, 1.0, 0.0 );
		newQuad[2].norm = vec3f( 0.0, 1.0, 0.0 );
		newQuad[3].norm = vec3f( 0.0, 1.0, 0.0 );

		// draw the side quads		
		buildSideQuad( -1, 0, mi, mj ); // LEFT SIDE
		buildSideQuad(  1, 0, mi, mj ); // RIGHT SIDE
		buildSideQuad(  0, -1, mi, mj ); // BACK SIDE
		buildSideQuad(  0, 1, mi, mj ); // FRONT SIDE
		
	}

	DBG::debug( "After buildMap, m_quadSize is %d\n", m_quadSize );

	// Make teh GL bufferses

	// TODO: free old buffers
	
	// Generate the vertex buffer object (VBO)
	glGenBuffers(1, &m_islandVBO );
	
	// Bind the VBO so we can fill it with data
	glBindBuffer(GL_ARRAY_BUFFER, m_islandVBO);
	
	// Set the buffer's data
	// Calc afVertices size (3 vertices * stride (3 GLfloats per vertex))
	unsigned int uiSize = sizeof( MapVert ) * m_quadSize * 4;
	glBufferData(GL_ARRAY_BUFFER, uiSize, m_islandDrawBuf, GL_STATIC_DRAW);	
	
	DBG::info("NumQuads %d size %d\n", m_quadSize, uiSize );
}

void IslandGame::buildSideQuad( int ii, int jj, int mi, int mj )
{
	int mi2, mj2;
	MapSquare &m = m_map[mi][mj];

	mi2 = mi+ii;
	mj2 = mj+jj;

	// See how far down we need to extend the tile
	float currElev = (m.m_elevation + 1.0) * 0.3f;
	float targElev = 0.0;
	if ((mi2 >=0) && (mi2 < m_mapSizeX) &&
		(mj2 >=0) && (mj2 < m_mapSizeY) )
	{
		targElev = (m_map[mi2][mj2].m_elevation + 1.0) * 0.3f;
	}
	
	const float tilesz = 1.0/16.0;
	int side_terrain = 48;
	while (currElev > targElev )
	{
		// TODO: figure this out dynamically			
		int smap = side_terrain % 16;
		int tmap = side_terrain / 16;
		MapVert *newQuad = addQuad();

		// normals are easy
		newQuad[0].norm = vec3f( ii, 0.0, jj );
		newQuad[1].norm = vec3f( ii, 0.0, jj );
		newQuad[2].norm = vec3f( ii, 0.0, jj );
		newQuad[3].norm = vec3f( ii, 0.0, jj );

		// tex coords
		const float subpixel_nudge = 1.0/1024.0;
		vec2f st = vec2f( smap * tilesz, tmap * tilesz );		
		newQuad[0].st  = st + vec2f( subpixel_nudge, subpixel_nudge );
		newQuad[1].st  = st + vec2f( tilesz, 0.0f ) + vec2f( -subpixel_nudge, subpixel_nudge );
		newQuad[2].st  = st + vec2f( tilesz, tilesz ) + vec2f( -subpixel_nudge, -subpixel_nudge );
		newQuad[3].st  = st + vec2f( 0.0f, tilesz ) + vec2f( subpixel_nudge, -subpixel_nudge );

		// There's probably a clever way to do this but
		// i can't do "clever" right now
		if ((ii==-1) && (jj==0))
		{
			// LEFT
			newQuad[0].pos = vec4f( mi, currElev, mj, 1.0f );
			newQuad[1].pos = vec4f( mi, currElev, mj+1.0f, 1.0f );
			newQuad[2].pos = vec4f( mi, currElev-1.0, mj+1.0f, 1.0f );
			newQuad[3].pos = vec4f( mi, currElev-1.0, mj, 1.0f );
		}
		else if ((ii==1) && (jj==0))
		{
			// RIGHT
			newQuad[0].pos = vec4f( mi+1, currElev, mj, 1.0f );
			newQuad[1].pos = vec4f( mi+1, currElev, mj+1.0f, 1.0f );
			newQuad[2].pos = vec4f( mi+1, currElev-1.0, mj+1.0f, 1.0f );
			newQuad[3].pos = vec4f( mi+1, currElev-1.0, mj, 1.0f );
		}
		else if ((ii==0) && (jj==-1))
		{
			// FRONT
			newQuad[0].pos = vec4f( mi, currElev, mj, 1.0f );
			newQuad[1].pos = vec4f( mi+1, currElev, mj, 1.0f );
			newQuad[2].pos = vec4f( mi+1, currElev-1.0, mj, 1.0f );
			newQuad[3].pos = vec4f( mi, currElev-1.0, mj, 1.0f );
		}
		else if ((ii==0) && (jj==1))
		{
			// BACK
			newQuad[0].pos = vec4f( mi+1, currElev, mj+1, 1.0f );
			newQuad[1].pos = vec4f( mi, currElev, mj+1, 1.0f );
			newQuad[2].pos = vec4f( mi, currElev-1.0, mj+1, 1.0f );
			newQuad[3].pos = vec4f( mi+1, currElev-1.0, mj+1, 1.0f );
		}

		currElev -= 1.0;
		side_terrain = 64;
	}
}

MapVert *IslandGame::addQuad()
{
	MapVert *newQuad;

	// Do we need to make more space for this quad??
	// NOTE: potentially confusing: size/capacity is in # of verts,
	// not num quads
	if (m_quadSize + 4 >= m_quadCapacity)
	{
		// increase capacity
		int m_quadTargetCapacity;
		if (m_quadCapacity==0)
		{
			// start with 8192 because it's a nice number
			m_quadTargetCapacity = 8192;
		}
		else
		{
			m_quadTargetCapacity = m_quadCapacity * 2;
		}

		DBG::debug("Grow quadbuff to new capacity %d\n", m_quadTargetCapacity );
		MapVert *newBuff = (MapVert*)calloc( m_quadTargetCapacity, 
											  sizeof( MapVert ) );

		// copy over existing if needed
		if (m_islandDrawBuf)
		{
			memcpy( newBuff, m_islandDrawBuf, sizeof(MapVert) * m_quadSize );
			free( m_islandDrawBuf );
		}
		m_islandDrawBuf = newBuff;
		m_quadCapacity = m_quadTargetCapacity;
	}

	// Ok, now we know there's space for the new quad
	newQuad = &m_islandDrawBuf[m_quadSize];
	m_quadSize += 4;

	return newQuad;
}

GLuint IslandGame::loadTexture( const char *filename, int upres )
{
	// First see if we have it cached
	std::string sfilename( filename );
	std::map<std::string, GLuint>::iterator itex;
	itex = m_textureCache.find( sfilename );

	if (itex != m_textureCache.end())
	{
		DBG::info("Texture %s found in cache, reusing...\n", filename );
		return (*itex).second;
	}

	// Load the texture image
	ILuint ilId;
	ilGenImages( 1, &ilId );
	ilBindImage( ilId );		
	
	if (!ilLoadImage( (ILstring)filename )) 	
	{		
		errorMessage( "Loading texture failed for: %s", filename );
	}

	// upres if requested
	if (upres > 1)
	{
		DBG::info("UPRES %s, orig size is %dx%d\n",
				filename,
				ilGetInteger( IL_IMAGE_WIDTH ),
				ilGetInteger( IL_IMAGE_HEIGHT) );

		iluScale( ilGetInteger( IL_IMAGE_WIDTH ) * upres,
					ilGetInteger( IL_IMAGE_HEIGHT ) * upres,
					ilGetInteger( IL_IMAGE_DEPTH ) );
	}
	
	// Make a GL texture for it
	GLuint glTexId;
	glTexId = ilutGLBindTexImage();

	ilDeleteImages(  1, &ilId );

	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );	

	// And add it to the texture cache
	DBG::info( "Adding %s to texture cache...\n", filename );
	m_textureCache[ sfilename ] = glTexId;
	
	return glTexId;
}

void IslandGame::move( int dx, int dy )
{
	// Are we trying to move off the map?
	if ((m_px + dx < 0) || (m_px + dx >= m_mapSizeX) ||
		(m_py + dy < 0) || (m_py + dy >= m_mapSizeY) )
	{
		pass();
		return;
	}

	int nx = m_px + dx;
	int ny = m_py + dy;
	
	MapSquare &mnew = m_map[nx][ny];

	// Look at what's going on with mnew
	
	// can only move +/- 1 elevation, and not on water
	if ((mnew.m_elevation < 0) || 
		(fabs( (float)(mnew.m_elevation - m_map[m_px][m_py].m_elevation)) > 1.01 ) )
	{
		// can't move there
		// TODO: SFX play bonk sound
		return;
	}

	// Check if there's an NPC there
	for (int i=0; i < m_npcs.size(); i++)
	{
		Npc *npc = m_npcs[i];
		if ((npc->m_x == nx) &&
			(npc->m_y == ny))
		{
			// don't move there, say NPC's speech
			// TODO: SFX talking
			showHudText( npc->m_displayName, npc->m_speech );
			return;
		}

	}

	// Move there
	m_px = nx;
	m_py = ny;
	DBG::info("move to %d %d\n", m_px, m_py );
	m_showHudText = false;

	// update camera pos
	m_camTarg = vec3f( m_px + 0.5f, 
					(mnew.m_elevation+1.0) * 0.3f, 
					m_py + 0.5f );

	// let critters move
	updateCritters();
}

void IslandGame::updateCritters()
{	
	for (int i=0; i < m_critters.size(); i++)
	{
		Critter *c = m_critters[i];
		bool doMove = true;
		int dir = DIR_NORTH;

		if (c->m_behavior == BEHAVIOR_STATIC)
		{
			// don't move
			doMove = false;
		}
		else if (c->m_behavior == BEHAVIOR_RANDOM)
		{
			// move randomly
			dir = rand() % 4;
		}
		else if (c->m_behavior == BEHAVIOR_SEEK_PLAYER)
		{
			// seek player
			if (rand() % 2)
			{
				// move east/west
				if (c->m_x < m_px) dir = DIR_WEST;
				else dir = DIR_EAST;
			}
			else
			{
				// move north/south
				if (c->m_y < m_py) dir = DIR_SOUTH;
				else dir = DIR_NORTH;
			}
		}

		if (doMove)
		{
			c->m_dir = dir;
			
			// now try to actually move there
			int nx, ny;
			switch (c->m_dir)
			{
				case DIR_NORTH:
					nx = c->m_x;
					ny = c->m_y-1;
					break;
				case DIR_SOUTH:
					nx = c->m_x;
					ny = c->m_y+1;
					break;
				case DIR_WEST:
					nx = c->m_x-1;
					ny = c->m_y;
					break;
				case DIR_EAST:
					nx = c->m_x+1;
					ny = c->m_y;
					break;
			}

			// can we move there?
			if ( (nx < 0) || (nx >= m_mapSizeX) ||
				 (ny < 0) || (ny >= m_mapSizeY))
			{
				// nope, out of bounds				
				continue;
			}

			// check elevation match
			if (m_map[nx][ny].m_elevation != m_map[c->m_x][c->m_y].m_elevation)
			{
				// elevtion mismatch
				continue;
			}

			// are we blocked
			bool blocked = false;
			for (int j=0; j < m_critters.size(); j++)
			{
				if (j==i) continue;
				if ((m_critters[j]->m_x == nx) &&
					(m_critters[j]->m_y == ny))
				{
					// someone's in our spot
					blocked = true;					
					break;
				}
			}

			if (blocked) continue;

			// Yay we can move
			c->m_x = nx;
			c->m_y = ny;
		}
	}
}

void IslandGame::pass()
{
	updateCritters();
}

float IslandGame::getRotForDir( int dir )
{
	return dir * 90.0f;
}

float IslandGame::getRotLookAt( int x, int y )
{
	float x1 = m_px;
	float y1 = m_py;
	float x2 = x;
	float y2 = y;

	return (-atan2( y2-y1, x2-x1 ) * 180.0f/3.1415f) - 115.0f;
}

void IslandGame::showHudText( const std::string &title,
							  const std::string &text )
{
	m_showHudText = true;
	m_hudText = "";
		

	// word wrap text
	gfEnableFont( m_fntFontId, 20 );
	char buff[2000];
	std::string text2 = text;
	std::transform( text2.begin(), text2.end(), text2.begin(), toupper );
	strcpy( buff, text2.c_str() );
	char *ch = strtok( buff, " \t" );
	std::string line;
	do
	{
		// attempt to append word
		std::string line2 = line + " " + ch;	

		// does it fit?
		int w = gfGetStringWidth( line2.c_str() );
		if ( w > 750 )
		{			
			// wrap
			m_hudText = m_hudText + line;
			m_hudText = m_hudText + "\n";

			line = ch;
		}
		else
		{
			line = line2;
		}
	} while( ch = strtok( NULL, " \t" ) );
	
	// append leftovers
	m_hudText += line;

	// upcase
	//std::transform( m_hudText.begin(), m_hudText.end(), m_hudText.begin(), toupper );

	m_hudTitle = title;
	std::transform( m_hudTitle.begin(), m_hudTitle.end(), m_hudTitle.begin(), toupper );
}