//
//  world.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//


#include <gamedata.h>
#include <load_obj.h>
#include <useful.h>


#include <tinyxml/tinyxml.h>

#include "world.h"
#include "tex_db.h"
#include "monster_def.h"

bool MapSquare::isWalkable()
{
    return ((!m_empty) && (m_passable));
}


void World::init()
{
    m_groundTile = load_obj( gameDataFile("", "ground_tile.obj").c_str() );    
}


void World::_createMap( const std::string &filename, const std::string &infoname, std::vector<SceneObj*> &scene )
{

    // Load the map data
    PNGImage worldImg = LoadImagePNG( filename.c_str(), false, true );
    PNGImage worldInfo = LoadImagePNG( infoname.c_str(), false, true );
    
    // make the map
    uint8_t *mapdata = worldImg.pixelData;
    uint8_t *infodata = worldInfo.pixelData;

    for (int i=1; i < 21; i++)
    {
        for (int j=1; j < 21; j++)
        {
            int j2 = 20-(j-1);
            uint8_t r,g,b,a, blk;
            uint32_t ndx = ((j2 * worldImg.widthPow2) + (i-1)) * 4;
            r = mapdata[ ndx + 0 ];
            g = mapdata[ ndx + 1 ];
            b = mapdata[ ndx + 2 ];
            a = mapdata[ ndx + 3 ];
            
            blk = infodata[ ndx + 0];
            
            
            if (a<128)
            {
                m_map[i][j].m_empty = true;
                m_map[i][j].m_passable = false;
            }
            else
            {
                m_map[i][j].m_empty = false;
                m_map[i][j].m_passable = (blk < 128);
                
                vec3f tileLoc( i-10.0, -0.5, j-9.0 );
                SceneObj *tile = new SceneObj( m_groundTile );
                tile->m_xform.Translate(tileLoc);
                tile->m_tintColor = vec3f( r / 256.0, g/256.0, b/256.0 );

                scene.push_back( tile );
            }
        }
    }
}

void World::load( const std::string &basename, std::vector<SceneObj*> &scene )
{
    // load/build the map
    _createMap( gameDataFile("", basename + "_map.png"),
                gameDataFile("", basename + "_info.png"),
                scene );

    // Load the story file
    std::string storyfile = basename + ".story.xml";
    _loadStoryFile( gameDataFile( "", storyfile.c_str() ), scene );

    std::string scenefile = basename + ".scene.xml";
    _loadSceneFile( gameDataFile( "", scenefile.c_str() ), scene );

}
void World::_loadStoryFile( const std::string &filename, std::vector<SceneObj*> &scene )
{
    TiXmlDocument doc( filename.c_str() );
    if (!doc.LoadFile())
    {
        printf( "Error loading story file: %s", filename.c_str() );
        return;
    }

    TiXmlElement *xStory;
    TiXmlNode *xText;

    xStory = doc.FirstChildElement( "story" );
    assert( xStory );

    m_title = xStory->Attribute( "title" );
    
    m_bgColor = parseColor( xStory->Attribute("bg") );
    
    const char *startPos = xStory->Attribute("startPos");
    sscanf( startPos, "%d,%d", &m_startPosX, &m_startPosY );

    // get intro
    TiXmlElement *xIntro = xStory->FirstChildElement( "intro" );
    xText = xIntro->FirstChild();
    m_intro = xText->Value();

    // get outro
    TiXmlElement *xOutro = xStory->FirstChildElement( "outro" );
    xText = xOutro->FirstChild();
    m_outro = xText->Value();

    // get groups
    TiXmlElement *xGroup = xStory->FirstChildElement( "group");
    while (xGroup)
    {
        GroupInfo ginfo;
        
        std::string groupName = xGroup->Attribute("name");
        ginfo.m_tint1 = parseColor(xGroup->Attribute("tint1"));
        ginfo.m_tint2 = parseColor(xGroup->Attribute("tint2"));
        
        printf(" GROUP: %s\n", groupName.c_str() );
        m_groupInfo[groupName] = ginfo;

        xGroup = xGroup->NextSiblingElement( "group" );
    }
    
    // get actors
    TiXmlElement *xMonster = xStory->FirstChildElement( "monster");
    while (xMonster)
    {
        std::string monsterId = xMonster->Attribute("id");
        MonsterDefinition mdef = MonsterDB::sharedDB()->getMonster( monsterId );
        
        SceneObj *actorObj = new SceneObj( mdef.m_meshName+".obj" );
        actorObj->m_tintColor = mdef.m_tintColor;
        actorObj->m_texId = texDBLookup( mdef.m_skinName + ".png" );
                
        int x, y;
        sscanf( xMonster->Attribute("pos"), "%d,%d", &x, &y );
        Actor *actor = new Actor( actorObj );
        actor->setPos(x, y );
        scene.push_back( actorObj );
        
        actor->m_alignment = mdef.m_align;
        actor->m_behavior = mdef.m_behavior;
        actor->m_hp = mdef.m_baseHP;
        
        m_actors.push_back(actor);
        
        xMonster = xMonster->NextSiblingElement( "monster" );
    }
    
    // DBG
    printf(" TITLE: %s\n", m_title.c_str() );
    printf(" INTRO: %s\n\n", m_intro.c_str() );
    printf(" OUTRO: %s\n\n", m_outro.c_str() );

}

void World::_loadSceneFile( const std::string &filename, std::vector<SceneObj*> &scene )
{
    TiXmlDocument doc( filename.c_str() );
    if (!doc.LoadFile())
    {
        printf( "Error loading scene file: %s", filename.c_str() );
        return;
    }

    TiXmlElement *xSceneObjs;

    xSceneObjs = doc.FirstChildElement( "sceneObjs" );
    assert( xSceneObjs );

    // get groups
    TiXmlElement *xGroup = xSceneObjs->FirstChildElement( "group");
    while (xGroup)
    {
        std::string groupName = xGroup->Attribute("name");
        GroupInfo &ginfo = m_groupInfo[groupName];
        

        // TODO: lookup group by name and add objects
        TiXmlElement *xObject = xGroup->FirstChildElement( "object");
        while (xObject)
        {
            std::string objName = xObject->Attribute("name");
            
            printf("Create object %s\n", objName.c_str() );
            SceneObj *obj = new SceneObj( objName + ".obj" );
            scene.push_back( obj );

            // get color
            float tcol = randUniform();
            obj->m_tintColor = (ginfo.m_tint1*tcol) + (ginfo.m_tint2*(1.0-tcol));


            // get transform
            vec3f pos = parseVec( xObject->Attribute("pos" ) );

            // hack adjust to line up with blender
            pos += vec3f( 1.0, -2.0, 0.0 );

            matrix4x4f xlate;
            xlate.Translate( pos.x, pos.z, -pos.y ); // blender axes are flipped
            
            
            vec3f euler = parseVec( xObject->Attribute("rot" ) );
            euler = vec3f( euler.x, euler.z, -euler.y );
            //euler = vec3f( euler.x, 0.0, 0.0 );
//            if (objName=="block_tall_tri")
//            {
//                euler = vec3f(M_PI_2, 0.0, 0.0 );
//            }

            matrix4x4f rot;
            rot.RotateEuler( euler, prmath::EULER_XZY );

//            rot = xlateInv * rot * xlate;

            // Apply to the scene obj
            obj->m_xform = rot * xlate;

            xObject = xObject->NextSiblingElement( "object" );
        }

        xGroup = xGroup->NextSiblingElement( "group" );
    }
}

void World::simTurn()
{
    printf("... actor turn (%ld actors)...\n", m_actors.size() );

    // update routing to player
    _updateRoutes();

    // sim actors
    for (auto ai = m_actors.begin(); ai != m_actors.end(); ++ai)
    {
        // sim this actor
        _simActor( *ai );
    }
}

void World::_updateRoutes()
{
    // reset distances
    for (int i=0; i < 22; i++)
    {
        for (int j=0; j < 22; j++)
        {
            m_map[i][j].m_playerDist = 999;
        }
    }
    
    // iterate until no more changes
    m_map[m_player->m_mapX][m_player->m_mapY].m_playerDist = 0;
    bool didChange = true;
    int c = 0;
    while ((didChange) && (c < 500))
    {
        didChange = false;
        for (int i=1; i < 22; i++)
        {
            for (int j=1; j < 22; j++)
            {
                if (!m_map[i][j].isWalkable()) continue;
                
                // check all four directions
                int bestDist = m_map[i][j].m_playerDist;
                for (int k=0; k < 4; k++)
                {
                    int cx=i, cy=j;
                    _adj(k, cx, cy);
                    if (m_map[cx][cy].m_playerDist + 1 < bestDist)
                    {
                        bestDist = m_map[cx][cy].m_playerDist + 1;
                        didChange = true;
                        m_map[i][j].m_playerDist = bestDist;
                    }
                }
            }
        }
    }

    for (int i=0; i < 22; i++)
    {
        for (int j=0; j < 22; j++)
        {
            printf(" %3d", m_map[i][j].m_playerDist );
        }
        printf("\n" );
    }
    
}

void World::_simActor( Actor *actor )
{
    // if they are evil, try to attack the player if nearby
    // TODO: flee if weakened
    if (actor->m_alignment == Alignment_EVIL)
    {
//        printf("Actor is EVIL\n" );
        int cx, cy; // check xy
        for (int i=0; i < 4; i++)
        {
            cx = actor->m_mapX;
            cy = actor->m_mapY;
            _adj(i, cx, cy);
            
//            printf("Checking player %d %d vs %d %d\n",
//                   m_player->m_mapX, m_player->m_mapY,
//                   cx, cy );
            
            if ((m_player->m_mapX==cx) && (m_player->m_mapY==cy))
            {
                // Yes! Attack!
                printf("yes attack\n");
                _attackPlayer( actor );
                return;
            }
        }
    }
    
    // not attacking, wander around
    int walkDir = -1;
    if (actor->m_behavior == Behavior_WANDER)
    {
        walkDir = rand() % 4;
    }
    else if (actor->m_behavior == Behavior_SEEKPLAYER)
    {
        int bestDist = m_map[actor->m_mapX][actor->m_mapY].m_playerDist;
        for (int i=0; i < 4; i++)
        {
            int cx = actor->m_mapX;
            int cy = actor->m_mapY;
            
            _adj( i, cx, cy );
            int mapDist = m_map[cx][cy].m_playerDist;
            printf("Dir %d dist is %d\n", i, mapDist );
            if (mapDist< bestDist)
            {
                walkDir = i;
                bestDist = mapDist;
            }
        }
        printf("Seekplayer, direction is %d (currDist %d, bestDist %d)\n",
               walkDir, m_map[actor->m_mapX][actor->m_mapY].m_playerDist, bestDist);
    }
    
    if (walkDir!=-1)
    {
        int cx = actor->m_mapX;
        int cy = actor->m_mapY;
        
        _adj( walkDir, cx, cy );
        if (!_isOccupied(cx, cy))
        {
            actor->setPos( cx, cy );
        }
    }
}

bool World::_isOccupied( int x, int y )
{
    // passable?
    if (!m_map[x][y].isWalkable()) return true;
    
    // player is there...
    if ((m_player->m_mapX == x) && (m_player->m_mapY == y)) return true;
    
    // another actor is there...
    for (auto ai = m_actors.begin(); ai != m_actors.end(); ++ai)
    {
        Actor *actor = *ai;
        if ((actor->m_mapX == x) && (actor->m_mapY==y))
        {
            return true;
        }
    }
    
    return false;
}

void World::_attackPlayer( Actor *actor )
{
    printf("Attack player!!\n");
}

void World::_adj( int dir, int &x, int &y )
{
    if (dir==0) x -= 1;
    else if (dir==1) x+= 1;
    else if (dir==2) y -= 1;
    else if (dir==3) y += 1;
}

vec3f World::parseVec( const char *vecStr )
{
    float x=0, y=0, z=0;
    sscanf( vecStr, "%f,%f,%f", &x, &y, &z);
    
    return vec3f( x, y, z );
}

vec3f World::parseColor( const char *colorStr )
{
    if (!colorStr)
    {
        return vec3f( 0.5, 0.5, 0.5 );
    }
    printf( "parseColor: %s\n", colorStr);
    int r, g, b;
    sscanf( colorStr, "%02x%02x%02x", &r, &g, &b );
    
    printf( "%d %d %d\n", r,g,b);
    
    return vec3f( r/256.0, g/256.0, b/256.0 );
}

