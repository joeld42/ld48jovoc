#include <crtdbg.h>

#include <assert.h>

#include <prmath/prmath.hpp>

#include "gamefontgl.h"

#include "ld14wall.h"
#include "GameView.h"
#include "Mesh.h"

using namespace std;

const char *menus[] = {
	"PLAY",
	"EDITOR",
	"QUIT",
	NULL
};

GameView::GameView( GlacierGame *game ) :
			m_game( game ),
			m_screenX( 0 ),
			m_screenY( 0 ),
			dbgDrawHeight( false ),
			m_oceanAnim( 0 ),
			m_editHite( 1 ),
			m_editBlock( NULL ),
			m_zoomOut( false ),
			m_currMenuItem( 0 ),
			m_levelNdx( 0 )
{	
}

GLuint GameView::loadTexture( const char *filename )
{
	// Load the font image
	ILuint ilId;
	ilGenImages( 1, &ilId );
	ilBindImage( ilId );		
	
	if (!ilLoadImage( (ILstring)filename )) 	
	{		
		errorMessage( string("Loading texture failed for: ") + filename );
	}
	
	// Make a GL texture for it
	GLuint glTexId;
	glTexId = ilutGLBindTexImage();
	
	return glTexId;
}

void GameView::loadResources()
{	
	// Load meshes
	vec3f blockScale( 10.0, 5.0, 10.0 );
	m_idTetBlock = LoadObjFile( "gamedata/tet_block.obj", blockScale );

	m_idTetLine   = LoadObjFile( "gamedata/tet_line.obj",   blockScale );
	m_idTetZee1   = LoadObjFile( "gamedata/tet_zee1.obj",   blockScale );
	m_idTetZee2   = LoadObjFile( "gamedata/tet_zee2.obj",   blockScale );
	m_idTetSquare = LoadObjFile( "gamedata/tet_square.obj", blockScale );
	m_idTetEll1   = LoadObjFile( "gamedata/tet_ell1.obj",   blockScale );
	m_idTetEll2   = LoadObjFile( "gamedata/tet_ell2.obj",   blockScale );
	m_idTetTee1   = LoadObjFile( "gamedata/tet_tee1.obj",   blockScale );
	m_idTetTee2   = LoadObjFile( "gamedata/tet_tee2.obj",   blockScale );
	
	// assign meshes to tets
	m_game->m_tets[0]->m_mesh = m_idTetLine;
	m_game->m_tets[1]->m_mesh = m_idTetZee1;
	m_game->m_tets[2]->m_mesh = m_idTetZee2;
	m_game->m_tets[3]->m_mesh = m_idTetSquare;
	m_game->m_tets[4]->m_mesh = m_idTetEll1;
	m_game->m_tets[5]->m_mesh = m_idTetEll2;
	m_game->m_tets[6]->m_mesh = m_idTetTee1;
	m_game->m_tets[7]->m_mesh = m_idTetTee2;

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

	// Logo
	m_glLogoTexId = loadTexture( "gamedata/title.png" );

	// Load textures
	m_glGroundTexId  = loadTexture( "gamedata/ground.png" );
	m_glGroundSideTexId  = loadTexture( "gamedata/gr_side.png" );
	m_glGlacierTexId = loadTexture( "gamedata/glacier.png" );
	m_glOceanTexId   = loadTexture( "gamedata/ocean.png" );
	m_glCritterTexId = loadTexture( "gamedata/critters.png" );
}

void GameView::setupLighting()
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };	
	glShadeModel (GL_SMOOTH);	

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable( GL_COLOR_MATERIAL );

}

void LandBlock::shade()
{
	if (m_hite==3)
	{
		glColor3f( 1.0, 1.0, 1.0 );
	}
	else if (m_hite==2)
	{
		glColor3f( 0.9, 0.9, 0.9 );
	}
	if (m_hite==1)
	{
		glColor3f( 0.8, 0.8, 0.8 );
	}
}

void LandBlock::drawTop()
{
	shade();

	if (m_pnts.size() < 3) return;

	glBegin( GL_POLYGON );
	for (size_t ndx=0; ndx < m_pnts.size(); ++ndx )
	{
		glTexCoord2f( m_pnts[ndx].x / 50.0, m_pnts[ndx].y / 50.0 );
		glNormal3f( 0.0, 1.0, 0.0 );
		glVertex3f( m_pnts[ndx].x, (m_hite * 5.0) - 0.2, m_pnts[ndx].y );
	}
	glEnd();	
}

void LandBlock::drawBottom()
{
	shade();

	if (m_pnts.size() < 3) return;

	glBegin( GL_QUADS );
	float perimeter = 0.0;
	vec3f up( 0.0, 1.0, 0.0 );
	vec3f edge, nrm;

	for (size_t ndx=0; ndx < m_pnts.size(); ++ndx )
	{
		size_t ndxNext = ndx +1;
		if (ndxNext==m_pnts.size()) ndxNext=0;				

		edge = vec3f(m_pnts[ndxNext].x, 0, m_pnts[ndxNext].y) - 
				vec3f( m_pnts[ndx].x, 0, m_pnts[ndx].y );
		edge.Normalize();
		nrm = prmath::CrossProduct( edge, up );

		glNormal3f( nrm.x, nrm.y, nrm.z );

		glTexCoord2f( perimeter, 0.0 );
		glVertex3f( m_pnts[ndx].x, m_hite * 5.0  - 0.2, m_pnts[ndx].y );

		glTexCoord2f( perimeter, 1.0 );
		glVertex3f( m_pnts[ndx].x, (m_hite-3)*5.0, m_pnts[ndx].y );		

		float d = prmath::Length(m_pnts[ndx] - m_pnts[ndxNext]);
		perimeter += d / 50.0;
		
		glTexCoord2f( perimeter, 1.0 );
		glVertex3f( m_pnts[ndxNext].x, (m_hite-3)*5.0, m_pnts[ndxNext].y );

		glTexCoord2f( perimeter, 0.0 );
		glVertex3f( m_pnts[ndxNext].x, m_hite*5.0  - 0.2, m_pnts[ndxNext].y );				
	}
	glEnd();	
}

void GameView::update( float dt )
{
	if (m_game->m_editMode)
	{
		// editor update
		if (m_editBlock)
		{
			m_editBlock->m_hite = m_editHite;
		}
		return;
	}

	// Update Ocean
	m_oceanAnim += dt * 0.1;

	// Update Ice blocks
	std::vector<IceBlock> keepBlocks;

	for (int ndx=0; ndx < m_blocks.size(); ++ndx )
	{
		IceBlock &block = m_blocks[ndx];
		
		float blockLat = (block.m_pos.z - m_game->m_latitude);
		if (blockLat < 0.0)
		{
			block.m_pos.y -= dt;
			block.m_sinkRotAmt += (dt * 3.0);
		}

		if (blockLat > -140.0)
		{
			keepBlocks.push_back( block );
		}
	}

	m_blocks = keepBlocks;
}

void GameView::redraw( int state )
{
	// Initialize
	static bool initted = false;
	if (!initted)
	{
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_TEXTURE );

		loadResources();
		setupLighting();

		initted = true;
	}
	glClearColor( 99 / 256.0, 178.0  / 256.0, 184.0 / 256.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	if (state != STATE_GAME)
	{
		// just draw the menus..
		glDisable( GL_LIGHTING );		
		gfEnableFont( m_fntFontId, 32 );

		for (int menuNdx=0; menus[menuNdx]; ++menuNdx)
		{		
			float c = (m_screenX - gfGetStringWidth( menus[menuNdx] ) )/2;

			if (m_currMenuItem == menuNdx)
			{
				glColor3f( 1.0, 0.5, 0.1 );
			}
			else
			{
				glColor3f( 1.0, 1.0, 1.0 );
			}

			gfBeginText();
			glTranslated( c, m_screenY - (300 + (menuNdx) * 35), 0 );		
			gfDrawString( menus[menuNdx] );
			gfEndText();						
		}

		gfEnableFont( m_fntFontId, 20 );
		glColor3f( 1.0, 1.0, 1.0 );

		gfBeginText();		
		glTranslated( 100, 60, 0 );
		string levelName = m_levelNames[ m_levelNdx ];
		for (int i=0; i < levelName.size(); ++i)
		{
			levelName[i] = toupper( levelName[i] );
		}		
		gfDrawStringFmt( "CURRENT LEVEL: %s\n(CHANGE WITH L/R ARROW KEYS)", levelName.c_str() );
		
		gfEndText();						

		// Logo
		gfBeginText();

		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, m_glLogoTexId );	
		//glBindTexture( GL_TEXTURE_2D, m_glGroundTexId );	

		float c = m_screenX/2;
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_QUADS );		
		glTexCoord2f( 0.0,   256.0); glVertex3f( c-256, m_screenY - 10, 0.1 );
		glTexCoord2f( 0.0,   0.0 ); glVertex3f( c-256, m_screenY - 266, 0.1 );
		glTexCoord2f( 256.0, 0.0 ); glVertex3f( c+256, m_screenY - 266, 0.1 );
		glTexCoord2f( 256.0, 256.0 ); glVertex3f( c+256, m_screenY - 10, 0.1 );
		glEnd();	
		
		gfEndText();

		return;
	}
	
	assert( m_screenX > 0 ); 
	assert( m_screenY > 0 );
	float aspect = (float)m_screenX/(float)m_screenY;
	
	// narrow fov to get a isometric-like view, but with
	// just a bit of perspective to look cool
	gluPerspective( 40.0, aspect, 0.1, 2000.0 );
	//glOrtho( -aspect/2.0, aspect/2.0, -1.0, 1.0, 0.1, 1000.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	
	if ((m_zoomOut) && (m_game->m_editMode))
	{
		glTranslated( 0.0, 0.0, -1000 );	
	}
	else
	{
		glTranslated( 0.0, 0.0, -200 );	
	}

	glRotated( 40.0, 1.0, 0.0, 0.0 );
	glRotated( 45.0, 0.0, 1.0, 0.0 );

	// Center map
	glTranslated( -50.0f, 0.0, -110.0 );

	// Draw guide lines	
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	
	if (!m_game->m_editMode)
	{
		// END line
		glLineWidth( 3.0 );
		glColor3f( 1.0, 0.5, 0.0 );
		glBegin( GL_LINES );
		glVertex3f( -5.0, 0.2, 0.0 );
		glVertex3f( 105.0, 0.2, 0.0 );
		glEnd();

		// play area line	
		glColor3f( 0.0, 1.0, 1.0 );
		glBegin( GL_LINES );

		// start line at 4
		glVertex3f( -5.0, 3.2, 40.0 );
		glVertex3f( 105.0, 3.2, 40.0 );

		// sides
		glVertex3f( 0.0, 3.2, 0.0 );
		glVertex3f( 0.0, 3.2, MAP_COLS * 10.0 );

		glVertex3f( 100.0, 3.2, 0.0 );
		glVertex3f( 100.0, 3.2, MAP_COLS * 10.0 );

		// progress indicator
		glColor3f( 1.0, 0.0, 1.0 );
		glVertex3f( 110.0, 3.2, 0.0 );
		glVertex3f( 110.0, 3.2, 100.0 );	

		// max level is 1000 (for now, arbitrary)
		glVertex3f( 105.0, 3.2, m_game->m_latitude / 10.0 );
		glVertex3f( 115.0, 3.2, m_game->m_latitude / 10.0 );

		glEnd();
	}

	// Draw the glacier	
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );

	if (!m_game->m_editMode)
	{
		glColor3f( 1.0, 1.0, 1.0 );
		glBindTexture( GL_TEXTURE_2D, m_glGlacierTexId );	

		glBegin( GL_QUADS );	
		glTexCoord2f( 0.0 , 0.0); glVertex3f( -200.0, 0.0, 0.0 );
		glTexCoord2f( 1.0, 0.0 ); glVertex3f( -200.0, 100.0, -50.0 );
		glTexCoord2f( 1.0, 4.0 ); glVertex3f( 200.0, 100.0,-50.0 );
		glTexCoord2f( 0.0,  4.0 ); glVertex3f( 200.0, 0.0, 0.0 );	
		glEnd();	
	}

	// travel to current latitude
	glPushMatrix();

	glTranslated( 0.0, 0.0, -m_game->m_latitude);

	// grab the camera matrices and update mouse -- this is
	// "world pos" 
	glGetDoublev( GL_MODELVIEW_MATRIX, m_modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, m_proj );
	glGetIntegerv( GL_VIEWPORT, m_viewport );
	updateMouse();	

	// Draw the ocean	
	if (!dbgDrawHeight)
	{
		glColor3f( 1.0, 1.0, 1.0 );
		glBindTexture( GL_TEXTURE_2D, m_glOceanTexId );	

		glBegin( GL_QUADS );	
		glTexCoord2f( 0.0 , 0.0); glVertex3f( -200.0,   3.0, -200.0 );
		glTexCoord2f( 20.0, 0.0 ); glVertex3f( -200.0,   3.0, 1200.0 );
		glTexCoord2f( 20.0, 20.0 ); glVertex3f( 1200.0, 3.0, 1200.0 );
		glTexCoord2f( 0.0,  20.0 ); glVertex3f( 1200.0, 3.0, -200.0 );	
		glEnd();	
	}

	if (m_game->m_editMode)
	{
		// Editor cursor
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
		glColor3f( 1.0, 1.0, 0.0 );

		glBegin( GL_LINES );
		
		glVertex3f( m_mousePosGround.x + 3.0, m_mousePosGround.y, m_mousePosGround.z );
		glVertex3f( m_mousePosGround.x - 3.0, m_mousePosGround.y, m_mousePosGround.z );

		glVertex3f( m_mousePosGround.x, m_mousePosGround.y, m_mousePosGround.z + 3.0 );
		glVertex3f( m_mousePosGround.x, m_mousePosGround.y, m_mousePosGround.z - 3.0 );

		glEnd();
	}

	// draw current land block in editor
	if ((m_game->m_editMode) && (m_editBlock))
	{
		glColor3f( 1.0, 0.0, 1.0 );
		glPointSize( 3.0 );

		if (m_editBlock->m_pnts.size() == 1) glBegin(GL_POINTS);
		else glBegin(GL_LINE_LOOP);			
		
		for (size_t ndx=0; ndx < m_editBlock->m_pnts.size(); ++ndx )
		{
			glVertex3f( m_editBlock->m_pnts[ndx].x,
						m_editBlock->m_hite * 5.0,
						m_editBlock->m_pnts[ndx].y );
		}

		glEnd();
		glColor3f( 1.0, 1.0, 1.0 );
	}

	if (!dbgDrawHeight)
	{	
		// Draw map as normal		
		glEnable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );
		glColor3f( 1.0, 1.0, 1.0 );

		glBindTexture( GL_TEXTURE_2D, m_glGroundTexId );	

		// Land Blocks -- tops
		for (size_t ndx=0; ndx < m_game->m_landBlocks.size(); ++ndx )
		{		
			m_game->m_landBlocks[ndx]->drawTop();
		}

		glBindTexture( GL_TEXTURE_2D, m_glGroundSideTexId );	
		
		// Land Blocks -- sides
		for (size_t ndx=0; ndx < m_game->m_landBlocks.size(); ++ndx )
		{		
			m_game->m_landBlocks[ndx]->drawBottom();
		}

		
		// dropped blocks
		//glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
		for (size_t ndx=0; ndx < m_blocks.size(); ++ndx )
		{
			IceBlock &block = m_blocks[ndx];
			glColor3f( block.m_iceCol.x, block.m_iceCol.y, block.m_iceCol.z );			
			glPushMatrix();
			
			glTranslated( block.m_pos.x,  block.m_pos.y,  block.m_pos.z );
			glRotated( block.m_rot * 90, 0.0, 1.0, 0.0 );

			// sink rot
			glRotated( block.m_sinkRotAmt, 
				block.m_sinkRot.x, block.m_sinkRot.y, block.m_sinkRot.z );

			glCallList( block.m_tet->m_mesh );
			glPopMatrix();
		}

		
	
	}

	// Tile Size
	vec3f tileSz( 10.0, 5.0, 10.0 );
	float cursorY;
	bool isValidDrop = true;			

	if (!m_game->m_editMode)
	{
		// Floaty Ice Cursor			
		int check;
		isValidDrop = m_game->dropTet( true, check );	
		cursorY = tileSz.y * check;

		glTranslated( m_mousePosGround.x, m_mousePosGround.y + cursorY, m_mousePosGround.z );
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
		glColor3f( 0.0, 0.5, 0.5 );	


		//_RPT1( _CRT_WARN, "currTet id is %d\n",  m_game->m_currTet->m_mesh );
		glRotated( m_game->m_tetRot * 90, 0.0, 1.0, 0.0 );
		glCallList( m_game->m_currTet->m_mesh );

		glPopMatrix();
	}

	// Draw map, offset by map start
	glTranslated( 0.0, 0.0, -(m_game->m_latitude - m_game->m_mapStartLat) );

	if (!m_game->m_editMode)
	{
		// Draw wireframe cursor for where next block will land	
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glPushMatrix();
		glTranslated( m_game->m_selX * tileSz.x, 
			   		cursorY, 
					m_game->m_selY * tileSz.z );

		if (isValidDrop) glColor3f( 0.0, 1.0, 0.0 );
		else glColor3f( 1.0, 0.0, 0.0 );
	
		glRotated( m_game->m_tetRot * 90, 0.0, 1.0, 0.0 );
		glCallList( m_game->m_currTet->m_mesh );

		glPopMatrix();

		glEnable( GL_LIGHTING );
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DBG : draw map as cubes	
	if (dbgDrawHeight)
	{
		color3f cold( 0.0, 1.0, 1.0 );
		color3f hot( 1.0, 0.0, 0.0 );

		float maxVal = 0.0;
		foreach_map
		{
			MapSquare &t = MAPITER( m_game->m_map );
			maxVal = max( t.m_path, maxVal );
		}

		foreach_map
		{
			MapSquare &t = MAPITER( m_game->m_map );
			for (int z=-1; z < t.m_hite; ++z)
			{
				glPushMatrix();
				glTranslated( tileSz.x * mi, 
							  (tileSz.y * z) + 0.5, 
							  tileSz.z * (mj+1) );

				float tval = t.m_path / maxVal;
				if (tval < 0.0) tval = 0.0;
				if (tval > 1.0) tval = 1.0;
				color3f col = (cold * (1.0-tval)) + (hot * tval);

				if (t.m_scurry)
				{
					col = color3f( 0.5, 1.0, 0.5 );
				}


				glColor3f( col.r, col.g, col.b );
				glCallList( m_idTetBlock );

				glPopMatrix();
			}
		}	
	}	

	// Draw Critters
#if 1
	glBindTexture( GL_TEXTURE_2D, m_glCritterTexId );
	glDisable( GL_LIGHTING );	
	glEnable( GL_TEXTURE_2D );
	glAlphaFunc( GL_GREATER, 0.5 );
	glEnable( GL_ALPHA_TEST );

	glColor3f( 1.0, 1.0, 1.0 );

	//vec3f right( m_modelview[0], m_modelview[1], m_modelview[2] );
	//vec3f up( m_modelview[4], -m_modelview[5], m_modelview[6] );
	//vec3f corner = right + up;

	float sz = 10.0;
	float sz2 = sz/2.0;

	//right *= sz;
	//corner *= sz;
	//up *= sz;

	for (int cndx=0; cndx < m_game->m_critters.size(); ++cndx )
	{
		Critter &c = m_game->m_critters[cndx];

		//_RPT4( _CRT_WARN, "critter %d pos %d %d hite %d\n",
		//	cndx, c.px, c.py, m_game->m_map[c.px][c.py].m_hite );

		glPushMatrix();
		glTranslated( tileSz.x * c.px + 5.0, 
			m_game->m_map[c.px][c.py].m_hite * tileSz.y,
			tileSz.z * c.py + 5.0 );		

		// undo camera rotate		
		glRotated( 45.0, 0.0, 1.0, 0.0 );
		glRotated( -40.0, 0.0, 0.0, 1.0 );
		
		

		int ndx = c.imageNdx;
		vec2f tx( (ndx / 4) * 0.25, (ndx % 4) * 0.25);				
		
		glBegin( GL_QUADS );
		glTexCoord2f( tx.x,tx.y+ 0.25  );
		glVertex3f( 0, 0.0, -sz2 );
		//glVertex3f( 0.0, 0.0, 0.0 );

		glTexCoord2f( tx.x, tx.y ); 
		glVertex3f( 0,  sz, -sz2 );
		//glVertex3f( right.x, right.y, right.z );

		glTexCoord2f( tx.x + 0.25, tx.y );
		glVertex3f( 0,  sz, sz2 );
		//glVertex3f( corner.x, corner.y, corner.z );

		glTexCoord2f( tx.x + 0.25, tx.y + 0.25 );
		glVertex3f( 0,  0, sz2  );
		//glVertex3f( up.x, up.y, up.z );

		glEnd();

		glPopMatrix();		
	}

	glDisable( GL_ALPHA_TEST );
#endif

#if 0
	glTranslated( 0, 0, 40 );
	glColor3f( 0.0, 0.6, 1.0 );
	glCallList( m_idTetTee1 );

	glTranslated( 30, 0, 0 );
	glColor3f( 1.0, 0.6, 1.0 );
	glCallList( m_idTetZee1 );

	glTranslated( 30, 0, 0 );
	glColor3f( 0.2, 1.0, 0.2 );
	glCallList( m_idTetEll1 );

	glTranslated( 0, 0, 50 );
	glColor3f( 1.2, 0.0, 1.0 );
	glCallList( m_idTetBlock );
#endif

	if (m_game->m_editMode)
	{		
		glDisable( GL_LIGHTING );
		glColor3f( 1.0, 1.0, 1.0 );
		gfEnableFont( m_fntFontId, 32 );
		float c = (m_screenX - gfGetStringWidth( "EDITOR MODE" ) )/2;
		gfBeginText();
		glTranslated( c, 50, 0 );
		gfDrawString( "EDITOR MODE" );
		gfEndText();

		gfEnableFont( m_fntFontId, 20 );
		gfBeginText();
		glTranslated( 10, m_screenY-30, 0 );
		gfDrawStringFmt( "LAT: %3.2f\n"
						  "HITE: %d\n"
						  "ZOOM: %s", 
						  m_game->m_latitude, m_editHite,
						  m_zoomOut ? "T" : "F"
						  );
		gfEndText();
	}
}

void GameView::mouseMove( int x, int y )
{
	//_RPT2( _CRT_WARN, "mouse: %d %d\n", x, y );
	m_mouseX = x;
	m_mouseY = y;

	// TODO: make aspect support different screen modes	
	float aspect=(float)m_screenX/(float)m_screenY;
	m_mousePos2d = vec2f( aspect*((float)(x-((m_screenX-m_screenY)/2) ) / (float)m_screenX), 1.0f-((float)y / (float)m_screenY) );

	updateMouse();
}

// update mouse when camera or mouse pos changes
void GameView::updateMouse()
{
	// 3d pos
	int x = m_mouseX;
	int y = m_screenY - m_mouseY;
	double x1, y1, z1, x2, y2, z2;

	// unproject two points at different z's
	gluUnProject( (float)(x), (float)(y), 0.001,
		           m_modelview, m_proj, m_viewport,
					&x1, &y1, &z1 );
	gluUnProject( (float)x, (float)(y), 0.01,
		           m_modelview, m_proj, m_viewport,
					&x2, &y2, &z2 );

	float offs = 0.0;
	if (m_game->m_editMode)
	{
		offs = m_editHite * 5.0;		
	}
	// and use that to form a ray and hit the ground
	vec3f dir(x2-x1, y2-y1, z2-z1 ), p( x1,y1,z1 );
	float t = ((-y2) + offs) / dir.y;
	m_mousePosGround = p + dir*t;

	// Update selected pos in game
	int mapX, mapY;
	mapX = (int)((m_mousePosGround.x + 5.0) / 10);
	mapY = (int)(((m_mousePosGround.z + 5.0 )- m_game->m_mapStartLat)/10);	

	m_game->m_selX = mapX;
	m_game->m_selY = mapY;
}

void GameView::mouseDown()
{
	if (m_game->m_editMode)
	{
		// are we editing a block?
		if (!m_editBlock)
		{
			// nope, create a new one
			m_editBlock = new LandBlock();
			m_game->m_landBlocks.push_back( m_editBlock );

			m_editBlock->m_hite = m_editHite;
		}
		
		m_editBlock->m_pnts.push_back( vec2f( m_mousePosGround.x, m_mousePosGround.z ) );
		
	}
	else
	{
		// check if drop is valid
		int checkVal;
		bool isValid = m_game->dropTet(true, checkVal );
		if (isValid)
		{	
			// Add it to our block list
			IceBlock b;		
			if ( checkVal == 0 )
			{
				// bottom layer, dark blue
				float blueVal = 0.2 + ((float)rand() / (float)RAND_MAX) * 0.8;
				b.m_iceCol = color3f( 0.0, ((float)rand() / (float)RAND_MAX) * blueVal, blueVal );
			}
			else if (checkVal == 1 )
			{

				b.m_iceCol = color3f( ((float)rand() / (float)RAND_MAX), 1.0, 1.0  );
			}
			else
			{
				// top level -- snowcaps
				b.m_iceCol = color3f( 1.0, 1.0, 1.0  );
			}

			//b.m_iceCol = color3f( 1.0, 1.0, 1.0 );
			b.m_pos = vec3f( (m_game->m_selX * 10.0), 
			   				 checkVal * 5.0, 
							 m_game->m_selY * 10.0  + m_game->m_mapStartLat );
			b.m_tet = m_game->m_currTet;
			b.m_rot = m_game->m_tetRot;

			b.m_sinkRotAmt = 0.0;
			b.m_sinkRot = vec3f( ((float)rand() / (float)RAND_MAX) -0.5,
								 ((float)rand() / (float)RAND_MAX) -0.5,
								 ((float)rand() / (float)RAND_MAX) -0.5 );
			prmath::Normalize( b.m_sinkRot );

			m_blocks.push_back( b );

			// and drop it
			m_game->dropTet( false, checkVal );	
		}
	}
}

void GameView::mouseUp()
{
}