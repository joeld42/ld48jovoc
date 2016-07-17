//------------------------------------------------------------------------------
//  TestApp.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Gfx/Setup/MeshSetup.h"
#include "Assets/Gfx/TextureLoader.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"

#include "SceneObject.h"

using namespace Oryol;

// derived application class
class TestApp : public App {
public:
    AppState::Code OnRunning();
    AppState::Code OnInit();
    AppState::Code OnCleanup();

private:
    glm::mat4 computeMVP(const glm::mat4& proj, float32 rotX, float32 rotY, const glm::vec3& pos);

    void loadMesh( const char *path);
    void createMaterials();
    
    void drawMeshAndTextureIfLoaded( Id mesh, Id texture );
    
    ResourceLabel curMeshLabel;
//    MeshSetup curMeshSetup;
    ResourceLabel curMaterialLabel;
    int numMaterials = 0;

    static const int numShaders = 3;
    static const char *shaderNames[numShaders];
    enum {
        Normals,
        Lambert,
        Phong
    };
    
    Id meshTree;
    Id dispShader;
    Id texture;
    
//    Id renderTarget;
//    DrawState offscrDrawState;
    DrawState mainDrawState;
    
//    ClearState offscrClearState;
    ClearState mainClearState;
    OffscreenShader::VSParams offscrVSParams;
    MainShader::VSParams mainVSParams;
    glm::mat4 view;
//    glm::mat4 offscreenProj;
    glm::mat4 displayProj;
    float32 angleX = 23.0f;
    float32 angleY = 0.0f;
    
    Scene *scene;
};
OryolMain(TestApp);

//const char* TestApp::shaderNames[numShaders] = {
//    "Normals",
//    "Lambert",
//    "Phong"
//};

//void
//TestApp::createMaterials() {
//    o_assert_dbg( this->mesh.IsValid() );
//    if (this->curMaterialLabel.IsValid()) {
//        Gfx::DestroyResources(this->curMaterialLabel);
//    }
//    
//    this->curMaterialLabel = Gfx::PushResourceLabel();
//    for (int i=0; i < this->numMaterials; i++) {
//        auto ps = PipelineSetup::FromLayoutAndShader(this->curMeshSetup.Layout, dispShader );
//        ps.DepthStencilState.DepthWriteEnabled = true;
//        ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
//        ps.RasterizerState.CullFaceEnabled = true;
//        ps.RasterizerState.SampleCount = 4;
//        
//    }
//    Gfx::PopResourceLabel();
//}

//void
//TestApp::loadMesh( const char *path) {
//    
//    this->numMaterials = 0;
//    this->curMeshLabel = Gfx::PushResourceLabel();
//    this->mesh = Gfx::LoadResource(MeshLoader::Create(MeshSetup::FromFile(path), [this](MeshSetup &setup) {
//        printf("LOADMESH finish block");
//        
//        this->curMeshSetup = setup;
//        this->numMaterials = setup.NumPrimitiveGroups();
//        
//        auto ps = PipelineSetup::FromLayoutAndShader(this->curMeshSetup.Layout, dispShader );
//        ps.DepthStencilState.DepthWriteEnabled = true;
//        ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
//        ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
//        this->mainDrawState.Pipeline = Gfx::CreateResource(ps);
//        
//        mainDrawState.Mesh[0] = this->mesh;
//    }));
//    Gfx::PopResourceLabel();
//}

void TestApp::drawMeshAndTextureIfLoaded( Id mesh, Id texture )
{
    const auto resStateTex = Gfx::QueryResourceInfo( texture ).State;
    const auto resStateMesh = Gfx::QueryResourceInfo( mesh).State;
    if ((resStateTex == ResourceState::Valid) && (resStateMesh == ResourceState::Valid)) {
        this->mainDrawState.FSTexture[Textures::Texture] = texture;
        this->mainDrawState.Mesh[0] = mesh;
        
        Gfx::ApplyDrawState(this->mainDrawState);
        
        this->mainVSParams.ModelViewProjection = this->computeMVP(this->displayProj, -this->angleX * 0.25f, this->angleY * 0.25f,
                                                                  glm::vec3(0.0, -200.0f, -1000.0f) );
        
        Gfx::ApplyUniformBlock(this->mainVSParams);
        
        for (int i=0; i < 3; i++) {
            Gfx::Draw(i);
        }
    }
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnRunning() {
    
    // update animated parameters
//    this->angleY += 0.01f;
//    this->angleX += 0.02f;
//    printf("ANGLEX: %3.2f\n", angleX );
//    this->offscrVSParams.ModelViewProjection = this->computeMVP(this->offscreenProj, this->angleX, this->angleY, glm::vec3(0.0f, 0.0f, -3.0f));
    this->mainVSParams.ModelViewProjection = this->computeMVP(this->displayProj, -this->angleX * 0.25f, this->angleY * 0.25f,
                                                              glm::vec3(0.0f, -200.0f, -1000.0f));;

    // render donut to offscreen render target
#if 0
    Gfx::ApplyRenderTarget(this->renderTarget, this->offscrClearState);
    Gfx::ApplyDrawState(this->offscrDrawState);
    Gfx::ApplyUniformBlock(this->offscrVSParams);
    Gfx::Draw(0);
#endif
    // render sphere to display, with offscreen render target as texture
//    if (this->mesh.IsValid()) {
//        mainDrawState.Mesh[0]
//    }
    
    
//    const auto resState = Gfx::QueryResourceInfo(this->texture).State;
//    if (resState == ResourceState::Valid) {
//        this->mainDrawState.FSTexture[Textures::Texture] = this->texture;
//    }

    for (int i=0; i < scene->sceneObjs.Size(); i++) {
        SceneObject *obj = scene->sceneObjs[i];
        
        obj->vsParams.ModelViewProjection = this->computeMVP(this->displayProj, -this->angleX * 0.25f, i * 10 * 0.25f,
//                                                             glm::vec3(0.0, -200.0f, -1000.0f)
                                                             obj->pos );
    }
    

    
    Gfx::ApplyDefaultRenderTarget(this->mainClearState);
//    if (this->mainDrawState.Pipeline.IsValid()) {
    
        scene->drawScene();
//        
//        this->drawMeshAndTextureIfLoaded( meshTree, texture );
    
//        const auto resState = Gfx::QueryResourceInfo(this->meshBowl).State;
//        if (resState == ResourceState::Valid) {
//            this->mainDrawState.Mesh[0] = meshBowl;
//            Gfx::ApplyDrawState(this->mainDrawState);
//            Gfx::ApplyUniformBlock(this->mainVSParams);
//            Gfx::Draw(0);
//        }

//        const auto resState2 = Gfx::QueryResourceInfo(this->meshTable).State;
//        if (resState2 == ResourceState::Valid) {
//            this->mainDrawState.Mesh[0] = meshTable;
//            Gfx::ApplyDrawState(this->mainDrawState);
//            Gfx::ApplyUniformBlock(this->mainVSParams);
//            Gfx::Draw(0);
//        }

//    }
    

    
    Gfx::CommitFrame();
    
    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnInit() {
    
    // set up IO system

    IOSetup ioSetup;
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
//    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
    ioSetup.Assigns.Add("msh:", "cwd:gamedata/");
    ioSetup.Assigns.Add("tex:", "cwd:gamedata/");
    IO::Setup(ioSetup);
    
    scene = new Scene();

    scene->gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Test App");
    Gfx::Setup(scene->gfxSetup);
    
    scene->init();
    
    
    // setup clear states
    this->mainClearState.Color = glm::vec4(0.25f, 0.5f, 1.0f, 1.0f);

    // setup static transform matrices
    float32 fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    float32 fbHeight = Gfx::DisplayAttrs().FramebufferHeight;
//    this->offscreenProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 20.0f);
    this->displayProj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.01f, 10000.0f);
    this->view = glm::mat4();

////    this->loadMesh( "msh:bowl.omsh" );
//    this->meshTree = Gfx::LoadResource(MeshLoader::Create(MeshSetup::FromFile( "msh:tree_062.omsh"), [this](MeshSetup &setup) {
////    this->meshTree = Gfx::LoadResource(MeshLoader::Create(MeshSetup::FromFile( "msh:ground1.omsh"), [this](MeshSetup &setup) {
//        printf("LOADMESH  TREE finish block");
//        auto ps = PipelineSetup::FromLayoutAndShader(setup.Layout, dispShader );
//        ps.DepthStencilState.DepthWriteEnabled = true;
//        ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
//        ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
//        this->mainDrawState.Pipeline = Gfx::CreateResource(ps);
//    }));
    
    SceneObject *ground = scene->addObject( "msh:ground1.omsh", "tex:ground1.dds");
    ground->pos = glm::vec3( 0.0, -200, -1000);

    const glm::vec3 minRand(-1500.0f, -250.0f, -2000.0f);
    const glm::vec3 maxRand(1500.0f, -200.0f, -1000.0f);

        
    for (int i=0; i < 20; i++) {
        
        SceneObject *obj1 = scene->addObject( "msh:tree_062.omsh", "tex:tree_062.dds");
        obj1->pos = glm::linearRand(minRand, maxRand);
    }

//    SceneObject *obj2 = scene->addObject( "msh:tree_062.omsh", "tex:tree_062.dds");
//    obj2->pos = glm::vec3( 250.0, -200, -1500);
//    
//    SceneObject *obj3 = scene->addObject( "msh:tree_062.omsh", "tex:tree_062.dds");
//    obj3->pos = glm::vec3( -200.0, -250, -1000);
    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
glm::mat4
TestApp::computeMVP(const glm::mat4& proj, float32 rotX, float32 rotY, const glm::vec3& pos) {
    glm::mat4 modelTform = glm::translate(glm::mat4(), pos);
    modelTform = glm::rotate(modelTform, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTform = glm::rotate(modelTform, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    return proj * this->view * modelTform;
}
