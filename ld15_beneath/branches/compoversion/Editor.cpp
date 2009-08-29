#include <Common.h>
#include <Editor.h>

#include <tweakval.h>
#include <gamefontgl.h>

#include <tinyxml.h>

Editor::Editor( GLuint fntID )
{
	m_fntFontId = fntID;
	m_level = NULL;
	m_showHelp = true;

	m_viewport = vec2f( 0, 0 );
	m_viewsize = vec2f( 800, 600 );

	m_gameview = vec2f( 0, 0 );
	m_useEditView = true;
}

void Editor::frameView()
{
	// frame level (assuming caves are taller than they are wide)
	m_useEditView = true;
	float aspect = 800.0f/600.0f;
	float w = m_level->m_mapSize.y * aspect;
	m_viewport = vec2f( -((w - m_level->m_mapSize.x )/2.0), 0.0 );
	//m_viewport = vec2f( 0, 0 );
	m_viewsize = vec2f( w, m_level->m_mapSize.y );									
}

void Editor::redraw()
{
	vec3f bgColor, gridColor;
	glColor3f( 1.0, 1.0, 1.0 );
	if (!m_level)
	{
		glClearColor( _TV( 0.2f ), _TV(0.0f), _TV( 0.0f ), 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		gfEnableFont( m_fntFontId, 32 );	
		gfBeginText();
		glTranslated( _TV(260), _TV(550), 0 );
		gfDrawString( "No Level Active" );
		gfEndText();
	}
	else
	{
		bgColor = m_level->m_bgColor;
		gridColor = (bgColor + vec3f( 1.0f, 1.0f, 1.0f )) / 2.0f;

		glClearColor( bgColor.r, bgColor.g, bgColor.b, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	if (m_showHelp)
	{
		gfEnableFont( m_fntFontId, 20 );	
		gfBeginText();
		glTranslated( _TV(50), _TV(500), 0 );
		
		gfDrawString( "------[ EDITOR KEYS ]------\n" 
			          "h - toggle this help\n" 
	    			  "1 - New Level (small)\n" 
					  "2 - New Level (medium)\n" 
					  "3 - New Level (large)\n"  
					  "f - frame view\n" 
					  "TAB - toggle editor/closeup\n"  
					  );			
		gfEndText();
	}	

	// at this point we need a level to draw
	if (!m_level) return;

	// reset the view to be map space
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	if (m_useEditView)
	{
		gluOrtho2D( m_viewport.x, m_viewport.x + m_viewsize.x, 
					m_viewport.y, m_viewport.y + m_viewsize.y ) ;
	}
	else
	{
		gluOrtho2D( m_gameview.x, m_gameview.x + 800,
				   m_gameview.y, m_gameview.y + 600 );
	}

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Draw the level grid
	glColor3f( gridColor.r, gridColor.g, gridColor.b );
	glDisable( GL_TEXTURE );

	glLineWidth( 2.0f );
	glBegin( GL_LINE_LOOP );
	glVertex3f( 0.0, 0.0, 0.0 );
	glVertex3f( m_level->m_mapSize.x, 0.0, 0.0 );
	glVertex3f( m_level->m_mapSize.x, m_level->m_mapSize.y, 0.0 );
	glVertex3f( 0.0, m_level->m_mapSize.y, 0.0 );	
	glEnd();

	
	// draw grid lines
	glLineWidth( 1.0f );
	glBegin( GL_LINES );
	for (float xpos=800; xpos < m_level->m_mapSize.x; xpos += 800.0)
	{
		glVertex3f( xpos, 0.0, 0.0 );
		glVertex3f( xpos, m_level->m_mapSize.y, 0.0 );
	}

	for (float ypos = 600; ypos < m_level->m_mapSize.y; ypos += 600.0)
	{
		glVertex3f( 0.0, ypos, 0.0 );
		glVertex3f( m_level->m_mapSize.x, ypos, 0.0 );
	}
	glEnd();

	glLineWidth( 1.0f );
	glEnable( GL_LINE_STIPPLE );
	glLineStipple( 1, 0xAAAAAAAA );
	glBegin( GL_LINES );
	for (float xpos=100; xpos < m_level->m_mapSize.x; xpos += 100.0)
	{
		glVertex3f( xpos, 0.0, 0.0 );
		glVertex3f( xpos, m_level->m_mapSize.y, 0.0 );
	}

	for (float ypos = 100; ypos < m_level->m_mapSize.y; ypos += 100.0)
	{
		glVertex3f( 0.0, ypos, 0.0 );
		glVertex3f( m_level->m_mapSize.x, ypos, 0.0 );
	}
	glEnd();
	glDisable( GL_LINE_STIPPLE );

	// draw the "game view" extents
	if (m_useEditView)
	{
		glLineWidth( 2.0f );
		glColor3f( 1.0f, 0.0f, 0.0f );
		glTranslated( m_gameview.x, m_gameview.y, 0.0 );
		glPushMatrix();
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 800, 0.0, 0.0 );
		glVertex3f( 800, 600, 0.0 );
		glVertex3f( 0.0, 600, 0.0 );	
		glPopMatrix();
		glEnd();
	}

}

void Editor::keypress( SDL_KeyboardEvent &key )
{
	switch( key.keysym.sym )
	{
	case SDLK_h:
		m_showHelp = !m_showHelp;		
		break;
	case SDLK_f:
		{
			if (!m_level) return;
			frameView();
		}
		break;
	case SDLK_TAB:
		 m_useEditView = !m_useEditView;
		 break;

	case SDLK_1:
		//newLevel( vec2f( 2400, 3000 ) );		
		newLevel( vec2f( 800, 1200 ) );		
		break;

	case SDLK_2:
		newLevel( vec2f( 4800, 6000 ) );		
		break;

	case SDLK_3:
		newLevel( vec2f( 9600, 12000 ) );		
		break;
	}
}

void Editor::newLevel( vec2f size )
{
	delete m_level;

	m_level = new Cavern();
	m_level->m_mapSize = size;
	frameView();
}

void Editor::update( float dt )
{
	vec2f scrollDir( 0.0f, 0.0f );

	// Continuous (key state) keys
	Uint8 *keyState = SDL_GetKeyState( NULL );
	if ((keyState[SDLK_LEFT]) && (!keyState[SDLK_RIGHT]))
	{
		scrollDir.x = -1.0;
	}
	else if ((!keyState[SDLK_LEFT]) && (keyState[SDLK_RIGHT]))
	{
		scrollDir.x = 1.0;
	}
	if ((keyState[SDLK_UP]) && (!keyState[SDLK_DOWN]))
	{
		scrollDir.y = 1.0;
	}
	else if ((!keyState[SDLK_UP]) && (keyState[SDLK_DOWN]))
	{
		scrollDir.y = -1.0;
	}

	// update view
	if (m_useEditView)
	{
		m_viewport += scrollDir * _TV( 0.6f ) * m_viewsize.x * dt;
	}
	else
	{
		m_gameview += scrollDir * _TV( 200.0f ) * dt;
	}
}

void Editor::loadShapes( const char *filename )
{
	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );

	if (!xmlDoc->LoadFile() ) {
		printf("ERR! Can't load %s\n", filename );
	}

	TiXmlElement *xShapeList, *xShape;
	//TiXmlNode *xText;

	xShapeList = xmlDoc->FirstChildElement( "ShapeList" );
	assert( xShapeList );

	xShape = xShapeList->FirstChildElement( "Shape" );
	while (xShape) 
	{
		Shape *shp = new Shape();
		
		shp->name = xShape->Attribute("name");
		shp->m_collide = (!stricmp( xShape->Attribute("collide"), "true" ));
		shp->m_pattern = (!stricmp( xShape->Attribute("pattern"), "true" ));

		vec2f st0, st1;
		sscanf( xShape->Attribute("rect"), "%f,%f,%f,%f", 
				&(st0.x), &(st0.y),
				&(st1.x), &(st1.y) );		
	
		shp->m_size = vec2f( st1.x - st0.y, st1.y - st0.y );

		// get texture and adjust sts
		int texw, texh;
		shp->m_texId = getTexture( std::string("gamedata/") + std::string(xShape->Attribute("map")),
						&texw, &texh );
		shp->st0 = st0 / (float)texw;
		shp->st1 = st1 / (float)texh;

		m_shapes.push_back( shp );
		xShape = xShape->NextSiblingElement( "Shape" );
	}
	
	// done
	xmlDoc->Clear();
	delete xmlDoc;
}