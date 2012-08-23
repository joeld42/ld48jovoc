//
//  template_game.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <font.h>
#include <gamedata.h>

#include "template_game.h"


TemplateGame::TemplateGame()
{
    
}

void TemplateGame::init()
{
    // Load font
    m_fontImg = LoadImagePNG( gameDataFile("", "nesfont.png" ).c_str() );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );

}

void TemplateGame::updateSim( float dtFixed )
{
    // Update stuff with a fixed timestep
}

void TemplateGame::updateFree( float dtRaw )
{
    // Update stuff with free timestep
}

void TemplateGame::redraw()
{
    glClearColor( 0.2, 0.1, 1.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // 3d stuff
    // TODO
    
    // 
    glDisable( GL_VERTEX_ARRAY );
    
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    
    glEnable( GL_TEXTURE );
    glEnable( GL_TEXTURE_2D );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // 2d stuff
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    glOrtho( 0, 800, 0, 600, -1, 1 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    _draw2d();

}

void TemplateGame::shutdown()
{
    // shutdown stuff
}

void TemplateGame::mouseMotion( float x, float y )
{
    
}

// For instantanious button effects 
void TemplateGame::mouseButton( SDL_MouseButtonEvent &btnEvent )
{
    
}

// For "continuous" button events
void TemplateGame::updateButtons( unsigned int btnMask )
{
    
}

void TemplateGame::keypress( SDLKey &key )
{
    switch(key)
    {
        case 'o':
            // orbit
//            doSpin = !doSpin;
//            printf("Orbit %s\n", doSpin?"true":"false" );
            break;

        default:
            break;
    }
}

void TemplateGame::_draw3d()
{
    // Draw 3d stuff
}

void TemplateGame::_draw2d()
{
    // Draw 2D stuff    
    m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
    m_nesFont->drawStringCentered( 400, 580, "Hello {{Project}}" );    
    m_nesFont->renderAll();
    m_nesFont->clear();

}
