#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "Core/Containers/Array.h"

#include "glm/gtc/quaternion.hpp"
#include "shaders.h"

class SceneObjectInfo
{
public:
    const char *meshName; // tmp
    
    int numPrims = 1;
	Oryol::Id mesh;
	Oryol::Id shader;
	Oryol::Id texture;
    
    bool ready = false;
};


class SceneObject
{
public:
    SceneObjectInfo *info;
    Oryol::MainShader::VSParams vsParams;
    Oryol::MainShader::FSParams fsParams;
    
    glm::vec3 pos;
    glm::vec3 scale;
    glm::quat rot;
    
    bool enabled;
};


class Scene
{
public:
    Scene();
    void init();
    
    void drawScene();
    
    Oryol::Array<SceneObjectInfo*> sceneInfos;
    Oryol::Array<SceneObject*> sceneObjs;
    
    SceneObject *addObject( const char *meshName, const char *textureName );
    void removeObject( SceneObject *obj );
    
    Oryol::GfxSetup gfxSetup;
    Oryol::DrawState mainDrawState;
    
    // for now, share a shader
    Oryol::Id dispShader;
    
    Oryol::TextureSetup texSetup;
    bool didSetupPipeline = false;
    Oryol::VertexLayout Layout;
};

SceneObject *makeObject( SceneObjectInfo *info );

#endif
