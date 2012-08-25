//
//  evoword_game.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <prmath/prmath.hpp>

#include <font.h>
#include <gamedata.h>
#include <useful.h>
#include <shader.h>

// offsetof() gives a warning about non-POD types with xcode, so use these old
// school macros. This is OK because while VertType is non-POD, it doesn't have
// a vtable so these still work.
#define offset_d(i,f)    (long(&(i)->f) - long(i))
#define offset_s(t,f)    offset_d((t*)1000, f)

#include "evoword_game.h"

EvoWordGame::EvoWordGame()
{
    
}

void EvoWordGame::init()
{
    // Load dictionary
    loadWordList(gameDataFile("", "2of12inf.txt" ).c_str() );
    
    // Load texture
    m_simpleTex = LoadImagePNG( gameDataFile("", "simpletex.png" ).c_str() );

    // Load font
    m_fontImg = LoadImagePNG( gameDataFile("", "nesfont.png" ).c_str() );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );    
    
    m_cube = make_cube();
//    setColorConstant( m_cube, vec4f( 1.0, 1.0, 1.0 ) );
    
    m_basicShader = loadShader( "evoword.Plastic" );
    
    m_rotate = 0.0;
    
    glEnable( GL_DEPTH_TEST );
    
    m_gamestate = GameState_MENU;
}

void EvoWordGame::updateSim( float dtFixed )
{
    // Update stuff with a fixed timestep
    m_rotate += (M_PI/180.0) * (10.0) * dtFixed;
}

void EvoWordGame::updateFree( float dtRaw )
{
    // Update stuff with free timestep
}

void EvoWordGame::redraw()
{
    glClearColor( 78.0/255.0, 114.0/255.0, 136.0/255.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // 3d stuff    
    glhPerspectivef2( m_proj, 40.0, 800.0/600.0, 0.1, 1000.0 );

    matrix4x4f xlate, rot;
    xlate.Translate( 0.0, 0.1, -3.0 );
    rot.RotateY( m_rotate );
    m_modelview = rot * xlate;
    
    m_modelviewProj = m_modelview * m_proj;

    // Draw 3D scene
    _draw3d();
    
    // set up 2D draw 
    glDisable( GL_VERTEX_ARRAY );
    
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    
    glEnable( GL_TEXTURE );
    glEnable( GL_TEXTURE_2D );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // 2d stuff
    // TODO: use shaders 
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    glOrtho( 0, 800, 0, 600, -1, 1 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    _draw2d();

}

void EvoWordGame::shutdown()
{
    // shutdown stuff
}

void EvoWordGame::mouseMotion( float x, float y )
{
    
}

// For instantanious button effects 
void EvoWordGame::mouseButton( SDL_MouseButtonEvent &btnEvent )
{
    
}

// For "continuous" button events
void EvoWordGame::updateButtons( unsigned int btnMask )
{
    
}

void EvoWordGame::keypress( SDLKey &key )
{
    if (m_gamestate == GameState_MENU)
    {
        switch(key)
        {
            case ' ':
                startGame();
                break;

            default:
                break;
        }
    }
    else m_gamestate = GameState_MENU;
}

    // Draw 3d stuff
void EvoWordGame::_draw3d()
{    
    // don't draw the scene if not playing
    if (m_gamestate==GameState_MENU) return;
    
    glEnable( GL_TEXTURE );
    glEnable( GL_TEXTURE_2D );

    // Set up basic shader
    glUseProgram( m_basicShader );    
    GLint mvp = glGetUniformLocation( m_basicShader, "matrixPMV");
    glUniformMatrix4fv( mvp, 1, 0, (GLfloat*)(&m_modelviewProj)  );
    
    glActiveTexture(GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_simpleTex.textureId );
    
    GLint paramTex = glGetUniformLocation( m_basicShader, "sampler_dif0" );
    glUniform1i( paramTex, 0 );        
    
    // Draw something
    _drawMesh( m_cube );
}

// Draw 2D stuff    
void EvoWordGame::_draw2d()
{
    // disable shaders (TODO: make a text shader so this ports to ES2 easier)
    glUseProgram(0);
    
    if (m_gamestate==GameState_MENU)
    {
        m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
        m_nesFont->drawStringCentered( 400, 580, "Press Start" );    
        m_nesFont->renderAll();
        m_nesFont->clear();
    }
    else if (m_gamestate==GameState_GAME)
    {
        // draw current word
        m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
        m_nesFont->drawStringCentered( 400, 100, m_currWord.c_str() );    
        m_nesFont->renderAll();
        m_nesFont->clear();
        
        // draw creature
        m_creature.draw( m_nesFont);
    }
}

void EvoWordGame::_drawMesh( QuadBuff<DrawVert> *mesh )
{
    // Buffer is already created, just bind it
    glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo() );

    // Bind buffer data
    glEnableVertexAttribArray( DrawVertAttrib_TEXCOORD );
    glVertexAttribPointer( DrawVertAttrib_TEXCOORD, 4, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_st ) );

    glEnableVertexAttribArray( DrawVertAttrib_POSITION );
    glVertexAttribPointer( DrawVertAttrib_POSITION, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_pos) );


    glEnableVertexAttribArray( DrawVertAttrib_NORMAL );
    glVertexAttribPointer( DrawVertAttrib_NORMAL, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_nrm) );

    glEnableVertexAttribArray( DrawVertAttrib_COLOR );
    glVertexAttribPointer( DrawVertAttrib_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_color) );

    // Draw it!
    glDrawArrays(GL_TRIANGLES, 0, mesh->size() );
}

#pragma mark - Game Stuff

void EvoWordGame::startGame()
{
    srand48( SDL_GetTicks() );
    
    // Pick a new startword
    int startWordNdx = (int)randUniform(1, m_startWords);
    printf("startWordNdx %d\n", startWordNdx );
    
    for (WordList::iterator wi = m_wordList.begin(); wi != m_wordList.end(); ++wi )
    {
        const std::string &w = wi->first;
        if (w.size() == 3)
        {
            startWordNdx--;
            if (startWordNdx==0)
            {
                m_currWord = w;
                break;
            }
        }
    }
    printf("CurrWord is %s", m_currWord.c_str() );

    m_creature.evolveCreature( m_currWord );
    
    // Start playing
    m_gamestate = GameState_GAME;
}

#pragma mark - Word Stuff

void EvoWordGame::loadWordList( const char *wordlist )
{
	FILE *fp = fopen( wordlist, "rt" );
	char word[100], clean_word[100];
    printf ("Word list: %s\n", wordlist );
    
    m_startWords = 0;
	while(!feof(fp))
	{
		fscanf( fp, "%s", word );
		char *ch2 = clean_word;
		char lastchar = 'z';
		for (char *ch=word; *ch; ++ch)
		{
			// remove %'s (indicates plurals)..
			// remove u's following q, for gameplay purposes
			// q is "qu"
			if ((*ch!='%') && ( (*ch!='u') || (lastchar!='q') ) )
			{
				*ch2 = toupper(*ch);
				ch2++;
			}
			lastchar = *ch;
		}
		*ch2 = '\0';
		
		// add word
		std::string strword(clean_word );
		m_wordList[strword] = true;
        
        // If this is a startword, count it
        if (strword.size()==3) m_startWords++;
	}
    
	// hack? easter egg?
	m_wordList["LUDUM"] = true;
	m_wordList["JOVOC"] = true;
    
	printf( "Loaded %lu words, %d startWords\n", m_wordList.size(), m_startWords );
}

bool EvoWordGame::isWord( const std::string &word )
{
    WordList::iterator wi = m_wordList.find( word );
    return (wi != m_wordList.end());
}
