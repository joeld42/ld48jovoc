#ifndef TAKE5_GAME
#define TAKE5_GAME
//
//  Take5Game.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include "GLee.h"

class Take5Game
{
public:
    Take5Game();
    
    void init();
    
    void updateSim( float dtFixed );
    void updateFree( float dtRaw );
    
    void redraw();
   
    static void shutdown();
    
private:
    void _shutdown();
    
    static Take5Game *_singleton;
    
};


#endif