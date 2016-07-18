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
#include "Gfx/Setup/MeshSetup.h"
#include "Assets/Gfx/TextureLoader.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "shaders.h"

#include "SceneObject.h"
#include "Camera.h"

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
    ClearState mainClearState;
  
    Camera camera;
    Scene *scene;
};
OryolMain(TestApp);

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
    

    
    Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    
    scene->drawScene();
    
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
    
    Input::Setup();
    Input::SetMousePointerLockHandler([](const Mouse::Event& event) -> Mouse::PointerLockMode {
        // switch pointer-lock on/off on left-mouse-button
        if ((event.Button == Mouse::LMB) || (event.Button == Mouse::RMB)) {
            if (event.Type == Mouse::Event::ButtonDown) {
                return Mouse::PointerLockModeEnable;
            }
            else if (event.Type == Mouse::Event::ButtonUp) {
                return Mouse::PointerLockModeDisable;
            }
        }
        return Mouse::PointerLockModeDontCare;
    });
    
    
    
    // setup clear states
    this->mainClearState.Color = glm::vec4(0.25f, 0.5f, 1.0f, 1.0f);

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
    const Keyboard& kbd = Input::Keyboard();
    if (kbd.Attached) {
        
        if (kbd.KeyPressed( Key::LeftShift)) {
            vel *= 10.0;
        }
        
        if (kbd.KeyPressed(Key::W) || kbd.KeyPressed(Key::Up)) {
            move.z -= vel;
        }
        if (kbd.KeyPressed(Key::S) || kbd.KeyPressed(Key::Down)) {
            move.z += vel;
        }
        if (kbd.KeyPressed(Key::A) || kbd.KeyPressed(Key::Left)) {
            move.x -= vel;
        }
        if (kbd.KeyPressed(Key::D) || kbd.KeyPressed(Key::Right)) {
            move.x += vel;
        }
    }
    const Mouse& mouse = Input::Mouse();
    if (mouse.Attached) {
        if (mouse.ButtonPressed(Mouse::Button::LMB)) {
            
            move.z -= vel;
            printf("move %3.2f %3.2f %3.2f\n",
                   camera.Pos.x, camera.Pos.y, camera.Pos.z );

        }
        if (mouse.ButtonPressed(Mouse::Button::LMB) || mouse.ButtonPressed(Mouse::Button::RMB)) {
            rot = mouse.Movement * glm::vec2(-0.01f, -0.007f);
        }
    }
    const Touchpad& touch = Input::Touchpad();
    if (touch.Attached) {
        if (touch.Panning) {
            move.z -= vel;
            rot = touch.Movement[0] * glm::vec2(-0.01f, 0.01f);
        }
    }
    this->camera.MoveRotate(move, rot);
}

