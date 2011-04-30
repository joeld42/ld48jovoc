//
//  MapRoom.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/30/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include <dirent.h>
#include <prmath/prmath.hpp>

#include "MapRoom.h"

std::map<std::string,VoxChunk*> MapRoom::m_tileset;


// note: zelda map size is 16x11
MapRoom::MapRoom( int x, int y, int z) :
    m_xSize(x),
    m_ySize(y),
    m_zSize(z)
{
    m_map.resize( x*y*z );
    
    // DBG: fill in some tiles
    VoxChunk *ground = m_tileset["overland_ground_open"];
    VoxChunk *ground2 = m_tileset["overland_ground_side"];
    VoxChunk *ground3 = m_tileset["overland_ground_corner"];
    VoxChunk *bush = m_tileset["bush"];
    VoxChunk *col1 = m_tileset["column"];
    VoxChunk *col2 = m_tileset["column_vines"];
    VoxChunk *col3 = m_tileset["column_ruins"];
    VoxChunk *tree = m_tileset["juniper"];
    
    printf("Ground chunk is %p name %s\n", ground, ground->m_chunkName.c_str() );
    printf("Bush chunk is %p name %s\n", bush, bush->m_chunkName.c_str() );
    
    m_map[index(m_xSize-1,1, m_zSize-1)].chunk = bush;
    
    
    for (int i=1; i < m_xSize-1; i++)
    {
        for (int j=0; j < m_zSize-1; j++)
        {
            MapTile &t = m_map[index(i,0,j)];
            t.chunk = ground;
            t.rot = (rand() % 4 ) * 90;
        }
    }
    
    for (int i=1; i < m_xSize-1; i++)
    {
        MapTile &t = m_map[index(i,0,0)];
        t.chunk = ground2;
        t.rot=90;
        
        MapTile &t2 = m_map[index(i,0,m_zSize-1)];
        t2.chunk = ground2;
        t2.rot=270;
    }
    
    for (int i=1; i < m_zSize-1; i++)
    {
        MapTile &t = m_map[index(0,0,i)];
        t.chunk = ground2;
        t.rot=180;
        
        MapTile &t2 = m_map[index(m_xSize-1,0,i)];
        t2.chunk = ground2;
        t2.rot=0;
    }

    
    for (int i=0; i < 10; i++)
    {
        MapTile &t = m_map[index(rand() % m_xSize,
                                 1,
                                 rand() % m_zSize)];
        t.chunk = bush;
        t.rot = (rand() % 4 ) * 90;
    }
    
    m_map[index(0,0,0)].chunk = ground3;
    m_map[index(0,0,0)].rot = 90;
    
    m_map[index(m_xSize-1,0,0)].chunk = ground3;
    m_map[index(m_xSize-1,0,0)].rot = 0;
    
    m_map[index(0,0,m_zSize-1)].chunk = ground3;
    m_map[index(0,0,m_zSize-1)].rot = 180;
    
    m_map[index(m_xSize-1,0,m_zSize-1)].chunk = ground3;
    m_map[index(m_xSize-1,0,m_zSize-1)].rot = 270;
                               
    m_map[index(4,1,5)].chunk = col1;
    m_map[index(4,1,5)].rot = 0;
    
    m_map[index(2,1,5)].chunk = col2;
    m_map[index(2,1,5)].rot = 0;
    
    m_map[index(0,1,5)].chunk = col1;
    m_map[index(0,1,5)].rot = 0;
    
    m_map[index(4,1,3)].chunk = col1;
    m_map[index(4,1,3)].rot = 0;
    
    m_map[index(2,1,3)].chunk = col3;
    m_map[index(2,1,3)].rot = 0;
    
    m_map[index(0,1,3)].chunk = col3;
    m_map[index(0,1,3)].rot = 0;
    
    m_map[index(12,1,4)].chunk = tree;
    m_map[index(12,1,4)].rot = 0;
    
}

size_t MapRoom::index( int x, int y, int z ) const
{
    return (z*m_xSize*m_ySize)+(y*m_xSize) + x;
}

// load the tileset
void MapRoom::initTiles()
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir("gamedata/voxtiles")) == NULL) {
        printf("Can't open tiles dir gamedata/voxtiles\n" );
        exit(1);
    }
    
    while ((dirp = readdir(dp)) != NULL) {
        char *tilename = dirp->d_name;
        
        char buff[1024];
        sprintf( buff, "gamedata/voxtiles/%s", tilename );
        
        std::string tileId = tilename;
        std::string tileExt;
        
        size_t extPos = tileId.rfind( '.' );
        if (extPos != std::string::npos)
        {
            tileExt = tileId.substr( extPos );
            tileId = tileId.substr( 0, extPos );
        }
        else continue;
       
        if (tileExt != ".csv") continue;
        
        printf("Load tile %s for tile id %s\n", 
               buff, tileId.c_str() );
        VoxChunk *tile = VoxChunk::loadCSVFile( buff );
        tile->m_chunkName = tileId;
        
        m_tileset[ tileId ] = tile;
    }
    closedir(dp);
}

size_t MapRoom::instMapGeo( VoxVert *dest, size_t maxNumVert )
{
    VoxVert *curr = dest;
    size_t currSize = 0;
    
    printf( "instMapGeo, size is %d %d %d\n", m_xSize, m_ySize, m_zSize );
    
    matrix4x4f center, centerI;
    center.Translate( -0.5, -0.5, -0.5 );
    centerI.Translate( 0.5, 0.5, 0.5);
    
    for (int i=0; i < m_xSize; i++)
    {
        for (int j=0; j < m_ySize; j++)
        {
            for (int k=0; k < m_zSize; k++)
            {
                size_t tileSz;
                VoxVert *vtile;
                
                size_t ndx = index(i,j,k);
                //printf("Ndx %zu size %zu\n", ndx, m_map.size() );
                MapTile &t = m_map[ndx];
                if (!t.chunk) continue;
                
                VoxChunk *chunk = t.chunk;
                vtile = chunk->genTris( tileSz );
                
                if (tileSz + currSize >= maxNumVert)
                {
                    // oops buffer is full
                    printf("WARNING: map buffer full (%zu verts)\n",
                           currSize );
                    return currSize;
                }
                printf("Adding tile %d,%d,%d size %zu\n", i,j,k, tileSz );
                
                // copy tile int dest
                memcpy( curr, vtile, sizeof(VoxVert)*tileSz );
                
                // now go translate the tile to it's final pos
                matrix4x4f  xlate, rot, m;
                xlate.Translate( i, j, k );
                rot.RotateY( t.rot * M_PI/180.0 );
                
                m = center * rot * xlate * centerI;
                
                for (int vi=0; vi != tileSz; vi++)
                {
                    //curr[vi].m_pos += vec3f( i,j,k );
                    curr[vi].m_pos *= m;
                }
                
                // incr. the end of buff
                curr += tileSz;
                currSize += tileSz;
            }
        }
    }
    
    return currSize;
}