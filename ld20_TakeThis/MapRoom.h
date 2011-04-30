#ifndef MAPROOM_H
#define MAPROOM_H
//
//  MapRoom.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/30/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include <map>
#include <string>

#include "VoxChunk.h"

struct MapTile
{
    MapTile() 
    {
        chunk = NULL;
        rot = 0;
    }
    
    VoxChunk *chunk;
    int rot;
};

//=========
class MapRoom
{
public:
    // note: zelda map size is 16x11
    MapRoom( int x=16, int y=5, int z=11);
    
    static void initTiles();
    
    size_t index( int x, int y, int z ) const;
    
    // instances the map geo into the buffer
    size_t instMapGeo( VoxVert *dest, size_t maxNumVert );
    
//protected:
    int m_xSize, m_ySize, m_zSize;
    std::vector<MapTile> m_map;
    
    // maprooms share a list of VoxChunk "tiles"
    static std::map<std::string,VoxChunk*> m_tileset;
};

#endif