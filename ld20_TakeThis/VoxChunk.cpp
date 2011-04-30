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
    if ((x<0)||(x>=m_xSize)||
        (y<0)||(y>=m_ySize)||
        (z<0)||(z>=m_zSize)) return BLOCKER_COL;
    
    return m_voxdata[ index(x,y,z) ];
}

VoxChunk *VoxChunk::loadCSVFile( const char *filename )
{
    VoxChunk *chunk = NULL;

    FILE* fp = fopen(filename, "rt");
    if (!fp) 
    {
        printf("Couldn't open voxtile %s\n", filename );
        return NULL;
    }
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
    
    printf("Loaded %s, size(%dx%dx%d)\n", filename, xsize, ysize, zsize);

    return chunk;
}

bool VoxChunk::isVisibleCol( GLuint col )
{
    return ((col & 0xff) && (col != FILLER_COL) && (col != BLOCKER_COL));
}

bool VoxChunk::isClearCol( GLuint col )
{
    return ((col & 0xff) && (col != BLOCKER_COL));
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
    const float voxSz = 1.0 / WORLD_TILE_SIZE;
    //const float voxSz = 1.0;
    
    for (int i=0; i < m_xSize; i++)
    {
        for (int j=0; j < m_ySize; j++)
        {
            for (int k=0; k < m_zSize; k++)
            {
                GLuint col = getVoxel(i,j,k);
                
                GLubyte cr, cg, cb;
                cr = (col >> 24) & 0xff;
                cg = (col >> 16) & 0xff;
                cb = (col >> 8) & 0xff;
                
                
                // anything there?
                if (isVisibleCol(col))
                {
                    // yep...
                    
                    // left neighbor?
                    if (!isClearCol( getVoxel(i-1,j,k)))
                    {
                        // Left face
                        VoxVert v;

                        v.m_col[0] = cr;
                        v.m_col[1] = cg;
                        v.m_col[2] = cb;
                        
                        //v.m_col[0]=255;
                        //v.m_col[1]=0;
                        //v.m_col[2]=0;
                        
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
                    
                    // right neighbor?
                    if (!isClearCol( getVoxel(i+1,j,k))) 
                    {
                        // Right face
                        VoxVert v;
                        
                        v.m_col[0] = cr;
                        v.m_col[1] = cg;
                        v.m_col[2] = cb;
                        
                        //v.m_col[0]=0;
                        //v.m_col[1]=255;
                        //v.m_col[2]=255;
                        
                        v.m_pos.x = (i+1) * voxSz;
                        v.m_pos.y = j * voxSz;
                        v.m_pos.z = k * voxSz;
                        
                        m_triData.push_back( v );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        
                        m_triData.push_back( v + vec3f( 0, voxSz, voxSz ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        
                        
                    }

                    // front neighbor?
                    if (!isClearCol( getVoxel(i,j,k-1)))
                    {
                        // front face
                        VoxVert v;
                        
                        v.m_col[0] = cr;
                        v.m_col[1] = cg;
                        v.m_col[2] = cb;
                        
                        v.m_pos.x = i * voxSz;
                        v.m_pos.y = j * voxSz;
                        v.m_pos.z = k * voxSz;
                        
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v );
                        
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( voxSz, voxSz, 0 ) );
                    }

                    // back neighbor?
                    if (!isClearCol( getVoxel(i,j,k+1)))
                    {
                        // back face
                        VoxVert v;
                        
                        v.m_col[0] = cr;
                        v.m_col[1] = cg;
                        v.m_col[2] = cb;
                        
                        v.m_pos.x = i * voxSz;
                        v.m_pos.y = j * voxSz;
                        v.m_pos.z = (k+1) * voxSz;
                        
                        
                        m_triData.push_back( v );
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        
                        m_triData.push_back( v + vec3f( voxSz, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( 0, voxSz, 0 ) );
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        
                    }
                    
                    // bottom neighbor?
                    if (!isClearCol( getVoxel(i,j-1,k)))
                    {
                        // bottom face
                        VoxVert v;
                        
                        v.m_col[0] = cr;
                        v.m_col[1] = cg;
                        v.m_col[2] = cb;
                        
                        v.m_pos.x = i * voxSz;
                        v.m_pos.y = j * voxSz;
                        v.m_pos.z = k * voxSz;
                        
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v );
                        
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v + vec3f( voxSz, 0, voxSz ) );
                    }

                    // top neighbor?
                    if (!isClearCol( getVoxel(i,j+1,k)))
                    {
                        // front face
                        VoxVert v;
                        
                        v.m_col[0] = cr;
                        v.m_col[1] = cg;
                        v.m_col[2] = cb;
                        
                        v.m_pos.x = i * voxSz;
                        v.m_pos.y = (j+1) * voxSz;
                        v.m_pos.z = k * voxSz;
                        
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v );
                        
                        m_triData.push_back( v + vec3f( voxSz, 0, 0 ) );
                        m_triData.push_back( v + vec3f( 0, 0, voxSz ) );
                        m_triData.push_back( v + vec3f( voxSz, 0, voxSz ) );
                    }

                    
                }
            }
        }
    }

   
    numVerts = m_triData.size();
    return &(m_triData[0]);  
}


