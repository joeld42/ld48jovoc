#include <map>

#include <tweakval.h>
#include <gamefontgl.h>

#include <Common.h>
#include <BeneathGame.h>

BeneathGame::BeneathGame() :
	m_isInit( false ),
	m_editor( NULL ),
	m_gameState( GameState_MENU )
{
}


void BeneathGame::update( float dt )
{
	if (m_gameState==GameState_GAME)
	{
		game_update( dt );
	}
	else if (m_gameState==GameState_EDITOR)
	{
		if (m_editor) m_editor->update( dt );
	}
}

void BeneathGame::updateSim( float dt )
{
	if (m_gameState==GameState_GAME)
	{
		game_updateSim( dt );
	}
}

void BeneathGame::game_updateSim( float dt )
{
	vec2f playerDir( 0.0f, 0.0f );

	// Continuous (key state) keys
	Uint8 *keyState = SDL_GetKeyState( NULL );
	if ((keyState[SDLK_LEFT]) && (!keyState[SDLK_RIGHT]))
	{
		playerDir.x = -1.0;
	}
	else if ((!keyState[SDLK_LEFT]) && (keyState[SDLK_RIGHT]))
	{
		playerDir.x = 1.0;
	}
	if ((keyState[SDLK_UP]) && (!keyState[SDLK_DOWN]))
	{
		playerDir.y = 1.0;
	}
	else if ((!keyState[SDLK_UP]) && (keyState[SDLK_DOWN]))
	{
		playerDir.y = -1.0;
	}

	// update player
	m_player->pos += playerDir * _TV( 100.0f ) * dt;
}

// "as fast as possible" update for effects and stuff
void BeneathGame::game_update( float dt )
{
//	printf("Update %3.2f\n", dt );
}

void BeneathGame::init()
{
	// Load the font image
	ILuint ilFontId;
	ilGenImages( 1, &ilFontId );
	ilBindImage( ilFontId );		
	
	if (!ilLoadImage( (ILstring)"gamedata/andelemo.png" )) {
		printf("Loading font image failed\n");
	}
	
	// Make a GL texture for it
	m_glFontTexId = ilutGLBindTexImage();
	m_fntFontId = gfCreateFont( m_glFontTexId );

	// A .finfo file contains the metrics for a font. These
	// are generated by the Fontpack utility.
	gfLoadFontMetrics( m_fntFontId, "gamedata/andelemo.finfo");

	printf("font has %d chars\n", 
		gfGetFontMetric( m_fntFontId, GF_FONT_NUMCHARS ) );					

	// Now load game shapes
	m_player = Shape::simpleShape( "gamedata/player.png" );
	m_player->pos = vec2f( 300, 200 );
}
	
void BeneathGame::redraw()
{	
	if (!m_isInit)
	{
		m_isInit = true;
		init();
	}
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	pseudoOrtho2D( 0, 800, 0, 600 ) ;


	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	if (m_gameState == GameState_GAME )
	{
		game_redraw();
	}
	else if (m_gameState == GameState_EDITOR )
	{
		if (m_editor)
		{
			m_editor->redraw();
		}
	}
	else if (m_gameState == GameState_MENU )
	{
		glClearColor( _TV( 0.1f ), _TV(0.2f), _TV( 0.4f ), 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		
		// Title text
		gfEnableFont( m_fntFontId, 32 );	
		gfBeginText();
		glTranslated( _TV(260), _TV(500), 0 );
		gfDrawString( "The World Beneath" );
		gfEndText();

		// Bottom text
		gfEnableFont( m_fntFontId, 20 );	
		gfBeginText();
		glTranslated( _TV(180), _TV(10), 0 );
		gfDrawString( "LudumDare 15 - Joel Davis (joeld42@yahoo.com)" );
		gfEndText();
	}
}

void BeneathGame::game_redraw()
{
	glClearColor( _TV( 0.2f ), _TV(0.2f), _TV( 0.3f ), 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Draw player shape
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		

	m_player->drawBraindead();
}

void BeneathGame::keypress( SDL_KeyboardEvent &key )
{
	if (m_gameState==GameState_GAME)
	{
		game_keypress( key );
	}
	else if (m_gameState==GameState_EDITOR)
	{
		if (m_editor) m_editor->keypress( key );		
	}
	else if (m_gameState==GameState_MENU)
	{
		switch( key.keysym.sym )
		{
		case SDLK_SPACE:
			m_gameState = GameState_GAME;
			break;

		case SDLK_F8:
			{
				// switch to editor
				m_gameState = GameState_EDITOR;
				if (!m_editor)
				{
					m_editor = new Editor( m_fntFontId );
					m_editor->loadShapes( "gamedata/shapes_Test.xml" );
				}
			}
			break;
		}
	}
}

void BeneathGame::mouse( SDL_MouseButtonEvent &mouse )
{
	if (m_gameState == GameState_EDITOR)
	{
		if (m_editor) m_editor->mousepress( mouse );
	}
}

void BeneathGame::game_keypress( SDL_KeyboardEvent &key )
{
}


// don't call this every frame or nothin'..
typedef std::map<std::string,GLuint> TextureDB;
TextureDB g_textureDB;
GLuint getTexture( const std::string &filename, int *xsize, int *ysize )
{
	GLuint texId;

	// See if the texture is already loaded
	TextureDB::iterator ti;
	ti = g_textureDB.find( filename );
	if (ti == g_textureDB.end() )
	{
		// Load the font image
		ILuint ilImgId;
		ilGenImages( 1, &ilImgId );
		ilBindImage( ilImgId );		
	
		if (!ilLoadImage( (ILstring)filename.c_str() )) {
			printf("Loading font image failed\n");
		}
		printf("Loaded Texture %s\n", filename.c_str() );
	
		// Make a GL texture for it
		texId = ilutGLBindTexImage();		

		// and remember it
		g_textureDB[ filename ] = texId;
	}
	else
	{
		printf("Texture %s already loaded\n", filename.c_str() );

		// found the texture
		texId = (*ti).second;
	}

	// now get the size if they asked for it
	if ((xsize) && (ysize))
	{
		glBindTexture( GL_TEXTURE_2D, texId );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, xsize );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, ysize );
	}

	return texId;
}

void pseudoOrtho2D( double left, double right, double bottom, double top )
{
#if 1
	// for now, real ortho
	gluOrtho2D( left, right, bottom, top );
#else	
	float w2, h2;
	w2 = (right - left) /2;
	h2 = (top - bottom) /2;
	float nv = _TV( 1.0f );
	glFrustum( -w2, w2, -h2, h2, nv, _TV(5.0f) );
	gluLookAt( left + w2, bottom + h2, nv,
			   left + w2, bottom + h2, 0.0,
			   0.0, 1.0, 0.0 );
#endif
}