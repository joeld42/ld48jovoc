//
//  evoword_game.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_evoword_game_h
#define ld48jovoc_evoword_game_h

#include <functional>
#include <string>
#include <vector>
#include <map>

#include <SDL.h>
#include <SDL_endian.h>

// minibasecode
#include <font.h>
#include <png_loader.h>
#include <shapes.h>

// Game
#include "creature.h"
#include "fragment.h"
#include "history_tree.h"

// Gamestate
enum
{
    GameState_MENU,
    GameState_GAME,
    GameState_REVIEW
};

// Simple 'controller'-like presses (multiple can be pressed)
// for movement
enum 
{
	BTN_LEFT  = ( 1 << 0 ),
	BTN_RIGHT = ( 1 << 1 ),
	BTN_UP    = ( 1 << 2 ),
	BTN_DOWN  = ( 1 << 3 ),
    
	BTN_A = ( 1 << 4 ),
	BTN_B = ( 1 << 5 ),
};

class EvoWordGame
{
public:
    EvoWordGame();
    
    void init();
    
    void updateSim( float dtFixed );
    void updateFree( float dtRaw );    
    
    void redraw();
    
    void shutdown();
    
    void mouseMotion( float x, float y );
    void mouseButton( SDL_MouseButtonEvent &btnEvent );
    
    void updateButtons( unsigned int btnMask );
    
    void keypress( SDLKey &key );
    
    bool isWord( const std::string &word );
    
protected:
    void _draw3d();
    void _draw2d();

    // Helper to draw a DrawVert based mesh
    void _drawMesh( QuadBuff<DrawVert> *mesh );    
    void loadWordList( const char *wordlist );
    
    void startGame();
    void updateCreatureFrags( );
    void checkWord();
    void saveCreature( bool pickNow );
    
    void initCreatureFragments();
    
    void drawTree();

    void drawCreatureEyes();
    void drawCreatureEyelids();
    
private:
    // The font
    PNGImage m_fontImg;
    Font *m_nesFont;  
        
    // A shape to draw
    QuadBuff<DrawVert> *m_cube;
    GLint m_basicShader;
    PNGImage m_simpleTex;
    PNGImage m_eyeballTex;
    
    // camera stuff
    matrix4x4f m_proj;
    matrix4x4f m_modelview;    
    matrix4x4f m_modelviewProj;

    float m_rotate;        
    float m_checkWordTimeleft;
//    std::function<void> m_undoLetter;

    typedef std::map<std::string,bool> WordList;
	WordList m_wordList;    
    
    std::vector<Fragment> m_floatyFrags;
    Fragment *m_floatyPicked;
    
    std::vector<Fragment> m_creatureFrags;
    std::vector<Fragment> m_oldCreatureFrags;
    
    // Creature viz parts
    QuadBuff<DrawVert> *m_eyeball;
    QuadBuff<DrawVert> *m_eyelid;
    
    // count of the number of 3 letter words in the list
    int m_startWords;
    
    // Gameplay stuff 
    size_t m_score;
    size_t m_displayedScore;
    std::vector<std::string> m_usedWords;

    std::vector<Creature> m_savedCreatures;

    HistoryNode *m_historyRoot;
    HistoryNode *m_historyCurr;
    float m_treeWidth;
    bool m_needsLayout;
    
    std::string m_startWord;
    std::string m_currWord;
    Creature m_creature;
    
    // Shader params
    GLint m_uModelViewProj;
    GLint m_uColorBase;
    GLint m_uColorAlt;
    GLint m_uColorAccent;
    GLint m_uLightPos0;
    
    int m_gamestate;
};

// The generated font data
// This is the NES font I used for LD20, within the rules to reuse this
// if necessary but should probably replace it ASAP 
Font *makeFont_nesfont_8( GLuint fontTex );


#endif
