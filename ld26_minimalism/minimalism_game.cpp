//
//  minimalism_game.cpp
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
#include <load_obj.h>

// offsetof() gives a warning about non-POD types with xcode, so use these old
// school macros. This is OK because while VertType is non-POD, it doesn't have
// a vtable so these still work.
#define offset_d(i,f)    (long(&(i)->f) - long(i))
#define offset_s(t,f)    offset_d((t*)1000, f)

#include "minimalism_game.h"

matrix4x4f LookAt2(const vec3f& target, const vec3f& view, const vec3f& up)
{
    matrix4x4f m44;

    vec3f z = Normalize(target - view);
    vec3f x = Normalize(CrossProduct(z,up));
    vec3f y = CrossProduct(x, z);

    m44[0][0] = x.x;
    m44[1][0] = x.y;
    m44[2][0] = x.z;
    m44[3][0] = DotProduct(x,view);
    m44[0][1] = y.x;
    m44[1][1] = y.y;
    m44[2][1] = y.z;
    m44[3][1] = DotProduct(y,view);
    m44[0][2] = -z.x;
    m44[1][2] = -z.y;
    m44[2][2] = -z.z;
    m44[3][2] = DotProduct(z,view);
    m44[0][3] = 0;
    m44[1][3] = 0;
    m44[2][3] = 0;
    m44[3][3] = 1;

    return m44;
}

BlocksGame::BlocksGame()
{
    
}

void BlocksGame::init()
{
    // Load texture
    m_simpleTex = LoadImagePNG( gameDataFile("", "simpletex.png" ).c_str() );

    // Load font
    m_fontImg = LoadImagePNG( gameDataFile("", "nesfont.png" ).c_str() );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );    
    
    m_cube = make_cube();
//    setColorConstant( m_cube, vec4f( 1.0, 1.0, 1.0 ) );
    
//    m_groundTile = load_obj( gameDataFile("", "ground_tile.obj").c_str() );
    m_groundTile = load_obj( gameDataFile("", "letter_f.obj").c_str() );

    m_basicShader = loadShader( "minimalism.Plastic" );
    
    m_rotate = 0.0;
    m_testval = 0.0;
    m_useLookat = true;

    glEnable( GL_DEPTH_TEST );
    
}

void BlocksGame::updateSim( float dtFixed )
{
    // Update stuff with a fixed timestep
    m_rotate += (M_PI/180.0) * (30.0) * dtFixed;

    m_testval = sin( m_rotate * (M_PI/180.0) * 50.0 );
}

void BlocksGame::updateFree( float dtRaw )
{
    // Update stuff with free timestep
}

void BlocksGame::redraw()
{
//    glDisable( GL_DEPTH_TEST );

    glClearColor( 78.0/255.0, 114.0/255.0, 136.0/255.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // 3d stuff    
    glhPerspectivef2( m_proj, 90.0, 800.0/600.0, 0.001, 1000.0 );

//    printf( "testval is %f\n", m_testval);
//    m_view.LookAt(vec3f(0.0, 0.0, 0.0), vec3f(0.0, 0.1, m_testval), vec3f( 0.0, 1.0, 0.0));
//    m_view.Inverse(true); // doing something weird here so i have to invert the lookat??
    m_view = LookAt2(vec3f(0.0, 0.0, 0.0), vec3f(m_testval*0.5, 0.1, m_testval), vec3f( 0.0, 1.0, 0.0));

    matrix4x4f xlate, rot;
    xlate.Translate( 0.0, 0.0, -m_testval );
    rot.RotateY( m_rotate );
    rot.Identity();

#if 0
    printf(" Lookat %3.2f %3.2f %3.2f %3.2f\n"
            "        %3.2f %3.2f %3.2f %3.2f\n"
            "        %3.2f %3.2f %3.2f %3.2f\n"
            "        %3.2f %3.2f %3.2f %3.2f\n",
            m_view.m16[0],m_view.m16[1],m_view.m16[2],m_view.m16[3],
            m_view.m16[4],m_view.m16[5],m_view.m16[6],m_view.m16[7],
            m_view.m16[8],m_view.m16[9],m_view.m16[10],m_view.m16[11],
            m_view.m16[12],m_view.m16[13],m_view.m16[14],m_view.m16[15] );
#endif


    m_view2 = xlate * rot;

#if 0
    printf(" view2 %3.2f %3.2f %3.2f %3.2f\n"
           "        %3.2f %3.2f %3.2f %3.2f\n"
           "        %3.2f %3.2f %3.2f %3.2f\n"
           "        %3.2f %3.2f %3.2f %3.2f\n",
            m_view2.m16[0],m_view2.m16[1],m_view2.m16[2],m_view2.m16[3],
            m_view2.m16[4],m_view2.m16[5],m_view2.m16[6],m_view2.m16[7],
            m_view2.m16[8],m_view2.m16[9],m_view2.m16[10],m_view2.m16[11],
            m_view2.m16[12],m_view2.m16[13],m_view2.m16[14],m_view2.m16[15] );

    printf ("=====================\n");
#endif


    //m_model.Identity();
    m_model.RotateY( m_rotate );

    // Draw 3D scene
    _draw3d();
    
    // set up 2D draw     
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
        
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

void BlocksGame::_prepViewMat()
{
    // combine model, view and proj
    matrix4x4f view = m_useLookat?m_view:m_view2;
    //m_modelViewProj = m_proj * view;
    m_modelViewProj = m_model * view * m_proj;

    // prep for lighting
    // todo

    // set this to the shader
    GLint mvp = glGetUniformLocation( m_basicShader, "matrixPMV");
    glUniformMatrix4fv( mvp, 1, 0, (GLfloat*)(&m_modelViewProj)  );

}


void BlocksGame::shutdown()
{
    // shutdown stuff
}

void BlocksGame::mouseMotion( float x, float y )
{
    
}

// For instantanious button effects 
void BlocksGame::mouseButton( SDL_MouseButtonEvent &btnEvent )
{
    
}

// For "continuous" button events
void BlocksGame::updateButtons( unsigned int btnMask )
{
    
}

void BlocksGame::keypress( SDLKey &key )
{
    switch(key)
    {
        case 'o':
            // orbit
//            doSpin = !doSpin;
//            printf("Orbit %s\n", doSpin?"true":"false" );
            break;

        case 'a':
            m_useLookat = !m_useLookat;
            break;

        default:
            break;
    }
}

    // Draw 3d stuff
void BlocksGame::_draw3d()
{
    glEnable( GL_TEXTURE_2D );

    // Set up basic shader
    glUseProgram( m_basicShader );

    _prepViewMat();

    glActiveTexture(GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_simpleTex.textureId );
    
    GLint paramTex = glGetUniformLocation( m_basicShader, "sampler_dif0" );
    glUniform1i( paramTex, 0 );        
    
    // Draw something
    _drawMesh( m_groundTile );
//    for (int i=0; i < 5; i++)
//    {
//        for (int j=0; j < 5; j++)
//        {
//            _drawGroundTile(i,j);
//        }
//    }
}

void BlocksGame::_drawGroundTile( int x, int y)
{
    vec3f tileLoc( x - 4.0, 0.0, y - 4.5 );

    matrix4x4f matXlate;
    matXlate.Translate(tileLoc);

    matrix4x4f matTile = m_proj * (m_model * m_view * matXlate);

    GLint mvp = glGetUniformLocation( m_basicShader, "matrixPMV");
    glUniformMatrix4fv( mvp, 1, 0, (GLfloat*)(&matTile)  );

    _drawMesh(m_groundTile);

}

// Draw 2D stuff    
void BlocksGame::_draw2d()
{
    // disable shaders (TODO: make a text shader so this ports to ES2 easier)
    glUseProgram(0);
    
    m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
    m_nesFont->drawStringCentered( 400, 580, m_useLookat?"Lookat":"View2" );

    char buff[50];
    sprintf( buff, "%f", m_testval);
    m_nesFont->drawString( 50, 580, buff );

    matrix4x4f m = m_useLookat?m_view:m_view2;
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[0], m.m16[1], m.m16[2], m.m16[3] );
    m_nesFont->drawString( 50, 500, buff );
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[4], m.m16[5], m.m16[6], m.m16[7] );
    m_nesFont->drawString( 50, 490, buff );
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[8], m.m16[9], m.m16[10], m.m16[11] );
    m_nesFont->drawString( 50, 480, buff );
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[12], m.m16[13], m.m16[14], m.m16[15] );
    m_nesFont->drawString( 50, 470, buff );



    m_nesFont->renderAll();
    m_nesFont->clear();

}

void BlocksGame::_drawMesh( QuadBuff<DrawVert> *mesh )
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
