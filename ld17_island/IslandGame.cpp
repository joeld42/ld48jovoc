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

#include "tweakval.h"
#include "debug.h"


IslandGame::IslandGame() :
	m_islandDrawBuf( NULL ),
	m_px(0), m_py(0)
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
	static bool gfxInit = false;
	if (!gfxInit)
	{
		initGraphics();
		gfxInit = true;
	}

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
	glBindTexture( GL_TEXTURE_2D, m_terrainTilesTexId );	
	//glColor3f( 1.0, 0.0, 1.0 );	

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );

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
	gfEnableFont( m_fntFontId, 32 );

	glColor3f( 1.0, 1.0, 1.0 );
		
	gfBeginText();
	glTranslated( 200, 500, 0 );		
	gfDrawString( "HELLO ISLAND WORLD..." );
	gfEndText();						

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

	// Load the tiles
	m_terrainTilesTexId  = loadTexture( "gamedata/crappytiles.png", 4 );

	m_waterTexId  = loadTexture( "gamedata/water.png", 4 );

	// Set up simple lighting	
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_direction[] = { -1.0, 0.6, 0.0, 0.0 };
	GLfloat light_ambient[] = { 2.0, 2.0, 2.0, 1.0 };
	GLfloat light_diffuse[] = { 6.0, 6.0, 6.0, 1.0 };


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
	
	TiXmlElement *xActor = xActorLayer->FirstChildElement( "pstart" );
	m_px = atoi( xActor->Attribute( "x" ) ) / 16;
	m_py = atoi( xActor->Attribute( "y" ) ) / 16;

	m_camTarg = vec3f( m_px + 0.5f, 
					(m_map[m_px][m_py].m_elevation+1.0) * 0.3f, 
					m_py + 0.5f );
	m_camPos = m_camTarg;

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
	// Load the font image
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

	// TODO: look at what's going on with mnew

	// Move there
	m_px = nx;
	m_py = ny;
	DBG::info("move to %d %d\n", m_px, m_py );

	// update camera pos
	m_camTarg = vec3f( m_px + 0.5f, 
					(mnew.m_elevation+1.0) * 0.3f, 
					m_py + 0.5f );
}

void IslandGame::pass()
{
}