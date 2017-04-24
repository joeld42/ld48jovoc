//------------------------------------------------------------------------------
//  TestApp.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Core/Time/Clock.h"
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
#include "IsosurfaceBuilder.h"
#include "Planet.h"
#include "GameObjects.h"

#define PAR_EASINGS_IMPLEMENTATION
#include "par_easings.h"

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


enum GameState {
    GameState_TITLE,
    GameState_GAME,
    GameState_GAME_OVER,
};


struct UIMedia {
    struct nk_font *font_14;
    struct nk_font *font_18;
    struct nk_font *font_20;
    struct nk_font *font_24;
    struct nk_image farmageddon_title;
} g_uiMedia;

// derived application class
class TestApp : public App {
public:
    AppState::Code OnRunning();
    AppState::Code OnInit();
    AppState::Code OnCleanup();

private:
    void handle_input( float dt ); // and update camera..
    
    glm::mat4 computeMVP(const glm::mat4& proj, float32 rotX, float32 rotY, const glm::vec3& pos);

    void loadMesh( const char *path);
    void createMaterials();
    
    void drawMeshAndTextureIfLoaded( Id mesh, Id texture );
    
    void BuildPlanet();
    
    ResourceLabel curMeshLabel;
//    MeshSetup curMeshSetup;
    ResourceLabel curMaterialLabel;
    int numMaterials = 0;

    bool paused;
    
    Id texture;
    DrawState mainDrawState;
    
    Planet planet;
    osn_context *noiseCtx;
  
    Camera dbgCamera;
    Camera gameCamera;
    
    Scene *scene;
    
    //SceneObject *treeCursor;
    
    SceneObject *cannonBase;
    SceneObject *cannonBushing;
    SceneObject *cannonBarrel;
    
    /// load a single icon, asynchronously
    void load_icon(const char* url, struct nk_image* img);
    
    bool tmpUI;
    
    bool useDebugCamera;
    Camera *currentCamera();

    float32 fbWidth;
    float32 fbHeight;
    glm::vec3 screenPosToPlanet( glm::vec2 screenPos );
    glm::vec3 arcballVector( glm::vec2 screenPos );
    
    glm::vec3 lastArcball;
    glm::vec3 arcballAxis;
    float arcballAngle;
    
    // camera moves
    glm::quat animCamTarg;
    glm::quat animCamStart;
    float animCamTimer;
    bool doAnimCam;
    
    float animT;
    
    // Gameplay thinggys
    int activeCannon;
    Oryol::Array<Cannon> cannons;
    Oryol::Array<Shot> shots;
    void finishTurn();
    void fireActiveCannon();
    void lookAtCannon( Cannon &cc );
    void lookAtPos( glm::vec3 pos );
    void SpawnCannons();
    void StartGame();
    
    Oryol::Array<TeamInfo> teams;
    
    struct nk_style_item mkColorStyleItem( const char *hexColor );
    
    void DoGameUI( nk_context* ctx );
    void DoGameUI_Title( nk_context* ctx );
    void DoGameUI_Gameplay( nk_context* ctx );
    void DoGameUI_Results( nk_context* ctx );

    bool fontValid = false;
    Buffer ttfData;


    GameState gameState;
    
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

Camera *TestApp::currentCamera()
{
    return useDebugCamera?&dbgCamera:&gameCamera;
}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnRunning() {
    
    static TimePoint tp;
    Duration frameDuration = Clock::LapTime(tp);
    float dt = frameDuration.AsSeconds();
    
    // safety check
    bool safeToUpdate = true;
    if ((!scene->didSetupPipeline) || (dt > 0.5f)) {
        safeToUpdate = false;
    }
    
    this->handle_input( dt );
    
    if ((!paused) && (safeToUpdate))
    {
#if 1
        // Update cannons "falling"
        for (int i=0; i < cannons.Size(); i++ ) {
            
            Cannon &cc = cannons[i];
            
            for(int step = 0; step < 10; step++)
            {
                glm::vec3 evalP = cc.objBase->pos / planet.worldSize;
                float d = planet.surfBuilder.evalSDF( evalP );

                if (d < 0.001) {
                    // on surface
                    break;
                } else {
                    // Cannon is off the ground, make falling
                    glm::vec3 upDir = planet.surfBuilder.evalNormal( evalP );
                    glm::vec3 upDir2 = glm::normalize( evalP );
                    //printf("upDir %3.2f %3.2f %3.2f -- dt %3.2f\n", upDir.x, upDir.y, upDir.z, dt );
                    
                    glm::vec3 newPos = cc.objBase->pos + (upDir*-100.0f*dt);
                    //glm::vec3 newPos = cc.objBase->pos;
                    cc.place( newPos, upDir );
                
                }
            }
        }
#endif
        
        // Update shots
        bool planetNeedsRebuild = false;
        for (int i=0; i < shots.Size(); i++) {
            Shot &ss = shots[i];
            
            glm::vec3 evalP = ss.objShot->pos / planet.worldSize;
            
            //glm::vec3 grav = planet.surfBuilder.evalNormal( evalP );
            glm::vec3 grav = glm::normalize( evalP );
            
            // Less gravity as you get near the center
            float gravStr = glm::smoothstep( 0.0f, planet.planetApproxRadius,
                                             glm::length(ss.objShot->pos) );
            
            grav *= -10000.0 * gravStr;
            
            ss.updateBallistic( dt, grav );
            evalP = ss.objShot->pos / planet.worldSize;
            
            // check if we hit..
            float f = planet.surfBuilder.evalSDF( evalP );
            if ((f <= 0.0f)||(ss.age <= 0.0)) {
                
                planetNeedsRebuild = true;
                printf("Hit Planet (%3.2f %3.2f %3.2f)...\n",
                       evalP.x, evalP.y, evalP.z );
                
                planet.surfBuilder.addDamage( evalP, 1000.0/planet.worldSize.x );
                scene->removeObject( ss.objShot );
                
                // FIXME: sloppy and might break stuff, removing while iterating
                shots.EraseSwapBack( i );
                
                lookAtPos( ss.objShot->pos );
            }
        }
        
        if (planetNeedsRebuild) {
            planet.Rebuild( scene );
        }
        
//        // DEMO, autofire if able
//        if (shots.Size()==0) {
//            fireActiveCannon();
//        }
    }
    
    // ----------------------------------------
    
    // Update model mats
    for (int i=0; i < scene->sceneObjs.Size(); i++) {
        SceneObject *obj = scene->sceneObjs[i];
        
        glm::mat4 modelTform = glm::translate(glm::mat4(), obj->pos);
        modelTform = modelTform * glm::mat4_cast( obj->rot );
//        return proj * this->view * modelTform;
        
        glm::mat4 mvp = currentCamera()->ViewProj * modelTform;
        obj->vsParams.ModelViewProjection = mvp;
    }
    
    // Planet has no model transform, is always at 0,0,0
    planet.UpdateCamera( currentCamera() );
    
    Gfx::BeginPass();
    
    // Draw scene objects
    scene->drawScene();
    
    // Draw planet isosurf
    planet.Draw();
    
    
    
#if 1
    // Draw UI
    nk_context* ctx = NKUI::NewFrame();
    
    enum {EASY, HARD};
    static int op = EASY;
    static float value = 0.6f;
    struct nk_panel layout;
    
    if (tmpUI) {
        if (nk_begin(ctx, &layout, "Show", nk_rect(50, 50, 220, 500),
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
    //        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
    //        {
    //            nk_layout_row_push (ctx, 50);
    //            nk_label(ctx, "Volume:", NK_TEXT_LEFT);
    //            nk_layout_row_push(ctx, 110);
    //            nk_slider_float(ctx, 0, &value, 1.0f, 0.1f);
    //        }
    //        nk_layout_row_end(ctx);
            
            
            char buff[20];
            bool needsRebuild = false;
            
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            nk_layout_row_push (ctx, 50);
            nk_label(ctx, "Flex:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 110);
            if (nk_slider_float(ctx, -1.0f, &(planet.surfBuilder.dbgPush), 1.0f, 0.1f)) {
                needsRebuild = true;
            }
            nk_layout_row_end(ctx);
            
            for (int i=0; i < 8; i++) {
                nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
                nk_layout_row_push (ctx, 50);
                sprintf( buff, "P%d:", i );
                nk_label(ctx, buff, NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 110);
                if (nk_slider_float(ctx, -1.0f, &(planet.surfBuilder.pdbg[i]), 1.0f, 0.1f)) {
                    needsRebuild = true;
                }
                nk_layout_row_end(ctx);
            }
            
            if (needsRebuild) {
                planet.Rebuild( scene );
            }
        
            nk_end(ctx);
        }
    }
    
    // Game Mode UI
    DoGameUI( ctx );
    
    NKUI::Draw();
#endif
    
    Gfx::EndPass();
    
    Gfx::CommitFrame();
    
    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

glm::vec3 TestApp::arcballVector( glm::vec2 screenPos )
{
    glm::vec3 P = glm::vec3(1.0*screenPos.x/fbWidth*2 - 1.0,
                            1.0*screenPos.y/fbHeight*2 - 1.0,
                            0);
    P.y = -P.y;
    float OP_squared = P.x * P.x + P.y * P.y;
    if (OP_squared <= 1*1)
        P.z = sqrt(1*1 - OP_squared);  // Pythagore
    else
        P = glm::normalize(P);  // nearest point
    return P;
}

//glm::vec3 TestApp::arcballVector( glm::vec2 screenPos )
//{
//    
//}

//------------------------------------------------------------------------------
AppState::Code
TestApp::OnInit() {
    
    gameState = GameState_TITLE;
    
    // Dev hack to ensure we're running from the right place...
    chdir("/Users/joeld/Projects/ld48jovoc/ld38_smallworld/smallworld");
    FILE *fp = fopen( "./gamedata/tree_062.omsh", "rt");
    if (!fp) {
        printf("Error! Not running from the right path...??\n");
        exit(1);
    }
    fclose(fp);
    
    useDebugCamera = false;
    animT = 0.0f;
    
    Oryol::TimePoint tp = Clock::Now();
    srand( tp.getRaw() );
    open_simplex_noise( tp.getRaw(), &noiseCtx );
    
    // set up IO system
    IOSetup ioSetup;
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
//    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
    ioSetup.Assigns.Add("msh:", "cwd:gamedata/");
    ioSetup.Assigns.Add("tex:", "cwd:gamedata/");
    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
    IO::Setup(ioSetup);
    
    scene = new Scene();

    scene->gfxSetup = GfxSetup::WindowMSAA4(1280, 720, "LD32 Small World");
    scene->gfxSetup.DefaultPassAction = PassAction::Clear(glm::vec4(0.12,0.11,0.24, 1.0f) );
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
    fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    fbHeight = Gfx::DisplayAttrs().FramebufferHeight;
    
    this->dbgCamera.Setup(glm::vec3(0.f, 0.f, 5000.0f), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 10000.0f);
    this->gameCamera.Setup(glm::vec3(0.0f, 0.0f, 4000.0f), glm::radians(90.0f), fbWidth, fbHeight, 1.0f, 10000.0f);
    //this->gameCamera.Setup(glm::vec3(0.0f, 0.0f, 6000.0f), glm::radians(45.0f), fbWidth, fbHeight, 1.0f, 10000.0f);
    
//    SceneObject *ground = scene->addObject( "msh:ground1_big.omsh", "tex:ground1.dds");
//    ground->pos = glm::vec3( 0.0, -3000.0, 0.0);

    const glm::vec3 minRand(-5500.0f, -3010.0, -5500.0f );
    const glm::vec3 maxRand(5500.0f, -2990.0, 5500.0f );

    
    //for (int i=0; i < 100; i++) {
        
//        SceneObject *obj1 = scene->addObject( "msh:tree_062.omsh", "tex:tree_062.dds");
//        obj1->rot = glm::quat( glm::vec3( 0.0, glm::linearRand( 0.0f, 360.0f), 0.0 ) );
//        obj1->pos = glm::linearRand(minRand, maxRand);
//    
//    treeCursor = obj1;
    //}
    
    
    // Note: there is a 100x scale from blender units
    cannonBase = scene->addObject( "msh:cannon_base.omsh", "tex:cannon_basecolor.dds");
    cannonBase->pos = glm::vec3(0.0, 0.0, 0.0 );
//
//    cannonBushing = scene->addObject( "msh:cannon_bushing.omsh", "tex:cannon_basecolor.dds");
//    cannonBushing->pos = glm::vec3(0.0, 0.0, 0.0 );
//    
//    cannonBarrel = scene->addObject( "msh:cannon_barrel.omsh", "tex:cannon_basecolor.dds");
//    cannonBarrel->pos = glm::vec3(0.0, 191.6, 0.0 );
    
    
    // Setup planet
    planet.planetTexture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "tex:ground1.dds",
                                                                                          scene->texSetup)));
    planet.Setup( &(scene->gfxSetup), noiseCtx );
    
    // Setup UI
    NKUI::Setup();
    
    // setup custom fonts
    IO::Load("data:ltromatic.ttf", [this](IO::LoadResult loadResult) {
        // need to make the data static
        this->ttfData = std::move(loadResult.Data);
        NKUI::BeginFontAtlas();
        g_uiMedia.font_14 = NKUI::AddFont(this->ttfData, 14.0f);
        g_uiMedia.font_18 = NKUI::AddFont(this->ttfData, 18.0f);
        g_uiMedia.font_20 = NKUI::AddFont(this->ttfData, 20.0f);
        g_uiMedia.font_24 = NKUI::AddFont(this->ttfData, 24.0f);
        NKUI::EndFontAtlas();
        this->fontValid = true;
    });

    
    load_icon( "data:farmageddon_title.png", &(g_uiMedia.farmageddon_title));
    
    // Initialize teams
    MakeDefaultTeams( teams );
    
    // Build initial planet
    BuildPlanet();
    
    
    
    // DEMO
//    paused = true;
    
    
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
TestApp::BuildPlanet()
{
    printf("Build planet...\n");
    
    planet.surfBuilder.clearDamage();

    // Start with some damage
    for (int i=0; i < 10; i++) {
        glm::vec3 p = glm::sphericalRand( 0.8 );
        planet.surfBuilder.addDamage( p, glm::linearRand( 0.1f, 0.2f) );
    }
    
    planet.Rebuild( scene );
    
}

void
TestApp::SpawnCannons()
{
    
    // see how many teams are active
    int numActiveTeams = 0;
    int assignIndex = -1;
    for (int i=0; i < teams.Size(); i++) {
        if (teams[i].playerType != Player_NONE) {
            numActiveTeams++;
            if (assignIndex == -1) {
                assignIndex = i;
            }
        }
    }
    
//    switch (numActiveTeams) {
//        
//    }
    
    // Add cannons
    while (cannons.Size() < 12) {
        
        glm::vec3 p = glm::sphericalRand( planet.planetApproxRadius  );
        glm::vec3 upDir = planet.surfBuilder.evalNormal( p );
        Cannon cc( scene, p, upDir );
        cc.applyTeamColor();
        cannons.Add(cc);
    }
    
    lookAtCannon( cannons[activeCannon] );
    
}

void
TestApp::StartGame()
{
    gameState = GameState_GAME;
    SpawnCannons();
}

void
TestApp::finishTurn()
{
    // HERE check if there's only one team with cannons remaining
    
    cannons[activeCannon].applyTeamColor();
    
    do {
        
        activeCannon++;
        if (activeCannon == cannons.Size()) {
            activeCannon = 0;
        }
        
    } while (cannons[activeCannon].health == 0);
    
    
    // Move camera to active cannon
    lookAtCannon( cannons[activeCannon] );
}

void
TestApp::fireActiveCannon()
{
    Cannon &cc = cannons[activeCannon];
    
    // Make sure there's not already a shot in-flight...
    if (shots.Size() == 0)
    {
        SceneObject *objShot = scene->addObject( "msh:pea_shot.omsh", "tex:pea_shot.dds");
        Shot shot( objShot, cc._shootyPoint, cc.calcProjectileVel() );
        shots.Add( shot );
    }
}

void
TestApp::lookAtCannon( Cannon &cc )
{
    lookAtPos( cc.objBase->pos );
}

void
TestApp::lookAtPos( glm::vec3 pos )
{
    doAnimCam = true;
    animCamStart = gameCamera.Rotq;
    glm::vec3 front(0.0f,0.0f,1.0f);
    glm::vec3 cannonDir = glm::normalize( pos );
    
    animCamTarg = glm::quat( front, cannonDir );
    animCamTimer = 0.0;
}

//------------------------------------------------------------------------------
void
TestApp::handle_input( float dt )
{
    
    
    // Update active
    animT += dt;
    
    if (activeCannon < cannons.Size()) {
        cannons[activeCannon].pulseActive( animT );
    }
    
    glm::vec3 move;
    glm::vec2 rot;
    float vel = 3.5f;
    float angSpeed = 1.0f;
    if (Input::KeyboardAttached() ) {
        
        if (Input::KeyPressed( Key::LeftShift)) {
            vel *= 10.0;
            angSpeed *= 10.0f;
        }
        
        if (Input::KeyDown(Key::P)) {
            paused = !paused;
        }
        
        if (useDebugCamera) {
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
        } else if (activeCannon < cannons.Size()) {
            Cannon &cc = cannons[activeCannon];
            if (Input::KeyPressed(Key::W) || Input::KeyPressed(Key::Up)) {
                cc.cannonAngle += angSpeed;
                if (cc.cannonAngle > 180.0) {
                    cc.cannonAngle = 180.0;
                }
            }
            if (Input::KeyPressed(Key::S) || Input::KeyPressed(Key::Down)) {
                cc.cannonAngle -= angSpeed;
                if (cc.cannonAngle < 0.0) {
                    cc.cannonAngle = 0.0;
                }
            }
            if (Input::KeyPressed(Key::A) || Input::KeyPressed(Key::Left)) {
                cc.aimHeading -= angSpeed;
                if (cc.aimHeading < -180.0){
                    cc.aimHeading = 180.0;
                }
            }
            if (Input::KeyPressed(Key::D) || Input::KeyPressed(Key::Right)) {
                cc.aimHeading += angSpeed;
                if (cc.aimHeading > 180.0){
                    cc.aimHeading = -180.0;
                }
            }
            cc.updatePlacement();
            

            // FIRE?
            if (Input::KeyDown(Key::Space) || Input::KeyDown(Key::Z) || Input::KeyDown(Key::X)) {
                
                fireActiveCannon();
            }
            
            if (Input::KeyDown(Key::N)) {
                finishTurn();
            }
            
        }
        
//        if (Input::KeyDown(Key::B)) {
//            if (scene->didSetupPipeline) {
//                BuildPlanet();
//            }
//        }
        
        
        if (Input::KeyDown(Key::GraveAccent)) {
            tmpUI = !tmpUI;
        }
        
        if (Input::KeyDown(Key::Tab)) {
            useDebugCamera = !useDebugCamera;
        }
    }
    
    if (!tmpUI)
    {
        // If debug camera is active, move it with WASD
        if (useDebugCamera)
        {
            if (Input::MouseAttached() ) {
                if (Input::MouseButtonPressed(MouseButton::Left)) {
                    
                    move.z -= vel;
    //                printf("move %3.2f %3.2f %3.2f\n",
    //                       camera.Pos.x, camera.Pos.y, camera.Pos.z );

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
            this->dbgCamera.MoveRotate(move, rot);
            
        } else {
            
            if (Input::MouseAttached() ) {
                // Game camera arcballs
                glm::vec2 mousePos = Input::MousePosition();
                glm::vec3 arcballVec = arcballVector( mousePos );
                
                //treeCursor->pos = arcballVec * planet.planetApproxRadius;
                
//                printf("Tree Cursor %f %f arcball %3.2f, %3.2f, %3.2f\n",
//                       mousePos.x, mousePos.y,
//                       arcballVec.x, arcballVec.y, arcballVec.z
//                       );
                
                if (Input::MouseButtonPressed(MouseButton::Left)) {
                    
                    // DBG:
                    //gameCamera.RotateArcball( glm::vec3(0.0, 1.0, 0.0), glm::radians(1.0f) );
                    
                    arcballAngle = acos(fmin( 1.0f, glm::dot( lastArcball, arcballVec )));
                    if (fabs( arcballAngle ) > 0.001) {
                        arcballAxis = glm::cross( arcballVec, lastArcball );
                        
                        //gameCamera.RotateArcball( axis, angle );
                    }
                }
                lastArcball = arcballVec;
            }
        }
        
        // AUTO-ORBIT
        if (gameState != GameState_GAME) {
            if (fabs( arcballAngle ) < 0.01) {
                arcballAxis = glm::normalize( glm::vec3( 0.2, -1.0, 0.0 ));
                arcballAngle = glm::radians( 0.1 );
            }
        }
        
        // Are we animating the camera?
        if (doAnimCam) {
            
            animCamTimer += dt;
            if (animCamTimer > 1.0f) {
                doAnimCam = false;
            } else {
                float t = par_easings_in_out_cubic( animCamTimer );
                glm::quat animQ = glm::mix( animCamStart, animCamTarg, t );
                gameCamera.SetRotationForAnim( animQ );
            }
            
        } else if (fabs( arcballAngle ) > 0.001) {
            
            // Are we arcballing?
            
            gameCamera.RotateArcball( arcballAxis, arcballAngle );
            arcballAngle = arcballAngle * 0.9;
        }
        
        glm::vec3 lightDir = glm::normalize( glm::vec3( -1.0, 0.5, 0.2 ) );
        planet.planetFSParams.LightDir = lightDir * glm::inverse( gameCamera.Rotq );
        //planet.planetFSParams.LightDir = glm::normalize( glm::ballRand(1.0));
    }
    
    
    
}

//------------------------------------------------------------------------------
static void
//ui_header(struct nk_context *ctx, struct media *media, const char *title)
ui_header(struct nk_context *ctx, const char *title)
{
    //nk_style_set_font(ctx, &media->font_18->handle);
    nk_layout_row_dynamic(ctx, 20, 1);
    nk_label(ctx, title, NK_TEXT_CENTERED );
}

struct nk_style_item TestApp::mkColorStyleItem( const char *hexColor )
{
    struct nk_style_item result;
    int r = 0xff;
    int g = 0xff;
    int b = 0xff;
    int a = 0xff;
    if (strlen(hexColor)==7) {
        sscanf(hexColor, "#%02x%02x%02x", &r, &g, &b);
    } else if (strlen(hexColor)==9) {
        sscanf(hexColor, "#%02x%02x%02x%02x", &r, &g, &b, &a);
    }
    
    // TODO: color modifiers
    
    result.type = NK_STYLE_ITEM_COLOR;
    result.data.color.r = glm::clamp( r, 0, 255 );
    result.data.color.g = glm::clamp( g, 0, 255 );
    result.data.color.b = glm::clamp( b, 0, 255 );
    result.data.color.a = glm::clamp( a, 0, 255 );
    
    return result;
}

void
TestApp::DoGameUI( nk_context* ctx )
{
    if (this->fontValid) {
        
        nk_style_set_font(ctx, &(g_uiMedia.font_20->handle));
        
        if (gameState == GameState_TITLE) {
            DoGameUI_Title( ctx );
        } else if (gameState == GameState_GAME) {
            DoGameUI_Gameplay( ctx );
        } else if (gameState == GameState_GAME_OVER) {
            DoGameUI_Results( ctx );
        }
    }
}

void
TestApp::DoGameUI_Title( nk_context* ctx )
{
    struct nk_panel layout;
    
    struct nk_style_window_header oldHeaderStyle = ctx->style.window.header;
    
    bool gameReady = false;
    
    // Only works for 4 teams right now
    for (int i=0; i < teams.Size(); i++) {
        
        TeamInfo &team = teams[i];
        if (!strcmp(team.teamName, "Dept. of Agriculture")) {
            // Special case "Dept. of Agriculture" so it fits
            nk_style_set_font(ctx, &(g_uiMedia.font_14->handle));
        } else {
            nk_style_set_font(ctx, &(g_uiMedia.font_20->handle));
        }
        
        
        int col = (fbWidth-220) * (i / 2);
        int row = (fbHeight / 2) * (i%2);
        struct nk_style_item hc;
        hc.type = NK_STYLE_ITEM_COLOR;
        hc.data.color.r = (int)(team.teamColor.r * 255.0);
        hc.data.color.g = (int)(team.teamColor.g * 255.0);
        hc.data.color.b = (int)(team.teamColor.b * 255.0);
        hc.data.color.a = 240;
        
        ctx->style.window.header.normal = hc;
        ctx->style.window.header.hover = hc;
        ctx->style.window.header.active = hc;
        
        ctx->style.window.header.label_normal = { 255, 255, 255, 255 };
        
        
        if (nk_begin(ctx, &layout, team.teamName, nk_rect(col, row, 220, fbHeight/2),
                            NK_WINDOW_BORDER|NK_WINDOW_BORDER_HEADER|NK_WINDOW_TITLE)) {
            
        
            nk_style_set_font(ctx, &(g_uiMedia.font_20->handle));
            
            nk_layout_row_dynamic(ctx, 30, 1);
            team.playerType = nk_option_label(ctx, "None", team.playerType==Player_NONE) ? Player_NONE : team.playerType;
            nk_layout_row_dynamic(ctx, 30, 1);
            team.playerType = nk_option_label(ctx, "Human", team.playerType==Player_HUMAN) ? Player_HUMAN : team.playerType;
            nk_layout_row_dynamic(ctx, 30, 1);
            team.playerType = nk_option_label(ctx, "CPU", team.playerType==Player_CPU) ? Player_CPU : team.playerType;
            
            // if at least one player is set, we can start
            if (team.playerType != Player_NONE) {
                gameReady = true;
            }
            
            nk_end(ctx);
        }
    }
    ctx->style.window.header = oldHeaderStyle;
    
    // Title Card, About and Start Game
    
    nk_color oldWindowBG = ctx->style.window.background;
    nk_color oldWindowFixedBG = ctx->style.window.fixed_background.data.color;
    
    ctx->style.window.background = { 0, 0, 0, 0 };
    ctx->style.window.fixed_background.data.color = { 0, 0, 0, 0 };
    
    if (nk_begin(ctx, &layout, "Farmageddon", nk_rect((fbWidth/2) - 300, 0,  600, fbHeight), 0)) {
    
        
        
        nk_layout_row_dynamic( ctx, 420, 1);
        //nk_label(ctx, "Image Goes Here.", NK_TEXT_CENTERED );
        nk_image( ctx, g_uiMedia.farmageddon_title );
        
        nk_layout_row_dynamic( ctx, 20, 1);
        
        // Start Game button
        nk_layout_row_dynamic( ctx, 50, 3);
        struct nk_style_button button_style = ctx->style.button;
        if (gameReady) {
            ctx->style.button.normal = mkColorStyleItem( "#84db45" );
            ctx->style.button.hover = mkColorStyleItem( "#b0e787" );
            ctx->style.button.active = mkColorStyleItem( "#6da247" );
            ctx->style.button.border_color = mkColorStyleItem( "#f5e458" ).data.color;
            
            ctx->style.button.text_normal = mkColorStyleItem( "#631e0c" ).data.color;
            ctx->style.button.text_hover = mkColorStyleItem( "#bb8e36" ).data.color;;
            ctx->style.button.text_active = mkColorStyleItem( "#ff0000" ).data.color;;
        } else {
            ctx->style.button.hover = ctx->style.button.normal;
        }
        
        nk_spacing(ctx, 1);
        nk_style_set_font(ctx, &(g_uiMedia.font_24->handle));
        if (nk_button_label(ctx, "Start Game!")) {
            StartGame();
        }
        
        nk_style_set_font(ctx, &(g_uiMedia.font_14->handle));
        
        nk_layout_row_dynamic( ctx, 15, 1);
        nk_label(ctx, (gameReady?"":"No Players Enabled"), NK_TEXT_CENTERED );
        
        nk_layout_row_dynamic( ctx, 50, 1);
        
        nk_layout_row_dynamic( ctx, 15, 1);
        nk_label(ctx, "A game for LudumDare 38", NK_TEXT_CENTERED );
        nk_layout_row_dynamic( ctx, 15, 1);
        nk_label(ctx, "by Joel Davis / @joeld42 / joeld42@gmail.com", NK_TEXT_CENTERED );
        
        nk_layout_row_dynamic( ctx, 12, 1);
        
        nk_layout_row_dynamic( ctx, 12, 1);
        nk_label(ctx, "Special Thanks:", NK_TEXT_LEFT );
        
        nk_layout_row_dynamic( ctx, 12, 1);
        nk_label(ctx, "Oryol Engine bt Andre Weissflog (@FlohOfWoe)", NK_TEXT_CENTERED );
        
        nk_layout_row_dynamic( ctx, 12, 1);
        nk_label(ctx, "NuklearUI by Micha Mettke (@vurtun)", NK_TEXT_CENTERED );
        
        ctx->style.button = button_style;
        
        nk_end(ctx);
    }
    
    ctx->style.window.background = oldWindowBG;
    ctx->style.window.fixed_background.data.color = oldWindowFixedBG;
}

void
TestApp::DoGameUI_Results( nk_context* ctx )
{
}


void
TestApp::DoGameUI_Gameplay( nk_context* ctx )
{
    struct nk_panel layout;
    if (nk_begin(ctx, &layout, "Player 1", nk_rect(0, 0, 220, fbHeight),
                NK_WINDOW_BORDER|NK_WINDOW_BORDER_HEADER|NK_WINDOW_TITLE)) {
        
        ui_header(ctx,  "Henry");
        
        Cannon &cc = cannons[activeCannon];
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Aim", NK_TEXT_LEFT );
        nk_slider_float(ctx, -180.0f, &(cc.aimHeading), 180.0f, 0.1f);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Tilt", NK_TEXT_LEFT );
        nk_slider_float(ctx, 0.0f, &(cc.cannonAngle), 180.0f, 0.1f);
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Power", NK_TEXT_LEFT );
        nk_slider_float(ctx, 0.0f, &(cc.power), 1.0f, 0.001f);
        
        nk_layout_row_dynamic(ctx, 30, 1);
        
        bool canFire = shots.Size() == 0;
        struct nk_style_button button_style = ctx->style.button;
        if (canFire) {
            ctx->style.button.normal = mkColorStyleItem( "#84db45" );
            ctx->style.button.hover = mkColorStyleItem( "#b0e787" );
            ctx->style.button.active = mkColorStyleItem( "#6da247" );
            ctx->style.button.border_color = mkColorStyleItem( "#f5e458" ).data.color;
            
            ctx->style.button.text_normal = mkColorStyleItem( "#631e0c" ).data.color;
            ctx->style.button.text_hover = mkColorStyleItem( "#bb8e36" ).data.color;;
            ctx->style.button.text_active = mkColorStyleItem( "#ff0000" ).data.color;;
        } else {
            ctx->style.button.hover = ctx->style.button.normal;
        }
        
        nk_style_set_font(ctx, &(g_uiMedia.font_24->handle));
        if (nk_button_label(ctx, "Fire!") && canFire) {
            fireActiveCannon();
        }
        nk_style_set_font(ctx, &(g_uiMedia.font_18->handle));
        
        ctx->style.button = button_style;
        
        
        nk_end(ctx);
    }
    
    if (nk_begin(ctx, &layout, "Weapons", nk_rect(fbWidth-220, 0, 220, fbHeight),
                 NK_WINDOW_BORDER|NK_WINDOW_BORDER_HEADER|NK_WINDOW_TITLE)) {
        
//        if (nk_group_begin(ctx, &layout, "Group",
//                           NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
        
        const char *weaponNames[] = {
            "Pea Shooter",
            "Pumpkin Eater",
            "Grapeshot",
            "Emoji Eggplant",
            "Guided Carrot",
            "Fertilizer"
        };
        
        char number[2] = "1";
        for (int i = 0; i < sizeof(weaponNames)/sizeof(weaponNames[0]); i++)
        {
            nk_layout_row_dynamic( ctx, 44.0, 2 );
            number[0] = '1'+i;
                nk_button_label( ctx, number );
                nk_label( ctx, weaponNames[i], NK_TEXT_RIGHT );
            nk_layout_row_end(ctx);
        }
//
//            nk_group_end(ctx);
//        }
        
        
        
        nk_end(ctx);
    }
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


