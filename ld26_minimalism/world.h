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
#include <map>

#include <SDL.h>
#include <SDL_endian.h>

// minibasecode
#include <font.h>
#include <png_loader.h>
#include <shapes.h>

#include "scene_obj.h"
#include "actor.h"

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
    
    int m_playerDist; // routable distance to player
};

struct GroupInfo
{
    // tine colors
    vec3f m_tint1, m_tint2;
};

class World
{
public:
    MapSquare m_map[MAP_SIZE][MAP_SIZE];
    void init();
    

    void load( const std::string &basename, std::vector<SceneObj*> &scene );

    void simTurn();
    
    // lighting
    vec3f m_lightDir; // main light
    vec3f m_specPos; // spec light pos
    vec3f m_fillDir0; // fill light 0
    
    vec3f m_bgColor;
    
    // story info
    std::string m_title;
    std::string m_intro;
    std::string m_outro;
    int m_startPosX, m_startPosY;
    
    // actors
    std::vector<Actor*> m_actors;
    Actor *m_player;

    static vec3f parseVec( const char *vecStr );
    static vec3f parseColor( const char *colorStr );
   

protected:

    // xy loc of adjacent square
    void _adj( int dir, int &x, int &y );
    
    void _updateRoutes();
    void _simActor( Actor *actor );
    
    void _attackPlayer( Actor *actor );
    
    void _createMap( const std::string &filename, const std::string &infoname, std::vector<SceneObj*> &scene );
    void _loadStoryFile( const std::string &filename, std::vector<SceneObj*> &scene );
    void _loadSceneFile( const std::string &filename, std::vector<SceneObj*> &scene );
    
    bool _isOccupied( int x, int y );

    
    // helpers
//    void _parseColor

    // Resources
    QuadBuff<DrawVert> *m_groundTile;
    
    std::map<std::string,GroupInfo> m_groupInfo;

};

#endif /* defined(__ld48jovoc__world__) */
