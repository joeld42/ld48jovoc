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

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"

using namespace Oryol;

//uint8_t *texdata = (uint8_t*)malloc(256*256*3);
uint8_t texdata[256*256*3];

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
    
    Oryol::GfxSetup gfxSetup;
    
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
        Gfx::ApplyUniformBlock(this->mainVSParams);
        Gfx::Draw(0);
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
                                                              glm::vec3(0.0f, -40.0f, -1000.0f));;

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
    
    
    const auto resState = Gfx::QueryResourceInfo(this->texture).State;
    if (resState == ResourceState::Valid) {
        this->mainDrawState.FSTexture[Textures::Texture] = this->texture;
    }
        
    Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    if (this->mainDrawState.Pipeline.IsValid()) {
        
        this->drawMeshAndTextureIfLoaded( meshTree, texture );
        
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

    }
    
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
    
//    IO::Load( "data:MESH_Cube.001.dat", [](IO::LoadResult res) {
//        Log::Info( "Loaded data file... size is %d\n", res.Data.Size());
//    });
    // setup rendering system
    this->gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Test App");
    Gfx::Setup(gfxSetup);
#if 0
    // create an offscreen render target, we explicitly want repeat texture wrap mode
    // and linear blending...
    auto rtSetup = TextureSetup::RenderTarget(128, 128);
    rtSetup.ColorFormat = PixelFormat::RGBA8;
    rtSetup.DepthFormat = PixelFormat::DEPTH;
    rtSetup.Sampler.WrapU = TextureWrapMode::Repeat;
    rtSetup.Sampler.WrapV = TextureWrapMode::Repeat;
    rtSetup.Sampler.MagFilter = TextureFilterMode::Linear;
    rtSetup.Sampler.MinFilter = TextureFilterMode::Linear;
    this->renderTarget = Gfx::CreateResource(rtSetup);
    
    // create offscreen rendering resources
    ShapeBuilder shapeBuilder;
    shapeBuilder.Layout
        .Add(VertexAttr::Position, VertexFormat::Float3)
        .Add(VertexAttr::Normal, VertexFormat::Byte4N);
    shapeBuilder.Box(1.0f, 1.0f, 1.0f, 1);
    this->offscrDrawState.Mesh[0] = Gfx::CreateResource(shapeBuilder.Build());
    Id offScreenShader = Gfx::CreateResource(OffscreenShader::Setup());
    auto offPipSetup = PipelineSetup::FromLayoutAndShader(shapeBuilder.Layout, offScreenShader);
    offPipSetup.DepthStencilState.DepthWriteEnabled = true;
    offPipSetup.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    offPipSetup.BlendState.ColorFormat = rtSetup.ColorFormat;
    offPipSetup.BlendState.DepthFormat = rtSetup.DepthFormat;
    this->offscrDrawState.Pipeline = Gfx::CreateResource(offPipSetup);
#endif
    
    // Make a texture

    for (int j=0; j < 256; j++) {
        for (int i=0; i < 256; i++) {
            int ii = (i>>3);
            int jj = (j>>3);
            int check = ((ii%2)==(jj%2))?0xff:0x00;
            texdata[(j*256 + i)*3 + 0] = check;
            texdata[(j*256 + i)*3 + 1] = i;
            texdata[(j*256 + i)*3 + 2] = j;
        }
    }
    
    auto texSetup = TextureSetup::FromPixelData(256, 256, 1, TextureType::Texture2D, PixelFormat::RGB8 );
    texSetup.ImageData.Sizes[0][0] = 256*256*3;
    
#if 1
    MeshBuilder meshBuilder;
    meshBuilder.Layout
        .Clear()
        .Add( VertexAttr::Position, VertexFormat::Float3 )
        .Add( VertexAttr::Normal, VertexFormat::Byte4N )
        .Add( VertexAttr::TexCoord0, VertexFormat::Float2 );
    meshBuilder.NumVertices = 3;
    meshBuilder.NumIndices = 3;
    
    PrimitiveGroup primGroup(0,3);
    meshBuilder.PrimitiveGroups.Add(primGroup);
    
    meshBuilder.Begin();

    meshBuilder.Vertex( 0, VertexAttr::Position, -0.5, 0.0, 0.0 );
    meshBuilder.Vertex( 0, VertexAttr::TexCoord0, 0.0, 0.0 );
    meshBuilder.Vertex( 0, VertexAttr::Normal, 0.0, 0.0, 1.0 );

    meshBuilder.Vertex( 1, VertexAttr::Position, 0.5, 0.0, 0.0 );
    meshBuilder.Vertex( 1, VertexAttr::TexCoord0, 1.0, 0.0 );
    meshBuilder.Vertex( 1, VertexAttr::Normal, 0.0, 0.0, 1.0 );

    meshBuilder.Vertex( 2, VertexAttr::Position, 0.5, 0.5, 0.0 );
    meshBuilder.Vertex( 2, VertexAttr::TexCoord0, 1.0, 1.0 );
    meshBuilder.Vertex( 2, VertexAttr::Normal, 0.0, 0.0, 1.0 );

    meshBuilder.Index( 0, 0 );
    meshBuilder.Index( 1, 1 );
    meshBuilder.Index( 2, 2 );
    
    SetupAndData<MeshSetup> result = meshBuilder.Build();
#endif
    
    // create display rendering resources
#if 1
    ShapeBuilder shapeBuilder;
    shapeBuilder.Layout
        .Clear()
        .Add(VertexAttr::Position, VertexFormat::Float3)
        .Add(VertexAttr::Normal, VertexFormat::Byte4N)
        .Add(VertexAttr::TexCoord0, VertexFormat::Float2);
    shapeBuilder.Sphere(0.5f, 72.0f, 40.0f);
    this->mainDrawState.Mesh[0] = Gfx::CreateResource(shapeBuilder.Build());
#endif
    
#if 1
    this->mainDrawState.Mesh[0] = Gfx::CreateResource( result );
#endif
    
    
//    auto meshSetup = MeshSetup::FromFile("msh:radonlabs_tiger.omsh");
//    this->mainDrawState.Mesh[0] = Gfx::LoadResource(
//                            MeshLoader::Create( meshSetup ));
//    
    dispShader = Gfx::CreateResource(MainShader::Setup());
    auto dispPipSetup = PipelineSetup::FromLayoutAndShader(shapeBuilder.Layout, dispShader);
//    auto dispPipSetup = PipelineSetup::FromLayoutAndShader(meshBuilder.Layout, dispShader);
//    auto dispPipSetup = PipelineSetup::FromLayoutAndShader(meshSetup.Layout, dispShader);

    dispPipSetup.DepthStencilState.DepthWriteEnabled = true;
    dispPipSetup.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    dispPipSetup.RasterizerState.SampleCount = gfxSetup.SampleCount;
    this->mainDrawState.Pipeline = Gfx::CreateResource(dispPipSetup);
    this->mainDrawState.FSTexture[Textures::Texture] = Gfx::CreateResource( texSetup, texdata, 256*256*3);
    //this->mainDrawState.FSTexture[Textures::Texture] = this->renderTarget;
    
    TextureSetup texBluePrint;
    texBluePrint.Sampler.MinFilter = TextureFilterMode::LinearMipmapLinear;
    texBluePrint.Sampler.MagFilter = TextureFilterMode::Linear;
    texBluePrint.Sampler.WrapU = TextureWrapMode::Repeat;
    texBluePrint.Sampler.WrapV = TextureWrapMode::Repeat;
    texture =  Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile("tex:tree_062.dds", texBluePrint)));
    
    // setup clear states
//    this->offscrClearState.Color = glm::vec4(1.0f, 0.5f, 0.25f, 1.0f);
    this->mainClearState.Color = glm::vec4(0.25f, 0.5f, 1.0f, 1.0f);

    // setup static transform matrices
    float32 fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    float32 fbHeight = Gfx::DisplayAttrs().FramebufferHeight;
//    this->offscreenProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 20.0f);
    this->displayProj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.01f, 1000.0f);
    this->view = glm::mat4();

//    this->loadMesh( "msh:bowl.omsh" );
    this->meshTree = Gfx::LoadResource(MeshLoader::Create(MeshSetup::FromFile( "msh:tree_062_onemtl.omsh"), [this](MeshSetup &setup) {
        printf("LOADMESH  TREE finish block");
        auto ps = PipelineSetup::FromLayoutAndShader(setup.Layout, dispShader );
        ps.DepthStencilState.DepthWriteEnabled = true;
        ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
        ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
        this->mainDrawState.Pipeline = Gfx::CreateResource(ps);
    }));
    
    
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
