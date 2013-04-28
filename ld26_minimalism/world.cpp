//
//  world.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//


#include <gamedata.h>
#include <load_obj.h>

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


void World::createMap( std::vector<SceneObj*> &scene )
{

    for (int i=1; i < 21; i++)
    {
        for (int j=1; j < 21; j++)
        {
            m_map[i][j].m_empty = false;
            m_map[i][j].m_passable = true;
            
            vec3f tileLoc( i-10.0, -0.5, j-9.0 );
            SceneObj *tile = new SceneObj( m_groundTile );
            tile->m_xform.Translate(tileLoc);
            
            scene.push_back( tile );
        }
    }
}

void World::load( const std::string &basename, std::vector<SceneObj*> &scene )
{
    // TODO: actually load/build the map
    createMap( scene );

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
        std::string groupName = xGroup->Attribute("name");
//        vec3f tintColor1 = parseColor( xGroup->Attribute("tint1") );
//        vec3f tintColor2 = parseColor( xGroup->Attribute("tint2") );

        printf(" GROUP: %s\n", groupName.c_str() );

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

        // TODO: lookup group by name and add objects
        TiXmlElement *xObject = xGroup->FirstChildElement( "object");
        while (xObject)
        {
            std::string objName = xObject->Attribute("name");
            
            printf("Create object %s\n", objName.c_str() );
            SceneObj *obj = new SceneObj( objName + ".obj" );
            scene.push_back( obj );
            
            vec3f pos = _parseVec( xObject->Attribute("pos" ) );
            
            matrix4x4f xlate, xlateInv;
            xlate.Translate( pos.x, pos.z, -pos.y ); // blender axes are flipped
            xlateInv = xlate;
            xlateInv.Inverse(true);
            
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


