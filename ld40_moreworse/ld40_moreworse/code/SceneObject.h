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
    bool hidden;
    bool collider;
    glm::mat4x4 xform;
    glm::mat4x4 invXform;
    
    Oryol::String objectName;
    SceneMesh *mesh;
    CrateShader::vsParams vsParams;
    CrateShader::fsParams fsParams;
    
    //glm::vec3 pos;
    //glm::quat rot;
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
    typedef std::function<void(bool success)> LoadCompleteFunc;
    void LoadScene( Oryol::StringAtom sceneName, LoadCompleteFunc loadComplete );
    
    SceneObject *spawnObject( SceneMesh *mesh );
    SceneObject *spawnObjectByName( Oryol::String name );
    void destroyObject( SceneObject *obj );
    
    SceneObject *FindNamedObject( Oryol::String name );
    
    Oryol::GfxSetup gfxSetup;
    
    Oryol::DrawState sceneDrawState;
    Oryol::VertexLayout meshLayout;
    CrateShader::vsParams sceneVSParams;
    Oryol::Id decalTexture;
    
    // for now, share a shader
    Oryol::Id dispShader;
    
    Oryol::Id defaultTexture;
    Oryol::TextureSetup texBluePrint;
    Oryol::TextureSetup decalBluePrint;
    bool didSetupPipeline = false;
    
    Oryol::Buffer sceneBuff;
    
    
};

#endif
