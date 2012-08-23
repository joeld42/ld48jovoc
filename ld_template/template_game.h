//
//  template_game.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_template_game_h
#define ld48jovoc_template_game_h


#include <SDL.h>
#include <SDL_endian.h>

// minibasecode
#include <font.h>
#include <png_loader.h>

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


class TemplateGame
{
public:
    TemplateGame();
    
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

private:
    // The font
    PNGImage m_fontImg;
    Font *m_nesFont;  
};

// The generated font data
// This is the NES font I used for LD20, within the rules to reuse this
// if necessary but should probably replace it ASAP 
Font *makeFont_nesfont_8( GLuint fontTex );


#endif
