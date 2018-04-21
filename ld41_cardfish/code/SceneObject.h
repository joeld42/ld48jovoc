#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "DebugDraw.h"
#include "Ray.h"

//#include "Gfx/Setup/MeshSetup.h"
#include "Core/Containers/Array.h"

#include "glm/gtc/quaternion.hpp"
#include "shaders.h"

namespace Tapnik
{

class SceneCamera
{
public:
    Oryol::String cameraName;
    glm::mat4x4 mat;
    int index;
};
    
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
    
    class InteractionInfo
    {
    public:
        glm::mat4x4 invXform;
        bool mouseHovering;
        bool hoverDirty; // Force recheck hover even if mouse hasn't moved
    };
    
    bool hidden;
    bool collider;
    glm::mat4x4 xform;
    
    // If this is true, invXForm will be calculated and picking functions will work...
    InteractionInfo *interaction;
    
    Oryol::String objectName;
    SceneMesh *mesh;
    
    WorldShader::vsParams vsParams;
    
    bool isCard;
    
    //glm::vec3 pos;
    //glm::quat rot;
    
    void makeInteractable();
};


class Scene
{
public:
    Scene();
    void Setup( Oryol::GfxSetup *gfxSetup );
    
    void drawScene();
    
    void finalizeTransforms(  glm::mat4 matViewProj, glm::mat4 matViewProjCards );
    
    Oryol::Array<SceneMesh> sceneMeshes;
    Oryol::Array<SceneObject*> sceneObjs;
    Oryol::Array<SceneCamera> sceneCams;
    
    //SceneObject *addObject( const char *meshName, const char *textureName );
    typedef std::function<void(bool success)> LoadCompleteFunc;
    void LoadScene( Oryol::StringAtom sceneName, LoadCompleteFunc loadComplete );
    
    void CreateCardMeshes();
    
    SceneMesh *findMeshByName( Oryol::String meshName );
    
    SceneObject *spawnObject( SceneMesh *mesh );
    SceneObject *spawnObjectByName( Oryol::String name );
    SceneObject *spawnObjectWithMeshNamed( Oryol::String meshName );
    
    SceneCamera findNamedCamera( Oryol::String cameraName );
    
    void destroyObject( SceneObject *obj );
    
    SceneObject *FindNamedObject( Oryol::String name );
    
    // Hack to reorder draw order... need some kind of real sort?
    void BringToFront( SceneObject *frontObj );
    
    void drawSceneLayer( Oryol::DrawState drawState, bool cardsLayer);
        
    Oryol::DrawState sceneDrawState;
    Oryol::DrawState cardDrawState;

    Oryol::VertexLayout meshLayout;
    
    WorldShader::vsParams sceneVSParams;
    //WorldShader::fsParams sceneFSParams;
        
    Oryol::Id tileFontTexture;
    Oryol::Id boardIconTexture;
    
    Oryol::Id worldShader;
    
    Oryol::Id defaultTexture;
    Oryol::Id cardsTexture;
    Oryol::TextureSetup texBluePrint;
    Oryol::TextureSetup decalBluePrint;
    bool didSetupPipeline = false;
    
    Oryol::Buffer sceneBuff;
        
};

bool RayHitObject( SceneObject *obj, Ray ray );
    
void dbgPrintMatrix( const char *label, glm::mat4 m );
void dbgDrawBBox( SceneObject *obj, float *color );

} // namespace Tapnik


#endif
