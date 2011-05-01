//
//  TakeThisGame.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/29/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//
#include <assert.h>

#include "TakeThisGame.h"

#include "VoxChunk.h"
#include "MapRoom.h"

#include "VoxSprite.h"

#include "PNGLoader.h"

TakeThisGame *TakeThisGame::_singleton = NULL;

// How much memory to allocate for the map geo
// let's try 20 MB
#define MAP_VERT_MEM (20 * 1024*1024)


// From http://www.opengl.org/wiki/GluPerspective_code

void glhFrustumf2(matrix4x4f &matrix, 
                  float left, float right, float bottom, float top,
                  float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0 * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    matrix[0][0] = temp / temp2;
    matrix[0][1] = 0.0;
    matrix[0][2] = 0.0;
    matrix[0][3] = 0.0;
    matrix[1][0] = 0.0;
    matrix[1][1] = temp / temp3;
    matrix[1][2] = 0.0;
    matrix[1][3] = 0.0;
    matrix[2][0] = (right + left) / temp2;
    matrix[2][1] = (top + bottom) / temp3;
    matrix[2][2] = (-zfar - znear) / temp4;
    matrix[2][3] = -1.0;
    matrix[3][0] = 0.0;
    matrix[3][1] = 0.0;
    matrix[3][2] = (-temp * zfar) / temp4;
    matrix[3][3] = 0.0;
}

//matrix will receive the calculated perspective matrix.
//You would have to upload to your shader
// or use glLoadMatrixf if you aren't using shaders.
void glhPerspectivef2(matrix4x4f &matrix, float fovyInDegrees, float aspectRatio,
                      float znear, float zfar)
{
    float ymax, xmax;
    float temp, temp2, temp3, temp4;
    ymax = znear * tanf(fovyInDegrees * M_PI / 360.0);
    //ymin = -ymax;
    //xmin = -ymax * aspectRatio;

    xmax = ymax * aspectRatio;
    glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}


TakeThisGame::TakeThisGame() :
    m_playerVel( 0.0, 0.0, 0.0 ),
    m_playerAng( 0.0 )
{
    assert( _singleton == NULL );
    _singleton = this;
}


void TakeThisGame::init()
{
    printf("In init..\n" );
    //chunk = VoxChunk::loadCSVFile( "gamedata/voxtiles/bush.csv" );
    chunk = VoxChunk::loadCSVFile( "gamedata/voxtiles/grass_corner.csv" );
    //chunk = VoxChunk::loadCSVFile( "gamedata/voxtiles/dbgshape.csv" );
    
    printf( "Loaded chunk, size is %d, %d, %d\n",
            chunk->m_xSize,
            chunk->m_ySize,
            chunk->m_zSize );
    
    camAng = 30;
    ang = 0;
    doSpin = false;
    doWire = false;
    doBBox = false;
    fpsMode = false;
    triforce = 0;
    
    m_camQuat.SetAngleAxis( M_PI/2, vec3f( 0,1,0) );
    m_camQuatX.Identity();
    m_camQuatY.Identity();
    
    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
    
    m_playerHurt = 0.0;
    m_heartCtrs = 12;
    m_hitPoints = m_heartCtrs;
    foundCaves.clear();
    hasSword = false;
    
    // Load font
    m_fontImg = LoadImagePNG( "gamedata/nesfont.png" );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );
    
    // Make the map geom
    m_mapVertCapacity = MAP_VERT_MEM / sizeof( VoxVert );
    printf("Alloc %zu mapverts (%zu kb)\n",
           m_mapVertCapacity, m_mapVertCapacity*sizeof(VoxVert) );
    
    m_mapVertData = new VoxVert[m_mapVertCapacity];
    
    // Load "sprites"
    m_player = VoxChunk::loadCSVFile( "gamedata/player.csv" );
    
    
    // Load map tiles
    MapRoom::initTiles();
    
    room = new MapRoom( );
    m_playerPos = vec3f( room->m_xSize/2, 1, room->m_zSize - 2 );
    m_playerPos.y = room->groundHeight( m_playerPos.x, m_playerPos.z );
    
    
    // inst the map
    m_mapVertSize = room->instMapGeo( m_mapVertData, m_mapVertCapacity);
    printf("Map has %zu verts\n", m_mapVertSize );

}

void TakeThisGame::shutdown()
{
    assert( _singleton );
    _singleton->_shutdown();
    

}

void TakeThisGame::_shutdown()
{
    // here .. release stuff
    delete [] m_mapVertData;
    
    SDL_ShowCursor( true );
	SDL_WM_GrabInput( SDL_GRAB_OFF );

    
}

void TakeThisGame::mkHeartCtr( char *buff )
{
    char *ch = buff;
    
    for (int i=1; i < m_heartCtrs; i += 2)
    {
        if (i < m_hitPoints)
        {
            *ch = HEART_FULL;
        }
        else if (i==m_hitPoints)
        {
            *ch = HEART_HALF;
        }
        else
        {
            *ch = HEART_EMPTY;
        }
        *ch++;
    }
    *ch = 0;
}

void TakeThisGame::updateSim( float dtFixed )
{
    if (doSpin)
    {
        ang += dtFixed * 20.0;
    }
    
    float PLAYER_SPEED = 8.0;
    
    // Update player pos
    vec3f newPos = m_playerPos;
    
    newPos = m_playerPos + (PLAYER_SPEED * m_playerVel * dtFixed);
    
    if (room->isVacant( newPos.x + 0.5, newPos.y + 0.6, newPos.z + 0.5))
    {
        
        // adjust ground height
        newPos.y = room->groundHeight( newPos.x + 0.5, newPos.z + 0.5 );
        
        // make sure it's not too great a fall
        if (fabs(newPos.y - m_playerPos.y) < 0.75)
        {
            m_playerPos = newPos;
        }
    }
    
    // did we teleport somewhere?
    int xx,yy,zz;
    xx = floor( m_playerPos.x+0.5 );
    yy = floor( m_playerPos.y+0.5 );
    zz = floor( m_playerPos.z+0.5 );
    MapTile &t = room->m_map[room->index(xx,yy,zz)];
    if (t.teleport)
    {
        
        // Special case -- if the room is CAVE_X, pick
        // a cave based on how many we've visited.
        if (t.teleWhere == MAP_CAVE_X)
        {
            foundCaves.push_back( std::pair<int,int>( xx, zz ) );
            t.teleWhere = MAP_CAVE_X + foundCaves.size();
        }
        
        vec3f pos = t.telePos;
        visitRoom( t.teleWhere );
        m_playerPos = pos;
    }
    
    // update hurt effect
    if (m_playerHurt > 0.0)
    {
        m_playerHurt -= dtFixed;
        if (m_playerHurt < 0.5)
        {
            m_playerVel = 0.0;
        }
        
        
    }

    // allright, now update enemies
    for (size_t i=0; i < room->m_enemies.size(); i++)
    {
        VoxSprite &spr = room->m_enemies[i];
        
        vec3f v = spr.getVelocity();
        vec3f enewPos = spr.m_pos + (v * dtFixed);
        
        if (room->isVacant( enewPos.x + 0.5, enewPos.y + 0.2, enewPos.z + 0.5))
        {
            // don't walk off a cliff
            if (fabs(room->groundHeight(enewPos.x, enewPos.z) - spr.m_pos.y) < 0.25)
            {
                spr.m_pos = enewPos;
            }
            else
            {
                spr.chooseRandomDir();    
            }
        }
        else
        {
            spr.chooseRandomDir();
        }
        
        spr.m_timeout -= dtFixed;
        if (spr.m_timeout < 0.0)
        {
            spr.chooseRandomDir();
        }
        
        //spr.m_pos += (v * dtFixed);
    }
    
    // Check for player <> enemy collision
    for (size_t i=0; i < room->m_enemies.size(); i++)
    {
        // if already injured, don't check hit
        if (m_playerHurt > 0) break;
        
        vec3f d = m_playerPos - room->m_enemies[i].m_pos;
        if (prmath::LengthSquared(d) < 0.8)
        {
            m_playerHurt = 1.0;
            m_playerVel = d * 0.5;
            
            // ouchie
            if (m_hitPoints > 1)
            {
                m_hitPoints--;
            }
            else
            {
                printf("TODO: Game overs...\n" );
            }
            printf("Current health %d/%d\n", m_hitPoints, m_heartCtrs );
        }
    }     
    
}

void TakeThisGame::updateFree( float dtRaw )
{
    
}

void TakeThisGame::redraw()
{
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glhPerspectivef2( m_proj, 40.0, 800.0/600.0, 0.1, 1000.0 );
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glLoadMatrixf( (GLfloat*)(&m_proj) );
   
    matrix4x4f xlate1, xlate2;
    matrix4x4f rot;
    matrix4x4f spin;
    
    if (fpsMode)
    {
        xlate1.Translate( -m_playerPos - vec3f( 0.0, 0.6, 0.0 ));
        
        rot = matrix4x4f( m_camQuat );
                         
        m_modelview = xlate1 * rot;
    }
    else
    {
        xlate1.Translate( -(room->m_xSize/2), 2.0, -(room->m_zSize/2) );
    
        // shift camera slightly based on user's pos
        float xval = ((m_playerPos.x / room->m_xSize) * 2.0) - 1.0;
        float zval = ((m_playerPos.z / room->m_zSize) * 2.0) - 1.0;
    
        xlate2.Translate( -(xval*3), -2, -15 + (zval*-2) );
    
        rot.RotateX( (camAng + (zval*-5)) * 3.14/180.0 );
    
        spin.RotateY( ((ang+ (xval*45))  * 3.14/180.0) );
        
        m_modelview = xlate1 *  spin * rot  * xlate2;
    }
    
               
    //m_modelview.Translate( 0.0, -2, -20 );
    if (doWire)
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );    
    }
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glLoadMatrixf( (GLfloat*)(&m_modelview) );
    
    glDisable( GL_TEXTURE_2D );
    glDisable(GL_BLEND );

    if (doBBox)
    {
        // DBG: draw axes
        glBegin( GL_LINES );
        
        glColor3f( 1.0, 0.0, 0.0 );
        glVertex3f( 0.0, 0.0, 0.0 );
        glVertex3f( 1.0, 0.0, 0.0 );
       
        glColor3f( 0.0, 1.0, 0.0 );
        glVertex3f( 0.0, 0.0, 0.0 );
        glVertex3f( 0.0, 1.0, 0.0 );
       
        glColor3f( 0.0, 0.0, 1.0 );
        glVertex3f( 0.0, 0.0, 0.0 );
        glVertex3f( 0.0, 0.0, 1.0 );
        
        glEnd();
        
        // dbg draw the chunk border
        glColor3f( 1.0, 1.0, 1.0 );

        glBegin( GL_LINES );
        
        for (int y=0; y < 2; y++)
        {
            glVertex3f( 0.0, y*room->m_ySize, 0.0 );
            glVertex3f( room->m_xSize, y*room->m_ySize, 0.0 );
            
            glVertex3f( 0.0, y*room->m_ySize, room->m_zSize );
            glVertex3f( room->m_xSize, y*room->m_ySize, room->m_zSize );
            
            glVertex3f( 0.0, y*room->m_ySize, 0.0 );
            glVertex3f( 0.0, y*room->m_ySize, room->m_zSize );
            
            glVertex3f( room->m_xSize, y*room->m_ySize, 0.0 );
            glVertex3f( room->m_xSize, y*room->m_ySize, room->m_zSize );
            
        }
        
        for (int x=0; x < 2; x++)
        {
            glVertex3f(  room->m_xSize*x, 0.0, 0.0 );
            glVertex3f(  room->m_xSize*x, room->m_ySize, 0.0 );

            glVertex3f(  room->m_xSize*x, 0.0, room->m_zSize );
            glVertex3f(  room->m_xSize*x, room->m_ySize, room->m_zSize );
        }
        
        glEnd();
    }

    // alright draw the map
    VoxVert *vertData;
    size_t numVerts;
    
    //vertData = chunk->genTris( numVerts );
    
    // Draw map
    
    glColor3f( 1.0, 1.0, 1.0 );
    glEnable( GL_CULL_FACE );
    
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof( VoxVert ), m_mapVertData );
    
    glEnableClientState( GL_COLOR_ARRAY );
    glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof( VoxVert ), &(m_mapVertData[0].m_col) );
    
    glDrawArrays( GL_TRIANGLES, 0, m_mapVertSize );
    //printf("Draw %d triangle verts\n", numVerts );
    
    // ----------------------------------
    // Draw player
    //matrix4x4f playerMat = m_modelview;
    matrix4x4f pxform, prot, mat;
    matrix4x4f center, centerI;
    center.Translate( -0.5,-0.5,-0.5 );
    centerI.Translate( 0.5, 0.5, 0.5 );
    
    pxform.Translate( int(m_playerPos.x*WORLD_TILE_SIZE) /WORLD_TILE_SIZE, 
                      int(m_playerPos.y*WORLD_TILE_SIZE) / WORLD_TILE_SIZE, 
                      int(m_playerPos.z*WORLD_TILE_SIZE) /WORLD_TILE_SIZE);
    prot.RotateY( m_playerAng );
    
    
    mat = center * prot * pxform * centerI * m_modelview;
    glLoadMatrixf( (GLfloat*)(&mat) );
    
    size_t playerSz;
    VoxVert *plrVert = m_player->genTris( playerSz );
    glVertexPointer( 3, GL_FLOAT, sizeof( VoxVert ), plrVert );
    glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof( VoxVert ), &(plrVert[0].m_col) );
    
    if (m_playerHurt > 0.0)
    {
        blink = !blink;
    }
    else
    {
        blink = true;
    }
    
    if ((blink) && (!fpsMode))
    {
        glDrawArrays( GL_TRIANGLES, 0, playerSz );
    }
    
    // draw enemies
    for (int i=0; i < room->m_enemies.size(); i++)
    {
        room->m_enemies[i].draw( m_modelview );
    }
    
    
    // restore stuff
    glDisable( GL_VERTEX_ARRAY );
    
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    
    glEnable( GL_TEXTURE );
    glEnable( GL_TEXTURE_2D );
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    // restore modelview
    glLoadMatrixf( (GLfloat*)(&m_modelview) );
    
    // Draw text stuff
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, 800, 0, 600, -1, 1 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    m_nesFont->setColor(224.0/255.0,80.0/255.0, 0.0, 1.0);
    m_nesFont->drawString(630, 600 - 30, "- LIFE -" );
    
    char buff[100];
    mkHeartCtr(buff);
    
    m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
    
    if (blink)
        m_nesFont->drawString(630, 600 - 40, buff );
    
    // stats, inv, etc
    sprintf( buff, "TRIFORCE: %d/4", triforce );
    m_nesFont->drawString( 20, 600-30, buff );
    
    m_nesFont->drawString( 20, 600-60, hasSword?"WEAPON: SWORD":"WEAPON: NONE" );
    
    // a message to the player?
    if (!room->m_message1.empty())
    {
        m_nesFont->drawStringCentered( 400, 450, room->m_message1.c_str() );
        m_nesFont->drawStringCentered( 400, 420, room->m_message2.c_str() );
    }

    m_nesFont->renderAll();
    m_nesFont->clear();
    
#if 0
    // DBG: Player "flag"
    glColor3f ( 1.0, 0.0, 1.0 );
    glBegin( GL_LINES );
    glVertex3f( m_playerPos.x, 0.0, m_playerPos.z );
    glVertex3f( m_playerPos.x, room->m_ySize, m_playerPos.z );
    
    for (float t=0; t < 3.14; t += 0.4 )
    {
        float x = cos(t) * 0.2;
        float z = sin(t) * 0.2;
        glVertex3f( m_playerPos.x + x, m_playerPos.y + 0.1, m_playerPos.z + z );
        glVertex3f( m_playerPos.x - x, m_playerPos.y + 0.1, m_playerPos.z - z );
    }
    
    glEnd();
#endif
    
  
}

void TakeThisGame::mouseMotion( float lookMoveX, float lookMoveY )
{
    const float mouseLookSpeed = 1.0;

    if (!fpsMode) return;
    
    quat4f qrot;		
    
    qrot.SetAngleAxis( (M_PI/180.0)*lookMoveY*mouseLookSpeed, vec3f( 1.0, 0.0, 0.0) );
    m_camQuatX *= qrot;
    
    qrot.SetAngleAxis( (M_PI/180.0)*lookMoveX*mouseLookSpeed, vec3f( 0.0, 1.0, 0.0) );
    m_camQuatY *= qrot;
    
    // combine both axes
    m_camQuat = m_camQuatX * m_camQuatY;  
    


}

void TakeThisGame::updateButtons( unsigned int btnMask )
{
    
    // disable controls if hurt
    if (m_playerHurt > 0.5) return;
    
	// Movment buttons
	if ((btnMask & BTN_LEFT) && (!(btnMask & BTN_RIGHT)) )
	{
		m_playerVel.x = -1.0;
	}
	else if ((btnMask & BTN_RIGHT) && (!(btnMask & BTN_LEFT)) )
	{
		m_playerVel.x = 1.0;
	}
    else m_playerVel.x = 0;
    
    if ((btnMask & BTN_UP) && (!(btnMask & BTN_DOWN)) )
	{
		m_playerVel.z = -1.0;
	}
	else if ((btnMask & BTN_DOWN) && (!(btnMask & BTN_UP)) )
	{
		m_playerVel.z = 1.0;
	}
	else m_playerVel.z = 0.0;
    
    // fpsMode -- rotate by quat cam
    if (fpsMode)
    {
        matrix4x4f rot( m_camQuat);
        m_playerVel = m_playerVel * rot;
    }
    
    
    if ( prmath::LengthSquared( m_playerVel ) > 0.01)
    {
        m_playerAng = -atan2( m_playerVel.z, m_playerVel.x );
    }
    
}

void TakeThisGame::keypress( SDLKey &key )
{
    printf("Key was %c\n", key );
    
    switch(key)
    {
        case 'o':
            // orbit
            doSpin = !doSpin;
            printf("Orbit %s\n", doSpin?"true":"false" );
            break;
        
        case 'i':
            // wIre
            doWire = !doWire;
            break;
        case 'b':
            // BBox
            doBBox = !doBBox;
            break;
        case 'f':
            // First person
            toggleFPSMode();
            break;
            
        default:
            break;
    }
}

void TakeThisGame::toggleFPSMode()
{
    fpsMode = !fpsMode;
    
    SDL_ShowCursor( !fpsMode );
	SDL_WM_GrabInput( fpsMode?SDL_GRAB_ON:SDL_GRAB_OFF );
    
}

void TakeThisGame::visitRoom( int mapCode )
{
    vec3f oldPos = m_playerPos - m_playerVel;
    
    room->buildMap( mapCode );
    m_mapVertSize = room->instMapGeo( m_mapVertData, m_mapVertCapacity);
    
    m_playerPos = room->m_playerStartPos;
    m_playerPos.y = room->groundHeight( m_playerPos.x, m_playerPos.z );
    
    // ugly -- restore any found caves over CAVEX
    for (size_t i=0; i < foundCaves.size(); i++)
    {
        for (int j=0; j < room->m_ySize; j++)
        {
            
            MapTile &t = room->m_map[room->index( foundCaves[i].first,j,
                                                 foundCaves[i].second)];
            if (t.teleWhere == MAP_CAVE_X)
            {
                t.teleWhere = MAP_CAVE_X + (i+1);
            }
        }
    }
    
    // If it's a cave, put a back-loc
    if ((mapCode >= MAP_CAVE_X) && (mapCode <= MAP_CAVE_SWORD))
    {
        for (int x=7; x <=9; x++)
        {
            room->m_map[room->index(x,1, room->m_zSize-1 )].telePos = oldPos;
        }
    }

}
