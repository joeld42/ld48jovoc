//
//  history_tree.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/25/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//
#include <prmath/prmath.hpp>

#include "history_tree.h"

#define NODE_WIDTH  (150)
#define NODE_HEIGHT (40)

HistoryNode::HistoryNode( HistoryNode *parent )
{
    m_parent = parent;
    
    if (parent)
    {
        parent->m_childs.push_back(this);
    }
}

HistoryNode::~HistoryNode()
{
    // delete all child nodes
    for (auto ci = m_childs.begin(); ci != m_childs.end(); ++ci)
    {
        delete (*ci);
    }
}

// Layout the subtree with self at 0, 0
// returns the width of the subtree
float HistoryNode::layoutSubtree()
{
    float totalWidth = 0;
    std::vector<float> subtreeWidths;
    
    // Set our pos to local 0, 0
    m_pos = vec2f( 0.0, 0.0 );
    
    printf( "%s has %lu childs.\n", m_word.c_str(), m_childs.size() );
    for (auto ci = m_childs.begin(); ci != m_childs.end(); ++ci)
    {
        // layout each child subtree
        float subTreeWidth = (*ci)->layoutSubtree();
        totalWidth += subTreeWidth;
        subtreeWidths.push_back(subTreeWidth);
    }
    
    // No childs?
    if (subtreeWidths.empty())
    {
        return 1.0; // return one node width
    }
    
    // redistribute child nodes
    float currOffs = -(totalWidth/2);
    for (int i=0; i < m_childs.size(); i++)
    {
        m_childs[i]->m_pos = vec2f( currOffs + (subtreeWidths[i]/2), 0.0 );
        currOffs += subtreeWidths[i];
    }
    
    printf("Layout subtree: node %s totalWidth %f\n", m_word.c_str(), totalWidth );
    return totalWidth;
}

void HistoryNode::drawSubtree( Font *font, vec2f pos, float yval )
{
    // draw current node
    vec2f nodePos = vec2f( pos.x + (m_pos.x * NODE_WIDTH), yval );
    
    font->drawStringCentered( nodePos.x, yval, m_word.c_str() );
    
    for (int i=0; i < m_childs.size(); i++) {
                
        vec2f childPos = vec2f( nodePos.x + (m_childs[i]->m_pos.x * NODE_WIDTH), yval - NODE_HEIGHT );
        
        glVertex2f( nodePos.x, nodePos.y );
        glVertex2f( childPos.x, childPos.y );
        
        m_childs[i]->drawSubtree( font, nodePos, yval - NODE_HEIGHT );
    }
    
    
}
