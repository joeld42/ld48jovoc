
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "shaders.h"
#import "SceneObject.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Assets/Gfx/TextureLoader.h"

using namespace Oryol;

SceneObject *makeObject( SceneObjectInfo *info )
{
    SceneObject *object = new SceneObject();
    object->info = info;
    
    return object;
}

Scene::Scene()
{
}

void Scene::init()
{
    dispShader = Gfx::CreateResource(MainShader::Setup());
        
    texSetup.Sampler.MinFilter = TextureFilterMode::LinearMipmapLinear;
    texSetup.Sampler.MagFilter = TextureFilterMode::Linear;
    texSetup.Sampler.WrapU = TextureWrapMode::Repeat;
    texSetup.Sampler.WrapV = TextureWrapMode::Repeat;
    
}

SceneObject *Scene::addObject( const char *meshName, const char *textureName )
{

    SceneObjectInfo *info = nullptr;
    for (int i=0; i < sceneInfos.Size(); i++) {
        SceneObjectInfo *currInfo = sceneInfos[i];
        if (!strcmp(meshName, currInfo->meshName)) {
            info = currInfo;
            break;
        }
    }
    
    if (!info) {
        printf("Will load SceneObjInfo..%s\n", meshName);
        info = new SceneObjectInfo();
        info->meshName = meshName;
        sceneInfos.Add( info );
        info->mesh = Gfx::LoadResource(MeshLoader::Create(MeshSetup::FromFile( meshName ), [this,info](MeshSetup &setup) {
            
            printf("LOADMESH  '%s' finish block setup (%ld)\n", info->meshName, sizeof(setup) );
            if (!didSetupPipeline) {
                didSetupPipeline = true;
                auto ps = PipelineSetup::FromLayoutAndShader(setup.Layout, dispShader );
                ps.DepthStencilState.DepthWriteEnabled = true;
                ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
                ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
                this->mainDrawState.Pipeline = Gfx::CreateResource(ps);
                this->Layout = setup.Layout;
            }
            
            // finalize info
            info->numPrims = setup.NumPrimitiveGroups();
            info->ready=true;
        }));

        
        info->texture =  Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( textureName, texSetup)));
        
    }
    
    SceneObject *obj = makeObject( info );
    
    sceneObjs.Add( obj );
    return obj;
}


void Scene::drawScene()
{
    
    if (!didSetupPipeline) {
        printf("Pipeline not setup...\n");
        return;
    }
    
    //printf("sizeof mainDrawState is %ld\n", sizeof(mainDrawState) );
    
    for (int i=0; i < sceneObjs.Size(); i++) {
        
        SceneObject *obj = sceneObjs[i];
        SceneObjectInfo *info = obj->info;
        
        if (!info->ready) continue;
        
        const auto resStateTex = Gfx::QueryResourceInfo( info->texture ).State;
        const auto resStateMesh = Gfx::QueryResourceInfo( info->mesh).State;
        
        if ((resStateTex == ResourceState::Valid) && (resStateMesh == ResourceState::Valid)) {
            this->mainDrawState.FSTexture[Textures::Texture] = info->texture;
            this->mainDrawState.Mesh[0] = info->mesh;
            
            Gfx::ApplyDrawState(this->mainDrawState);            
            Gfx::ApplyUniformBlock( obj->vsParams);
            
            for (int j=0; j < info->numPrims; j++) {
                Gfx::Draw(j);
            }
        }
        
    }
}
