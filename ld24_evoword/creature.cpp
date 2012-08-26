//
//  creature.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/24/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <string>
#include <font.h>

#include "creature.h"

Creature::Creature()
{
    reset();
    m_historyNode = NULL;
}

void Creature::reset()
{
    m_eyeNum = 0;
    m_legNum = 0;
    m_mouthNum = 0;
    m_tailNum = 0;
    m_bodySculptNum = 0;
    m_size = 1.0;
}

void Creature::draw( Font *font )
{
    // TMP DRAW
    static const char *eyeNames[] = { "Eyeless", "Basic Eyes", "Simple Eyes", "Cat Eyes", "Human Eyes", "Eyestalks" };
    static const char *legNames[] = { "Legless", "Flagellum",  "Flippers", "Insect Legs", "Human Legs" };
    static const char *mouthNames[] = { "Mouthless", "Fish Pucker", "Beak", "Thick Lips", "Teethy", "Tentacles Bunch" };
    static const char *tailNames[] = { "None", "Slender", "Spiked Club", "Horsewhip", "Prehensile" };
    static const char *bodyNames[] = { "Blob", "Trilobite", "Fishlike", "Turtle", "Bearish", "Biped", "Elegant" };
    
    int baseY = 550;
    char cschm[10];
    
    font->setColor(1.0, 1.0, 1.0, 1.0);    
    font->drawStringCentered( 400, baseY, "CREATURE" );
    font->drawString( 200, baseY-30, (std::string("Colors   : ")+cschm).c_str() );
    font->drawString( 200, baseY-60, (std::string("Body   : ")+bodyNames[m_bodySculptNum]).c_str() );
//    font->drawString( 200, baseY-90, (std::string("Eyes   : ")+eyeNames[m_eyeNum]).c_str() );
//    font->drawString( 200, baseY-90, (std::string("Mouth  : ")+mouthNames[m_mouthNum]).c_str() );
//    font->drawString( 200, baseY-120, (std::string("Legs   : ")+legNames[m_legNum]).c_str() );
//    font->drawString( 200, baseY-150, (std::string("Tail   : ")+tailNames[m_tailNum]).c_str() );
}


void Creature::evolveCreature( std::string genomeWord, const std::vector<Pally> &creaturePalettes )
{
    // tmp -- need to smoothly morph into next stage of evolution
    reset();
    
    m_word = genomeWord;
    
    // First character defines the color scheme
    int paletteKey = genomeWord[0] - 'A';

    if (paletteKey < creaturePalettes.size())
    {
        m_colorScheme = creaturePalettes[paletteKey];
    }
    else
    {
        printf(">>>> WARNING:: no palette for letter '%c'\n", genomeWord[0] );
        m_colorScheme.generate( paletteKey );
    }
    
    // Length of word defines the body type
    m_bodySculptNum = (genomeWord.size() - 3);
    if (m_bodySculptNum >= NUM_BodyType)
    {
        m_bodySculptNum = NUM_BodyType-1;
    }
    
    // Remaining character alter genome 
    for (int i=1; i < genomeWord.size(); i++)
    {
        switch (genomeWord[i])
        {
            case 'A': shiftColor( 120.0 ); break;
            case 'E': evolveEyes(); break;
            case 'I': growSize( 1.5 ); break;
            case 'O': evolveMouth(); break;
            case 'U': evolveTail(); break;
                
//                B - Eyes
//                C - Body Sculpt
//                D - Body Sculpt 
//                G - Tail
//                F - Mouth
//                H - Invert coloration
//                J - Body Pattern
//                K - Back Spikes
//                L - Hue shift 51 degrees
//                M - Body Pattern
//                N - Tail size x2
//                P - Turtle legs
//                Q - Body Pattern
//                R - Hue shift -72.0 degrees
//                S - Leg Type
//                T - Body Pattern
//                V - Mouth
//                W - Butterfly Wings
//                X - Hue shift 27.0 degrees
//                Y - Body Sculpt
//                Z - Eyes
                
            case 'B': evolveEyes(); break;
            case 'C': evolveBody(); break;
            case 'D': evolveBody(); break;
            case 'F': evolveMouth(); break;
            case 'G': evolveTail(); break;
            case 'H': invertColoration(); break;
            case 'J': evolvePattern(); break;
//            case 'K': addBackSpikes(); break;
            case 'L': shiftColor( 51.0 ); break;
            case 'M': evolvePattern(); break;
//            case 'N': growTailSize(); break;
            case 'P': evolveLegs(); break;
            case 'Q': evolvePattern(); break;
            case 'R': shiftColor( -72.0 ); break;
            case 'S': evolveLegs(); break;
            case 'T': evolvePattern(); break;
            case 'V': evolveMouth(); break;
//            case 'W': addWings(); break;
            case 'X': shiftColor( -27.0 ); break;
            case 'Y': evolveBody(); break;
            case 'Z': evolveEyes(); break;
        }
    }
}

void Creature::shiftColor( float degrees )
{
    // TODO: hue shift creature's dest colorscheme by degrees
}

void Creature::invertColoration()
{
    // TODO: invert creature's texture
}

void Creature::evolveEyes()
{
    m_eyeNum++;
    if (m_eyeNum > (NUM_EYES+1)) m_eyeNum = 1;
}

void Creature::evolveLegs()
{
    m_legNum++;
    if (m_legNum > (NUM_LEGS+1)) m_legNum = 1;
}

void Creature::evolveMouth()
{
    m_mouthNum++;
    if (m_mouthNum > (NUM_MOUTHPART+1)) m_mouthNum = 1;
}

void Creature::evolveTail ()
{
    m_tailNum++;
    if (m_tailNum > (NUM_TAIL+1)) m_tailNum = 1;
}

void Creature::evolveBody()
{
    m_bodySculptNum++;
    if (m_bodySculptNum > NUM_BODYSCULPT) m_bodySculptNum = 0;
}

void Creature::evolvePattern()
{
    // TODO
}

void Creature::growSize(float percent)
{
    m_size *= percent;
}



