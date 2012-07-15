#include <assert.h>

#define M_PI (3.1415926535897932384626433832795)
#define D2R (M_PI/180.0)
#define R2D (180.0/M_PI)

#include "ResourceFile.h"
#include "PNGLoader.h"
#include "GameViewGL.h"

enum 
{
	DRAWPASS_GROUND,
	DRAWPASS_GRID,	
	DRAWPASS_TILES,
	DRAWPASS_GHOST_TILES,
	DRAWPASS_BLOCKED,
	DRAWPASS_LETTERS,	

	DRAWPASS_LAST
};

GameViewGL::GameViewGL( GameState *game ) :
	m_game( game ),
	dbg_bgBlue( 0.5 ),
	m_dragging( false ),
	m_dragTileIndex( -1 )
{
}

void GameViewGL::setupDrawPass( int drawPass )
{
	switch (drawPass)
	{
		case DRAWPASS_GROUND:
			glDisable( GL_BLEND );
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glBindTexture( GL_TEXTURE_2D, m_glGrassTexId );
			break;
		case DRAWPASS_GRID:
			glColor3f( 0.0, 0.5, 0.0 );
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable( GL_TEXTURE_2D );
			glDisable(GL_LIGHTING);
			break;
		case DRAWPASS_TILES:			
			glColor3f( 1.0, 1.0, 1.0 );
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);			
			glEnable( GL_TEXTURE_2D );
			glEnable(GL_LIGHTING);
			glBindTexture( GL_TEXTURE_2D, m_glTileTexId );
			break;
		case DRAWPASS_GHOST_TILES:			
			glColor3f( 0.0, 0.0, 1.0 );
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable( GL_TEXTURE_2D );
			glDisable(GL_LIGHTING);			
			break;
		case DRAWPASS_BLOCKED:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);			
			glEnable( GL_TEXTURE_2D );
			glEnable(GL_LIGHTING);
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, m_glBlockTexId );
			break;
		case DRAWPASS_LETTERS:
			glEnable( GL_TEXTURE_2D );
			glEnable( GL_BLEND );
			glDisable(GL_LIGHTING);
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		

			glBindTexture( GL_TEXTURE_2D, m_glTileFontTexId );
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			break;
	}
}

void GameViewGL::setupLighting()
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
}

void GameViewGL::drawQuad( float i, float j, float yval )
{
	glBegin( GL_QUADS );
	glNormal3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 );
	glVertex3f( i,   -yval, j );

	glTexCoord2f( 0.0, 1.0 );
	glVertex3f( i+1, -yval, j );

	glTexCoord2f( 1.0, 1.0 );
	glVertex3f( i+1, -yval, j+1 );

	glTexCoord2f( 1.0, 0.0 );
	glVertex3f( i,   -yval, j+1 );
	glEnd();			
}

void GameViewGL::drawTile( const Tile &t, int drawPass, float i, float j )
{
	switch (drawPass)
	{
		// draw a quad
		case DRAWPASS_GROUND:
			if ((!t.blocked) && (t.letter == ' '))
			{
				drawQuad( i, j, 0.2 );
			}
			break;
		case DRAWPASS_LETTERS:
			if ((t.target) || (((!t.blocked) && (t.letter != ' ')) || (t.ghost_letter != ' ')) )
			{
				// draw with special texture coordinates				
				float index;
				vec4f color( 1.0, 1.0, 1.0, 1.0);
				
				if (t.letter != ' ')
				{										
					index = (float)(t.letter - 'A');
				}
				else if (t.ghost_letter != ' ')
				{				
					color = vec4f( 0.4, 1.0, 1.0, 0.8);
					index = (float)(t.ghost_letter - 'A');
				}
				else //if (t.target)
				{					
					index = 36;
				}

				if (t.target)
				{
					color = vec4f( 1.0, 0.8, 0.0, 1.0);
				}
				
				vec4f c = t.error*vec4f( 1.0, 0.0, 0.0, 1.0) + (1.0f-t.error)*color;
				glColor4f( c.x, c.y, c.z, c.w);

				float amt = 1.0f / 8.0f;
				vec2f stbase( fmod( index, 8.0f), floor( index / 8.0f ) );
				stbase /= 8.0;
				glBegin( GL_QUADS );
				glNormal3f( 0.0, 1.0, 0.0 );
				glTexCoord2f( stbase.x, stbase.y);
				glVertex3f( i, 0.02, j );

				glTexCoord2f( stbase.x+amt, stbase.y  );
				glVertex3f( i+1, 0.02, j );

				glTexCoord2f( stbase.x+amt, stbase.y+amt );
				glVertex3f( i+1,0.02, j+1 );

				glTexCoord2f( stbase.x, stbase.y+amt);
				glVertex3f( i, 0.02, j+1 );
				glEnd();			
			}
			break;			
		case DRAWPASS_GRID:					

			if ((!t.blocked) && (t.letter == ' '))
			{
				float yval = 0.19;
				if ( (m_dragging) && 
					  ((int)m_mousePosGround.x == (int)i ) &&
					  ((int)m_mousePosGround.z == (int)j ) )
				{
					glColor3f( 0.0, 1.0, 0.0 );
					yval = 0.15;
					glLineWidth( 5.0 );
				}
				else
				{
					glColor3f( 0.0, 0.5, 0.0 );
				}

				drawQuad( i, j, 0.19 );
				glLineWidth( 2.0 );
			}
			break;			
		
		// draw an actual mesh
		case DRAWPASS_TILES:		
			if (t.letter != ' ')
			{
				glPushMatrix();
				glTranslated( i, -0.2, j );
				glCallList( m_idTile );
				glPopMatrix();
			}
			break;
		case DRAWPASS_GHOST_TILES:
			if (t.ghost_letter != ' ')
			{
				glPushMatrix();
				glTranslated( i, -0.2, j );
				glCallList( m_idTile );
				glPopMatrix();
			}
			break;
		case DRAWPASS_BLOCKED:			
			if (t.blocked)
			{
				glPushMatrix();
				glTranslated( i, -0.2, j );
				glCallList( m_idTileBlocked );
				glPopMatrix();
			}
			break;				
	}
}

void GameViewGL::redraw( float dt )
{
	// Initialize
	static bool initted = false;
	if (!initted)
	{
		initResources();
		setupLighting();
		initted = true;
	}

	glClearColor( 0.0, 0.0, dbg_bgBlue, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );

	int BW = m_game->boardWidth();
	int BH = m_game->boardHeight();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	// TODO: make aspect support different screen modes
	float aspect = (float)SCREEN_RES_X/(float)SCREEN_RES_Y;
	gluPerspective( 90.0, aspect, 0.1, 100.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	// Look down
	float zoom = std::max( m_game->boardHeight(), m_game->boardWidth() );
	if (zoom < 4.0) zoom = 4.0;
	zoom *= 0.7;
	static float currzoom = 100.0;
	
	currzoom = (dt * zoom) + ((1.0-dt)*currzoom);

	glTranslated( 0.0, 0.0, -currzoom);
	glRotated( 80.0, 1.0, 0.0, 0.0 );
	
	// position camera
	glTranslated( -((float)BW)/2.0f, 0.0, -((float)BH)/2.0f );
	//glTranslated( 20.0, 0.0, 0.0 );

	// grab the camera matrices
	glGetDoublev( GL_MODELVIEW_MATRIX, m_modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, m_proj );
	glGetIntegerv( GL_VIEWPORT, m_viewport );

	// Enable grid texture
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_glTileFontTexId );
	
	// draw board
	for (int boardPass=DRAWPASS_GROUND; boardPass < DRAWPASS_LAST; ++boardPass)
	{
		//if (boardPass==DRAWPASS_GRID) continue;

		setupDrawPass( boardPass );				
		for (int j=0; j < BH; ++j )
		{
			for (int i=0; i < BW; ++i )
			{
				drawTile( m_game->tile( i, j), boardPass, i, j );
			}
		}		

		if (boardPass==DRAWPASS_GRID)
		{
			glBegin( GL_LINES );
			glVertex3f( m_mousePosGround.x - 0.1, m_mousePosGround.y + 0.02, m_mousePosGround.z );
			glVertex3f( m_mousePosGround.x + 0.1, m_mousePosGround.y + 0.02, m_mousePosGround.z );

			glVertex3f( m_mousePosGround.x, m_mousePosGround.y + 0.02, m_mousePosGround.z - 0.1 );
			glVertex3f( m_mousePosGround.x, m_mousePosGround.y + 0.02, m_mousePosGround.z + 0.1 );

			glVertex3f( m_mousePosGround.x, m_mousePosGround.y + 0.02 - 0.1, m_mousePosGround.z );
			glVertex3f( m_mousePosGround.x, m_mousePosGround.y + 0.02 + 0.1, m_mousePosGround.z );

			glEnd();
		}
	}

	

	// draw car
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_glCarTexId );

	vec2f carpos = m_game->getCarPos();
	vec2f cardir = m_game->getCarDir();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();	
	glTranslated( carpos.x, 0.0, carpos.y );
	glRotated( R2D*atan2( cardir.x, cardir.y ), 0.0, 1.0, 0.0 );
	glColor3f( 1.0, 0.5, 0.5 );
	glCallList( m_idCar );
	glPopMatrix();	

	// game debug display
	//m_game->drawDbgDisplay();


	// 2D Draw part

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	float marg = (aspect-1.0) / 2.0;
	gluOrtho2D( -marg, 1.0+marg, 0.0, 1.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );

	// draw pick letters
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glColor3f( 0.0, 1.0, 1.0 );

	glBegin( GL_LINE_LOOP );
	glVertex3f( 0.25, 0.95, 0.0 );
	glVertex3f( 0.25+m_game->getMaxTiles()*0.05, 0.95, 0.0 );
	glVertex3f( 0.25+m_game->getMaxTiles()*0.05, 0.90, 0.0 );
	glVertex3f( 0.25, 0.90, 0.0 );
	glEnd();

#if 0
	glBegin( GL_LINE_LOOP );
	glVertex3f( 0, 0, 0.0 );
	glVertex3f( 0, 1, 0.0 );
	glVertex3f( 1, 1, 0.0 );
	glVertex3f( 1, 0, 0.0 );
	glEnd();


	glBegin( GL_LINES );
	glVertex3f( m_mousePos2d.x - 0.1, m_mousePos2d.y, 0.0 );
	glVertex3f( m_mousePos2d.x + 0.1, m_mousePos2d.y, 0.0 );
	glVertex3f( m_mousePos2d.x, m_mousePos2d.y - 0.1, 0.0 );
	glVertex3f( m_mousePos2d.x, m_mousePos2d.y + 0.1, 0.0 );
	glEnd();
#endif
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
	glBindTexture( GL_TEXTURE_2D, m_glTileFontTexId );	

	std::vector<PickTile> &picks = m_game->pickTiles();		
	for (int i=0; i < picks.size(); ++i)
	{
		// draw with special texture coordinates				
		float index = (float)(picks[i].letter - 'A');
		float amt = 1.0f / 8.0f;
		
		float targxpos = i;
		
		// ease tile pos
		float dxpos = (targxpos -picks[i].xpos) * dt * 5.0;
		
		picks[i].xpos = picks[i].xpos + dxpos;				

		float xpos0 = picks[i].xpos;
		float xpos = xpos0 * 0.05;
		float xpos2 = (xpos0+1.0) * 0.05;

		if ((picks[i].pending) ||(i==m_dragTileIndex))
		{
			glColor3f( 0.0, 0.3, 0.6 );
		}
		else
		{
			glColor3f( 0.0, 1.0, 1.0 );
		}

		vec2f stbase( fmod( index, 8.0f), floor( index / 8.0f ) );
		stbase /= 8.0;
		glBegin( GL_QUADS );
		glNormal3f( 0.0, 1.0, 0.0 );
		glTexCoord2f( stbase.x, stbase.y);
		glVertex3f( 0.25 + xpos, 0.95, 0.0 );

		glTexCoord2f( stbase.x+amt, stbase.y  );
		glVertex3f( 0.25 + xpos2, 0.95, 0.0 );

		glTexCoord2f( stbase.x+amt, stbase.y+amt );
		glVertex3f( 0.25 + xpos2, 0.9, 0.0 );

		glTexCoord2f( stbase.x, stbase.y+amt);
		glVertex3f( 0.25 + xpos, 0.9, 0.0 );
		glEnd();

		// is the user trying to drag this tile??
		if ((m_dragging) && (!picks[i].pending))
		{
			if ((m_mousePos2d.y > 0.9) && (m_mousePos2d.y < 0.95) &&
				(m_mousePos2d.x > 0.25+xpos) && (m_mousePos2d.x < 0.25+xpos2) )
			{
				m_dragTileIndex = i;
			}
		}
	}	

	// draw the dragged tile
	if ((m_dragging) && (m_dragTileIndex >= 0))
	{
		glColor3f( 0.0, 1.0, 1.0 );

		// arggg stop cutting and pasting this
		int index = picks[m_dragTileIndex].letter - 'A';
		vec2f stbase( fmod( index, 8.0f), floor( index / 8.0f ) );
		float sz = 0.02;
		float amt = 1.0f / 8.0f;
		stbase /= 8.0;
		glBegin( GL_QUADS );
		glNormal3f( 0.0, 1.0, 0.0 );
		glTexCoord2f( stbase.x, stbase.y);		
		glVertex3f( m_mousePos2d.x-sz, m_mousePos2d.y+sz, 0.0 );

		glTexCoord2f( stbase.x+amt, stbase.y  );
		glVertex3f( m_mousePos2d.x+sz, m_mousePos2d.y+sz, 0.0 );

		glTexCoord2f( stbase.x+amt, stbase.y+amt );
		glVertex3f( m_mousePos2d.x+sz, m_mousePos2d.y-sz, 0.0 );

		glTexCoord2f( stbase.x, stbase.y+amt);
		glVertex3f( m_mousePos2d.x-sz, m_mousePos2d.y-sz, 0.0 );
		glEnd();
	}

	

	glEnable( GL_LIGHTING );

}

void GameViewGL::update( float dt )
{
}

void GameViewGL::initResources()
{
	// Initialize GL
	glEnable( GL_DEPTH_TEST );

	// Load Meshes
	m_idCar = LoadObjFile(  gameDataFile("", "car.obj").c_str() );
	m_idTile = LoadObjFile(  gameDataFile("", "tile.obj" ).c_str() );
	m_idTileBlocked = LoadObjFile( gameDataFile("", "tile_blocked.obj" ).c_str() );

	// load textures
	m_glTileFontTexId = loadTexture( gameDataFile("", "tilefont.png").c_str() );
	m_glTileTexId = loadTexture(  gameDataFile("","tile.png").c_str() );
	m_glBlockTexId = loadTexture(  gameDataFile("","block.png").c_str() );
	m_glGrassTexId = loadTexture(  gameDataFile("","grass.png").c_str() );
	m_glCarTexId = loadTexture(  gameDataFile("","car.png").c_str() );	
}

GLuint GameViewGL::loadTexture( const char *filename )
{
	// Load the font image
    PNGImage pngImage;    
    pngImage = LoadImagePNG( filename );
    
    return pngImage.textureId;
    
#if 0
	ILuint ilId;
	ilGenImages( 1, &ilId );
	ilBindImage( ilId );		
	
	if (!ilLoadImage( (ILstring)filename )) 	
	{
		printf( "Loading texture %s failed\n", filename);
		assert(0);
	}
	
	// Make a GL texture for it
	GLuint glTexId;
	glTexId = ilutGLBindTexImage();
    
	return glTexId;
#endif
}

void GameViewGL::mouseMove( int x, int y )
{
	//_RPT2( _CRT_WARN, "mouse: %d %d\n", x, y );

	// TODO: make aspect support different screen modes	
	float aspect=(float)SCREEN_RES_X/(float)SCREEN_RES_Y;
	m_mousePos2d = vec2f( aspect*((float)(x-((SCREEN_RES_X-SCREEN_RES_Y)/2) ) / (float)SCREEN_RES_X), 1.0f-((float)y / (float)SCREEN_RES_Y) );

	// 3d pos
	y = SCREEN_RES_Y - y;
	double x1, y1, z1, x2, y2, z2;
	gluUnProject( (float)x, (float)(y), 0.001,
		           m_modelview, m_proj, m_viewport,
					&x1, &y1, &z1 );
	gluUnProject( (float)x, (float)(y), 0.01,
		           m_modelview, m_proj, m_viewport,
					&x2, &y2, &z2 );
	vec3f dir(x2-x1, y2-y1, z2-z1 ), p( x1,y1,z1 );
	float t = (-y2) / dir.y;
	m_mousePosGround = p + dir*t;

}

void GameViewGL::mouseDown()
{
	m_dragging = true;
	m_dragTileIndex = -1;
}
void GameViewGL::mouseUp()
{
	int tilex = (int)m_mousePosGround.x;
	int tiley = (int)m_mousePosGround.z;
	if ( (m_dragTileIndex >=0) &&
		 (tilex >= 0) && (tiley >=0) &&
		 (tilex < m_game->boardWidth() ) &&
		 (tiley < m_game->boardHeight() ) )
	{
		Tile &tile = m_game->tile( tilex, tiley);

		// if this is a valid tile location, place tile
		std::vector<PickTile> &picks = m_game->pickTiles();		
		if ( (!tile.blocked) && (tile.letter==' ') )
		{
			picks[m_dragTileIndex].pending = true;			
			tile.ghost_letter = picks[m_dragTileIndex].letter;
		}

	}

	m_dragging = false;
	m_dragTileIndex = -1;

	
}
