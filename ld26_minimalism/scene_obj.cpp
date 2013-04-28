//
//  scene_obj.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//

#include <map>
#include <load_obj.h>
#include <gamedata.h>

#include "scene_obj.h"

typedef std::map<std::string,QuadBuff<DrawVert>*> MeshLookup;
QuadBuff<DrawVert> *meshFactory( const std::string &objname );

SceneObj::SceneObj(QuadBuff<DrawVert> *mesh) : m_mesh(mesh)
{
    m_tintColor = vec3f( 1.0, 1.0, 1.0 );
    m_xform.Identity();
}

SceneObj::SceneObj( const std::string &meshName )
{
    m_mesh = meshFactory( meshName );
    m_tintColor = vec3f( 1.0, 1.0, 1.0 );
    m_xform.Identity();
}


QuadBuff<DrawVert> *meshFactory( const std::string &objname )
{
    static MeshLookup loadedMeshes;
    
    // Check if we already have the object loaded?
    MeshLookup::iterator findMesh = loadedMeshes.find(objname);
    
    if (findMesh == loadedMeshes.end())
    {
        printf("Didn't find mesh: %s\n", objname.c_str() );
        QuadBuff<DrawVert> * mesh = load_obj( gameDataFile("", objname).c_str() );
        if (!mesh)
        {
            // failed to load
            return NULL;
        }

        // store the mesh for future lookups
        loadedMeshes[objname] = mesh;
        return mesh;
    }
    else
    {
        printf("Found mesh: %s\n", objname.c_str());
        return (*findMesh).second;
    }
    
    
}

