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

bool MapSquare::isWalkable()
{
    return ((!m_empty) && (m_passable));
}


void World::init()
{
    m_groundTile = load_obj( gameDataFile("", "ground_tile.obj").c_str() );    
}


void World::_createMap( const std::string &filename, std::vector<SceneObj*> &scene )
{

    // Load the map data
    PNGImage worldImg = LoadImagePNG( filename.c_str(), false, true );
    
    
    // make the map
    uint8_t *mapdata = worldImg.pixelData;
    for (int i=1; i < 21; i++)
    {
        for (int j=1; j < 21; j++)
        {
            int j2 = 20-(j-1);
            uint8_t r,g,b,a;
            uint32_t ndx = ((j2 * worldImg.widthPow2) + (i-1)) * 4;
            printf("i,j %d %d ndx %d widthPow2 %d\n", i,j, ndx, worldImg.widthPow2 );
            r = mapdata[ ndx + 0 ];
            g = mapdata[ ndx + 1 ];
            b = mapdata[ ndx + 2 ];
            a = mapdata[ ndx + 3 ];
            
            if (a<128)
            {
                m_map[i][j].m_empty = true;
                m_map[i][j].m_passable = false;
            }
            else
            {
                m_map[i][j].m_empty = false;
                m_map[i][j].m_passable = true;
                
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
    _createMap( gameDataFile("", basename + "_map.png"), scene );

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
        ginfo.m_tint1 = _parseColor(xGroup->Attribute("tint1"));
        ginfo.m_tint2 = _parseColor(xGroup->Attribute("tint2"));
        
        printf(" GROUP: %s\n", groupName.c_str() );
        m_groupInfo[groupName] = ginfo;

        xGroup = xGroup->NextSiblingElement( "group" );
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
            vec3f pos = _parseVec( xObject->Attribute("pos" ) );

            // hack adjust to line up with blender
            pos += vec3f( 1.0, -2.0, 0.0 );

            matrix4x4f xlate;
            xlate.Translate( pos.x, pos.z, -pos.y ); // blender axes are flipped
            
            
            vec3f euler = _parseVec( xObject->Attribute("rot" ) );
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

vec3f World::_parseVec( const char *vecStr )
{
    float x=0, y=0, z=0;
    sscanf( vecStr, "%f,%f,%f", &x, &y, &z);
    
    return vec3f( x, y, z );
}

vec3f World::_parseColor( const char *colorStr )
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

