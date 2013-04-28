//
//  minimalism_game.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_minimalism_game_h
#define ld48jovoc_minimalism_game_h

#include <vector>

#include <SDL.h>
#include <SDL_endian.h>

// minibasecode
#include <font.h>
#include <png_loader.h>
#include <shapes.h>

// game include
#include "scene_obj.h"
#include "world.h"
#include "actor.h"

// Simple 'controller'-like presses (multiple can be pressed)
// for movement
enum 
{
	BTN_LEFT  = ( 1 << 0 ),
	BTN_RIGHT = ( 1 << 1 ),
	BTN_UP    = ( 1 << 2 ),
	BTN_DOWN  = ( 1 << 3 ),
    
	BTN_A = ( 1 << 4 ),
	BTN_B = ( 1 << 5 ),
};


class BlocksGame
{
public:
    BlocksGame();
    
    void init();
    
    void updateSim( float dtFixed );
    void updateFree( float dtRaw );    
    
    void redraw();
    
    void shutdown();
    
    void mouseMotion( float x, float y );
    void mouseButton( SDL_MouseButtonEvent &btnEvent );
    
    void updateButtons( unsigned int btnMask );
    
    void keypress( SDLKey &key );
    
protected:
    void _draw3d();
    void _draw2d();

    void _drawGroundTile( int x, int y, int hite);

    // Helper to draw a DrawVert based mesh
    void _drawMesh( QuadBuff<DrawVert> *mesh );

    void _prepViewMat();
    
    void movePlayer( int xx, int yy );


private:
    // The font
    PNGImage m_fontImg;
    Font *m_nesFont;  
        
    // A shape to draw
    QuadBuff<DrawVert> *m_cube;
    GLint m_basicShader;
    PNGImage m_simpleTex;
    
    QuadBuff<DrawVert> *m_testPost;

    // The objects in the scene
    std::vector<SceneObj*> m_scene;

    // The game world
    World *m_world;
    
    // The player's actor
    Actor *m_player;
    
    // camera stuff
    matrix4x4f m_proj;   // projection matrix
    matrix4x4f m_model;  // transform for the current object world->obj
    matrix4x4f m_view;   // camera placement transform

    vec3f m_camPos;
    bool m_useLookat;
    bool m_paused;

    vec3f m_playerPos;
    matrix4x4f m_modelViewProj;

    float m_rotate;
    float m_testval;
    
};

// The generated font data
// This is the NES font I used for LD20, within the rules to reuse this
// if necessary but should probably replace it ASAP 
Font *makeFont_nesfont_8( GLuint fontTex );


#endif
