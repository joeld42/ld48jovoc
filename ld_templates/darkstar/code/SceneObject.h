#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

//#include "Gfx/Setup/MeshSetup.h"
#include "Core/Containers/Array.h"

#include "glm/gtc/quaternion.hpp"
#include "shaders.h"

class SceneMesh
{
public:
    Oryol::String meshName;
    
    int numPrims = 1;
	Oryol::Id mesh;
	Oryol::Id shader;
	Oryol::Id texture;
    
    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    
    bool ready = false;
};


class SceneObject
{
public:
    SceneMesh *mesh;
    TestShader::vsParams vsParams;
    
    glm::vec3 pos;
    glm::quat rot;
};

class Scene
{
public:
    Scene();
    void Setup();
    
    void drawScene();
    
    Oryol::Array<SceneMesh> sceneMeshes;
    Oryol::Array<SceneObject*> sceneObjs;
    
    
    //SceneObject *addObject( const char *meshName, const char *textureName );

    void LoadScene( Oryol::StringAtom sceneName );
    
    Oryol::GfxSetup gfxSetup;
    
    Oryol::DrawState sceneDrawState;
    Oryol::VertexLayout meshLayout;
    TestShader::vsParams sceneVSParams;
    
    // for now, share a shader
    Oryol::Id dispShader;
    
    Oryol::TextureSetup texBluePrint;
    bool didSetupPipeline = false;
};

#endif
