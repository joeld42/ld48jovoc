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

class Actor
{
public:
    Actor( SceneObj *mesh );

    void setPos( int x, int y );

    SceneObj *m_mesh;
    int m_mapX, m_mapY;


};

#endif /* defined(__ld48jovoc__actor__) */
