//
//  VoxSprite.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/30/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include "VoxSprite.h"

#include "random.h"

VoxSprite::VoxSprite( VoxChunk *chunk ) :
    m_pos(0.0, 0.0, 0.0),
    m_angle( 0 ),
    m_chunk( chunk )
{
    chooseRandomDir();
}

void VoxSprite::draw( const matrix4x4f &modelview )
{
    // Draw sprite
    matrix4x4f pxform, prot, mat;
    matrix4x4f center, centerI;
    center.Translate( -0.5,-0.5,-0.5 );
    centerI.Translate( 0.5, 0.5, 0.5 );
    
    // translate, snapped to voxels
    pxform.Translate( int(m_pos.x*WORLD_TILE_SIZE) /WORLD_TILE_SIZE, 
                     int(m_pos.y*WORLD_TILE_SIZE) / WORLD_TILE_SIZE, 
                     int(m_pos.z*WORLD_TILE_SIZE) /WORLD_TILE_SIZE);
    prot.RotateY( (m_angle+90) * (3.1415/180.0) );
    
    
    mat = center * prot * pxform * centerI * modelview;
    glLoadMatrixf( (GLfloat*)(&mat) );
    
    size_t spriteSz;
    VoxVert *sprVert = m_chunk->genTris( spriteSz );
    glVertexPointer( 3, GL_FLOAT, sizeof( VoxVert ), sprVert );
    glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof( VoxVert ), &(sprVert[0].m_col) );
    glDrawArrays( GL_TRIANGLES, 0, spriteSz );

}

void VoxSprite::chooseRandomDir()
{
    int d = int( randUniform() * 4.0 );
    
    m_angle = d * 90.0;
}

vec3f VoxSprite::getVelocity()
{
    float a = m_angle * (3.1415/180.0);
    
    return vec3f( cos(a), 0, sin(a) );
}