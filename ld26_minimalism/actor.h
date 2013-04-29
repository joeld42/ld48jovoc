//
//  actor.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//

#ifndef __ld48jovoc__actor__
#define __ld48jovoc__actor__

#include "scene_obj.h"
#include "png_loader.h"


enum {
    Alignment_EVIL, // tries to attack player if nearby
    Alignment_NEUTRAL, // i don't know what this means yet
    Alignment_GOOD, // Won't attack player
};


enum {
    Behavior_STILL,
    Behavior_WANDER,
    Behavior_SEEKPLAYER,
};


class Actor
{
public:
    Actor( SceneObj *mesh );

    void setPos( int x, int y );

    SceneObj *m_mesh;
    int m_mapX, m_mapY;
    int m_behavior;
    int m_alignment;
    int m_hp;
        
    std::string m_dialogue; // what they say 

};

#endif /* defined(__ld48jovoc__actor__) */
