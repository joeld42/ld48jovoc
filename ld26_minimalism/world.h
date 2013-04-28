//
//  world.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//

#ifndef __ld48jovoc__world__
#define __ld48jovoc__world__

#include <vector>

#include <SDL.h>
#include <SDL_endian.h>

// minibasecode
#include <font.h>
#include <png_loader.h>
#include <shapes.h>

#include "scene_obj.h"

// 20x20 with 1-wide border
#define MAP_SIZE (22)

struct MapSquare
{
    MapSquare() : m_empty( true ), m_passable( false )
    {
    }
    
    bool isWalkable();
    
    bool m_empty;
    bool m_passable;
};

class World
{
public:
    MapSquare m_map[MAP_SIZE][MAP_SIZE];
    void init();
    
    void createMap( std::vector<SceneObj*> &scene );

    void load( const std::string &basename, std::vector<SceneObj*> &scene );

    // story info
    std::string m_title;
    std::string m_intro;
    std::string m_outro;

protected:

    void _loadStoryFile( const std::string &filename, std::vector<SceneObj*> &scene );
    void _loadSceneFile( const std::string &filename, std::vector<SceneObj*> &scene );

    // helpers
//    void _parseColor
    vec3f _parseVec( const char *vecStr );
    
    // Resources
    QuadBuff<DrawVert> *m_groundTile;


};

#endif /* defined(__ld48jovoc__world__) */
