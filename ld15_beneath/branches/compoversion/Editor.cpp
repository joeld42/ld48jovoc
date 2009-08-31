#include <Common.h>
#include <Editor.h>

#include <tweakval.h>
#include <gamefontgl.h>

#include <tinyxml.h>

Editor::Editor( GLuint fntID, std::vector<Shape*> &shapes ) : 
		m_shapes( shapes )
{
	m_fntFontId = fntID;
	m_level = NULL;
	m_showHelp = true;

	m_viewport = vec2f( 0, 0 );
	m_viewsize = vec2f( 800, 600 );

	m_activeShape = NULL;

	m_gameview = vec2f( 0, 0 );
	m_useEditView = true;
	m_tool = Tool_PLACE;
	m_sort = 1000;
	
	isInit = false;
	drawLine = false;
	currSegType = SegType_COLLIDE;

	if (m_shapes.size())
	{
		m_actShapeIndex = 0;

		Shape *newShape = new Shape();
		*newShape = *m_shapes[ m_actShapeIndex ];
		m_activeShape = newShape;
		
	}

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

void Editor::initEditor()
{
	float iconHite = 32.0 / 512.0;
	iconSpawnPoint = Shape::simpleShape( "gamedata/ed_icons.png" );
	iconSpawnPoint->m_size = vec2f( 32, 32 );
	iconSpawnPoint->st1.y = iconHite;
	iconSpawnPoint->st0.y = 0;
}

void Editor::redraw()
{

	if (!isInit)
	{
		initEditor();
		isInit = true;
	}

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


	// at this point we need a level to draw
	if (m_level)
	{
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
			pseudoOrtho2D( m_gameview.x, m_gameview.x + 800,
				m_gameview.y, m_gameview.y + 600 );
		}

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		
		// Draw the level grid
		float zval = _TV( -0.5f );
		glColor3f( gridColor.r, gridColor.g, gridColor.b );
		glDisable( GL_TEXTURE );

		glLineWidth( 2.0f );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0, 0.0, zval );
		glVertex3f( m_level->m_mapSize.x, 0.0, zval );
		glVertex3f( m_level->m_mapSize.x, m_level->m_mapSize.y, zval );
		glVertex3f( 0.0, m_level->m_mapSize.y, zval );	
		glEnd();


		// draw grid lines
		glLineWidth( 1.0f );
		glBegin( GL_LINES );
		for (float xpos=800; xpos < m_level->m_mapSize.x; xpos += 800.0)
		{
			glVertex3f( xpos, 0.0, zval );
			glVertex3f( xpos, m_level->m_mapSize.y, zval );
		}

		for (float ypos = 600; ypos < m_level->m_mapSize.y; ypos += 600.0)
		{
			glVertex3f( 0.0, ypos, zval);
			glVertex3f( m_level->m_mapSize.x, ypos, zval );
		}
		glEnd();

		glLineWidth( 1.0f );
		glEnable( GL_LINE_STIPPLE );
		glLineStipple( 1, 0xAAAAAAAA );
		glBegin( GL_LINES );
		for (float xpos=100; xpos < m_level->m_mapSize.x; xpos += 100.0)
		{
			glVertex3f( xpos, 0.0, zval );
			glVertex3f( xpos, m_level->m_mapSize.y, zval );
		}

		for (float ypos = 100; ypos < m_level->m_mapSize.y; ypos += 100.0)
		{
			glVertex3f( 0.0, ypos,zval );
			glVertex3f( m_level->m_mapSize.x, ypos, zval );
		}
		glEnd();
		glDisable( GL_LINE_STIPPLE );		

		// draw the "game view" extents
		if (m_useEditView)
		{
			glLineWidth( 2.0f );
			glColor3f( 0.7f, 0.4f, 0.7f );

			glPushMatrix();
			glTranslated( m_gameview.x, m_gameview.y, 0.0 );

			glBegin( GL_LINE_LOOP );
			glVertex3f( 0.0, 0.0, 0.0 );
			glVertex3f( 800, 0.0, 0.0 );
			glVertex3f( 800, 600, 0.0 );
			glVertex3f( 0.0, 600, 0.0 );			
			glEnd();

			glPopMatrix();
		}

		

		// draw the level
		m_level->draw();
	
		// draw icons		
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
		glEnable( GL_ALPHA_TEST );		
		glAlphaFunc( GL_GREATER, 0.5 );

		iconSpawnPoint->pos = m_level->m_spawnPoint;
		iconSpawnPoint->pos.y += 16.0;
		iconSpawnPoint->drawBraindead();
								
		
		if (drawLine)
		{
			// drawing a line (for collision)
			glDisable( GL_TEXTURE_2D );
			
			vec3f col = segColor( currSegType );
			glColor3f( col.r, col.g, col.b );

			glLineWidth( 5 );
			glBegin( GL_LINES );
			glVertex3f( m_dragStart.x, m_dragStart.y, 0.0 );
			glVertex3f( m_mousePos.x, m_mousePos.y, 0.0 );
			glEnd();

			glColor3f( 1.0f, 1.0f, 1.0f );
		} 
		else if (m_activeShape)
		{
			// draw the active shape (aka "cursor")		
			glEnable( GL_TEXTURE_2D );
			if (m_activeShape->blendMode == Blend_OFF)
			{
				glDisable( GL_BLEND );
				glEnable( GL_ALPHA_TEST );
				glAlphaFunc( GL_GREATER, 0.5 );
			}
			else
			{
				glEnable( GL_BLEND );
				glDisable( GL_ALPHA_TEST );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
			}

			m_activeShape->sortNum = m_sort;
			m_activeShape->drawBraindead();			
		}				
	
	}

	// draw regular collision lines
	if (m_level)
	{
		glDisable( GL_TEXTURE_2D );
		glColor3f( 1.0f, 1.0f, 0.5f );
		glLineWidth( 3 );
			
		glBegin( GL_LINES );
		for (int i=0; i < m_level->m_collision.size(); i++)
		{
			Segment &s = m_level->m_collision[i];
			vec3f col = segColor( s.segType );
			glColor3f( col.r, col.g, col.b );

			glVertex3f( s.a.x, s.a.y, 0.0 );
			glVertex3f( s.b.x, s.b.y, 0.0 );		
			
		}		
		glEnd();
	}

	// now draw any text overlay		
	if (m_showHelp)
	{
		glColor3f( 1.0f, 1.0f, 1.0f );
		gfEnableFont( m_fntFontId, 20 );	
		gfBeginText();
		glTranslated( _TV(50), _TV(500), 0 );
		
		gfDrawString( "------[ EDITOR KEYS ]------\n" 
			          "h - toggle this help\n" 
					  "F2 - save level\n"
					  "F3 - load level\n"
	    			  "1,2,3 - New Level (small, med, large)\n" 					  
					  "f - frame view\n" 
					  "<,> - select brush\n" 
					  "TAB - toggle editor/closeup\n" 
					  "ARROWS - move view\n"
					  "a,z,s,x - change sort\n"
					  "o,p,brackets - brush size\n"
					  "k,l - zoom view\n" 
					  "8,9 - segment type\n"
					  "0 - spawn point\n"
					  "mousewheel - rotate brush\n"
					  "SPC, LMB - stamp brush\n"
					  "Drag RMB - draw segment\n"
					  );			
		gfEndText();
	}	

	// draw the active shape info
	if (m_activeShape)
	{
		glEnable( GL_TEXTURE_2D );		

		glColor3f( 1.0f, 1.0f, 1.0f );

		gfEnableFont( m_fntFontId, 20 );	
		gfBeginText();
		glTranslated( _TV(650), _TV(570), 0 );		
		
		gfDrawStringFmt( "%s (%d)",
				m_activeShape->name.c_str(),
				m_sort );
		gfEndText();

		vec3f col = segColor( currSegType );
		glColor3f( col.r, col.g, col.b );
		gfBeginText();
		glTranslated( _TV(650), _TV(550), 0 );		
				
		gfDrawString( SegTypeNames[ currSegType ] );

		gfEndText();
		
		glDisable( GL_TEXTURE_2D );
	}
}

void Editor::keypress( SDL_KeyboardEvent &key )
{
	switch( key.keysym.sym )
	{
	case SDLK_h:
		m_showHelp = !m_showHelp;		
		break;
	case SDLK_F2:
		if (m_level) m_level->saveLevel( "_editorLevel.xml" );
		break;

	case SDLK_F3:
		delete m_level;
		m_level = new Cavern();
		m_level->loadLevel( "_editorLevel.xml", m_shapes );
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

	case SDLK_8:
		if (currSegType == SegType_COLLIDE) currSegType = SegType_KILL;
		else currSegType--;
		break;
	
	case SDLK_9:
		if (currSegType == SegType_KILL) currSegType = SegType_COLLIDE;
		else currSegType++;
		break;

	case SDLK_a:
		m_sort += 10;
		break;

	case SDLK_z:
		m_sort -= 10;
		break;

	case SDLK_s:
		m_sort += 1;
		break;

	case SDLK_x:
		m_sort -= 1;
		break;

	case SDLK_0:
		if (m_level) 
		{
			m_level->m_spawnPoint = m_mousePos;
			//printf("Spawn Pos: %f %f\n", 
			//	m_level->m_spawnPoint.x,
			//	m_level->m_spawnPoint.y );
		}
		break;

	case SDLK_COMMA:
		{
			delete m_activeShape;

			if (m_actShapeIndex > 0 ) m_actShapeIndex--;
			else m_actShapeIndex = m_shapes.size()-1;						

			Shape *newShape = new Shape();
			*newShape = *m_shapes[ m_actShapeIndex ];
			//printf("Current shape: %d %s\n", m_actShapeIndex, newShape->name.c_str() );

			m_activeShape = newShape;
		}
		break;
	
	case SDLK_PERIOD:
		{
			delete m_activeShape;
			
			if (m_actShapeIndex < m_shapes.size()-1 ) m_actShapeIndex++;
			else m_actShapeIndex = 0;
			
			Shape *newShape = new Shape();
			*newShape = *m_shapes[ m_actShapeIndex ];
			//printf("Current shape: %d %s\n", m_actShapeIndex, newShape->name.c_str() );
			m_activeShape = newShape;
		}
		break;	
	case SDLK_SPACE:
		stampActiveShape();		
		break;	
	
	case SDLK_RIGHTBRACKET:		
		m_activeShape->m_size *= 2.0;
		break;

	case SDLK_LEFTBRACKET:
		m_activeShape->m_size *= 0.5;
		break;

	case SDLK_o:
		m_activeShape->m_size *= 0.9;
		break;

	case SDLK_p:		
		m_activeShape->m_size *= 1.1;
		break;

	case SDLK_k:
		m_viewsize *= 0.75;
		break;

	case SDLK_l:
		m_viewsize *= 1.25;
		break;

	}
}

void Editor::mousepress( SDL_MouseButtonEvent &mouse )
{
	if (mouse.type == SDL_MOUSEBUTTONDOWN)
	{
		if ( mouse.button == 1)
		{
			stampActiveShape();
		}
		else if (mouse.button == 4)
		{
			// wheel up
			m_activeShape->angle += 15.0;
		}	
		else if (mouse.button == 5)
		{
			// wheel down
			m_activeShape->angle -= 15.0;
		} 
		
		if ( mouse.button == 3)
		{
			
			m_dragStart = m_mousePos;
			drawLine = true;
		}
	}
	else // mouse up
	{
		if (( mouse.button == 3) && (drawLine))
		{
			m_level->addSegment( m_dragStart, m_mousePos, currSegType );
			drawLine = false;
		}
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

	// update mouse
	int mx, my;
	SDL_GetMouseState( &mx, &my );
	my = 600-my;
	if (m_activeShape)
	{
		if (m_useEditView)
		{
			m_activeShape->pos.x = m_viewport.x + ((float)mx / 800.0)*m_viewsize.x;
			m_activeShape->pos.y = m_viewport.y + ((float)my / 600.0)*m_viewsize.y;			
		}
		else
		{
			m_activeShape->pos.x = mx + m_gameview.x;
			m_activeShape->pos.y = my + m_gameview.y;
		}
		
		// remember this pos for otehr use
		m_mousePos = m_activeShape->pos;

		// offset so active shape is centered
		m_activeShape->pos.x -= m_activeShape->m_size.x/2;
		m_activeShape->pos.y -= m_activeShape->m_size.y/2;
	}
}


void Editor::stampActiveShape()
{
	// drop a copy of the shape on the map
	if ((m_level) && (m_activeShape))
	{
		Shape *mapShape = new Shape();
		*mapShape = *m_activeShape;
		mapShape->sortNum = m_sort;
		m_level->addShape( mapShape );
	}
}

vec3f Editor::segColor( int type )
{
	switch (type)
	{
	case SegType_COLLIDE:
		return vec3f( 1.0, 1.0, 0.0 );
		break;
	case SegType_DIALOGUE:
		return vec3f( 0.0, 0.0, 1.0 );
		break;
	case SegType_KILL:
		return vec3f( 1.0, 0.0, 0.0 );
		break;
	default:
		return vec3f( 1.0, 1.0, 1.0 );
		break;
	}
}