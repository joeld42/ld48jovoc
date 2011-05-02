#ifndef VOXCHUNK_H
#define VOXCHUNK_H

//
//  VoxChunk.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//


#include <string>
#include <vector>

#include <prmath/prmath.hpp>

#include "VoxChunk.h"
#include "GLee.h"

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

// How many voxels fit into a unit cube on a side
#define WORLD_TILE_SIZE (16.0)

// Filler acts like it's filled for drawing
// neighbors but isn't drawn itself. Used for
// internal voxels that will never be visible.
#define FILLER_COL (0xFF00FFFF)

// Blocker acts like it's filled for collision
// purposes but is drawn as if empty. for example
// for a footbridge with gaps in it.
#define BLOCKER_COL (0xFFFF00FF)

// vertex data for vox prims
struct VoxVert
{
    vec3f m_pos;
    unsigned char m_col[3];
    
    // Adding a vec3 adds to pos
    VoxVert operator+( const vec3f &other )
    {
        VoxVert v2( *this );
        v2.m_pos += other;
        return v2;
    }
};

// A chunk of voxels... a tile or sprite
class VoxChunk
{
public:
    VoxChunk( int x, int y, int z );
    ~VoxChunk();
    
    
    size_t index( int x, int y, int z ) const;
    
    void setVoxel( int x, int y, int z, GLuint col );
    GLuint getVoxel( int x, int y, int z ) const;
    
    static VoxChunk *loadCSVFile( const char *filename );
    
    // generates a bunch of triangles for the chunk.
    // caches them for next time
    // numVerts is the number of verts generated.
    VoxVert *genTris( size_t &numVerts );
    
    // utils
    bool isClearCol( GLuint c );
    bool isVisibleCol( GLuint c );
    
//protected:
    int m_xSize;
    int m_ySize;
    int m_zSize;
    
    GLuint *m_voxdata;
    
    std::vector<VoxVert> m_triData;
    std::string m_chunkName;
    
    // HACK: pixel-height is not working, 
    // just hack it for now
    float m_floorHeight;
    
};

#endif
