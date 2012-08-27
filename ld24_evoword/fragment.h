//
//  fragment.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/24/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_fragment_h
#define ld48jovoc_fragment_h

#include <prmath/prmath.hpp>
#include <sprite.h>

// A "fragment" is a letter, it represents a fragment of the creature's DNA

class Fragment
{
public:
    Fragment();
    Fragment( char letter, vec2f pos );
    
    // Init as a bubble, resets letter
    void initBubble();
    
//protected:
    vec2f m_pos;
    char m_letter;
    
    // Highlight for replacing
    bool m_selected;
    
    // Bubbling up behavior
    float m_bubbleBaseX; // unwiggled x (target x for word fragments)
    float m_bubbleVel; // how fast it rises
    float m_bubbleTval; // for the wiggle
    float m_wiggleAmount; // How much wiggle
    float m_tScale; // How fast wiggle
    
//    Sprite *m_bubbleSprite;
    
    
};

#endif
