
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "shaders.h"
#import "SceneObject.h"

#include "IO/IO.h"
#include "Core/String/StringBuilder.h"
#include "Core/Memory/Memory.h"
#include "Dbg/Dbg.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Assets/Gfx/TextureLoader.h"

#include "ldjamfile.h"

using namespace Oryol;
using namespace Tapnik;

extern void dbgPrintMatrix( const char *label, glm::mat4 m );

SceneObject *makeObject( SceneMesh *mesh )
{
    //SceneObject *object = new SceneObject();
    SceneObject *object = Memory::New<SceneObject>();
    Memory::Fill( object, sizeof(SceneObject), 0 );
    
    object->mesh = mesh;
    object->hidden = false;
    object->collider = false;
    object->vsParams.tintColor = glm::vec4(1);
    object->vsParamsLake.tintColor = glm::vec4(1);
//    object->tileFSParams.decalColor = glm::vec4(1,1,0,1);
//
    //object->vsParams.decalTint = glm::vec4(0);
    
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

SceneMesh *Scene::findMeshByName( Oryol::String meshName )
{
    for (int i=0; i < sceneMeshes.Size(); i++) {
        if (sceneMeshes[i].meshName == meshName) {
            return &sceneMeshes[i];
        }
    }
    return NULL;
}

SceneObject *Scene::spawnObjectWithMeshNamed( Oryol::String meshName )
{
    SceneMesh *namedMesh = findMeshByName( meshName );
    if (!namedMesh) {
        printf("WARN: spawnObjectWithMeshNamed('%s') didn't find mesh.\n", meshName.AsCStr() );
        return NULL;
    }
    
    return spawnObject( namedMesh );
}

void SceneObject::makeInteractable()
{
    if (!this->interaction) {
        this->interaction = Memory::New<InteractionInfo>();
        //this->vsParams.tintColor = yellow or something
    }
}

Scene::Scene()
{
}


void Scene::Setup( Oryol::GfxSetup *gfxSetup )
{
    // create shader and pipeline-state-object
    meshLayout = {
        { VertexAttr::Position,  VertexFormat::Float3 },
        { VertexAttr::Normal,    VertexFormat::Float3 },
        { VertexAttr::TexCoord0, VertexFormat::Float2 },
        { VertexAttr::TexCoord1, VertexFormat::Float2 },
    };
    
    
    // Setup our texture blueprint
    texBluePrint.Sampler.MinFilter = TextureFilterMode::LinearMipmapLinear;
    texBluePrint.Sampler.MagFilter = TextureFilterMode::Linear;
    texBluePrint.Sampler.WrapU = TextureWrapMode::Repeat;
    texBluePrint.Sampler.WrapV = TextureWrapMode::Repeat;
    
    // World Shader
    worldShader = Gfx::CreateResource(WorldShader::Setup());
    auto ps = PipelineSetup::FromLayoutAndShader( meshLayout, worldShader);
    ps.RasterizerState.CullFaceEnabled = true;
    ps.RasterizerState.CullFace = Face::Code::Front;
    ps.RasterizerState.SampleCount = gfxSetup->SampleCount;
    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    
    this->sceneDrawState.Pipeline = Gfx::CreateResource(ps);

    // Make a second drawstate for Cards
    ps.DepthStencilState.DepthWriteEnabled = false;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::Always;
    ps.BlendState.BlendEnabled = true;
    ps.BlendState.SrcFactorAlpha = BlendFactor::Code::SrcAlpha;
    ps.BlendState.DstFactorAlpha = BlendFactor::Code::OneMinusSrcAlpha;
    ps.BlendState.SrcFactorRGB = BlendFactor::Code::SrcAlpha;
    ps.BlendState.DstFactorRGB = BlendFactor::Code::OneMinusSrcAlpha;
    this->cardDrawState.Pipeline = Gfx::CreateResource(ps);
    
    // Drawstate for transparent stuff (the lake)
    lakeShader = Gfx::CreateResource(LakeShader::Setup());
    auto psLake = PipelineSetup::FromLayoutAndShader( meshLayout, lakeShader);
    psLake.RasterizerState.CullFaceEnabled = true;
    psLake.RasterizerState.CullFace = Face::Code::Front;
    psLake.RasterizerState.SampleCount = gfxSetup->SampleCount;
    psLake.DepthStencilState.DepthWriteEnabled = true;
    psLake.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    psLake.BlendState.BlendEnabled = true;
    psLake.BlendState.SrcFactorRGB = BlendFactor::Code::SrcAlpha;
    psLake.BlendState.DstFactorRGB = BlendFactor::Code::OneMinusSrcAlpha;
    this->lakeDrawState.Pipeline = Gfx::CreateResource(psLake);

    
    // Load test texture
    decalBluePrint.Sampler.MinFilter = TextureFilterMode::LinearMipmapLinear;
    decalBluePrint.Sampler.MagFilter = TextureFilterMode::Linear;
    decalBluePrint.Sampler.WrapU = TextureWrapMode::Repeat;
    decalBluePrint.Sampler.WrapV = TextureWrapMode::Repeat;
//    tileFontTexture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "gamedata:tilefont.dds", decalBluePrint)));
//    boardIconTexture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "gamedata:board_icons.dds", decalBluePrint)));
    
    defaultTexture =  Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "gamedata:wood.dds", texBluePrint)));
    cardsTexture =  Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "gamedata:cardfish_cards.dds", texBluePrint)));
}

void Scene::LoadScene( Oryol::StringAtom sceneName, Scene::LoadCompleteFunc loadComplete )
{
    StringBuilder strBuilder;
    strBuilder.Format( 4096, "gamedata:%s.ldjam", sceneName.AsCStr() );
    Log::Info("fetch scene %s", strBuilder.GetString().AsCStr() );
    IO::Load(strBuilder.GetString(), [this,sceneName,loadComplete](IO::LoadResult loadResult) {
        
        Log::Info("Loadresult scene %s size is %d\n", sceneName.AsCStr(), loadResult.Data.Size() );
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
            
            mesh.meshName = String( meshInfo->m_name );
            
            size_t meshDataSize = (sizeof(LDJamFileVertex) * numVerts) + (sizeof(uint16_t) * meshContent->m_triIndices);
            mesh.mesh = Gfx::CreateResource(meshSetup, meshVertData, meshDataSize );
            
            
            // Assign texture if there is one
            Log::Info("Texture is '%s'\n", meshInfo->m_texture );
            if (*(meshInfo->m_texture)) {
                StringBuilder texLocBuilder;
                texLocBuilder.Format( 4096, "gamedata:%s.dds", meshInfo->m_texture );
                Log::Info("Locator is %s\n", texLocBuilder.GetString().AsCStr() );
                mesh.texture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( Locator(texLocBuilder.GetString()), texBluePrint)));
            } else {
                Log::Info("Using default texture\n");
                mesh.texture = defaultTexture;
            }
            mesh.numPrims = 1;
            sceneMeshes.Add( mesh );
        }
        // Add the cameras
        Log::Info("%d cameras...\n",  fileHeader->m_numCameras );
        LDJamFileSceneCamera *sceneCamBase = (LDJamFileSceneCamera*)(sceneBuff.Data() + fileHeader->m_cameraOffs);
        for (uint32_t i=0; i < fileHeader->m_numCameras; i++) {
            LDJamFileSceneCamera *sceneCamInfo = sceneCamBase + i;
            
            SceneCamera camera;
            camera.cameraName = String( sceneCamInfo->m_name );
            camera.mat = glm::mat4( sceneCamInfo->m_transform );
            
            camera.index = i;
            
//            printf("Loaded camera %s\n", sceneCamInfo->m_name );
//            dbgPrintMatrix( "mat", camera.mat);
            
            sceneCams.Add( camera );
        }
        
        // Now add the SceneObjs
        LDJamFileSceneObject *sceneObjBase = (LDJamFileSceneObject*)(sceneBuff.Data() + fileHeader->m_sceneObjOffs);
        Log::Info("%d scene objs...\n",  fileHeader->m_numSceneObjs );
        
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
        
        CreateCardMeshes();
      
        loadComplete( true );
    });

}

void Scene::CreateCardMeshes()
{
    const float cardUVWidth = 0.173828125f; // from mk_cards.py
    const float cardUVHite = 0.5f;
    const float cardAspect = 1.44f;
    
    int numCards = cardIds.Size();
    for (int cardNdx=0; cardNdx < numCards; cardNdx++)
    {
        int numVerts = 8;
        int numTriIndices = 12;
        int slot = cardNdx + 1; //slot 0 is card back
        int col = slot % 5;
        int row = slot / 5;
        
        Oryol::Buffer buff;
        buff.Reserve(sizeof(LDJamFileVertex) * numVerts + sizeof(uint16_t) * numTriIndices);
        LDJamFileVertex *vertData = (LDJamFileVertex *)buff.Add(sizeof(LDJamFileVertex) * numVerts);
        uint16_t *indexData = (uint16_t *)buff.Add(sizeof(uint16_t) * numTriIndices);
        
        auto meshSetup = MeshSetup::FromData();
        meshSetup.NumVertices = numVerts;
        meshSetup.NumIndices = numTriIndices;
        
        vertData[0].m_pos = glm::vec3( -0.5f, -0.5f*cardAspect, 0.0f );
        vertData[0].m_st0 = glm::vec2( cardUVWidth * (col+0), cardUVHite * (row+1)  );
        
        vertData[1].m_pos = glm::vec3(  0.5f, -0.5f*cardAspect, 0.0f );
        vertData[1].m_st0 = glm::vec2( cardUVWidth * (col+1), cardUVHite * (row+1) );
        
        vertData[2].m_pos = glm::vec3(  0.5f,  0.5f*cardAspect, 0.0f );
        vertData[2].m_st0 = glm::vec2( cardUVWidth * (col+1), cardUVHite * (row+0) );
        
        vertData[3].m_pos = glm::vec3( -0.5f,  0.5f*cardAspect, 0.0f );
        vertData[3].m_st0 = glm::vec2( cardUVWidth * (col+0), cardUVHite * (row+0) );
        
        // Card Back
        vertData[4].m_pos = glm::vec3( -0.5f, -0.5f*cardAspect, 0.0f );
        vertData[4].m_st0 = glm::vec2( 0.0f, cardUVHite );
        
        vertData[5].m_pos = glm::vec3(  0.5f, -0.5f*cardAspect, 0.0f );
        vertData[5].m_st0 = glm::vec2( cardUVWidth, cardUVHite );
        
        vertData[6].m_pos = glm::vec3(  0.5f,  0.5f*cardAspect, 0.0f );
        vertData[6].m_st0 = glm::vec2( cardUVWidth, 0.0f );
        
        vertData[7].m_pos = glm::vec3( -0.5f,  0.5f*cardAspect, 0.0f );
        vertData[7].m_st0 = glm::vec2( 0.0f, 0.0f );


        for (int i=0; i < numVerts; i++) {
            vertData[i].m_nrm = glm::vec3( 0.0f, 0.0f, 1.0f);
            vertData[i].m_st1 = vertData[i].m_st0;
        }
        
        // card front
        indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
        indexData[3] = 0; indexData[4] = 2; indexData[5] = 3;

        // card back
        indexData[6] = 4; indexData[7] = 6; indexData[8] = 5;
        indexData[9] = 4; indexData[10] = 7; indexData[11] = 6;

        meshSetup.IndicesType = IndexType::Index16;
        meshSetup.Layout = meshLayout;
        
        meshSetup.AddPrimitiveGroup({0, numTriIndices });
        meshSetup.VertexDataOffset = 0;
        meshSetup.IndexDataOffset = sizeof(LDJamFileVertex) * numVerts;
        
        SceneMesh mesh = {};
        mesh.meshName = cardIds[cardNdx];
        
        printf("CARD: %s\n", mesh.meshName.AsCStr() );
        
        mesh.bboxMin =  glm::vec3( -0.5f,  -0.5f*cardAspect, -0.1f );
        mesh.bboxMax =  glm::vec3(  0.5f,   0.5f*cardAspect, 0.1f );
        
        size_t meshDataSize = buff.Size();
        mesh.mesh = Gfx::CreateResource(meshSetup, vertData, meshDataSize );
        mesh.texture = cardsTexture;

        mesh.numPrims = 1;
        sceneMeshes.Add( mesh );
    }
}

SceneObject *Scene::FindNamedObject( Oryol::String name )
{
    for (int i=0; i < sceneObjs.Size(); i++) {
        if (sceneObjs[i]->objectName == name) {
            return sceneObjs[i];
        }
    }
    return NULL;
}

SceneCamera Scene::findNamedCamera( Oryol::String cameraName )
{
    SceneCamera result;
    result.cameraName = "notfound";
    result.mat = glm::mat4();
    result.index = -1;
    
    for ( int i=0; i < sceneCams.Size(); i++) {
        printf("Camera: %s\n", sceneCams[i].cameraName.AsCStr() );
        if (sceneCams[i].cameraName == cameraName) {
            result = sceneCams[i];
            break;
        }
    }
    
    return result;
}

void Scene::BringToFront( SceneObject *frontObj )
{
    for (int i=0; i < sceneObjs.Size(); i++) {
        if (sceneObjs[i] == frontObj) {
            sceneObjs[i] = sceneObjs[sceneObjs.Size()-1];
            sceneObjs[sceneObjs.Size()-1] = frontObj;
            return;
        }
    }
}


void Scene::drawSceneLayer( Oryol::DrawState drawState, bool cardsLayer, bool lake)
{
    // Draw all the world objects
    for (int i=0; i < sceneObjs.Size(); i++) {
        
        SceneObject *obj = sceneObjs[i];
        SceneMesh *mesh = obj->mesh;
        
        if (obj->lake != lake) continue;
        if (obj->hidden) continue;
        if (obj->isCard != cardsLayer) continue;
        
        const auto resStateTex = Gfx::QueryResourceInfo( mesh->texture ).State;
        const auto resStateMesh = Gfx::QueryResourceInfo( mesh->mesh ).State;
        
        if (resStateMesh == ResourceState::Valid) {
            if (resStateTex == ResourceState::Valid) {
                drawState.FSTexture[WorldShader::tex] = mesh->texture;
            }
            
            drawState.Mesh[0] = mesh->mesh;
                        
            Gfx::ApplyDrawState( drawState );
            if (obj->lake) {
                Gfx::ApplyUniformBlock( obj->vsParamsLake);
                Gfx::ApplyUniformBlock( obj->fsParamsLake);
            } else {
                Gfx::ApplyUniformBlock( obj->vsParams);
                //Gfx::ApplyUniformBlock( obj->fsParams);
            }
            
            for (int j=0; j < mesh->numPrims; j++) {
                Gfx::Draw(j);
            }
        }
        
    } // foreach sceneobj

}


void Scene::destroyObject( SceneObject *obj )
{
    for (int i=0; i < sceneObjs.Size(); i++) {
        if (sceneObjs[i] == obj) {
            sceneObjs.EraseSwap( i );
            break;
        }
    }
    
    Memory::Delete<SceneObject>( obj );
    
}

void Scene::finalizeTransforms(  glm::mat4 matViewProj, glm::mat4 matViewProjCards )
{
    // Update scene transforms. TODO this should go into scene
    for (int i=0; i < sceneObjs.Size(); i++) {
        SceneObject *obj = sceneObjs[i];
        
        // Used for ray-hit test..
        if (obj->interaction)
        {
            obj->interaction->invXform = glm::inverse( obj->xform );
        }
        
        if (obj->isCard) {
            glm::mat4 mvp = matViewProjCards * obj->xform;
            obj->vsParams.mvp = mvp;
            //obj->tileVSParams.mvp = mvp;
        } else {
            glm::mat4 mvp = matViewProj * obj->xform;
            obj->vsParams.mvp = mvp;
            //obj->tileVSParams.mvp = mvp;
        }
        
        if (obj->lake) {
            obj->vsParamsLake.mvp = obj->vsParams.mvp;
        }
        
        
//        if (obj->interaction) {
//            obj->interaction->outlineVSParams.mvp = mvp;
//        }
    }
    
      
}

bool Tapnik::RayHitObject( SceneObject *obj, Ray ray )
{
    assert( obj->interaction);
    
    // Transform ray
    Ray ray2;
    ray2.pos = glm::vec3( obj->interaction->invXform * glm::vec4( ray.pos, 1.0 ));
    ray2.dir = glm::vec3( obj->interaction->invXform * glm::vec4( ray.dir, 0.0 ));
    
//    Dbg::PrintF( "%s: pos %3.2f %3.2f %3.2f dir %3.2f %3.2f %3.2f -- ",
//                obj->mesh->meshName.AsCStr(),
//                ray2.pos.x, ray2.pos.y, ray2.pos.z,
//                ray2.dir.x, ray2.dir.y, ray2.dir.z );
    
    return RayIsectAABB(ray2, obj->mesh->bboxMin, obj->mesh->bboxMax );
}

void Tapnik::dbgDrawBBox( SceneObject *obj, float *color )
{
    glm::vec3 box[8];
    
    box[0] = glm::vec3( obj->mesh->bboxMin.x, obj->mesh->bboxMin.y, obj->mesh->bboxMin.z );
    box[1] = glm::vec3( obj->mesh->bboxMax.x, obj->mesh->bboxMin.y, obj->mesh->bboxMin.z );
    box[2] = glm::vec3( obj->mesh->bboxMax.x, obj->mesh->bboxMax.y, obj->mesh->bboxMin.z );
    box[3] = glm::vec3( obj->mesh->bboxMin.x, obj->mesh->bboxMax.y, obj->mesh->bboxMin.z );
    
    box[4] = glm::vec3( obj->mesh->bboxMin.x, obj->mesh->bboxMin.y, obj->mesh->bboxMax.z );
    box[5] = glm::vec3( obj->mesh->bboxMax.x, obj->mesh->bboxMin.y, obj->mesh->bboxMax.z );
    box[6] = glm::vec3( obj->mesh->bboxMax.x, obj->mesh->bboxMax.y, obj->mesh->bboxMax.z );
    box[7] = glm::vec3( obj->mesh->bboxMin.x, obj->mesh->bboxMax.y, obj->mesh->bboxMax.z );
    
    // transform the bbox by the object xform
    for (int j=0; j < 8; j++) {
        glm::vec4 pnt = glm::vec4( box[j], 1.0 );
        glm::vec4 pnt2 = obj->xform * pnt;
        box[j] = glm::vec3( pnt2 );
    }

    dd::box( (const ddVec3 *)glm::value_ptr(box[0]), color );
}

void Tapnik::dbgPrintMatrix( const char *label, glm::mat4 m )
{
    printf("mat4 %10s| %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n",
           label,
           m[0][0], m[0][1], m[0][2], m[0][3],
           m[1][0], m[1][1], m[1][2], m[1][3],
           m[2][0], m[2][1], m[2][2], m[2][3],
           m[3][0], m[3][1], m[3][2], m[3][3] );
}

