#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "Gfx/Setup/MeshSetup.h"
#include "Core/Containers/Array.h"
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
    
    glm::vec3 pos;
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

    Oryol::GfxSetup gfxSetup;
    Oryol::DrawState mainDrawState;
    
    // for now, share a shader
    Oryol::Id dispShader;
    
    Oryol::TextureSetup texBluePrint;
    bool didSetupPipeline = false;
};

#endif