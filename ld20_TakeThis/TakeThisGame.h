#ifndef TAKE5_GAME
#define TAKE5_GAME
//
//  TakeThisGame.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include "GLee.h"

#include <prmath/prmath.hpp>

#include "VoxChunk.h"
#include "MapRoom.h"

// 'controller' presses (multiple can be pressed)
enum 
{
	BTN_LEFT  = ( 1 << 0 ),
	BTN_RIGHT = ( 1 << 1 ),
	BTN_UP    = ( 1 << 2 ),
	BTN_DOWN  = ( 1 << 3 ),
    
	BTN_A = ( 1 << 4 ),
	BTN_B = ( 1 << 5 ),
};


class TakeThisGame
{
public:
    TakeThisGame();
    
    void init();
    
    void updateSim( float dtFixed );
    void updateFree( float dtRaw );
    
    void redraw();
   
    static void shutdown();
    
    void updateButtons( unsigned int btnMask );
    
private:
    void _shutdown();
    
    static TakeThisGame *_singleton;
    
    // camera stuff
    matrix4x4f m_proj;
    matrix4x4f m_modelview;
    
    matrix4x4f m_modelviewProj;
    
    // The raw triangle data for the map
    VoxVert *m_mapVertData;
    size_t m_mapVertCapacity;
    size_t m_mapVertSize;
    
    // The Player
    vec3f m_playerPos;
    vec3f m_playerVel;
    float m_playerAng;
    VoxChunk *m_player;
    
    // dbg
    MapRoom *room;
    VoxChunk *chunk;
    float ang;
    
};


#endif