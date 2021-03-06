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
#include "VoxSprite.h"
#include "PNGLoader.h"
#include "font.h"

#include <SDL.h>
#include <SDL_endian.h>


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

// heart containers in font
enum 
{
    HEART_FULL = 30,
    HEART_HALF = 29,
    HEART_EMPTY = 28
};

enum 
{
    TRIFORCE_0 = 0x01,
    TRIFORCE_1 = 0x02,
    TRIFORCE_2 = 0x04
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
    
    void mouseMotion( float x, float y );
    
    void updateButtons( unsigned int btnMask );
    
    void keypress( SDLKey &key );
    
    void visitRoom( int mapCode );
    
    void toggleFPSMode();
    
    // return -1 if you lose, 1 if you win, 0 otherwise
    int isGameOver();
    
private:
    void _shutdown();
    
    static TakeThisGame *_singleton;
    
    // camera stuff
    matrix4x4f m_proj;
    matrix4x4f m_modelview;
    
    matrix4x4f m_modelviewProj;
    
    quat4f m_camQuat; // for FPS mode
    quat4f m_camQuatX;
    quat4f m_camQuatY;
    
    // The raw triangle data for the map
    VoxVert *m_mapVertData;
    size_t m_mapVertCapacity;
    size_t m_mapVertSize;
    
    // The Player
    vec3f m_playerPos;
    vec3f m_playerVel;
    float m_playerAng;
    float m_playerHurt; // timeout
    float m_playerStrike; // timeout
    bool blink;
    bool hasSword;
    int  triforce; // bitfield
    int rubees;
    float msgShow;
    VoxChunk *m_player;
    VoxSprite *m_weapon;
    
    bool messageDone();
    
    VoxChunk *m_itemSword;
    VoxChunk *m_itemRubee;
    VoxChunk *m_itemHeart;
    VoxChunk *m_itemTriforce;
    
    std::vector<std::pair<int,int> > foundCaves;
    
    void mkHeartCtr( char *buff );
    int m_heartCtrs;
    int m_hitPoints;
    
        
    // The font
    PNGImage m_fontImg;
    Luddite::Font *m_nesFont;
    
    
    
    
    
    // dbg
    MapRoom *room;
    float ang, camAng;
    bool doSpin;
    bool doWire;
    bool doBBox;
    bool fpsMode;
    
    
    
};


#endif