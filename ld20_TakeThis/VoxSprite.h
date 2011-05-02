#ifndef VOX_SPRITE_H
#define VOX_SPRITE_H
//
//  VoxSprite.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/30/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include "VoxChunk.h"

// VoxSprite is a chunk with position/orientation

class VoxSprite
{
public:
    VoxSprite( VoxChunk *chunk );
    
    void draw( const matrix4x4f &modelview );
    
    void chooseRandomDir();
    vec3f getVelocity();
//protected:
    VoxChunk *m_chunk;
    
    vec3f m_pos;
    float m_angle;
    float m_xRot;
    float m_timeout;
    int m_triforce;
};


#endif