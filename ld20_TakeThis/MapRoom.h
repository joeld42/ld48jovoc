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
    }
    
    VoxChunk *chunk;
};

//=========
class MapRoom
{
public:
    MapRoom( int x, int y, int z);
    
    static void initTiles();
    
    size_t index( int x, int y, int z ) const;
    
protected:
    int m_xSize, m_ySize, m_zSize;
    std::vector<MapTile> m_map;
    
    // maprooms share a list of VoxChunk "tiles"
    static std::map<std::string,VoxChunk*> m_tileset;
};

#endif