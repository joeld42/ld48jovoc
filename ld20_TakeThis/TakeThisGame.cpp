//
//  TakeThisGame.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//
#include <assert.h>

#include "TakeThisGame.h"

TakeThisGame *TakeThisGame::_singleton = NULL;

// From http://www.opengl.org/wiki/GluPerspective_code

void glhFrustumf2(matrix4x4f &matrix, 
                  float left, float right, float bottom, float top,
                  float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0 * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    matrix[0][0] = temp / temp2;
    matrix[0][1] = 0.0;
    matrix[0][2] = 0.0;
    matrix[0][3] = 0.0;
    matrix[1][0] = 0.0;
    matrix[1][1] = temp / temp3;
    matrix[1][2] = 0.0;
    matrix[1][3] = 0.0;
    matrix[2][0] = (right + left) / temp2;
    matrix[2][1] = (top + bottom) / temp3;
    matrix[2][2] = (-zfar - znear) / temp4;
    matrix[2][3] = -1.0;
    matrix[3][0] = 0.0;
    matrix[3][1] = 0.0;
    matrix[3][2] = (-temp * zfar) / temp4;
    matrix[3][3] = 0.0;
}

//matrix will receive the calculated perspective matrix.
//You would have to upload to your shader
// or use glLoadMatrixf if you aren't using shaders.
void glhPerspectivef2(matrix4x4f &matrix, float fovyInDegrees, float aspectRatio,
                      float znear, float zfar)
{
    float ymax, xmax;
    float temp, temp2, temp3, temp4;
    ymax = znear * tanf(fovyInDegrees * M_PI / 360.0);
    //ymin = -ymax;
    //xmin = -ymax * aspectRatio;
    xmax = ymax * aspectRatio;
    glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}


TakeThisGame::TakeThisGame()
{
    assert( _singleton == NULL );
    _singleton = this;
}


void TakeThisGame::init()
{
    printf("In init..\n" );
    chunk = VoxChunk::loadCSVFile( "gamedata/voxtiles/column.csv" );
    //chunk = VoxChunk::loadCSVFile( "gamedata/voxtiles/grass_corner.csv" );
    
    printf( "Loaded chunk, size is %d, %d, %d\n",
            chunk->m_xSize,
            chunk->m_ySize,
            chunk->m_zSize );
    
    ang = 0;
}

void TakeThisGame::shutdown()
{
    assert( _singleton );
    _singleton->_shutdown();
}

void TakeThisGame::_shutdown()
{
    // here .. release stuff
}

void TakeThisGame::updateSim( float dtFixed )
{
    ang += dtFixed * 20.0;
}

void TakeThisGame::updateFree( float dtRaw )
{
    
}

void TakeThisGame::redraw()
{
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glhPerspectivef2( m_proj, 90.0, 800.0/600.0, 0.001, 1000.0 );
    
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( (GLfloat*)(&m_proj) );
   
    matrix4x4f xlate;
    matrix4x4f rot;
    xlate.Translate( 0.0, -2, -20 );
    rot.RotateY( ang * 3.14/180.0 );
    
    m_modelview = rot * xlate;
               
    //m_modelview.Translate( 0.0, -2, -20 );
    
    
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( (GLfloat*)(&m_modelview) );
    
#if 1
    // DBG: draw axes
    glBegin( GL_LINES );
    
    glColor3f( 1.0, 0.0, 0.0 );
    glVertex3f( 0.0, 0.0, 0.0 );
    glVertex3f( 1.0, 0.0, 0.0 );
   
    glColor3f( 0.0, 1.0, 0.0 );
    glVertex3f( 0.0, 0.0, 0.0 );
    glVertex3f( 0.0, 1.0, 0.0 );
   
    glColor3f( 0.0, 0.0, 1.0 );
    glVertex3f( 0.0, 0.0, 0.0 );
    glVertex3f( 0.0, 0.0, 1.0 );
    
    glEnd();
#endif
    
    // dbg draw the chunk border
    glColor3f( 1.0, 1.0, 1.0 );
#if 1
    glBegin( GL_LINES );
    
    for (int y=0; y < 2; y++)
    {
        glVertex3f( 0.0, y*chunk->m_ySize, 0.0 );
        glVertex3f( chunk->m_xSize, y*chunk->m_ySize, 0.0 );
        
        glVertex3f( 0.0, y*chunk->m_ySize, chunk->m_zSize );
        glVertex3f( chunk->m_xSize, y*chunk->m_ySize, chunk->m_zSize );
        
        glVertex3f( 0.0, y*chunk->m_ySize, 0.0 );
        glVertex3f( 0.0, y*chunk->m_ySize, chunk->m_zSize );
        
        glVertex3f( chunk->m_xSize, y*chunk->m_ySize, 0.0 );
        glVertex3f( chunk->m_xSize, y*chunk->m_ySize, chunk->m_zSize );
        
    }
    
    for (int x=0; x < 2; x++)
    {
        glVertex3f(  chunk->m_xSize*x, 0.0, 0.0 );
        glVertex3f(  chunk->m_xSize*x, chunk->m_ySize, 0.0 );

        glVertex3f(  chunk->m_xSize*x, 0.0, chunk->m_zSize );
        glVertex3f(  chunk->m_xSize*x, chunk->m_ySize, chunk->m_zSize );
    }
    
    glEnd();
#endif
    
    // alright draw the chunk
    VoxVert *vertData;
    size_t numVerts;
    
    vertData = chunk->genTris( numVerts );
    
    glColor3f( 1.0, 1.0, 1.0 );
    glEnable( GL_CULL_FACE );
    
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof( VoxVert ), vertData );
    
    glEnableClientState( GL_COLOR_ARRAY );
    glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof( VoxVert ), &(vertData[0].m_col) );
    
    glDrawArrays( GL_TRIANGLES, 0, numVerts );
    //printf("Draw %d triangle verts\n", numVerts );
    
    glDisable( GL_VERTEX_ARRAY );
}
