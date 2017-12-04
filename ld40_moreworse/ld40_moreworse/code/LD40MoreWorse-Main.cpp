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
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "shaders.h"

#include "SceneObject.h"
#include "Camera.h"
#include "DebugDraw.h"

#define PAR_EASINGS_IMPLEMENTATION
#include "par_easings.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

//#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_STDIO
//#define STBI_ONLY_PNG
//#define STBI_NO_SIMD
//#include "stb_image.h"


// For sprintf, get rid of this..
#include <string.h>


using namespace Oryol;

struct UIMedia {
    struct nk_font *font_14;
    struct nk_font *font_18;
    struct nk_font *font_20;
    struct nk_font *font_24;
    struct nk_image farmageddon_title;
    struct nk_image farmageddon_gameover;
} g_uiMedia;


// Item Notes:
// owner
// retreival date (owner gives hints)
// color
// category: toys, electronics, industrial, etc..
// substituteable (e.g. any "red barrel" is interchangabe, but "family keepsake" not)
// Gift Wrapped

enum ItemState {
    ItemState_OFFLINE,
    ItemState_WAITING,
    ItemState_DROPOFF,
    ItemState_STORAGE,
    ItemState_PICKUP
};

struct CrateItemInfo {
    Oryol::String itemName;

    int points;
    int payment;
    double pickupTimeMin;
    double pickupTimeMax;

    Oryol::String dropoffChatter;
    Oryol::String pickupChatter;
    Oryol::String pickupChatterGood;
    Oryol::String pickupChatterOkay;
    Oryol::String pickupChatterBad;

    double retreivalTime; // How long the player has to get the item
    
    CrateItemInfo( Oryol::String _itemName,
              int _points,
              int _payment,
              double _pickupTimeMin,
              double _pickupTimeMax,
              Oryol::String _dropoffChatter,
              Oryol::String _pickupChatter,
              Oryol::String _pickupChatterGood,
              Oryol::String _pickupChatterOkay,
              Oryol::String _pickupChatterBad ) :
                itemName(_itemName),
                points(_points),
                payment(_payment),
                pickupTimeMin(_pickupTimeMin),
                pickupTimeMax(_pickupTimeMax),
                dropoffChatter( _dropoffChatter),
                pickupChatter( _pickupChatter ),
                pickupChatterGood( _pickupChatterGood ),
                pickupChatterOkay( _pickupChatterOkay ),
                pickupChatterBad( _pickupChatterBad )
    {
        retreivalTime = 30.0; // Should this be variable?
    }
};

// An item instantiated into the scene
struct CrateItemFoo {
    CrateItemInfo *info;
    
    SceneObject *sceneObj;
    double pickupTime; // When she will come back for the item
    ItemState state;
    
    // Silly stuff
    glm::vec3 wobAxis;

    CrateItemFoo( CrateItemInfo *_info=NULL ) : info(_info) {
        sceneObj = NULL;
        if (info != NULL) {
            pickupTime = glm::linearRand( info->pickupTimeMin, info->pickupTimeMax );
        } else {
            pickupTime = 60.0;
        }
        state = ItemState_OFFLINE;
        wobAxis = glm::vec3(0.0f,1.0f, 0.0f);
    }
};

struct _CratePickupHelper {
    bool needsPickup = false;
    double soonestPickupTime = 99999.0;
    CrateItemFoo pickupItem;
};

struct CustomerSlot {
    SceneObject *loadingZoneObj;
    SceneObject *personObj;
    
    CrateItemFoo item;
    
    bool hasItem;
    int index;
    CustomerSlot() : loadingZoneObj(NULL), hasItem(false)
    {
    }
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
    
    CrateItemFoo *hoverItem; // NOTE: Must be in looseItems
    CustomerSlot *hoverSlot;
    
    Oryol::Array<CrateItemFoo> items; // Items player is holding
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
    void updateCustomers();
    void updateGameCamera();
    
    void scanForPickupItems( Oryol::Array<CrateItemFoo> &items, _CratePickupHelper &helper );
    
    //void titleScreenUI();
    void gameUI();
    void displayMessage( const Oryol::String &message );
    
    void populateCustomers( Oryol::Array<CrateItemInfo> &allItems );
    
    bool tryPickupItem( CrateItemFoo pickupItem );
    
    CrateItemFoo spawnItem( glm::mat4 &xform );
    
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
    
    const int kPlayerItemCapacity = 5;
    const double kWriteTickTime = 0.02;
    const float32 kPlayerFrontDist = 7.0;
    const float32 kPlayerRadius = 3.5f;
    const float32 kPlayerHeight = 3.87031;
    const float32 kCamBorderDist = 10.0f;
    const float32 kCamHeightLow = 60.0;
    const float32 kCamHeightHigh = 100.0;
    
    bool showCameraBorder;
    float32 targetHeightNorm;
    float32 targetHeight;
    float32 currHeight;
    
    // Gameplay stuff
    Oryol::TimePoint startTimePoint;
    Oryol::TimePoint lastTimePoint;
    Oryol::Duration frameDt;
    
    PlayerBot player;
    glm::vec3 gameCameraTarget;
    float32 time;
    
    // player stats
    uint32_t gamePoints;
    uint32_t money;
    uint32_t gameTime; // not sure what units this should be displayed in yet
    double gameTimeSeconds = 0;
    
    double nextCustomerTime;
    
    // Master copy of all items
    Oryol::Array<CrateItemInfo> allItems;
    
    // Items not carried by the player or in storage
    Oryol::Array<CrateItemFoo> looseItems;
    
    // Spaces where the player can trigger a customer
    Oryol::Array<CustomerSlot> custSlots;
    
    // Messages
    Oryol::String message;
    Oryol::String writeMessage;
    int messageWriteLen;
    double writeTick;
    
    
    // Master copies of objects for spawning
    SceneObject *masterCrate;
    SceneObject *masterPerson;
    
    glm::vec4 testCrateColors[5];
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
    double frameDtSecs = frameDt.AsSeconds();
    time += frameDtSecs;
    
    this->handleInput();
    
    Camera *activeCamera = getActiveCamera();
    
    // Update the player's transform, make this better
    if (player.sceneObj) {
        
        // TODO: don't increment this if we're showing a popup
        gameTimeSeconds += frameDtSecs;
        
        updateCustomers();
        updatePlayer();
        updateGameCamera();
        
        // Update message
        if (!message.Empty()) {
            writeTick += frameDtSecs;
            if (writeTick > kWriteTickTime ) {
                writeTick = 0.0;
                messageWriteLen += 1;
                if (messageWriteLen < message.Length()) {
                    writeMessage = Oryol::String( message,  0, messageWriteLen );
                } else if (messageWriteLen > message.Length() + 30) {
                    // Clear message
                    message = "";
                    writeMessage = "";
                }
            }
        }

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
    Oryol::Duration appTime = lastTimePoint.Since( startTimePoint );
    dd::flush( appTime.AsMilliSeconds() );

    // Do game UI
    gameUI();
    
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
    
    // Setup UI
    NKUI::Setup();
    Memory::Clear(&g_uiMedia, sizeof(g_uiMedia));
    Log::Info("---- setup NKUI ---\n");

//    // setup custom fonts
//    IO::Load("data:ltromatic.ttf", [this](IO::LoadResult loadResult) {
//        // need to make the data static
//        this->ttfData = std::move(loadResult.Data);
//        NKUI::BeginFontAtlas();
//        g_uiMedia.font_14 = NKUI::AddFont(this->ttfData, 14.0f);
//        g_uiMedia.font_18 = NKUI::AddFont(this->ttfData, 18.0f);
//        g_uiMedia.font_20 = NKUI::AddFont(this->ttfData, 20.0f);
//        g_uiMedia.font_24 = NKUI::AddFont(this->ttfData, 24.0f);
//        NKUI::EndFontAtlas();
//        this->fontValid = true;
//    });
//
//
//    load_icon( "data:farmageddon_title.png", &(g_uiMedia.farmageddon_title));
//    load_icon( "data:farmageddon_gameover.png", &(g_uiMedia.farmageddon_gameover));
//
    
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

    populateCustomers( allItems );
    
    //Log::Info("Hello...\n");
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
    this->startTimePoint = this->lastTimePoint;
    
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

// TODO: Move this to another file because its going to get big...
void
TestApp::populateCustomers( Oryol::Array<CrateItemInfo> &allItems )
{
    allItems.Add( CrateItemInfo( "Barrel", 10, 5, 60, 300,
                            "Got a barrel for you. It's just a regular barrel.",
                            "I'll be needing a barrel, thanks.",
                            "That was fast, thanks. I've got time to grab a sandwich.",
                            "Yep, that's a barrel.",
                            "Jeez, how long does it take to find a stinkin barrel?" ) );
    
    allItems.Add( CrateItemInfo( "Wooden Crate", 10, 8, 60, 300,
                            "Boss said to drop off some crates here.",
                            "I need a crate, just a wooden one is fine.",
                            "Nice work, you're pretty efficient around here.",
                            "This crate will do nicely.",
                            "Man, took you that long to find a crate in a video game?" ) );
}

void
TestApp::startGame() {
    
    Log::Info("Hello from startGame..." );
    
    showCameraBorder = false;
    
    testCrateColors[0] = glm::vec4(0.23f,0.81f,0.98f,1.0f); // Blue
    testCrateColors[1] = glm::vec4(0.97f,0.20f,0.12f,1.0f); // Red
    testCrateColors[2] = glm::vec4(0.96f,0.59f,0.18f,1.0f); // Orange
    testCrateColors[3] = glm::vec4(0.39f,0.73f,0.30f,1.0f); // Green
    testCrateColors[4] = glm::vec4(0.92f,0.88f,0.46f,1.0f); // Yellow
    
    // Init player
    player.sceneObj = scene->FindNamedObject( "robot" );
    if (player.sceneObj == NULL) {
        Log::Warn("Couldn't find player object.");
    }
    player.pos = glm::vec3( 0.0 );
    
    masterCrate =  scene->FindNamedObject( "Crate" );
    masterCrate->hidden = true;
    
    masterPerson =  scene->FindNamedObject( "Person" );
    masterPerson->hidden = true;
    
    targetHeight = kCamHeightLow;
    currHeight = targetHeight;
    targetHeightNorm = 1.0f;
    gameTimeSeconds = 0;
    
    // Find customer crate slots
    custSlots.Clear();
    for (int i=0; i < scene->sceneObjs.Size(); i++) {
        SceneObject *sceneObj = scene->sceneObjs[i];
        
        if (sceneObj->objectName.Length() >= 8) {
            Oryol::String prefix( sceneObj->objectName, 0, 8 );
            //Log::Info("Prefix is >%s<\n", prefix.AsCStr() );
            if (prefix == "Customer") {
                CustomerSlot slot;
                slot.loadingZoneObj = sceneObj;
                sceneObj->invXform = glm::inverse( sceneObj->xform );
                
                slot.personObj = scene->spawnObject( masterPerson->mesh );
                glm::mat4 &zoneXform = slot.loadingZoneObj->xform;
                glm::vec3 zonePos = glm::vec3(zoneXform[3].x,
                                              zoneXform[3].y + glm::linearRand(9.0f, 11.0f),
                                              zoneXform[3].z - glm::linearRand(0.0f, 1.5f) );
                slot.personObj->xform = glm::translate( glm::mat4(), zonePos );
                slot.personObj->hidden = true;
                
                slot.index = custSlots.Size();
                custSlots.Add( slot );
            }
        }
    }
    
    // reset stats
    gameTime = 0;
    money = 0;
    gamePoints = 0;
    nextCustomerTime = 5.0;
    
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
    
    
    // Zoom to customer service if player is hovering a customer
    if (player.hoverSlot != NULL) {
        targetHeightNorm -= heightMove;
        
        glm::mat4 &zoneXform = player.hoverSlot->loadingZoneObj->xform;
        glm::vec3 zonePos = glm::vec3(zoneXform[3].x, zoneXform[3].y + 15.0f, zoneXform[3].z);
        
        // Fast zoom to zonePos
        float t = 0.02;
        gameCameraTarget = gameCameraTarget*(1.0f-t) + zonePos*t;
    } else {
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
    }
    targetHeightNorm = PAR_CLAMP(targetHeightNorm, 0.0f, 1.0f);
    
    // Ugh so bad
    //Log::Info("Vel is %3.2f\n", velScale );
    
    float hite = par_easings_in_out_cubic( targetHeightNorm );
    float maxHite = (kCamHeightHigh-kCamHeightLow);
    currHeight = kCamHeightLow + hite * maxHite;
    
    gameCamera.Pos = gameCameraTarget + glm::vec3( 0.0f, 0.0f, currHeight );
    glm::mat4 tilt = glm::rotate( glm::mat4(), glm::radians( 20.0f ), glm::vec3(1,0,0) );
    gameCamera.Model = glm::translate( tilt, gameCamera.Pos );
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

CrateItemFoo TestApp::spawnItem( glm::mat4 &xform )
{
    SceneObject *newCrate = scene->spawnObject( masterCrate->mesh );
    newCrate->xform = glm::mat4( xform );
    
    glm::vec4 randCrateColor = testCrateColors[ rand() % 5 ];
    newCrate->vsParams.tintColor = randCrateColor;
    
    int index = rand() % allItems.Size();
    CrateItemFoo crate = CrateItemFoo( &(allItems[index]) );
    
    // FIXME: Destory old sceneObj if it's already been created
    
    crate.sceneObj = newCrate;
    return crate;
}

void TestApp::scanForPickupItems( Oryol::Array<CrateItemFoo> &items, _CratePickupHelper &helper )
{
    for (int i=0; i < items.Size(); i++ ) {
        if (gameTimeSeconds > looseItems[i].pickupTime ) {
            if (looseItems[i].pickupTime < helper.soonestPickupTime) {
                helper.pickupItem = looseItems[i];
                helper.needsPickup = true;
                helper.soonestPickupTime = helper.pickupItem.pickupTime;
            }
        }
    }
}

void
TestApp::updateCustomers()
{
    // Spawn a new customer?
    double frameTimeSec = frameDt.AsSeconds();
    if (nextCustomerTime > frameTimeSec ) {
        nextCustomerTime -= frameTimeSec;
        
    } else {
        
        // Find a slot for this customer to go to
        CustomerSlot *slot = NULL;
        
        // Pick in a random order for variety
        int custSlotIndex[5] = { 0, 1, 2, 3, 4};
        for (int i=4; i >= 2; i--) {
            int j = rand() % i;
            int t = custSlotIndex[j];
            custSlotIndex[j] = custSlotIndex[i];
            custSlotIndex[i] = t;
        }
        
        for (int i=0; i < custSlots.Size(); i++) {
            // It's available!
            int ndx = custSlotIndex[i];
            if (!custSlots[ndx].hasItem) {
                slot = &(custSlots[ndx]);
                Log::Info("Spawn Customer in slot %d\n", i );
                break;
            }
        }
        
        // Spawn a new customer
        if (slot==NULL) {
            Log::Warn("All Slots are full! can't spawn");
        } else {
            
            // See if there are any items that need to be retreived
            _CratePickupHelper pickup;
            scanForPickupItems( looseItems, pickup );
            scanForPickupItems( player.items, pickup );
            
            // TODO: Randomly add new customers even if there are stuff to pickup
            if (pickup.needsPickup) {
                Log::Info("Item needs pickup!" );
                
                slot->hasItem = true;
                slot->item = pickup.pickupItem;
                slot->item.state = ItemState_PICKUP;
                slot->item.sceneObj = NULL;
                slot->personObj->hidden = false;
                
            } else {
            
                // Spawn on the counter in front of player
                glm::mat4 &zoneXform = slot->loadingZoneObj->xform;
                glm::vec3 spawnPos = glm::vec3(zoneXform[3].x, zoneXform[3].y + 5.0f, zoneXform[3].z + 1.5f);
                
                glm::mat4 itemLoc = glm::translate( glm::mat4(), spawnPos );
                
                slot->hasItem = true;
                slot->personObj->hidden = false;
                slot->item = spawnItem( itemLoc );
                slot->item.state = ItemState_DROPOFF;
                
                //Log::Info("Customer Says: %s\n", slot->item.info->dropoffChatter.AsCStr() );
            }
        }
        
        // Another customer in 10s
        nextCustomerTime = 10.0;
    }
}

bool collidePointBBox( glm::vec4 p, glm::mat4 bboxInvXform, SceneObject *obj, float32 expandRadius )
{
    glm::vec4 pntInBboxSpace = bboxInvXform * p;
    if ((pntInBboxSpace.x >= (obj->mesh->bboxMin.x - expandRadius) ) &&
        (pntInBboxSpace.x <= (obj->mesh->bboxMax.x + expandRadius) ) &&
        (pntInBboxSpace.y >= (obj->mesh->bboxMin.y - expandRadius) ) &&
        (pntInBboxSpace.y <= (obj->mesh->bboxMax.x + expandRadius) ) ) {
        return true;
    }
    return false;
}

void
TestApp::updatePlayer()
{
    // update stats
    gameTime += frameDt.AsMilliSeconds();

    glm::vec3 playerOldPos = glm::vec3( player.pos );
    
    // update player's pos
    float t = 0.1;
    player.vel = (player.vel * (1.0f-t)) + (player.targetVel * t);
    player.pos += player.vel;
    
    if (glm::length( player.vel) > 0.01f) {
        
        // if we're not "backing up"
        glm::vec3 velDir = glm::normalize( player.vel );
        float dot = glm::dot( player.facingDir, velDir );
        if (dot >= -0.8)
        {
            player.facingDir = glm::normalize( player.vel );
        }
    }
    
    // Check collision
    glm::vec4 playerVec4 = glm::vec4( player.pos, 1.0f );
    glm::vec4 playerTestPoints[4];
    
    playerTestPoints[0] = playerVec4 + glm::vec4( playerVec4.x + player.facingDir.x * 3.5, playerVec4.y + player.facingDir.y * 3.5, 0.0f, 1.0f );
    playerTestPoints[1] = playerVec4 + glm::vec4( playerVec4.x - player.facingDir.x * 3.5, playerVec4.y - player.facingDir.y * 3.5, 0.0f, 1.0f );
    playerTestPoints[2] = playerVec4 + glm::vec4( playerVec4.x + player.facingDir.y * 3.5, playerVec4.y + player.facingDir.x * 3.5, 0.0f, 1.0f );
    playerTestPoints[3] = playerVec4 + glm::vec4( playerVec4.x - player.facingDir.x * 3.5, playerVec4.y - player.facingDir.x * 3.5, 0.0f, 1.0f );
    
    //const float *playerBoundsCol = dd::colors::SeaGreen;
    bool didCollideWalls = false;
    for (int i=0; i < scene->sceneObjs.Size() && !didCollideWalls; i++) {
        SceneObject *obj = scene->sceneObjs[i];
        if (obj->collider) {
            // TODO: store this on sceneObj and only updated when moved
            glm::mat4 objInv = glm::inverse( obj->xform );
            
            //const char *hitStr = "No Hit";
            for (int j=0; j < 4; j++) {
                
                if (collidePointBBox( playerTestPoints[j], objInv, obj, kPlayerRadius)) {
                    didCollideWalls = true;
                    break;
                }
            }
        }
    }
    //dd::sphere( glm::value_ptr(player.pos), playerBoundsCol, kPlayerRadius );
    //Log::Info("---\n");
    
    // If collision, just undo the movement (but not the facing rotation)
    if (didCollideWalls) {
        player.pos = playerOldPos;
    }
    
    
    // Check on what's in front of player
    glm::mat4 mSensor = glm::translate( player.sceneObj->xform,
                               glm::vec3(0.0, kPlayerFrontDist, 0.0) );
    glm::vec4 frontPos = mSensor * glm::vec4(0,0,0,1);
    
    
    const float *cursorCol = dd::colors::Red;
    player.hoverItem = NULL;
    for (int i=0; i < looseItems.Size(); i++) {
        // TODO: cache this inverse somewhere
        CrateItemFoo &item = looseItems[i];
        glm::mat4 crateInv = glm::affineInverse( item.sceneObj->xform );
        
        if (collidePointBBox(frontPos, crateInv, item.sceneObj, 0.5)) {
            cursorCol = dd::colors::YellowGreen;
            player.hoverItem = &item;
            break;
        }
    }
    
    player.hoverSlot = NULL;
    for (int i=0; i < custSlots.Size(); i++) {
        CustomerSlot *slot = &(custSlots[i]);
        slot->loadingZoneObj->hidden = true; // DBG
        
        if (collidePointBBox( frontPos, slot->loadingZoneObj->invXform, slot->loadingZoneObj, 0.5)) {
            cursorCol = dd::colors::PaleTurquoise;
            player.hoverSlot = slot;
            break;
        }
    }
    
    dd::sphere( glm::value_ptr(frontPos), cursorCol, 3.0 );
    if (player.hoverSlot) {
        player.hoverSlot->loadingZoneObj->hidden = false;
    }
    
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
    if (Input::KeyDown(Key::G)) {
        displayMessage( "This is a test message. Hello there out in message land. It's pretty late." );
    }
    
    // Mode-specific input
    if (debugMode) {
        this->handleInputDebug();
    } else {
        this->handleInputGame();
    }
}

void
TestApp::displayMessage( const Oryol::String &_message )
{
    Log::Info( "MESSAGE: %s\n", _message.AsCStr() );
    this->message = _message;
    this->writeMessage = "";
    this->writeTick = 0.0;
    this->messageWriteLen = 0;
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
        
        if (Input::KeyDown(Key::Space)) {
            
            // Are we selecting an item?
            if ((player.hoverSlot) && (player.hoverSlot->hasItem)) {
                
                if (player.hoverSlot->item.state == ItemState_PICKUP) {
                    
                    CrateItemInfo *topItemInfo = NULL;
                    if (player.items.Size() > 0) {
                        topItemInfo = player.items.Back().info;
                    
                        // Does item match?
                        if (player.hoverSlot->item.info == topItemInfo) {
                            // Yay.. cash it in
                            //CrateItemFoo topItem = player.items.PopBack();
                            
                            gamePoints += topItemInfo->points;
                            money += topItemInfo->payment;
                            
                            player.hoverSlot->hasItem = false;
                            player.hoverSlot->personObj->hidden = true;
      
                        } else {
                            char buff[500];
                            sprintf( buff, "That's not what I'm looking for. %s.",
                                    player.hoverSlot->item.info->pickupChatter.AsCStr() );
                            displayMessage( buff );
                        }
                        
                    } else {
                        char buff[500];
                        sprintf( buff, "Hello there! %s",
                                player.hoverSlot->item.info->pickupChatter.AsCStr() );
                        displayMessage( buff );
                    }
                    
                } else {
                    // Show Message for customer TODO
                    if (tryPickupItem( player.hoverSlot->item)) {
                        
                        displayMessage( player.hoverSlot->item.info->dropoffChatter );
                        
                        player.hoverSlot->item.state = ItemState_STORAGE;
                        
                        player.hoverSlot->item.pickupTime = gameTimeSeconds +
                                glm::linearRand( player.hoverSlot->item.info->pickupTimeMin,
                                                 player.hoverSlot->item.info->pickupTimeMax );

                        // DBG
                        player.hoverSlot->item.pickupTime = glm::linearRand( 3.0f, 10.0f );
                        
                        player.hoverSlot->hasItem = false;
                        player.hoverSlot->personObj->hidden = true; // TODO: walk out the door
                        player.hoverSlot->item = NULL;
                        
                        
                        
                    } else {
                        displayMessage( "Looks like you're full, I'll wait..." );
                    }
                }
            }
            // Is there something in our sensor range?
            else if (player.hoverItem) {
                
                if (tryPickupItem( *player.hoverItem) ) {
                    // Mark it not a collider
                    player.hoverItem->sceneObj->collider = false;
                    
                    // Remove it from loose items
                    for (int i=0; i < looseItems.Size(); i++ ) {
                        if (player.hoverItem == &(looseItems[i]) ) {
                            looseItems.EraseSwapBack(i);
                            break;
                        }
                    }
                }
                
            } else {
                // Drop top crate in front
                if (!player.items.Empty()) {
                    CrateItemFoo crate = player.items.PopBack();
                    crate.sceneObj->xform = glm::translate( player.sceneObj->xform,
                                                           glm::vec3(0.0, kPlayerFrontDist, 0.0) );
                    crate.sceneObj->collider = true;
                    looseItems.Add( crate );
                }
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

bool TestApp::tryPickupItem( CrateItemFoo pickupItem )
{
    if (player.items.Size() >= kPlayerItemCapacity) {
        
        ddVec3 textPos2D = { 0.0f, 20.0f, 0.0f };
        textPos2D[0] = fbWidth / 2.0;
        textPos2D[1] = fbHeight - 20.0;
        dd::screenText("Carry Limit", textPos2D, dd::colors::Orange, 1.0, 500 );
        return false;
    } else {
        
        if (player.items.Empty()) {
            pickupItem.wobAxis = glm::vec3( 0,1,0);
        } else {
            pickupItem.wobAxis = player.items[player.items.Size()-1].wobAxis;
            pickupItem.wobAxis += (glm::sphericalRand(0.3f)  );
            pickupItem.wobAxis = glm::normalize( pickupItem.wobAxis );
        }
        
        player.items.Add( pickupItem );
        return true;
    }
}

void
TestApp::gameUI()
{
    // Draw UI Layer
    nk_context* ctx = NKUI::NewFrame();
    
    //ctx->style.window.fixed_background.data.color.a = 100;
    //ctx->style.button.normal.data.color.a = 100;
    
    struct nk_color origBorder = nk_rgb(65, 65, 65); //ctx->style.window.border_color;
    
    // Can't figure out how to nest NKUI rows, so doing this
    // the clunky way for now
    float infoThinggyWidth = fbWidth * 0.2;
    float custStatusWidth = fbWidth * 0.6;
    float itemSlotWidth = custStatusWidth / 5.0;
    
    char buff[256];
    struct nk_panel layout;
    static nk_flags window_flags = 0;
    window_flags |= NK_WINDOW_BORDER;
    if (nk_begin(ctx, &layout, "StatusWin", nk_rect( 0, 0, infoThinggyWidth, 100), window_flags))
    {
        nk_layout_row_dynamic( ctx, 20, 1);
        sprintf( buff, "POINTS: %d", gamePoints );
        nk_label_colored(ctx, buff, NK_TEXT_LEFT, nk_rgb(255,255,0));
    }
    nk_end(ctx);
    

    
    for (int i=0; i < 5; i++) {
        
        int ndx = 4-i; // Items are accidentally placed right to left
        
        // Highlight border if a slot is active
        if ((custSlots.Size() > ndx) && (player.hoverSlot) && (player.hoverSlot->index == ndx)) {
            ctx->style.window.border_color = nk_rgb( 220, 200, 30);
        } else {
            ctx->style.window.border_color = origBorder;
        }
    
        
        
        float tilex = infoThinggyWidth + itemSlotWidth*i;
        sprintf( buff, "tile%d", i ); // Window name must be unique
        if (nk_begin(ctx, &layout, buff, nk_rect( tilex, 0, itemSlotWidth, 100), window_flags))
        {
            nk_layout_row_begin( ctx, NK_DYNAMIC, 12, 1 );
            if ((custSlots.Size() > ndx) && (custSlots[ndx].hasItem)) {
                CrateItemInfo *info = custSlots[ndx].item.info;
                nk_label_colored(ctx, info->itemName.AsCStr(), NK_TEXT_CENTERED, nk_rgb(255,255,255) );

                sprintf(buff, "Pnts: %d", info->points );
                nk_label_colored(ctx, "Pnts:", NK_TEXT_CENTERED, nk_rgb(255,0,255) );

                sprintf(buff, "Pays: $%d", info->payment );
                nk_label_colored(ctx, buff, NK_TEXT_CENTERED, nk_rgb(100,255,100) );
                
                if (custSlots[ndx].item.state == ItemState_DROPOFF) {
                    nk_label_colored(ctx, "DROPOFF", NK_TEXT_CENTERED, nk_rgb(255,255,255) );
                } else {
                    nk_label_colored(ctx, "PICKUP", NK_TEXT_CENTERED, nk_rgb(255,255,255) );
                }
            } else {
                nk_label_colored(ctx, "No Customer", NK_TEXT_CENTERED, nk_rgb(100,100,100) );
            }
            nk_layout_row_end(ctx);
        }
        nk_end(ctx);
    }
    ctx->style.window.border_color = origBorder;
    
    if (nk_begin(ctx, &layout, "StatsWin", nk_rect( fbWidth-infoThinggyWidth, 0, infoThinggyWidth, 120), window_flags))
    {
        sprintf( buff, "Game Time: %d", gameTime / 100 );
        nk_layout_row_begin( ctx, NK_DYNAMIC, 20, 1 );
        nk_label_colored(ctx, buff, NK_TEXT_RIGHT, nk_rgb(255,255,255));
        nk_layout_row_end(ctx);
        
        sprintf( buff, "Next Cust: %3.2f", nextCustomerTime );
        nk_layout_row_begin( ctx, NK_DYNAMIC, 20, 1 );
        nk_label_colored(ctx, buff, NK_TEXT_RIGHT, nk_rgb(255,255,255));
        nk_layout_row_end(ctx);
        
        sprintf( buff, "Money: $%d", money );
        nk_layout_row_begin( ctx, NK_DYNAMIC, 20, 1 );
        nk_label_colored(ctx, buff, NK_TEXT_RIGHT, nk_rgb(100,255,100));
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
    
    // Text message
    if (!message.Empty()) {
        
        const float msgHeight = 100;
        float msgWidth = fbWidth - 100;
        if (nk_begin(ctx, &layout, "MessageWin", nk_rect( 50, fbHeight - (msgHeight + 10), msgWidth, msgHeight ), window_flags))
        {
            //nk_layout_row_dynamic(ctx, 50, 1);
            nk_layout_row_static(ctx, 90 /* height */, msgWidth - 30 /* item width */, 1 /* cols */);
            nk_label_wrap(ctx, writeMessage.AsCStr() );
        }
        nk_end(ctx);
    }
    
    NKUI::Draw();

}

