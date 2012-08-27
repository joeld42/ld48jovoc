//
//  evoword_game.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <prmath/prmath.hpp>

#include <font.h>
#include <gamedata.h>
#include <useful.h>
#include <shader.h>
#include <load_obj.h>
#include <GLee.h>

// offsetof() gives a warning about non-POD types with xcode, so use these old
// school macros. This is OK because while VertType is non-POD, it doesn't have
// a vtable so these still work.
#define offset_d(i,f)    (long(&(i)->f) - long(i))
#define offset_s(t,f)    offset_d((t*)1000, f)

#define NUM_FLOATIES (30)
#define NUM_SLOTS (6)

#define THUMBNAIL_SIZE (64)
//#define THUMBNAIL_SIZE (512)

#include "evoword_game.h"

// award points 
// scoring is like "BuyWord" game
const int g_letterPoints[] = { 
    1, // A
    3, // B
    2, // C
    2, // D
    1, // E
    3, // F
    2, // G
    2, // H
    1, // I
    4, // J
    2, // K
    2, // L
    3, // M
    1, // N
    1, // O
    2, // P
    4, // Q    
    1, // R
    1, // S
    1, // T
    1, // U
    3, // V
    3, // W
    4, // X
    3, // Y    
    4  // Z
};


EvoWordGame::EvoWordGame()
{
    
}

void EvoWordGame::init()
{
    // Load dictionary
    loadWordList(gameDataFile("", "2of12inf.txt" ).c_str() );
    
    // Load color schemes
    loadPalettes();
    
    // Load texture
    m_simpleTex = LoadImagePNG( gameDataFile("", "critter_map.png" ).c_str() );
    m_eyeballTex = LoadImagePNG( gameDataFile( "", "eyeball.png" ).c_str() );
    
    m_mouths.push_back( LoadImagePNG( gameDataFile("", "mouth1.png" ).c_str() ));
    
    PNGImage bubbleImage = LoadImagePNG( gameDataFile( "", "bubble.png" ).c_str() );
    m_sbBubbles = new SpriteBuff(bubbleImage.textureId, 3000 );
        
//    m_testBubble = m_sbBubbles->makeSprite();
//    m_testBubble->x = 100.0;
//    m_testBubble->y = 100.0;    
//
//    m_testBubble->sx = 64.0;
//    m_testBubble->sy = 64.0;    
//    m_testBubble->update();
    
    // Background
    m_imgBackground = LoadImagePNG( gameDataFile( "", "background.png" ).c_str() );
    m_sbBackground = new SpriteBuff( m_imgBackground.textureId );
    m_background = m_sbBackground->makeSprite( 0.0, 1.0, 1.0, 0.0 );
    m_background->x = 512.0;
    m_background->y = 600-512;    

    m_background->sx = 1024.0;
    m_background->sy = 1024.0;    
    m_background->update();


    
    m_vboThumbnail = 0;
    m_cursorOn = false;
    
//    m_sbThumb = NULL;
//    m_testThumb = NULL;
    
    // Load font
    m_fontImg = LoadImagePNG( gameDataFile("", "nesfont.png" ).c_str() );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );    
    
//    PNGImage m_fontImgGrobold;
//    Font *m_fontGrobold12;  
//    Font *m_fontGrobold20;  
//    Font *m_fontGrobold48;  
    m_fontImgGrobold = LoadImagePNG( gameDataFile("", "grobold.png" ).c_str() );
    m_fontGrobold12 = makeFont_grobold_12( m_fontImgGrobold.textureId );
    m_fontGrobold20 = makeFont_grobold_20( m_fontImgGrobold.textureId );
    m_fontGrobold48 = makeFont_grobold_48( m_fontImgGrobold.textureId );
    
//    m_cube = make_cube();
    m_cube = load_obj( gameDataFile("", "urchin.obj" ).c_str()  );

    m_eyelid = load_obj( gameDataFile("", "eyelid.obj" ).c_str()  );
    m_eyeball = load_obj( gameDataFile("", "eyeball.obj" ).c_str()  );

    m_mouthDecal = load_obj( gameDataFile("", "mouthcard.obj" ).c_str()  );
    
//    setColorConstant( m_cube, vec4f( 1.0, 1.0, 1.0 ) );
    
    m_basicShader = loadShader( "evoword.Plastic" );
    
    m_uModelViewProj = glGetUniformLocation( m_basicShader, "matrixPMV");
    
    m_uColorBase = glGetUniformLocation( m_basicShader, "colorBase" );
    m_uColorAlt = glGetUniformLocation( m_basicShader, "colorAlt" ) ;
    m_uColorAccent = glGetUniformLocation( m_basicShader, "colorAccent" ) ;

    m_uLightPos0 = glGetUniformLocation( m_basicShader, "lightPos0" );
    
    m_decalShader = loadShader( "evoword.Decal" );
    m_decal_uModelViewProj = glGetUniformLocation( m_decalShader, "matrixPMV");

    m_rotate = 0.0;
    
    glEnable( GL_DEPTH_TEST );
    
    m_gamestate = GameState_MENU;
    m_floatyPicked = NULL;
    m_historyRoot = NULL;
    m_historyCurr = NULL;
 
    m_blinkTimeout = 0.0;
    m_blink = 0.0;
    
    m_lookTimeout = 0.0;
    m_currLook = 0.0;
    m_targLook = 0.0;
}

void EvoWordGame::updateSim( float dtFixed )
{
    // Update stuff with a fixed timestep
    m_rotate += (M_PI/180.0) * (10.0) * dtFixed;
    
    // Add some floating fragments
    while (m_floatyFrags.size() < NUM_FLOATIES)
    {
        Fragment f;
        
        // since this is the new bubbles, init all over ths screen
        f.m_pos.y = randUniform( 0.0, 600.0 ); 
        m_floatyFrags.push_back(f);
    }
 
    // make fragments bubble up
    for (std::vector<Fragment>::iterator fi = m_floatyFrags.begin();
         fi != m_floatyFrags.end(); ++fi )
    {            
        Fragment &f = (*fi);
        
        // make sure it has a sprite
        if (!f.m_bubbleSprite)
        {
            f.m_bubbleSprite = m_sbBubbles->makeSprite( 0.0, 1.0, 1.0, 0.0 );            
            f.m_bubbleSprite->sx = randUniform( 32.0, 64.0 );
            f.m_bubbleSprite->sy =  f.m_bubbleSprite->sx;
            f.m_bubbleSprite->update();
        }
        
        // DON'T update floatyPicked
        if (&f == m_floatyPicked) continue;
        
        f.m_pos.x = f.m_bubbleBaseX + sin( f.m_bubbleTval * f.m_tScale) * f.m_wiggleAmount;
        f.m_pos.y += f.m_bubbleVel * dtFixed;
        
        f.m_bubbleTval += dtFixed;
        
        // If bubble went offscreen, recycle it
        if (f.m_pos.y > 620)
        {
            f.initBubble();
        }                
        
        // Update bubble sprite
        f.m_bubbleSprite->x = f.m_pos.x;
        f.m_bubbleSprite->y = f.m_pos.y;   
        f.m_bubbleSprite->update();
    }
    
    // Update all bubble sprites
//    for (std::vector<Fragment>::iterator fi = m_floatyFrags.begin();
//         fi != m_floatyFrags.end(); ++fi )
//    {
//        if (fi->m_bubbleSprite) fi->m_bubbleSprite->update();
//    }
    
    // Move genome fragments
    for (auto fi = m_creatureFrags.begin(); fi != m_creatureFrags.end(); ++fi)
    {
        float dx = fi->m_bubbleBaseX - fi->m_pos.x;
        fi->m_pos.x += (dx * dtFixed);
    }
    
    // Check timer?
    if (m_checkWordTimeleft > 0.0)
    {
        m_checkWordTimeleft -= dtFixed;
        if ( m_checkWordTimeleft <= 0.0)
        {
            // Timer ran out, check if word is valid
            checkWord();
        }
    }
    
    // Add points to displayed score
    if (m_score > m_displayedScore)
    {
        m_displayedScore++;
    }
    else if (m_score < m_displayedScore)
    {
        m_displayedScore--;
    }
    
    // Blink update
    m_blinkTimeout -= dtFixed;
    if (m_blinkTimeout < 0.0)
    {
        m_blinkTimeout = 1.0 + randNormal( 3.0, 1.0 );
        
        m_blink = 2.0;
    }

    
    if (m_blink > 0.0)
    {
        m_blink -= (dtFixed * 15.0); // blink rate
        if (m_blink < 0.0) m_blink = 0.0;
    }
    
    // Look update
    m_lookTimeout -= dtFixed;
    if (m_lookTimeout < 0.0)
    {
        m_lookTimeout = 1.0 + randNormal( 2.0, 1.0 );
        m_targLook = randNormal( 0.0, 30.0 );
    }
    
    if (fabs(m_currLook-m_targLook) > 2.0 )
    {
        m_currLook += sgn(m_targLook - m_currLook) * (dtFixed * 100.0); // eye rate
    }
    
    // different camera dist to give the impression
    // that the creature is growing
    int wordSize = m_currWord.size();
    if (wordSize != 0)
    {
        float targCameraDist = 15.0 - wordSize * 1.5;
        if (targCameraDist < 3.0) targCameraDist = 3.0;

        if (fabs(m_cameraDist-targCameraDist) > 0.5 )
        {
            m_cameraDist += sgn(targCameraDist - m_cameraDist) * (dtFixed * 20.0); // zoom rate
        }
        else
        {
            m_cameraDist = targCameraDist;   
        }
    }
    
    // Update scroll
    const float scrollSpeed = 300.0;
    if (m_buttons & BTN_UP)
    {
        m_scrollPos -= scrollSpeed * dtFixed;
        if (m_scrollPos < 0.0) m_scrollPos = 0.0;
    }

    if (m_buttons & BTN_DOWN)
    {
        float oldScrollPos = m_scrollPos;
        
        m_scrollPos += scrollSpeed * dtFixed;
        
        if (m_scrollPos > (m_scrollLimit-500)) 
        {
            m_scrollPos = oldScrollPos;
        }
    }
    

    
}

void EvoWordGame::updateFree( float dtRaw )
{
    // Update stuff with free timestep
}

void EvoWordGame::redraw()
{
    glClearColor( 11.0/255.0, 57.0/255.0, 7.0/255.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glDisable( GL_BLEND );
    
    // background
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    glOrtho( 0, 800, 0, 600, -1, 1 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    _drawBackground();


    // TEST thumbnail
//    if ( (!m_sbThumb) && (m_gamestate == GameState_GAME))
//    {
//        m_sbThumb = renderThumbnail();
//        m_testThumb = m_sbThumb->makeSprite( 0.0, 1.0, 1.0, 0.0 ); // flip st Y
//        
//        m_testThumb->x = 200.0;
//        m_testThumb->y = 300.0;    
//        
//        m_testThumb->sx = THUMBNAIL_SIZE * 4;
//        m_testThumb->sy = THUMBNAIL_SIZE * 4;    
//        m_testThumb->update();
//
//    }
    
    // 3d stuff    
    glhPerspectivef2( m_proj, 40.0, 800.0/600.0, 0.1, 1000.0 );
    
    matrix4x4f xlate, rot, rot2;    
    xlate.Translate( 0.0, -0.3, -m_cameraDist );
    rot.RotateY( m_rotate );
    rot2.RotateX( 10.0 * (M_PI/180.0));
    m_modelview = (rot*rot2) * xlate;
    
    m_modelviewProj = m_modelview * m_proj;
    CHECKGL( "before draw3d" );
    
    // Draw 3D scene
    _draw3d();
    CHECKGL( "after draw3d" );
    
    // set up 2D draw     
    glDisableClientState( GL_VERTEX_ARRAY );    
    glDisableClientState( GL_COLOR_ARRAY );
    CHECKGL( "disable client state" );
    
    glEnable( GL_TEXTURE_2D );
    CHECKGL( "enable texture" );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // 2d stuff
    // TODO: use shaders 
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    glOrtho( 0, 800, 0, 600, -1, 1 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    _draw2d();

}

void EvoWordGame::shutdown()
{
    // shutdown stuff
}

void EvoWordGame::mouseMotion( float x, float y )
{
    if (m_floatyPicked)
    {
        m_floatyPicked->m_pos = vec2f(x, 600-y);
        m_floatyPicked->m_bubbleBaseX = x;
        
        m_floatyPicked->m_bubbleSprite->x = m_floatyPicked->m_pos.x;
        m_floatyPicked->m_bubbleSprite->y = m_floatyPicked->m_pos.y;
        m_floatyPicked->m_bubbleSprite->update();
        
        replaceLetter( true );
    }
}

// For instantanious button effects 
void EvoWordGame::mouseButton( SDL_MouseButtonEvent &btnEvent )
{
    printf("EvoWordGame::mouseButton (button %d state %d)\n", btnEvent.button, btnEvent.state );
    
    if (btnEvent.button == SDL_BUTTON_LEFT)
    {
        if (btnEvent.state == SDL_RELEASED)
        {
            
            replaceLetter( false );
            
            m_floatyPicked = NULL; // release picked floaty
        }
        else if (btnEvent.state == SDL_PRESSED)
        {

            // Pick closest floaty
            vec2f mousePos( btnEvent.x, 600-btnEvent.y );
            
            printf("Looking for closest floaty mouse pos %f %f\n", 
                   mousePos.x, mousePos.y );
            
            Fragment *closestF = NULL;
            float bestDist = 0.0;
            for (auto fi = m_floatyFrags.begin(); fi != m_floatyFrags.end(); ++fi )
            {
                float d = prmath::Length( (fi->m_pos - mousePos) );
//                printf("Letter %c dist %f\n", fi->m_letter, d );
                if (d < 20.0)
                {
                    if ((!closestF) || (d < bestDist))
                    {
                        bestDist = d;
                        closestF = &(*fi);
                    }
                }
            }
            
            m_floatyPicked = closestF;
        }
            
    }
}

void EvoWordGame::replaceLetter( bool preview )
{
    // See if this is close to a current creature fragment            
    if (m_floatyPicked)
        printf("ydist %f\n", fabs(m_floatyPicked->m_pos.y - 100) );
    
    const float letterDist = 15.0;
    const float checkTime = 0.3;
    
    m_cursorOn = false;
    
    // clear selected flags
    for (auto fi = m_creatureFrags.begin(); fi != m_creatureFrags.end(); ++fi)
    {
        fi->m_selected = false;
    }
    
    if ((m_checkWordTimeleft<=0.0) && (m_floatyPicked) && (fabs(m_floatyPicked->m_pos.y - 100) < 20))
    {
        printf("Checking frags: xpos %f\n", m_floatyPicked->m_pos.x );
        
        bool changedWord = false;
        for (auto fi = m_creatureFrags.begin(); fi != m_creatureFrags.end(); ++fi)
        {
            // insert between this letter and the next?
            auto fiNext = fi+1;
            if (fiNext != m_creatureFrags.end())
            {
                float insX = (fi->m_pos.x + fiNext->m_pos.x) / 2.0;
                printf("xdists between %c, %c = %f\n", 
                       fi->m_letter, fiNext->m_letter, insX );
                
                if (fabs(m_floatyPicked->m_pos.x - insX) < letterDist)
                {
                    if (preview)
                    {
                        printf("TODO: cursor at %f", insX );
                        m_cursorOn = true;
                        m_cursorX = insX;
                    }
                    else
                    {
                        // Save the old fragments in case the new
                        // word isn't real
                        m_oldCreatureFrags = m_creatureFrags;
                        
                        printf("INSERT BETWEEN LETTERS %c %c", fi->m_letter, fiNext->m_letter );
                        Fragment fnew( m_floatyPicked->m_letter, vec2f( insX, fi->m_pos.y ) );
                        m_creatureFrags.insert( fiNext, fnew );
                        changedWord = true;
                    }
                    break;
                }
            }
            
            // Replace a letter?
            printf( "Replace dist %c = %f\n", fi->m_letter, m_floatyPicked->m_pos.x - fi->m_pos.x );
            if ((fi->m_letter != '$') && (fabs(m_floatyPicked->m_pos.x - fi->m_pos.x) < letterDist))
            {
                if (preview)
                {
                    // Just highlight the letter
                    fi->m_selected = true;
                }
                else
                {
                    // Save the old fragments in case the new
                    // word isn't real
                    m_oldCreatureFrags = m_creatureFrags;
                    
                    printf("REPLACE LETTER %c", fi->m_letter );
                    fi->m_letter = m_floatyPicked->m_letter;                                                
                    changedWord = true;
                }
                break;
            }
        }
        
        // if we changed the word, do stuff
        if (changedWord)
        {
            // Start the check timer
            m_checkWordTimeleft = checkTime;
        }
        
        if (!preview)
        {
            // reset floatyPicked's yval so it will get recycled
            m_floatyPicked->m_pos.y = 5000.0;
            
            // update positions in case the word changed
            updateCreatureFrags();
        }
    }
    
}

// For "continuous" button events
void EvoWordGame::updateButtons( unsigned int btnMask )
{
    m_buttons = btnMask;
}

void EvoWordGame::keypress( SDLKey &key )
{
    if ( (m_gamestate == GameState_MENU) ||
         (m_gamestate == GameState_REVIEW) )
    {
        switch(key)
        {
            case ' ':
                 if (m_gamestate != GameState_REVIEW)
                 {
                    startGame();
                 }
                break;
                
            // DBG: Toggle review tree
            case 'r':
                if (m_savedCreatures.size() < NUM_SLOTS)
                    m_gamestate = GameState_GAME;
                
                break;


            default:
                break;
        }
    }
    else
    {
        // gameplay keys
        switch(key)
        {
            case ' ':
                m_gamestate = GameState_MENU;
                break;
            
            // DBG: Toggle review tree
            case 'r':
                m_gamestate = GameState_REVIEW;
                m_scrollLimit = m_historyRoot->maxDepth() * NODE_HEIGHT;
                m_needsLayout = true;
                break;
                
            // DBG: save color palete
//            case 'p':
//                saveCurrentPalette();
//                break;

            case SDLK_RETURN:
                if ((m_checkWordTimeleft <= 0.0) && (!m_currWord.empty()))
                {
                    saveCreature( true );
                }
                break;
                
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':                
                {
                    // restore a saved creature
                    int ndx = key - '1';
                    if (m_savedCreatures.size() > ndx)
                    {
                        m_creature = m_savedCreatures[ndx];
                        m_currWord = m_creature.m_word;
                        
                        // restore history from creature
                        m_historyCurr = m_creature.m_historyNode;
                        
                        initCreatureFragments();
                    }
                    break;
                }
                
            default:
                break;
        }

    }

}

    // Draw 3d stuff
void EvoWordGame::_draw3d()
{    
    // don't draw the scene if not playing
    if (m_gamestate!=GameState_GAME) return;
    
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );
    CHECKGL( "draw3d: gltexture" );
    
    // Set up basic shader
    glUseProgram( m_basicShader );    

    glUniformMatrix4fv( m_uModelViewProj, 1, 0, (GLfloat*)(&m_modelviewProj)  );
    
    glUniform3f( m_uColorBase, 
                m_creature.m_colorScheme.m_colorOrganic1.x,
                m_creature.m_colorScheme.m_colorOrganic1.y,
                m_creature.m_colorScheme.m_colorOrganic1.z );

    glUniform3f( m_uColorAlt, 
                m_creature.m_colorScheme.m_colorMineral1.x,
                m_creature.m_colorScheme.m_colorMineral1.y,
                m_creature.m_colorScheme.m_colorMineral1.z );

    glUniform3f( m_uColorAccent, 
                m_creature.m_colorScheme.m_colorAccent.x,
                m_creature.m_colorScheme.m_colorAccent.y,
                m_creature.m_colorScheme.m_colorAccent.z );

    
    glActiveTexture(GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_simpleTex.textureId );
    
    GLint paramTex = glGetUniformLocation( m_basicShader, "sampler_dif0" );
    glUniform1i( paramTex, 0 );        
    
    vec3f lightPos( 1.0, 1.0, 1.0);
    lightPos.Normalize();
    
    matrix4x4f lightRot;
    lightRot.RotateY( -m_rotate );
    lightPos = lightPos * lightRot;
    
    glUniform3f( m_uLightPos0, lightPos.x, lightPos.y, lightPos.z );
    
    // Draw something    
    _drawMesh( m_cube );
    
    // Draw eyeballs
    drawCreatureEyelids();
    
    glUseProgram( m_decalShader );
    drawCreatureEyes();
    
    // draw mouth
    glUniformMatrix4fv( m_decal_uModelViewProj, 1, 0, (GLfloat*)(&m_modelviewProj)  );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    glBindTexture( GL_TEXTURE_2D, m_mouths[0].textureId );
    _drawMesh( m_mouthDecal );
}

void EvoWordGame::_drawBackground()
{
    glUseProgram(0);
    glColor3f( 1.0, 1.0, 1.0 );    
    glDisable( GL_DEPTH_TEST );

    if (m_gamestate==GameState_GAME)
    {
        m_sbBackground->renderAll();
    }
}

// Draw 2D stuff    
void EvoWordGame::_draw2d()
{
    // disable shaders (TODO: make a text shader so this ports to ES2 easier)
    glUseProgram(0);
    
    glColor3f( 1.0, 1.0, 1.0 );
    
    glDisable( GL_DEPTH_TEST );
    
    if (m_gamestate==GameState_MENU)
    {
        m_fontGrobold48 ->setColor(0.5, 1.0, 0.5, 1.0);
        m_fontGrobold48->drawStringCentered( 400, 500, "EvoWord" );    
        m_fontGrobold48->renderAll();
        m_fontGrobold48->clear();

        m_fontGrobold20->setColor(1.0, 1.0, 1.0, 1.0);
        m_fontGrobold20->drawStringCentered( 400, 300, "Press SPACE to start" );    
        m_fontGrobold20->renderAll();
        m_fontGrobold20->clear();
    }
    else if (m_gamestate==GameState_REVIEW)
    {
        CHECKGL( "review draw start" );
        
        // Game over.. show their awesome word tree
        char buff[256];
        sprintf( buff, "You earned %zu points!", m_score );
        m_fontGrobold20->setColor(1.0, 1.0, 1.0, 1.0);
        m_fontGrobold20->drawStringCentered( 400, 560, buff );    

        m_fontGrobold12->setColor(1.0, 1.0, 1.0, 1.0);                        
        m_fontGrobold12->drawStringCentered( 400, 535, "Press ESC to quit." );    
        
        // draw saved creatures
        m_fontGrobold12->setColor(1.0, 1.0, 0.0, 1.0);                        
        int currY = 580;
        int index = 1;
        for (auto wi = m_savedCreatures.begin(); wi != m_savedCreatures.end(); ++wi)
        {
            char slotbuf[100];
            sprintf( slotbuf, "%d. %s", index, (*wi).m_word.c_str() );
            m_fontGrobold12->drawString( 20, currY, slotbuf );        
            currY -= 30;
            index += 1;
        }
        
        // draw the history tree
        if (m_needsLayout)
        {
            m_needsLayout = false;
            m_treeWidth = m_historyRoot->layoutSubtree();
            printf("TreeWidth is %f\n", m_treeWidth );
        }
        
        CHECKGL( "before tree" );
        currY = 500 + m_scrollPos;
        m_fontGrobold12->setColor(0.0, 1.0, 1.0, 1.0);
        glDisable( GL_TEXTURE_2D );
        CHECKGL( "disable texture" );
        
        glLineWidth( 4.0 );
        glBegin( GL_LINES );
        glColor3f( 1.0, 1.0, 1.0 );
        m_historyRoot->drawSubtree( m_fontGrobold12, vec2f( 400, currY), currY );
        glEnd();        
        glEnable( GL_TEXTURE_2D );
        CHECKGL( "tree done" );
        
        // draw thumbnails
        m_historyRoot->drawThumbnails();

        // DBG scroll
//        sprintf( buff, "%3.2f (%3.2f)", m_scrollPos, m_scrollLimit );
//        m_nesFont->drawString( 500, 550, buff );    
        
        // Draw all text
        m_fontGrobold12->renderAll();
        m_fontGrobold12->clear();

        m_fontGrobold20->renderAll();
        m_fontGrobold20->clear();

        m_fontGrobold48->renderAll();
        m_fontGrobold48->clear();
        
        CHECKGL( "after text" );

    }
    else if (m_gamestate==GameState_GAME)
    {
        // draw current word
//        m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
//        m_nesFont->drawStringCentered( 400, 100, m_currWord.c_str() );    
        
        // Draw score
        char buff[100];
        sprintf( buff, "S: %zu", m_displayedScore );
        if (m_displayedScore > m_score)
        {
            // Losing points, bad
            m_fontGrobold20->setColor(1.0, 0.0, 0.0, 1.0);
        }
        else if (m_displayedScore < m_score )
        {
            // earning points, good!
            m_fontGrobold20->setColor(1.0, 1.0, 0.0, 1.0);            
        }
        else
        {
            // stable score
            m_fontGrobold20->setColor(1.0, 1.0, 1.0, 1.0);                        
        }
        m_fontGrobold20->drawString( 650, 50, buff );    
        
        
        // draw saved creatures
        m_fontGrobold20->setColor(1.0, 1.0, 0.0, 1.0);                        
        m_fontGrobold12->setColor(1.0, 1.0, 0.0, 1.0);                        
        
        int currY = 580;
#if 0
        int index = 1;
//        for (auto wi = m_savedCreatures.begin(); wi != m_savedCreatures.end(); ++wi)
        for (int i=0; i < NUM_SLOTS; i++)
        {
            char slotbuf[100];
            std::string cname;
            if (i < m_savedCreatures.size() )
            {
                cname = m_savedCreatures[i].m_word;
            }
            sprintf( slotbuf, "%d. %s", index, cname.c_str() );
            m_nesFont->drawStringCentered( 20, currY, slotbuf );        
            currY -= 30;
            index += 1;
        }
#endif
        drawSavedCreatures();
        
        
        if (m_currWord.empty())
        {
            // Make user pick another word
            m_fontGrobold20->setColor(1.0, 1.0, 1.0, 1.0);
            m_fontGrobold20->drawStringCentered( 400, 300, "Choose a creature [1-5]" );    
            m_fontGrobold20->renderAll();
            m_fontGrobold20->clear();
        }
        else
        {
            
            // draw creature
//            m_creature.draw( m_nesFont);
            
            // draw insert cursor
            if (m_cursorOn)
            {
                glDisable( GL_TEXTURE_2D );
                glLineWidth( 4.0 );
                glBegin( GL_LINES );
                glColor3f( 0.0, 1.0, 1.0 );
                glVertex2f( m_cursorX, 50 );
                glVertex2f( m_cursorX, 150 );
                glEnd();        
                glEnable( GL_TEXTURE_2D );
                
                glColor3f( 1.0, 1.0, 1.0 );
            }

            
            // draw "fragments"
            m_fontGrobold20->setColor(0.5, 1.0, 0.5, 1.0);
            for ( std::vector<Fragment>::iterator fi = m_floatyFrags.begin();
                 fi != m_floatyFrags.end(); ++fi )
            {
                Fragment &f = (*fi);
                char buff[2];
                buff[1] = '\0';
                buff[0] = f.m_letter;
                
                m_fontGrobold20->drawString( f.m_pos.x - 8, f.m_pos.y-8, buff );
            }
            
            // draw current word genome
            m_fontGrobold48->setColor(1.0, 1.0, 0.5, 1.0);
            for ( auto fi = m_creatureFrags.begin(); fi != m_creatureFrags.end(); ++fi )
            {
                Fragment &f = (*fi);
                
                if (f.m_letter=='$') continue; // skip marker
                
                char buff[2];
                buff[1] = '\0';
                buff[0] = f.m_letter;
                
                if (f.m_selected)
                {
                    m_fontGrobold48->setColor(0.0, 1.0, 1.0, 1.0);
                }
                else
                {
                    m_fontGrobold48->setColor(1.0, 1.0, 0.5, 1.0);
                }
                
                m_fontGrobold48->drawString( f.m_pos.x - 8, f.m_pos.y-8, buff );
            }

        }
        m_fontGrobold12->renderAll();
        m_fontGrobold12->clear();

        m_fontGrobold20->renderAll();
        m_fontGrobold20->clear();

        m_fontGrobold48->renderAll();
        m_fontGrobold48->clear();

        
        // Draw sprites
        m_sbBubbles->renderAll();
        
//        if (m_sbThumb)
//        {
//            m_sbThumb->renderAll();
//        }
        
        // Draw history sprites
        for (auto wi = m_savedCreatures.begin(); wi != m_savedCreatures.end(); ++wi)
        {
            
            SpriteBuff *sb = (*wi).m_thumb->m_sbThumbnail;
            if (sb)
            { 
                sb->renderAll();
            }
        }
    }
}

void EvoWordGame::drawCreatureEyelids()
{
    // blink
    float blink = 0.0;
    if (m_blink > 0.0)
    {
        if (m_blink > 1.0)
        {
            blink = 1.0 - (m_blink - 1.0);
        }
        else 
        {
            blink = m_blink;
        }
        blink *= 45.0;
    }

    
    // draw eyelids with the current creature shader
    matrix4x4f xlate, rot, scl, localmv;
    scl.Scale(0.2, 0.2, 0.2 );
    for (int i=0; i < 2; i++)
    {
        xlate.Translate( 0.45 * (i==0?-1:1), 0.78, 0.55 );
        
        rot.RotateX( -(45.0 - blink) * (M_PI/180.0) );
    //    rot.RotateX( m_rotate );
        localmv = (scl * rot * xlate) * m_modelview;
        
        matrix4x4f mvp;
        mvp = localmv * m_proj;    
        
        glUniformMatrix4fv( m_uModelViewProj, 1, 0, (GLfloat*)(&mvp)  );        
        _drawMesh( m_eyelid );
        
        // bottom eyelid
        xlate.Translate( 0.45 * (i==0?-1:1), 0.77, 0.55 );
        rot.RotateX( -(135.0 + blink) * (M_PI/180.0) );
        localmv = (scl * rot * xlate) * m_modelview;
        mvp = localmv * m_proj;
        
        glUniformMatrix4fv( m_uModelViewProj, 1, 0, (GLfloat*)(&mvp)  );        
        _drawMesh( m_eyelid );
    }

}

void EvoWordGame::drawCreatureEyes()
{
    // TODO: Turn off shader for eyes
    
    glBindTexture( GL_TEXTURE_2D, m_eyeballTex.textureId );
    
    matrix4x4f xlate, rot, rot2, scl, localmv;
    scl.Scale(0.2, 0.2, 0.2 );
    for (int i=0; i < 2; i++)
    {
        xlate.Translate( 0.45 * (i==0?-1:1), 0.78, 0.55 );

        rot2.RotateY( m_currLook * (M_PI/180.0) );
        
        rot.RotateX( 90.0 * (M_PI/180.0) );
        //    rot.RotateX( m_rotate );
        localmv = (scl * rot * rot2 * xlate) * m_modelview;
        
        matrix4x4f mvp;
        mvp = localmv * m_proj;
        
        glUniformMatrix4fv( m_decal_uModelViewProj, 1, 0, (GLfloat*)(&mvp)  );        
        _drawMesh( m_eyeball );
    }
    
}


void EvoWordGame::_drawMesh( QuadBuff<DrawVert> *mesh )
{
    // Buffer is already created, just bind it
    glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo() );

    // Bind buffer data
    glEnableVertexAttribArray( DrawVertAttrib_TEXCOORD );
    glVertexAttribPointer( DrawVertAttrib_TEXCOORD, 4, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_st ) );

    glEnableVertexAttribArray( DrawVertAttrib_POSITION );
    glVertexAttribPointer( DrawVertAttrib_POSITION, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_pos) );


    glEnableVertexAttribArray( DrawVertAttrib_NORMAL );
    glVertexAttribPointer( DrawVertAttrib_NORMAL, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_nrm) );

    glEnableVertexAttribArray( DrawVertAttrib_COLOR );
    glVertexAttribPointer( DrawVertAttrib_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_color) );

    // Draw it!
    glDrawArrays(GL_TRIANGLES, 0, mesh->size() );
}


HistoryNode *EvoWordGame::historyNodeForCurrentCritter( HistoryNode *parent )
{
    HistoryNode *hist = new HistoryNode( parent );
    hist->m_word = m_currWord;
    
    // make a thumbnail
    hist->m_sbThumbnail = renderThumbnail();
    hist->m_thumbnail = hist->m_sbThumbnail->makeSprite( 0.0, 1.0, 1.0, 0.0 ); // flip st Y
    
    hist->m_thumbnail->x = 500.0;
    hist->m_thumbnail->y = 1000.0;    // offscreen
    
    hist->m_thumbnail->sx = THUMBNAIL_SIZE;
    hist->m_thumbnail->sy = THUMBNAIL_SIZE;    
    hist->m_thumbnail->update();

     
    return hist;
}

SpriteBuff *EvoWordGame::renderThumbnail()
{
    CHECKGL( "renderThumbnail" );
    printf( "======= renderThumbnail ========\n" );
    
    // Set up VBO if not set up
    if (!m_vboThumbnail)        
    {
        glGenFramebuffersEXT( 1, &m_vboThumbnail );
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_vboThumbnail );
        printf( "m_vboThumbnail is %d\n", m_vboThumbnail );	
        
        // Attach a depth buffer
        GLuint depthbuffer; 
        glGenRenderbuffersEXT(1, &depthbuffer); 
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, THUMBNAIL_SIZE, THUMBNAIL_SIZE);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
    }
    else
    {
        // Already created, just bind it
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_vboThumbnail );
    }
    
    
    // Create a texture for the thumbnail
    GLuint texIdFbo;
    unsigned char *data = (unsigned char *)malloc( THUMBNAIL_SIZE*THUMBNAIL_SIZE*4 );

//	glGenTexturesEXT( 1, &texIdFbo );	
    glGenTextures(1, &texIdFbo);
	glBindTexture( GL_TEXTURE_2D, texIdFbo );
    
    printf("genTexture and bound %d\n", texIdFbo );
    
	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
	glTexImage2D( GL_TEXTURE_2D, 0, 4, 
                 THUMBNAIL_SIZE, THUMBNAIL_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
    
    CHECKGL( "before attach" );
    
    // attach a texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                              GL_TEXTURE_2D, texIdFbo, 0 );
    
	checkFBO( );
	CHECKGL( "attach fbo to texture" );
    printf("Attach fbo to texture %d\n", texIdFbo );

    glViewport( 0, 0, THUMBNAIL_SIZE, THUMBNAIL_SIZE );
    glClearColor( 0.0, 0.0, 0.0 , 0.0 );
//    glClearColor( 0.0, 0.0, 0.0 , 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Draw scene
    glhPerspectivef2( m_proj, 40.0, 1.0, 0.1, 1000.0 );
    
    matrix4x4f xlate, rot, rot2;    
    xlate.Translate( 0.0, -0.3, -3.5 );
    
    rot.RotateY( randNormal( 0.0, 10.0) * (M_PI/180.0));
    rot2.RotateX( 10.0 * (M_PI/180.0));
    m_modelview = (rot*rot2) * xlate;
    
    m_modelviewProj = m_modelview * m_proj;
    
    // Draw 3D scene
    _draw3d();

    // Detach framebuffer from texture
    printf("Detach texture.." );
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                              GL_TEXTURE_2D, 0, 0 );
    

    
    // restore the main buffer
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
    glhPerspectivef2( m_proj, 40.0, 800.0/600.0, 0.1, 1000.0 );
    glViewport( 0, 0, 800, 600 );
    
    SpriteBuff *sbuff = new SpriteBuff( texIdFbo );
    
    printf("RenderThumbnail done...\n" );
    
    return sbuff;
}

#pragma mark - Game Stuff

void EvoWordGame::startGame()
{
    srand48( SDL_GetTicks() );
    
    m_gamestate = GameState_GAME;

    // Pick a new startword
    int startWordNdx = (int)randUniform(1, m_startWords);
    printf("startWordNdx %d\n", startWordNdx );
    
    for (WordList::iterator wi = m_wordList.begin(); wi != m_wordList.end(); ++wi )
    {
        const std::string &w = wi->first;
        if (w.size() == 3)
        {
            startWordNdx--;
            if (startWordNdx==0)
            {
                m_currWord = w;
                break;
            }
        }
    }
//    printf("CurrWord is %s\n", m_currWord.c_str() );
        
    initCreatureFragments();
    
    // Reset history
    if (m_historyRoot)
    {
        delete m_historyRoot;
        m_historyRoot = NULL;
        m_historyCurr = NULL;
    }
    m_historyRoot = historyNodeForCurrentCritter( NULL );
    m_historyCurr = m_historyRoot;
    m_needsLayout = true;
    
    m_savedCreatures.clear();
    saveCreature( false );
    
    // Start playing
    m_startWord = m_currWord;
    m_checkWordTimeleft = -1.0;
    
    // Score
    m_usedWords.clear();
    m_usedWords.push_back( m_currWord );
    
    m_score = 0;
    m_displayedScore = 0;
    
    m_cameraDist = 30.0;
}

void EvoWordGame::initCreatureFragments()
{    
    // Make word fragments
    m_creatureFrags.clear();
    m_creatureFrags.push_back( Fragment( '$', vec2f( 400.0, 100 ) ) ); // extra frags to make word boundries easier
    for (auto ch = m_currWord.begin(); ch != m_currWord.end(); ++ch)
    {
        Fragment f( (*ch), vec2f( 400.0, 100 ) );
        
        m_creatureFrags.push_back(f);
    }
    m_creatureFrags.push_back( Fragment( '$', vec2f( 400.0, 100 ) ) ); // extra frags to make word boundries easier
    
    updateCreatureFrags();
    
    m_creature.evolveCreature( m_currWord, m_creaturePalettes );

}

void EvoWordGame::updateCreatureFrags()
{
    // just update the positions of the creature's DNA
    // target fragments (letters)
    const float letterSize = 50;
    float wordStartX = 400 - (m_creatureFrags.size() * (letterSize/2) );
    for (int i=0; i < m_creatureFrags.size(); i++)
    {
        m_creatureFrags[i].m_bubbleBaseX = wordStartX + (i*letterSize);
    }
}

void EvoWordGame::checkWord()
{
    // Build a candidate word from the creature fragments
    std::string testWord;
    for (auto fi = m_creatureFrags.begin(); fi != m_creatureFrags.end(); ++fi)    
     {
         if (fi->m_letter != '$')
         {
             testWord += fi->m_letter;
         }
     };
    
    printf("Checking test word %s\n", testWord.c_str() );

    // base score = sum of letter tiles
    int wordScore = 0;
    for (auto ch=m_currWord.begin(); ch != m_currWord.end(); ++ch)
    {
        wordScore += g_letterPoints[ (*ch) - 'A' ];
    }

    bool alreadyUsed = false;
    for (auto wi = m_usedWords.begin(); wi != m_usedWords.end(); ++wi)
    {
        if ((*wi)==testWord)
        {
            printf("Already used '%s'....\n", testWord.c_str() );
            alreadyUsed = true;
            break;
        }
    }
    
    if ((!alreadyUsed) && (isWord(testWord)) )
    {
        // Yay, it's a real word
        printf("GOOD WORD!\n" );
        m_currWord = testWord;
        m_creature.evolveCreature( m_currWord, m_creaturePalettes );

        printf("BaseScore %d, finalScore %d\n", wordScore, wordScore*wordScore );
        
        // Final score is base score squared
        wordScore = wordScore*wordScore;
        
        // Add it to the history tree
        HistoryNode *hist = historyNodeForCurrentCritter( m_historyCurr );
        m_historyCurr = hist;
        
        // Remember it was used
        m_usedWords.push_back( m_currWord );
        
        // Add to score
        m_score += wordScore;        
    }
    else
    {
        // Not a real word
        printf("NOT A VALID WORD! Lose %d points\n", wordScore );
        m_creatureFrags = m_oldCreatureFrags;
        
        // Lose points!
        if (m_score > wordScore)
        {
            m_score -= wordScore;
        }
        else m_score = 0;
        
        printf("m_creatureFrags.size() %lu", m_creatureFrags.size() );
    }
}

void EvoWordGame::saveCreature( bool pickNow )
{
    
    // Mark current history
    m_creature.m_historyNode = m_historyCurr;
    
    // Copy thumbnail
    if (m_historyCurr->m_sbThumbnail)
    {        
        // HACK use a history node as a thumbnail holder
        HistoryNode *thumb = new HistoryNode(NULL);
        thumb->m_sbThumbnail = new SpriteBuff( m_historyCurr->m_sbThumbnail->m_texId );
        
        thumb->m_thumbnail = thumb->m_sbThumbnail->makeSprite( 0.0, 1.0, 1.0, 0.0 ); // flip st Y

        thumb->m_thumbnail->x = 500.0;
        thumb->m_thumbnail->y = 1000.0;    

        thumb->m_thumbnail->sx = THUMBNAIL_SIZE;
        thumb->m_thumbnail->sy = THUMBNAIL_SIZE;    
        thumb->m_thumbnail->update(); 
        
        m_creature.m_thumb = thumb;
    }
    
    // save creature
    m_creature.m_word = m_currWord;
    m_savedCreatures.push_back( m_creature );    
    
    if (pickNow)
    {
        // Make user pick a new word
        m_currWord = "";
    }
    
    // Are all the jars full???
    if (m_savedCreatures.size() == 6)
    {
        // Yep, go to review state
        m_gamestate = GameState_REVIEW;
        
        m_scrollPos = 0.0;
        m_scrollLimit = m_historyRoot->maxDepth() * NODE_HEIGHT;        
    }
}

void EvoWordGame::drawSavedCreatures()
{
    const float slotWidth = 133;
    int index = 1;
    for (int i=0; i < NUM_SLOTS; i++)
    {
        char slotbuf[100];
        std::string cname;
        if (i < m_savedCreatures.size() )
        {
            cname = m_savedCreatures[i].m_word;
        }
        sprintf( slotbuf, "[%d]", index );
        float xpos =  (int)((slotWidth/2) + (i*slotWidth));
        m_fontGrobold20->drawStringCentered( xpos, 580.0, slotbuf );    
        m_fontGrobold12->drawStringCentered( xpos, 480.0, cname.c_str() );    
        
        // Align the thumbnail for this creature
        if (i < m_savedCreatures.size() )
        {
            Sprite *thumbSprite = m_savedCreatures[i].m_thumb->m_thumbnail;
            if (thumbSprite)
            {
                thumbSprite->x = xpos;
                thumbSprite->y = 530;
                 
                thumbSprite->update();
            }
        }
        
        index += 1;
    }

}

// draws a tree of saved creatures
void EvoWordGame::drawTree()
{
}

void EvoWordGame::saveCurrentPalette()
{
    std::string filename = gameDataFile("", "palettes.txt" );
    FILE *fp = fopen( filename.c_str(), "a"  );
    
    Pally &pal = m_creature.m_colorScheme;
    fprintf( fp, "%f %f %f\n", pal.m_colorSky.r, pal.m_colorSky.g, pal.m_colorSky.b );
    fprintf( fp, "%f %f %f\n", pal.m_colorOrganic1.r, pal.m_colorOrganic1.g, pal.m_colorOrganic1.b );
    fprintf( fp, "%f %f %f\n", pal.m_colorOrganic2.r, pal.m_colorOrganic2.g, pal.m_colorOrganic2.b );
    fprintf( fp, "%f %f %f\n", pal.m_colorMineral1.r, pal.m_colorMineral1.g, pal.m_colorMineral1.b );
    fprintf( fp, "%f %f %f\n", pal.m_colorMineral2.r, pal.m_colorMineral2.g, pal.m_colorMineral2.b );
    fprintf( fp, "%f %f %f\n\n", pal.m_colorAccent.r, pal.m_colorAccent.g, pal.m_colorAccent.b );

    fclose( fp );
    printf("Saved palette '%s'...\n", filename.c_str() );
}

void EvoWordGame::loadPalettes()
{
    std::string filename = gameDataFile("", "palettes.txt" );

    Pally pal;
    FILE *fp = fopen( filename.c_str(), "r"  );
    while (!feof(fp))
    {
        fscanf( fp, "%f %f %f\n", &pal.m_colorSky.r, &pal.m_colorSky.g, &pal.m_colorSky.b );
        fscanf( fp, "%f %f %f\n", &pal.m_colorOrganic1.r, &pal.m_colorOrganic1.g, &pal.m_colorOrganic1.b );
        fscanf( fp, "%f %f %f\n", &pal.m_colorOrganic2.r, &pal.m_colorOrganic2.g, &pal.m_colorOrganic2.b );
        fscanf( fp, "%f %f %f\n", &pal.m_colorMineral1.r, &pal.m_colorMineral1.g, &pal.m_colorMineral1.b );
        fscanf( fp, "%f %f %f\n", &pal.m_colorMineral2.r, &pal.m_colorMineral2.g, &pal.m_colorMineral2.b );
        fscanf( fp, "%f %f %f\n\n", &pal.m_colorAccent.r, &pal.m_colorAccent.g, &pal.m_colorAccent.b );                   
        
        // this might read an extra palette but who cares since it's
        // past the end of the alphabet
        m_creaturePalettes.push_back( pal );
    }
    fclose(fp);
    printf( "Loaded %d palettes...\n", m_creaturePalettes.size() );
}


#pragma mark - Word Stuff

void EvoWordGame::loadWordList( const char *wordlist )
{
	FILE *fp = fopen( wordlist, "rt" );
	char word[100], clean_word[100];
    printf ("Word list: %s\n", wordlist );
    
    m_startWords = 0;
	while(!feof(fp))
	{
		fscanf( fp, "%s", word );
		char *ch2 = clean_word;
		char lastchar = 'z';
		for (char *ch=word; *ch; ++ch)
		{
			// remove %'s (indicates plurals)..
			if (*ch!='%')
			{
				*ch2 = toupper(*ch);
				ch2++;
			}
			lastchar = *ch;
		}
		*ch2 = '\0';
		
		// add word
		std::string strword(clean_word );
		m_wordList[strword] = true;
        
        // If this is a startword, count it
        if (strword.size()==3) m_startWords++;
	}
    
	// hack? easter egg?
	m_wordList["LUDUM"] = true;
	m_wordList["JOVOC"] = true;
    
	printf( "Loaded %lu words, %d startWords\n", m_wordList.size(), m_startWords );
}

bool EvoWordGame::isWord( const std::string &word )
{
    return true;
    
    
    WordList::iterator wi = m_wordList.find( word );
    return (wi != m_wordList.end());
}
