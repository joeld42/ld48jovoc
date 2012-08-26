//
//  creature.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/24/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_creature_h
#define ld48jovoc_creature_h

#include <string>

#define NUM_EYES (5)
#define NUM_LEGS (4)
#define NUM_MOUTHPART (5)
#define NUM_BODYSCULPT (6)
#define NUM_TAIL (4)

class Font;
struct HistoryNode;

class Creature
{
public:
    Creature();
    
    // bookkeeping data
    std::string m_word;
    HistoryNode *m_historyNode;
    
    void draw( Font *font );
    void reset();
    void evolveCreature( std::string genomeWord );
    
    // genetic manipulations
    void shiftColor( float degrees );
    void invertColoration();
    void growSize( float percent );
    void evolveEyes();
    void evolveLegs();
    void evolveMouth();
    void evolveBody();
    void evolveTail();
    void evolvePattern();
    
    int m_eyeNum;
    int m_legNum;
    int m_mouthNum;
    int m_tailNum;
    int m_bodySculptNum;    
    
    float m_size;
};




#endif
