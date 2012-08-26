//
//  history_tree.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/25/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_history_tree_h
#define ld48jovoc_history_tree_h

#include <prmath/prmath.hpp>
#include <string>
#include <vector>

#include "font.h"

struct HistoryNode
{
    HistoryNode( HistoryNode *parent );
    
    ~HistoryNode();
    
    float layoutSubtree();
    void drawSubtree( Font *font, vec2f pos, float yval );
    
    std::string m_word;
    vec2f m_pos;
//    int m_depth;
    
    HistoryNode *m_parent;
    std::vector<HistoryNode*> m_childs;
};


#endif
