
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "shaders.h"
#import "SceneObject.h"

#include "IO/IO.h"
#include "Core/String/StringBuilder.h"

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
    SceneObject *object = new SceneObject();
    object->mesh = mesh;
    
    return object;
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
    
    // Add all the chunkset identifiers
    //AddNamedChunkset( StringAtom("TESTCUBE") );
    //AddNamedChunkset( StringAtom("CAVE") );
    
}

void Scene::LoadScene( Oryol::StringAtom sceneName )
{

    StringBuilder strBuilder;
    strBuilder.Format( 4096, "gamedata:%s.ldjam", sceneName.AsCStr() );
    IO::Load(strBuilder.GetString(), [this](IO::LoadResult loadResult) {
        
        Oryol::Buffer chunkBuff = std::move(loadResult.Data);
        
        LDJamFileHeader *fileHeader = (LDJamFileHeader*)chunkBuff.Data();
        Log::Info("Loaded chunks, loadresult sz %d, numChunks is %d\n", loadResult.Data.Size(), fileHeader->m_numChunks );
        
        LDJamFileMeshInfo *meshInfos = (LDJamFileMeshInfo *)(chunkBuff.Data() + sizeof(LDJamFileHeader));
        
        
        for (size_t i=0; i < fileHeader->m_numChunks; i++) {
            
            SceneMesh mesh = {};
            LDJamFileMeshInfo *meshInfo = meshInfos + i;
            
            mesh.bboxMin = meshInfo->m_bboxMin;
            mesh.bboxMax = meshInfo->m_bboxMax;
            
            // For now, everything is loaded, TODO load the contents part
            // when needed
            LDJamFileMeshContent *meshContent = (LDJamFileMeshContent*)( chunkBuff.Data() + meshInfo->m_contentOffset );
            LDJamFileVertex *meshVertData = (LDJamFileVertex *)(chunkBuff.Data() + meshInfo->m_contentOffset + sizeof(LDJamFileMeshContent) );
            
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
            
            sceneMeshes.Add( mesh );
        }
        
        // Now add the SceneObjs
        LDJamFileSceneObject *sceneObjBase = (LDJamFileSceneObject*)(chunkBuff.Data() + fileHeader->m_sceneObjOffs);
        for (uint32_t i=0; i < fileHeader->m_numSceneObjs; i++) {
            LDJamFileSceneObject *sceneObjInfo = sceneObjBase + i;
            //printf("SceneObj[%d] is %s\n", i, sceneObjInfo->m_name );
            SceneObject *sceneObj = makeObject( &sceneMeshes[sceneObjInfo->m_meshIndex] );
            sceneObj->objectName = String( sceneObjInfo->m_name );
            sceneObj->xform = sceneObjInfo->m_transform;

            //dbgPrintMatrix( "sceneObj xform",sceneObj->xform );
            sceneObjs.Add( sceneObj );
            
        }
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

void Scene::drawScene()
{
    
    for (int i=0; i < sceneObjs.Size(); i++) {
        
        SceneObject *obj = sceneObjs[i];
        SceneMesh *mesh = obj->mesh;
        
        //if (!mesh->ready) continue;
        
        //const auto resStateTex = Gfx::QueryResourceInfo( mesh->texture ).State;
        const auto resStateMesh = Gfx::QueryResourceInfo( mesh->mesh).State;
        
        //if ((resStateTex == ResourceState::Valid) && (resStateMesh == ResourceState::Valid)) {
        if (resStateMesh == ResourceState::Valid) {
            //this->sceneDrawState.FSTexture[TestShader::tex] = mesh->texture;
            this->sceneDrawState.Mesh[0] = mesh->mesh;
            
            Gfx::ApplyDrawState(this->sceneDrawState);            
            Gfx::ApplyUniformBlock( obj->vsParams);
            
            for (int j=0; j < mesh->numPrims; j++) {
                Gfx::Draw(j);
            }
        }
        
    }
}
