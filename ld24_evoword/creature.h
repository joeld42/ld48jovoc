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
#include <vector>
#include <pally.h>

#define NUM_EYES (5)
#define NUM_LEGS (4)
#define NUM_MOUTHPART (5)
#define NUM_BODYSCULPT (6)
#define NUM_TAIL (4)

class Font;
struct HistoryNode;

//static const char *bodyNames[] = { "Blob", "Trilobite", "Fishlike", "Turtle", "Bearish", "Biped", "Elegant" };

enum
{
    BodyType_BLOB,
    BodyType_TRILOBITE,
    BodyType_FISHLIKE,
    BodyType_TURTLE,
    BodyType_BEARISH,
    BodyType_BIPED,
    BodyType_ELEGANT,
    
    NUM_BodyType
};

class Creature
{
public:
    Creature();
    
    // bookkeeping data
    std::string m_word;
    HistoryNode *m_historyNode;
    
    void draw( Font *font );
    void reset();
    void evolveCreature( std::string genomeWord, const std::vector<Pally> &creaturePalettes );
    
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
        
    Pally m_colorScheme;
    
    int m_eyeNum;
    int m_legNum;
    int m_mouthNum;
    int m_tailNum;
    int m_bodySculptNum;    
    
    float m_size;
};




#endif
