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
#include "VoxSprite.h"

//=========

// maps
enum
{
    MAP_START_ZONE,
    MAP_CAVE,
};


//=========

struct MapTile
{
    MapTile() 
    {
        chunk = NULL;
        rot = 0;
        teleport = false;
        teleWhere = MAP_START_ZONE;
    }
    
    VoxChunk *chunk;
    int rot;
    
    // does this tile teleport you somewhere else?
    bool teleport;
    int  teleWhere; // if so, where? (map enum)
    vec3f telePos; //start pos;
    
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
    
    // queries -- right now it's pretty hacky 
    bool isVacant( float x, float y, float z ) const;
    float groundHeight( float x, float z ) const;
    
    void buildMap( int map );
    
    void clearMap();
    void buildMap_StartRoom();
    void buildMap_Cave();
    
    // The Enemies
    static VoxChunk *m_octorok;
    std::vector<VoxSprite> m_enemies;

    // start pos
    vec3f m_playerStartPos;
    
    // pithy message for the player
    std::string m_message1, m_message2;
    
//protected:
    int m_xSize, m_ySize, m_zSize;
    std::vector<MapTile> m_map;
    
    // maprooms share a list of VoxChunk "tiles"
    static std::map<std::string,VoxChunk*> m_tileset;
    
    // map drawing stuff
    void drawSlab( int x0, int y0, int z0,
                   int x1, int y1, int z1,
                  VoxChunk *mid,
                  VoxChunk *side,
                  VoxChunk *corner );
    
};

#endif