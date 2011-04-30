//
//  VoxChunk.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include "VoxChunk.h"

VoxChunk::VoxChunk( int x, int y, int z ) :
    m_xSize( x ),
    m_ySize( y ),
    m_zSize( z )
{
    m_voxdata = new GLuint[ x*y*z ];
}

VoxChunk::~VoxChunk()
{
    delete [] m_voxdata;
}

size_t VoxChunk::index( int x, int y, int z ) const
{
    return (x*m_xSize*m_ySize)+(y*m_zSize) + z;
}

void VoxChunk::setVoxel( int x, int y, int z, GLuint col )
{
    m_voxdata[ index(x,y,z) ] = col;
}

GLuint VoxChunk::getVoxel( int x, int y, int z ) const
{
    return m_voxdata[ index(x,y,z) ];
}

VoxChunk *VoxChunk::loadCSVFile( const char *filename )
{
    VoxChunk *chunk = NULL;

    FILE* fp = fopen(filename, "rt");
    if (!fp) return false;
    
    // Read the dimensions
    int xsize, ysize, zsize;
    fscanf(fp, "%d,%d,%d\n", &xsize, &ysize, &zsize);
    chunk = new VoxChunk( xsize, ysize, zsize );
    
    // Read the data
    for (int y = ysize-1; y >= 0; y--)
    {
        for (int z = 0; z < zsize; z++)
        {
            for (int x = 0; x < xsize; x++)
            {
                int r, g, b, a;
                fscanf(fp, "#%02X%02X%02X%02X,", &r, &g, &b, &a);
                
                //printf( "%d %d %d -- %d %d %d %d\n", x, y, z, r, g, b, a);
                chunk->setVoxel( x, y, z, (r <<24) | (g<<16) | (b<<8) | a);
                
                if (x != xsize-1)
                    fscanf(fp, ",");
            }
            fscanf(fp, "\n");
        }
        fscanf(fp, "\n");
    }
    
    fclose(fp);

    return chunk;
}

bool VoxChunk::isClearCol( GLuint col )
{
    return ((col & 0xff) && (col != FILLER_COL) && (col != BLOCKER_COL));
}

VoxVert *VoxChunk::genTris( size_t &numVerts )
{
    // did we already do this?
    if (m_triData.size())
    {
        numVerts = m_triData.size();
        return &(m_triData[0]);
    }
    
    // nope. Generate voxels
    
    // size of a single voxel
    //const float voxSz = 1.0 / 8.0;
    const float voxSz = 1.0;
    
    for (int i=0; i < m_xSize; i++)
    {
        for (int j=0; j < m_ySize; j++)
        {
            for (int k=0; k < m_zSize; k++)
            {
                GLuint col = getVoxel(i,j,k);
                printf("%d %d %d color #%010X\n", i,j,k, col );
                
                
                // anything there?
                if (isClearCol(col))
                {
                    // yep...
                    
                    // no left neighbor?
                    if ( (i > 0) && (!isClearCol( getVoxel(i-1,j,k))) )
                    {
                        // Left face
                        VoxVert v;
                        
                        v.m_col.r = 0.5;
                        v.m_col.g = 0.6;
                        v.m_col.b = 0.5;
                        
                        v.m_pos.x = i * voxSz;
                        v.m_pos.y = j * voxSz;
                        v.m_pos.z = k * voxSz;
                        
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v );
                        
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, voxSz ) );
                        
                    }
                    
                    // no right neighbor?
                    if ( (i < m_xSize-1) && (!isClearCol( getVoxel(i+1,j,k))) )
                    {
                        // Right face
                        VoxVert v;
                        
                        v.m_col.r = 0.6;
                        v.m_col.g = 0.5;
                        v.m_col.b = 0.5;
                        
                        v.m_pos.x = i * voxSz;
                        v.m_pos.y = j * voxSz;
                        v.m_pos.z = k * voxSz;
                        
                        m_triData.push_back( v );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        
                        m_triData.push_back( v + vec3f( 0, voxSz, voxSz ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        
                        
                    }
                }
            }
        }
    }
    
    for (int i=0; i < 10; i++)
    {
        printf("m_triData[%d].m_pos %f %f %f\n", i, 
               m_triData[i].m_pos.x,
               m_triData[i].m_pos.y,
               m_triData[i].m_pos.z );
    }
   
    numVerts = m_triData.size();
    return &(m_triData[0]);  
}


