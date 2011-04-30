//
//  Take5Game.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//
#include <assert.h>

#include "Take5Game.h"

Take5Game *Take5Game::_singleton = NULL;

Take5Game::Take5Game()
{
    assert( _singleton == NULL );
    _singleton = this;
}


void Take5Game::init()
{
}

void Take5Game::shutdown()
{
    assert( _singleton );
    _singleton->_shutdown();
}

void Take5Game::_shutdown()
{
    // here .. release stuff
}

void Take5Game::updateSim( float dtFixed )
{
    
}

void Take5Game::updateFree( float dtRaw )
{
    
}

void Take5Game::redraw()
{
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}
