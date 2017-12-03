//------------------------------------------------------------------------------
//  MoreWorse-Main.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"
#include "Core/Time/Clock.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"
#include "HttpFS/HTTPFileSystem.h"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
//#include "Gfx/Setup/MeshSetup.h"
#include "Assets/Gfx/TextureLoader.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "shaders.h"

#include "SceneObject.h"
#include "Camera.h"
#include "DebugDraw.h"

#define PAR_EASINGS_IMPLEMENTATION
#include "par_easings.h"

using namespace Oryol;



// Item Notes:
// owner
// retreival date (owner gives hints)
// color
// category: toys, electronics, industrial, etc..
// substituteable (e.g. any "red barrel" is interchangabe, but "family keepsake" not)
// Gift Wrapped

struct CrateItem {
    SceneObject *sceneObj;
    
    Oryol::String itemName;
    
    // Silly stuff
    glm::vec3 wobAxis;
};


// Storage notes:
// -- Item loose on the floor
// -- Pallet: Stack up to 3 items, griddable
// -- Metal Pallet: Stack up to 3 items, griddable
// -- Small Shelf: Random access 3 items
// -- Shelf: Random access 3 items
// -- Container: Storage that fits into an item slot, can store 5 items...
// -- Offsite Storage: Truck that comes once a day to bring/return packages

struct PlayerBot {
    SceneObject *sceneObj;
    glm::vec3 pos;
    glm::vec3 targetVel;
    glm::vec3 vel;
    glm::vec3 facingDir;
    
    Oryol::Array<CrateItem> items; // Items player is holding
};


// derived application class
class TestApp : public App {
public:
    AppState::Code OnRunning();
    AppState::Code OnInit();
    AppState::Code OnCleanup();

private:
    void startGame();
    
    void handleInput();
    void handleInputDebug();
    void handleInputGame();
    
    void updatePlayer();
    void updateGameCamera();
    
    glm::mat4 computeMVP(const glm::mat4& proj, float32 rotX, float32 rotY, const glm::vec3& pos);

    void loadMesh( const char *path);
    void createMaterials();
    
    void drawMeshAndTextureIfLoaded( Id mesh, Id texture );
    
    Camera *getActiveCamera() {
        
        Camera *activeCamera = debugMode?&dbgCamera:&gameCamera;
        return activeCamera;
    }
    
    ResourceLabel curMeshLabel;
//    MeshSetup curMeshSetup;
    ResourceLabel curMaterialLabel;
    int numMaterials = 0;
    
    DrawState mainDrawState;
    TestShader::vsParams shaderVSParams;
    PassAction passAction;
    
    bool doTestThing = false;
    int testObjIndex = 0;
  
    bool debugMode = false;
    
    Camera gameCamera;
    Camera dbgCamera;
    //Camera cinematicCamera; // TODO
    
    Scene *scene;
    
    float32 fbWidth, fbHeight;
    DebugDrawRenderer *dbgDraw;
    
    const float32 kPlayerFrontDist = 7.0;
    const float32 kPlayerHeight = 3.87031;
    const float32 kCamBorderDist = 10.0f;
    const float32 kCamHeightLow = 60.0;
    const float32 kCamHeightHigh = 100.0;
    bool showCameraBorder;
    float32 targetHeightNorm;
    float32 targetHeight;
    float32 currHeight;
    
    // Gameplay stuff
    Oryol::TimePoint lastTimePoint;
    Oryol::Duration frameDt;
    
    PlayerBot player;
    glm::vec3 gameCameraTarget;
    float32 time;
    
    // Items not carried by the player or in storage
    Oryol::Array<CrateItem> looseItems;
    
    // Master copies of objects for spawning
    SceneObject *masterCrate;
};
OryolMain(TestApp);

//#if !ORYOL_EMSCRIPTEN
void dbgPrintMatrix( const char *label, glm::mat4 m )
{
    Log::Info("mat4 %10s| %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n",
           label,
           m[0][0], m[0][1], m[0][2], m[0][3],
           m[1][0], m[1][1], m[1][2], m[1][3],
           m[2][0], m[2][1], m[2][2], m[2][3],
           m[3][0], m[3][1], m[3][2], m[3][3] );
}
//#endif

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnRunning() {

    frameDt = Clock::LapTime(this->lastTimePoint);
    time += frameDt.AsSeconds();
    
    this->handleInput();
    
    Camera *activeCamera = getActiveCamera();
    
    // Update the player's transform, make this better
    if (player.sceneObj) {
        updatePlayer();
        updateGameCamera();
    }
    
    // Update scene transforms. TODO this should go into scene
    for (int i=0; i < scene->sceneObjs.Size(); i++) {
        SceneObject *obj = scene->sceneObjs[i];
        
        //glm::mat4 modelTform = glm::translate(glm::mat4(), obj->pos);
        //modelTform = modelTform * glm::mat4_cast( obj->rot );

        glm::mat4 mvp = activeCamera->ViewProj * obj->xform;
        //glm::mat4 mvp = this->camera.ViewProj;
        obj->vsParams.mvp = mvp;
    }
    
    dbgDraw->debugDrawMVP = activeCamera->ViewProj;
    dbgDraw->debugDrawOrthoMVP = glm::ortho(0.0f, fbWidth, fbHeight, 0.0f );
    //Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    
    // render one frame
    Gfx::BeginPass(this->passAction);

    if (doTestThing) {
        //SceneMesh mesh = scene->sceneMeshes[testObjIndex];
        //scene->sceneDrawState.Mesh[0] = mesh.mesh;
        this->mainDrawState.FSTexture[TestShader::tex] = scene->testTexture;
        Gfx::ApplyDrawState( this->mainDrawState );
        
        //shaderVSParams.mvp =  this->camera.ViewProj;
        shaderVSParams.mvp =  activeCamera->ViewProj;
        Gfx::ApplyUniformBlock(shaderVSParams);
        Gfx::Draw();
        
    } else {
        
        scene->drawScene();
        
        //Gfx::Draw();
    }
    
//    const ddVec3 boxColor  = { 0.0f, 0.8f, 0.8f };
//    const ddVec3 boxCenter = { 0.0f, 0.0f, 0.0f };
//    float boxSize = 1.0f;
//    dd::box(boxCenter, boxColor, boxSize, boxSize, boxSize );
//    dd::cross(boxCenter, 1.0f);
    
    if (debugMode) {
        ddVec3 textPos2D = { 0.0f, 20.0f, 0.0f };
        textPos2D[0] = fbWidth / 2.0;
        dd::screenText("Debug Mode", textPos2D, dd::colors::Orange );
    } else {
        
        dd::screenText("TAB - Debug Camera\nZ - Spawn Crate",
                       (ddVec3){ 10.0, 10.0}, dd::colors::Blue );
    }
    
    // Flush debug draws
    dd::flush();
    
    Gfx::EndPass();
    Gfx::CommitFrame();
    
    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnInit() {
    
    // set up IO system

    IOSetup ioSetup;
#if ORYOL_EMSCRIPTEN
    ioSetup.FileSystems.Add("http", HTTPFileSystem::Creator());
    ioSetup.Assigns.Add("gamedata:", "http://localhost:8000/gamedata/");
    //    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
#else
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
    ioSetup.Assigns.Add("gamedata:", "cwd:gamedata/");
    //ioSetup.Assigns.Add("tex:", "cwd:gamedata/");

#endif
    
    IO::Setup(ioSetup);
    
    scene = new Scene();

    scene->gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Test App");
    Gfx::Setup(scene->gfxSetup);
    
    scene->Setup();
    
    Input::Setup();
    Input::SetPointerLockHandler([this](const InputEvent& event) -> PointerLockMode::Code {
        if (event.Button == MouseButton::Left) {
            if (event.Type == InputEvent::MouseButtonDown) {
                //this->pointerLock = true;
                return PointerLockMode::Enable;
            }
            else if (event.Type == InputEvent::MouseButtonUp) {
                //this->pointerLock = false;
                return PointerLockMode::Disable;
            }
        }
        return PointerLockMode::DontCare;
    });
    
    /*
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
     */
    
    // setup clear states
    this->passAction.Color[0] = glm::vec4( 0.2, 0.2, 0.2, 1.0 );
    
    // setup static transform matrices
    fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    fbHeight = Gfx::DisplayAttrs().FramebufferHeight;

    //this->camera.Setup(glm::vec3(-2531.f, 1959.f, 3241.0), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 25000.0f);
    gameCamera = {};
    //gameCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 1000.0f);
    gameCamera.Pos = glm::vec3(0.0, 0.0, 35.0);
    //glm::mat4 lookat = glm::lookAt( glm::vec3( 0.0, -5.0f, 25.0f), glm::vec3(0.0), glm::vec3( 0.0f, 0.0f, 1.0f) );
    
    gameCamera.Model = glm::translate(glm::mat4(), gameCamera.Pos );
    gameCamera.UpdateProj(glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 1000.0f);


    dbgCamera = {};
    dbgCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 1000.0f);

    
    Log::Info("Hello...\n");
    //scene->LoadScene( "TEST_Stuff" );
    scene->LoadScene( "WORSE_warehouse", [this](bool success) {
        Log::Info("Load complete...");
        this->startGame();
    } );
    
    // create a donut mesh, shader and pipeline object
    ShapeBuilder shapeBuilder;
    shapeBuilder.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Normal, VertexFormat::Byte4N }
    };
    shapeBuilder.Torus(0.8f, 1.5f, 20, 36);
    SetupAndData<MeshSetup> meshSetup = shapeBuilder.Build();
    this->mainDrawState.Mesh[0] = Gfx::CreateResource( meshSetup );

    Id shd = Gfx::CreateResource(TestShader::Setup());
    
    auto ps = PipelineSetup::FromLayoutAndShader( shapeBuilder.Layout, shd);
    ps.RasterizerState.CullFaceEnabled = false;
    ps.RasterizerState.CullFace = Face::Code::Front;
    ps.RasterizerState.SampleCount = scene->gfxSetup.SampleCount;
    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    this->mainDrawState.Pipeline = Gfx::CreateResource(ps);
    
//    SceneObject *ground = scene->addObject( "msh:ground1_big.omsh", "tex:ground1.dds");
//    ground->pos = glm::vec3( 0.0, 0.0, 0.0);
//
//    const glm::vec3 minRand(-5500.0f, 0.0, -5500.0f );
//    const glm::vec3 maxRand(5500.0f, 0.0, 5500.0f );
//
//
//    for (int i=0; i < 40000; i++) {
//
//        SceneObject *obj1 = scene->addObject( "msh:tree_062.omsh", "tex:tree_062.dds");
//        obj1->rot = glm::quat( glm::vec3( 0.0, glm::linearRand( 0.0f, 360.0f), 0.0 ) );
//        obj1->pos = glm::linearRand(minRand, maxRand);
    
//    }
    
    dbgDraw = new DebugDrawRenderer();
    dbgDraw->Setup( scene->gfxSetup );
    dd::initialize( dbgDraw );
    
    player = {};
    
    this->lastTimePoint = Clock::Now();
    
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

void
TestApp::startGame() {
    
    Log::Info("Hello from startGame..." );
    
    showCameraBorder = false;
    
    // Init player
    player.sceneObj = scene->FindNamedObject( "robot" );
    if (player.sceneObj == NULL) {
        Log::Warn("Couldn't find player object.");
    }
    player.pos = glm::vec3( 0.0 );
    
    masterCrate =  scene->FindNamedObject( "Crate" );
    masterCrate->hidden = true;
    
    targetHeight = kCamHeightLow;
    currHeight = targetHeight;
    
}

float fsgn( float v) {
    if (v < 0) {
        return -1.0;
    } else {
        return 1.0;
    }
}

void
TestApp::updateGameCamera()
{
    float heightMove = 0.02;
    const float fudge = 0.001;
    bool isMoving = false;
    glm::vec3 lastTarget = glm::vec3( gameCameraTarget );
    glm::vec3 offs = gameCameraTarget - player.pos;
    if (fabs(offs.x) >= kCamBorderDist + fudge ) {
        gameCameraTarget -= glm::vec3( offs.x - fsgn(offs.x) * kCamBorderDist, 0.0f, 0.0f );
        isMoving = true;
    }
    
    if (fabs(gameCameraTarget.y - player.pos.y) >= kCamBorderDist + fudge ) {
        gameCameraTarget -= glm::vec3( 0.0f, offs.y - fsgn(offs.y) * kCamBorderDist, 0.0f );
        isMoving = true;
    }
    
    float velScale = glm::length( gameCameraTarget - lastTarget );
    if (isMoving) {
        if (showCameraBorder) dd::screenText( "MOVING", (ddVec3){ 10.0f, 10.0f }, dd::colors::Orange );
        targetHeightNorm += heightMove * velScale;
    } else {
        if (showCameraBorder) dd::screenText( "still", (ddVec3){ 10.0f, 10.0f }, dd::colors::Orange );
        targetHeightNorm -= heightMove;
    }
    targetHeightNorm = PAR_CLAMP(targetHeightNorm, 0.0f, 1.0f);
    
    // Ugh so bad
    //Log::Info("Vel is %3.2f\n", velScale );
    
    float hite = par_easings_in_out_cubic( targetHeightNorm );
    float maxHite = (kCamHeightHigh-kCamHeightLow);
    currHeight = kCamHeightLow + hite * maxHite;

    
    gameCamera.Pos = gameCameraTarget + glm::vec3( 0.0f, 0.0f, currHeight );
    gameCamera.Model = glm::translate(glm::mat4(), gameCamera.Pos );
    gameCamera.UpdateProj(glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 1000.0f);
    
    
    //Log::Info("TARG: %3.2f CURR: %3.2f\n", targetHeightNorm, currHeight  );
    
    if (showCameraBorder) {
        ddVec3 marker;
        marker[0] = gameCameraTarget.x;
        marker[1] = gameCameraTarget.y;
        marker[2] = gameCameraTarget.z + 2.0;
        dd::cross(marker, 1.0f);
        
        ddVec3 cameraBox;
        cameraBox[0] = player.pos.x;
        cameraBox[1] = player.pos.y;
        cameraBox[2] = player.pos.z + 2.0;
        dd::box(cameraBox, dd::colors::AliceBlue, 2.0f*kCamBorderDist, 2.0f*kCamBorderDist,
                4.0f * targetHeightNorm );
    }
}

// ===============================================================================
//   Game Updates
// ===============================================================================
void
TestApp::updatePlayer()
{
    // update player's pos
    float t = 0.1;
    player.vel = (player.vel * (1.0f-t)) + (player.targetVel * t);
    player.pos += player.vel;
    
    if (glm::length( player.vel) > 0.01f) {
        player.facingDir = glm::normalize( player.vel );
    }
    
    // Check collision (TODO)
    
    // Check on what's in front of player
    glm::mat4 mSensor = glm::translate( player.sceneObj->xform,
                               glm::vec3(0.0, kPlayerFrontDist, 0.0) );
    glm::vec4 frontPos = mSensor * glm::vec4(0,0,0,1);
    
    
    dd::sphere( glm::value_ptr(frontPos) , dd::colors::Red, 3.0 );
    
    // Update player's scene obj from pos
    glm::mat4 mpos = glm::translate( glm::mat4(), player.pos );
    player.sceneObj->xform = glm::rotate( mpos, atan2(-player.facingDir.x, player.facingDir.y),
                                glm::vec3( 0,0,1) );
//    player.sceneObj->xform = glm::translate( glm::mat4(), player.pos );
    
    // Update carried items
    float wobble = sin( time ) * cos(time*1.5) * 0.05f;
    
    //Log::Info("updatePlayer %d items\n", player.items.Size() );
    float32 currHeight = kPlayerHeight;
    for (int i =0; i < player.items.Size(); i++) {
        glm::mat4 mwob = glm::rotate( player.sceneObj->xform, wobble*(i+1), player.items[i].wobAxis );
        
        player.items[i].sceneObj->xform = glm::translate( mwob,
                                                         glm::vec3( 0.0f, 0.0f, currHeight ) );
        
        currHeight += (player.sceneObj->mesh->bboxMax.y * 1.2); // HACK
    }
}

// ===============================================================================
//   Handle Input
// ===============================================================================
void
TestApp::handleInput()
{
    // Common input
    if (Input::KeyDown(Key::Tab)) {
        debugMode = !debugMode;
        Log::Info( "Debug Mode: %s\n", debugMode?"ON":"OFF" );
    }
    if (Input::KeyDown(Key::N9)) {
        showCameraBorder = !showCameraBorder;
    }
    
    // Mode-specific input
    if (debugMode) {
        this->handleInputDebug();
    } else {
        this->handleInputGame();
    }
}

void
TestApp::handleInputGame()
{
    glm::vec3 move;
    glm::vec2 rot;
    float vel = 0.5f;
    if (Input::KeyboardAttached() ) {
        
//        if (Input::KeyPressed( Key::LeftShift)) {
//            vel *= 5.0;
//        }
        
        if (Input::KeyPressed(Key::W) || Input::KeyPressed(Key::Up)) {
            move.y += vel;
        }
        if (Input::KeyPressed(Key::S) || Input::KeyPressed(Key::Down)) {
            move.y -= vel;
        }
        if (Input::KeyPressed(Key::A) || Input::KeyPressed(Key::Left)) {
            move.x -= vel;
        }
        if (Input::KeyPressed(Key::D) || Input::KeyPressed(Key::Right)) {
            move.x += vel;
        }
        
        //gameCamera.MoveCrappy( move );
        player.targetVel = move;
        
        
        if (Input::KeyDown(Key::Z)) {
            // Spawn a crate and give it to the player
            SceneObject *newCrate = scene->spawnObject( masterCrate->mesh );
            newCrate->xform = glm::mat4( player.sceneObj->xform );
            
            CrateItem crate;
            crate.sceneObj = newCrate;
            crate.itemName = "Crate";
            if (player.items.Empty()) {
                crate.wobAxis = glm::vec3( 0,1,0);
            } else {
                crate.wobAxis = player.items[player.items.Size()-1].wobAxis;
                crate.wobAxis += (glm::sphericalRand(0.3f)  );
                crate.wobAxis = glm::normalize( crate.wobAxis );
            }
            player.items.Add(crate);
            Log::Info("Added crate...\n");
        }
        if (Input::KeyDown(Key::Space)) {
            // Drop top crate in front
            if (!player.items.Empty()) {
                CrateItem crate = player.items.PopBack();
                crate.sceneObj->xform = glm::translate( player.sceneObj->xform,
                                                       glm::vec3(0.0, kPlayerFrontDist, 0.0) );
                looseItems.Add( crate );
            }
        }
    }
}
//------------------------------------------------------------------------------
void
TestApp::handleInputDebug() {
    
    glm::vec3 move;
    glm::vec2 rot;
    float vel = 0.5f;
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
        
        
        if (Input::KeyDown(Key::Z)) {
            doTestThing = !doTestThing;
        }
        if (Input::KeyDown(Key::X)) {
            testObjIndex++;
            if (testObjIndex >= scene->sceneMeshes.Size()) {
                testObjIndex = 0;
            }
        }
        if (Input::KeyDown(Key::C)) {
            float32 fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
            float32 fbHeight = Gfx::DisplayAttrs().FramebufferHeight;

            dbgCamera.Setup(glm::vec3(0.0, 0.0, 15.0), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 1000.0f);
//            this->
        }
    }
    
    if (Input::MouseAttached() ) {
        if (Input::MouseButtonDown(MouseButton::Left)) {
            
            move.z -= vel;
            //printf("move %3.2f %3.2f %3.2f\n",
                       //camera.Pos.x, camera.Pos.y, camera.Pos.z );

        }
        if (Input::MouseButtonPressed(MouseButton::Left) ||
            Input::MouseButtonPressed(MouseButton::Right)) {
            
            rot = Input::MouseMovement() * glm::vec2(-0.01f, -0.007f);
        }
    }

    if (Input::TouchpadAttached()) {
        if (Input::TouchPanning() ) {
            move.z -= vel;
            rot = Input::TouchMovement(0) * glm::vec2(-0.01f, 0.01f);
        }
    }
    dbgCamera.MoveRotate(move, rot);
}

