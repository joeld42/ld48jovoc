//------------------------------------------------------------------------------
//  TestApp.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Assets/Gfx/TextureLoader.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "shaders.h"

#include "SceneObject.h"
#include "Camera.h"

#include "NKUI/NKUI.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#include "stb_image.h"

// FIXME: Not really used...
#include <unistd.h>
#include <stdio.h>

using namespace Oryol;

// derived application class
class TestApp : public App {
public:
    AppState::Code OnRunning();
    AppState::Code OnInit();
    AppState::Code OnCleanup();

private:
    void handle_input();
    
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
    DrawState mainDrawState;
  
    Camera camera;
    Scene *scene;
    
    /// load a single icon, asynchronously
    void load_icon(const char* url, struct nk_image* img);

};
OryolMain(TestApp);

struct HUDAssets {
    struct nk_image titleImage;
};

void dbgPrintMatrix( const char *label, glm::mat4 m )
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

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnRunning() {
    
    this->handle_input();
    
    for (int i=0; i < scene->sceneObjs.Size(); i++) {
        SceneObject *obj = scene->sceneObjs[i];
        
        glm::mat4 modelTform = glm::translate(glm::mat4(), obj->pos);
        modelTform = modelTform * glm::mat4_cast( obj->rot );
//        return proj * this->view * modelTform;
        
        glm::mat4 mvp = this->camera.ViewProj * modelTform;
        obj->vsParams.ModelViewProjection = mvp;
    }
    
    Gfx::BeginPass();
    scene->drawScene();
    
    nk_context* ctx = NKUI::NewFrame();
    
    enum {EASY, HARD};
    static int op = EASY;
    static float value = 0.6f;
    struct nk_panel layout;
    
    if (nk_begin(ctx, &layout, "Show", nk_rect(50, 50, 220, 220),
                 NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        /* fixed widget pixel width */
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button")) {
            /* event handling */
        }
        
        /* fixed widget window ratio width */
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
        
        /* custom widget pixel width */
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push (ctx, 50);
            nk_label(ctx, "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 110);
            nk_slider_float(ctx, 0, &value, 1.0f, 0.1f);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
    
    NKUI::Draw();
    
    Gfx::EndPass();
    
    Gfx::CommitFrame();
    
    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnInit() {
    
    // Dev hack to ensure we're running from the right place...
    chdir("/Users/joeld/Projects/ld48jovoc/ld38_smallworld/smallworld");
    FILE *fp = fopen( "./gamedata/tree_062.omsh", "rt");
    if (!fp) {
        printf("Error! Not running from the right path...??\n");
        exit(1);
    }
    fclose(fp);
    
    // set up IO system
    IOSetup ioSetup;
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
//    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
    ioSetup.Assigns.Add("msh:", "cwd:gamedata/");
    ioSetup.Assigns.Add("tex:", "cwd:gamedata/");
    IO::Setup(ioSetup);
    
    scene = new Scene();

    scene->gfxSetup = GfxSetup::WindowMSAA4(800, 600, "LD32 Small World");
    scene->gfxSetup.DefaultPassAction = PassAction::Clear(glm::vec4(0.25f, 0.5f, 1.0f, 1.0f) );
    Gfx::Setup(scene->gfxSetup);
    
    scene->init();
    
    Input::Setup();
//    Input::SetMousePointerLockHandler([](const Mouse::Event& event) -> Mouse::PointerLockMode {
//        // switch pointer-lock on/off on left-mouse-button
//        if ((event.Button == Mouse::LMB) || (event.Button == Mouse::RMB)) {
//            if (event.Type == Mouse::Event::ButtonDown) {
//                return Mouse::PointerLockModeEnable;
//            }
//            else if (event.Type == Mouse::Event::ButtonUp) {
//                return Mouse::PointerLockModeDisable;
//            }
//        }
//        return Mouse::PointerLockModeDontCare;
//    });
    
    // setup static transform matrices
    float32 fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    float32 fbHeight = Gfx::DisplayAttrs().FramebufferHeight;

    this->camera.Setup(glm::vec3(-2531.f, 1959.f, 3241.0), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 25000.0f);
    
    SceneObject *ground = scene->addObject( "msh:ground1_big.omsh", "tex:ground1.dds");
    ground->pos = glm::vec3( 0.0, 0.0, 0.0);

    const glm::vec3 minRand(-5500.0f, 0.0, -5500.0f );
    const glm::vec3 maxRand(5500.0f, 0.0, 5500.0f );

    
    for (int i=0; i < 4000; i++) {
        
        SceneObject *obj1 = scene->addObject( "msh:tree_062.omsh", "tex:tree_062.dds");
        obj1->rot = glm::quat( glm::vec3( 0.0, glm::linearRand( 0.0f, 360.0f), 0.0 ) );
        obj1->pos = glm::linearRand(minRand, maxRand);
    }
    
    // Setup UI
    NKUI::Setup();
    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}

////------------------------------------------------------------------------------
//glm::mat4
//TestApp::computeMVP(const glm::mat4& proj, float32 rotX, float32 rotY, const glm::vec3& pos) {
//    glm::mat4 modelTform = glm::translate(glm::mat4(), pos);
//    modelTform = glm::rotate(modelTform, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
//    modelTform = glm::rotate(modelTform, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
//    return proj * this->view * modelTform;
//}

//------------------------------------------------------------------------------
void
TestApp::handle_input() {
    
    glm::vec3 move;
    glm::vec2 rot;
    float vel = 3.5f;
    if (Input::KeyboardAttached() ) {
        
        if (Input::KeyPressed( Key::LeftShift)) {
            vel *= 10.0;
        }
        
        if (Input::KeyPressed(Key::W) || Input::KeyPressed(Key::Up)) {
            move.z -= vel;
        }
        if (Input::KeyPressed(Key::S) || Input::KeyPressed(Key::Down)) {
            move.z += vel;
        }
        if (Input::KeyPressed(Key::A) || Input::KeyPressed(Key::Left)) {
            move.x -= vel;
        }
        if (Input::KeyPressed(Key::D) || Input::KeyPressed(Key::Right)) {
            move.x += vel;
        }
    }
    
    if (Input::MouseAttached() ) {
        if (Input::MouseButtonPressed(MouseButton::Left)) {
            
            move.z -= vel;
            printf("move %3.2f %3.2f %3.2f\n",
                   camera.Pos.x, camera.Pos.y, camera.Pos.z );

        }
        if (Input::MouseButtonPressed(MouseButton::Left) || Input::MouseButtonPressed(MouseButton::Right)) {
            rot = Input::MouseMovement() * glm::vec2(-0.01f, -0.007f);
        }
    }

    if (Input::TouchpadAttached() ) {
        if (Input::TouchPanning() ) {
            move.z -= vel;
            rot = Input::TouchMovement(0) * glm::vec2(-0.01f, 0.01f);
        }
    }
    this->camera.MoveRotate(move, rot);
}

//------------------------------------------------------------------------------
void
TestApp::load_icon(const char* url, struct nk_image* img) {
    // grab an image handle before data is loaded
    *img = NKUI::AllocImage();
    IO::Load(url, [img](IO::LoadResult loadResult) {
        // decode the PNG data via stb_image
        int w = 0, h = 0, n = 0;
        uint8_t* imgData = stbi_load_from_memory(loadResult.Data.Data(), loadResult.Data.Size(), &w, &h, &n, 0);
        
        // create an Oryol texture from the loaded data
        auto texSetup = TextureSetup::FromPixelData2D(w, h, 1, PixelFormat::RGBA8);
        // hmm... no mipmaps will not look good
        texSetup.Sampler.MinFilter = TextureFilterMode::Linear;
        texSetup.Sampler.MagFilter = TextureFilterMode::Linear;
        texSetup.Sampler.WrapU = TextureWrapMode::ClampToEdge;
        texSetup.Sampler.WrapV = TextureWrapMode::ClampToEdge;
        const int imgDataSize = w * h * PixelFormat::ByteSize(PixelFormat::RGBA8);
        texSetup.ImageData.Sizes[0][0] = imgDataSize;
        Id texId = Gfx::CreateResource(texSetup, imgData, imgDataSize);
        stbi_image_free(imgData);
        
        // ...and associate the Oryol texture with the Nuklear image handle
        NKUI::BindImage(*img, texId);
    });
}


