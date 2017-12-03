
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "shaders.h"
#import "SceneObject.h"

#include "IO/IO.h"
#include "Core/String/StringBuilder.h"
#include "Core/Memory/Memory.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Assets/Gfx/TextureLoader.h"

#include "ldjamfile.h"

using namespace Oryol;

extern void dbgPrintMatrix( const char *label, glm::mat4 m );

SceneObject *makeObject( SceneMesh *mesh )
{
    //SceneObject *object = new SceneObject();
    SceneObject *object = Memory::New<SceneObject>();
    object->mesh = mesh;
    object->hidden = false;
    object->collider = false;
    object->vsParams.tintColor = glm::vec4(1);
    
    return object;
}

// Spawns a new object from the mesh and adds it to scene
SceneObject *Scene::spawnObject( SceneMesh *mesh )
{
    SceneObject *object = makeObject( mesh );
    sceneObjs.Add( object );
    return object;
}

SceneObject *Scene::spawnObjectByName( Oryol::String name )
{
    SceneObject *srcObject = FindNamedObject( name );
    if (srcObject) {
        return spawnObject( srcObject->mesh );
    } else {
        Log::Warn( "spawnObjectByName: Didn't find object '%s'\n", name.AsCStr() );
        return NULL;
    }
}

Scene::Scene()
{
}


void Scene::Setup()
{
    // create shader and pipeline-state-object
    meshLayout = {
        { VertexAttr::Position,  VertexFormat::Float3 },
        { VertexAttr::Normal,    VertexFormat::Float3 },
        { VertexAttr::TexCoord0, VertexFormat::Float2 },
        { VertexAttr::TexCoord1, VertexFormat::Float2 },
    };
    Id shd = Gfx::CreateResource(TestShader::Setup());
    
    // Setup our texture blueprint
    texBluePrint.Sampler.MinFilter = TextureFilterMode::LinearMipmapLinear;
    texBluePrint.Sampler.MagFilter = TextureFilterMode::Linear;
    texBluePrint.Sampler.WrapU = TextureWrapMode::Repeat;
    texBluePrint.Sampler.WrapV = TextureWrapMode::Repeat;
    
    auto ps = PipelineSetup::FromLayoutAndShader( meshLayout, shd);
    ps.RasterizerState.CullFaceEnabled = false;
    ps.RasterizerState.CullFace = Face::Code::Front;
    ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    
    this->sceneDrawState.Pipeline = Gfx::CreateResource(ps);
    
    // Load test texture
    texBluePrint.Sampler.MinFilter = TextureFilterMode::LinearMipmapLinear;
    texBluePrint.Sampler.MagFilter = TextureFilterMode::Linear;
    texBluePrint.Sampler.WrapU = TextureWrapMode::Repeat;
    texBluePrint.Sampler.WrapV = TextureWrapMode::Repeat;
    testTexture =  Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "gamedata:TestTile_basecolor.dds", texBluePrint)));
}

void Scene::LoadScene( Oryol::StringAtom sceneName, Scene::LoadCompleteFunc loadComplete )
{

    StringBuilder strBuilder;
    strBuilder.Format( 4096, "gamedata:%s.ldjam", sceneName.AsCStr() );
    Log::Info("fetch scene %s", strBuilder.GetString().AsCStr() );
    IO::Load(strBuilder.GetString(), [this,loadComplete](IO::LoadResult loadResult) {
        
        Log::Info("Loadresult size is %d\n", loadResult.Data.Size() );
        sceneBuff = std::move(loadResult.Data);
        
        LDJamFileHeader *fileHeader = (LDJamFileHeader*)sceneBuff.Data();
        Log::Info("Loaded chunks, loadresult sz %d, fileVer %d numChunks is %d \n", loadResult.Data.Size(), fileHeader->m_fileVersion, fileHeader->m_numChunks );
        
        if (fileHeader->m_fileVersion != LDJAMFILE_VERSION) {
            Log::Error("ldjam file version is stale! expected version %d, got %d!\n",
                       LDJAMFILE_VERSION, fileHeader->m_fileVersion );
        }
        
        LDJamFileMeshInfo *meshInfos = (LDJamFileMeshInfo *)(sceneBuff.Data() + sizeof(LDJamFileHeader));
        
        
        for (size_t i=0; i < fileHeader->m_numChunks; i++) {
            
            SceneMesh mesh = {};
            LDJamFileMeshInfo *meshInfo = meshInfos + i;
            
            mesh.bboxMin = meshInfo->m_bboxMin;
            mesh.bboxMax = meshInfo->m_bboxMax;
            
            // For now, everything is loaded, TODO load the contents part
            // when needed
            LDJamFileMeshContent *meshContent = (LDJamFileMeshContent*)( sceneBuff.Data() + meshInfo->m_contentOffset );
            LDJamFileVertex *meshVertData = (LDJamFileVertex *)(sceneBuff.Data() + meshInfo->m_contentOffset + sizeof(LDJamFileMeshContent) );
            
//            printf("Chunk %zu -- %s sz %f %f %f -- %f numtris %d\n", i, chunkInfo->m_name,
//                   chunkInfo->m_bboxMin.x, chunkInfo->m_bboxMin.y, chunkInfo->m_bboxMin.z,
//                   glm::length( chunkInfo->m_bboxMin ), chunkContent->m_triIndices/3 );
            
            
            auto meshSetup = MeshSetup::FromData();
            meshSetup.NumVertices = meshContent->m_numVerts;
            meshSetup.NumIndices = meshContent->m_triIndices;
            
            meshSetup.IndicesType = IndexType::Index16;
            meshSetup.Layout = meshLayout;
            
            int numVerts = meshContent->m_numVerts;
            meshSetup.AddPrimitiveGroup({0, meshContent->m_triIndices });
            meshSetup.VertexDataOffset = 0;
            meshSetup.IndexDataOffset = sizeof(LDJamFileVertex) * numVerts;
            
            
            //            uint16_t *indices = (uint16_t*)( (uint8_t*)(chunkVertData) + meshSetup.IndexDataOffset );
            //            for (int i=0; i < chunkContent->m_triIndices; i++) {
            //                printf("%d: %d\n", i, indices[i] );
            //            }
            //
            //            for (int i=0; i< numVerts; i++) {
            //                TKChunkFileVertex *cv = chunkVertData + i;
            //                //cv->m_pos = glm::vec3( float(i), 3.14f, 1234.0f );
            //                printf("point %d [%p]: %f %f %f st0 %f %f\n", i, cv, cv->m_pos.x, cv->m_pos.y, cv->m_pos.z, cv->m_st0.x, cv->m_st0.y );
            //            }
            
            size_t meshDataSize = (sizeof(LDJamFileVertex) * numVerts) + (sizeof(uint16_t) * meshContent->m_triIndices);
            mesh.mesh = Gfx::CreateResource(meshSetup, meshVertData, meshDataSize );
            
            mesh.meshName = String( meshInfo->m_name );
            
            // Assign texture if there is one
            Log::Info("Texture is '%s'\n", meshInfo->m_texture );
            if (*(meshInfo->m_texture)) {
                StringBuilder texLocBuilder;
                texLocBuilder.Format( 4096, "gamedata:%s.dds", meshInfo->m_texture );
                Log::Info("Locator is %s\n", texLocBuilder.GetString().AsCStr() );
                mesh.texture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( Locator(texLocBuilder.GetString()), texBluePrint)));
            } else {
                Log::Info("Using default texture");
                mesh.texture = testTexture;
            }
            mesh.numPrims = 1;
            sceneMeshes.Add( mesh );
        }
        // Now add the SceneObjs
        LDJamFileSceneObject *sceneObjBase = (LDJamFileSceneObject*)(sceneBuff.Data() + fileHeader->m_sceneObjOffs);
        Log::Info("%d scene objs...",  fileHeader->m_numSceneObjs );
        
        for (uint32_t i=0; i < fileHeader->m_numSceneObjs; i++) {
            LDJamFileSceneObject *sceneObjInfo = sceneObjBase + i;
            Log::Info("SceneObj[%d] is %s\n", i, sceneObjInfo->m_name );
            SceneObject *sceneObj = makeObject( &sceneMeshes[sceneObjInfo->m_meshIndex] );
            sceneObj->objectName = String( sceneObjInfo->m_name );
            
            // NOTE: for some reason sceneObj->xform = sceneObjInfo->m_transform doesn't work in emscripten
            // TODO: investigate why
            
            //sceneObj->xform = sceneObjInfo->m_transform; // Doesn't work in emscripten
//            Log::Info("alignment of lhs SceneObj %lu\n", ((unsigned long)(sceneObj) & 31) );
//            Log::Info("alignment of lhs xform %lu\n", ((unsigned long)(&sceneObj->xform) & 31) );
//            dbgPrintMatrix( "scene xform", sceneObj->xform );
//            dbgPrintMatrix( "file xform", sceneObjInfo->m_transform );
            
            sceneObj->xform = glm::mat4x4( sceneObjInfo->m_transform ); // works in emscripten
            
            //dbgPrintMatrix( "file xform", sceneObjInfo->m_transform );
            //dbgPrintMatrix( "scene xform", sceneObj->xform );

            //dbgPrintMatrix( "sceneObj xform",sceneObj->xform );
            
            // Semi-HACK, if an object name starts with _, it's a collider
            if (sceneObjInfo->m_name[0]=='_') {
                sceneObj->hidden = true;
                sceneObj->collider = true;
            }
            
            
            sceneObjs.Add( sceneObj );        
        }
      
        loadComplete( true );
    });

}

/*
SceneObject *Scene::addObject( const char *meshName, const char *textureName )
{

    SceneMesh *mesh = nullptr;
    for (int i=0; i < sceneMeshes.Size(); i++) {
        SceneMesh *currMesh = sceneMeshes[i];
        if (!strcmp(meshName, currMesh->meshName)) {
            mesh = currMesh
            break;
        }
    }
    
    if (!info) {
        info = new SceneObjectInfo();
        info->meshName = meshName;
        sceneInfos.Add( info );
        info->mesh = Gfx::LoadResource(MeshLoader::Create(MeshSetup::FromFile( meshName ), [this,info](MeshSetup &setup) {
            
//            printf("LOADMESH  '%s' finish block setup (%ld)\n", info->meshName, sizeof(setup) );
            if (!didSetupPipeline) {
                didSetupPipeline = true;
                auto ps = PipelineSetup::FromLayoutAndShader(setup.Layout, dispShader );
                ps.DepthStencilState.DepthWriteEnabled = true;
                ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
                ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
                this->sceneDrawState.Pipeline = Gfx::CreateResource(ps);
            }
            
            // finalize info
            info->numPrims = setup.NumPrimitiveGroups();
            info->ready=true;
        }));

        
        info->texture =  Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( textureName, texBluePrint)));
        
    }
    
    SceneObject *obj = makeObject( info );
    
    sceneObjs.Add( obj );
    return obj;
}
*/

SceneObject *Scene::FindNamedObject( Oryol::String name )
{
    for (int i=0; i < sceneObjs.Size(); i++) {
        if (sceneObjs[i]->objectName == name) {
            return sceneObjs[i];
        }
    }
    return NULL;
}

void Scene::drawScene()
{
    for (int i=0; i < sceneObjs.Size(); i++) {
        
        SceneObject *obj = sceneObjs[i];
        SceneMesh *mesh = obj->mesh;
        
        if (obj->hidden) continue;
        
        const auto resStateTex = Gfx::QueryResourceInfo( mesh->texture ).State;
        const auto resStateMesh = Gfx::QueryResourceInfo( mesh->mesh ).State;
        
        if (resStateMesh == ResourceState::Valid) {
            if (resStateTex == ResourceState::Valid) {
                this->sceneDrawState.FSTexture[TestShader::tex] = mesh->texture;
            }
            this->sceneDrawState.Mesh[0] = mesh->mesh;
            
            Gfx::ApplyDrawState(this->sceneDrawState);            
            Gfx::ApplyUniformBlock( obj->vsParams);
            
            for (int j=0; j < mesh->numPrims; j++) {
                Gfx::Draw(j);
            }
        }
    }
    
}
