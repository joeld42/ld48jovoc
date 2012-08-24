//
//  template_game.cpp
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

#include "template_game.h"

TemplateGame::TemplateGame()
{
    
}

void TemplateGame::init()
{
    // Load texture
    m_simpleTex = LoadImagePNG( gameDataFile("", "simpletex.png" ).c_str() );

    // Load font
    m_fontImg = LoadImagePNG( gameDataFile("", "nesfont.png" ).c_str() );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );    
    
    m_cube = make_cube();
//    setColorConstant( m_cube, vec4f( 1.0, 1.0, 1.0 ) );
    
    m_basicShader = loadShader( "template.Plastic" );
    
    m_rotate = 0.0;
    
    glEnable( GL_DEPTH_TEST );
    
}

void TemplateGame::updateSim( float dtFixed )
{
    // Update stuff with a fixed timestep
    m_rotate += (M_PI/180.0) * (10.0) * dtFixed;
}

void TemplateGame::updateFree( float dtRaw )
{
    // Update stuff with free timestep
}

void TemplateGame::redraw()
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

void TemplateGame::shutdown()
{
    // shutdown stuff
}

void TemplateGame::mouseMotion( float x, float y )
{
    
}

// For instantanious button effects 
void TemplateGame::mouseButton( SDL_MouseButtonEvent &btnEvent )
{
    
}

// For "continuous" button events
void TemplateGame::updateButtons( unsigned int btnMask )
{
    
}

void TemplateGame::keypress( SDLKey &key )
{
    switch(key)
    {
        case 'o':
            // orbit
//            doSpin = !doSpin;
//            printf("Orbit %s\n", doSpin?"true":"false" );
            break;

        default:
            break;
    }
}

    // Draw 3d stuff
void TemplateGame::_draw3d()
{    
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
void TemplateGame::_draw2d()
{
    // disable shaders (TODO: make a text shader so this ports to ES2 easier)
    glUseProgram(0);
    
    m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
    m_nesFont->drawStringCentered( 400, 580, "Hello {{Project}}" );    
    m_nesFont->renderAll();
    m_nesFont->clear();

}

void TemplateGame::_drawMesh( QuadBuff<DrawVert> *mesh )
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