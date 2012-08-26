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

#include <sprite.h>
#include <font.h>

struct HistoryNode
{
    HistoryNode( HistoryNode *parent );
    
    ~HistoryNode();
    
    float layoutSubtree();
    void drawSubtree( Font *font, vec2f pos, float yval );
    void HistoryNode::drawThumbnails();
    
    std::string m_word;
    vec2f m_pos;
//    int m_depth;
    
    HistoryNode *m_parent;
    std::vector<HistoryNode*> m_childs;
    
    // Thumbnail
    SpriteBuff *m_sbThumbnail;
    Sprite *m_thumbnail;
};


#endif
