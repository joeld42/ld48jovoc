//
//  scene_obj.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/27/13.
//
//

#ifndef __ld48jovoc__scene_obj__
#define __ld48jovoc__scene_obj__

#include <string>

#include <SDL.h>
#include <SDL_endian.h>

// minibasecode
#include <font.h>
#include <png_loader.h>
#include <shapes.h>


// One mesh in the scene
class SceneObj
{
public:

    SceneObj( const std::string &meshName );
    
    SceneObj(QuadBuff<DrawVert> *mesh);
    
    vec3f m_tintColor;
    matrix4x4f m_xform;
    GLint m_texId;
    QuadBuff<DrawVert> *m_mesh;
};


#endif /* defined(__ld48jovoc__scene_obj__) */
