//
//  fragment.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/24/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <useful.h>
#include <fragment.h>

Fragment::Fragment()
{
    initBubble();
}

Fragment::Fragment( char letter, vec2f pos ) :
    m_pos( pos),
    m_letter(letter)
{
}

void Fragment::initBubble()
{
    m_letter = 'A' + (int)(randUniform() * 26);
    
    // kind of a "inverse normal", fewer in the center more towards the sides
    m_pos = vec2f( randNormal( 800.0, 200.0), 20 );                   
    if (m_pos.x > 800)
    {
        m_pos.x = m_pos.x - 800;
    }
    
    // Make bubbly
    m_bubbleVel = randNormal( 30.0, 15.0 );
    m_bubbleTval = 0.0;
    m_wiggleAmount = randUniform( 3.0, 30.0 );
    m_bubbleBaseX = m_pos.x;  
    m_tScale = randNormal( 5.0, 2.0 );
}