//
//  actor.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//

#include "actor.h"

Actor::Actor( SceneObj *mesh ) : m_mesh(mesh)
{
    m_behavior = Behavior_STILL;
    m_mapX = 1;
    m_mapY = 1;
}

void Actor::setPos( int x, int y )
{
    m_mapX = x;
    m_mapY = y;
    
    // Update our xform
    m_mesh->m_xform.Translate( m_mapX - 9.5, 0.0, m_mapY - 9.5 );
}

