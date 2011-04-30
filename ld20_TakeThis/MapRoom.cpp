//
//  MapRoom.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/30/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include <dirent.h>

#include "MapRoom.h"

std::map<std::string,VoxChunk*> MapRoom::m_tileset;


// note: zelda map size is 16x11
MapRoom::MapRoom( int x=16, int y=5, int z=11) :
    m_xSize(x),
    m_ySize(y),
    m_zSize(z)
{
    m_map.resize( x*y*z );
    
    // DBG: fill in some tiles
    VoxChunk *ground = m_tileset["grass_open"];
    VoxChunk *bush = m_tileset["bush"];
    
    for (int i=0; i < m_xSize; i++)
    {
        for (int j=0; j < m_ySize; j++)
        {
            MapTile &t = m_map[index(i,j,0)];
            t.chunk = ground;
        }
    }
    
    for (int i=0; i < 10; i++)
    {
        MapTile &t = m_map[index(rand() % m_xSize,
                                 rand() % m_ySize,
                                 1)];
        t.chunk = bush;
        
    }
}

size_t MapRoom::index( int x, int y, int z ) const
{
    return (x*m_xSize*m_ySize)+(y*m_zSize) + z;
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
        sprintf( buff, "gamedata/voxtiles/%s\n", tilename );
        
        std::string tileId = tilename;
        size_t extPos = tileId.rfind( '.' );
        if (extPos != std::string::npos)
        {
            tileId = tileId.substr( 0, extPos );
        }
        
        //printf("TODO: Load tile %s for tile id %s\n", 
        //       buff, tileId.c_str() );
        VoxChunk *tile = VoxChunk::loadCSVFile( buff );
        
        m_tileset[ tileId ] = tile;
    }
    closedir(dp);
    
}
