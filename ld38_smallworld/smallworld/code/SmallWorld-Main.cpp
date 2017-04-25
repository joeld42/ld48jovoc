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

// for bundle path
#include <CoreFoundation/CoreFoundation.h>

using namespace Oryol;

#define CHEATS_ENABLED (1)

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
    struct nk_image farmageddon_gameover;
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
    void beginTurn();
    void finishTurn();
    void fireActiveCannon();
    void selectAmmo( int ammoIndex );
    
    void fireCannonDown();
    void fireCannonUp();
    bool fireCharging;
    float chargingRawPower;
    
    void lookAtCannon( Cannon &cc );
    void lookAtPos( glm::vec3 pos, bool update );
    void SpawnCannons();
    void StartGame();
    void RestartGame();
    float nextTurnTimer;
    bool isFiring;
    bool quitRequested;
    
    Oryol::Array<TeamInfo> teams;
    Oryol::Array<AmmoInfo> ammos;
    int selectedAmmo;
    
    // For Results screen
    int playerCount;
    TeamInfo *winningTeam;
    Cannon *winningCannon;
    
    SceneObject *boom;
    glm::vec3 boomPos;
    float boomTimer;
    int planetNeedsRebuild;
    
    struct nk_style_item mkColorStyleItem( const char *hexColor );
    
    void DoGameUI( nk_context* ctx );
    void DoGameUI_Title( nk_context* ctx );
    void DoGameUI_Gameplay( nk_context* ctx );
    void DoGameUI_Results( nk_context* ctx );

    bool fontValid = false;
    Buffer ttfData;

    // stuff for AI player..
    float aiTargAngle;
    float aiTargHeading;
    float aiChargeTime;
    bool aiFinished;

    GameState gameState;
    GameState nextGameState; // don't change gamestate mid-frame
    
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
    Oryol::Duration frameDuration = Clock::LapTime(tp);
    float dt = frameDuration.AsSeconds();
    
    // safety check
    bool safeToUpdate = true;
    if ((!scene->didSetupPipeline) || (dt > 0.5f)) {
        safeToUpdate = false;
    }
    
    if (safeToUpdate) {
        this->handle_input( dt );
    }
    

    
    
    if ((!paused) && (safeToUpdate))
    {
        
        // Update boom
        if (boom->enabled) {
            boom->scale = glm::vec3( boom->scale.x + glm::gaussRand(0.0, 1.0) );
            
            boom->pos = boomPos + glm::ballRand( 100.0f );
            
            boomTimer -= dt;
            if (boomTimer <= 0.0) {
                boom->enabled = false;
            }
        }
        
        
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
                    //glm::vec3 upDir2 = glm::normalize( evalP );
                    //printf("upDir %3.2f %3.2f %3.2f -- dt %3.2f\n", upDir.x, upDir.y, upDir.z, dt );
                    
                    glm::vec3 newPos = cc.objBase->pos + (upDir*-100.0f*dt);
                    //glm::vec3 newPos = cc.objBase->pos;
                    cc.place( newPos, upDir );
                }
            }
            
            // Update color and status
            cc.update( dt, animT, (i == activeCannon) && (!isFiring) );
        }
#endif
        
        // Update shots
        bool shouldRebuild = false;
        for (int i=0; i < shots.Size(); i++) {
            Shot &ss = shots[i];
            
            // track the first pos
            if (i==0) {
                lookAtPos( ss.objShot->pos, true );
            }
            
            glm::vec3 evalP = ss.objShot->pos / planet.worldSize;
            
            glm::vec3 grav = glm::normalize( evalP );
            if (ss.ammo->wackyGravity) {
                
                // Use distance field for gravity for weird physics
                grav = planet.surfBuilder.evalNormal( evalP );
            }
            
            
            // Less gravity as you get near the center
            float gravStr = glm::smoothstep( 0.0f, planet.planetApproxRadius,
                                             glm::length(ss.objShot->pos) );
            
            grav *= -5000.0 * gravStr;
            
            ss.updateBallistic( dt, grav );
            evalP = ss.objShot->pos / planet.worldSize;
            
            // check if we hit..
            float f = planet.surfBuilder.evalSDF( evalP );
            if ((f <= 0.0f)||(ss.age >= ss.ammo->boomAge)) {
                
                shouldRebuild = true;
                printf("Hit Planet (%3.2f %3.2f %3.2f)...\n",
                       evalP.x, evalP.y, evalP.z );
                
                scene->removeObject( ss.objShot );
                if (ss.ammo->isDirt) {
                    planet.surfBuilder.addDirt( evalP, ss.ammo->damageRadius/planet.worldSize.x );
                } else {
                    planet.surfBuilder.addDamage( evalP, ss.ammo->damageRadius/planet.worldSize.x );
                    
                    // Make boom at the location
                    float boomScale = ss.ammo->damageRadius / 100.0;
                    boom->scale = glm::vec3( boomScale );
                    
                    boomPos = ss.objShot->pos;
                    boom->pos = boomPos; // this one gets jittered
                    boom->enabled = true;
                    boomTimer = 0.2;
                    
                    // Check if any cannons are within the fatal range
                    for (int j=0; j < cannons.Size(); j++) {
                        Cannon &cc = cannons[j];
                        float hitDist = glm::length(cc.objBase->pos - ss.objShot->pos);
                        
                        //printf("Hit Distance to '%s' %3.2f\n", cc.name, hitDist );
                        int origHealth = cc.health;
                        if ( hitDist < ss.ammo->fatalRadius) {
                            cc.health = 0;
                        } else if (hitDist < ss.ammo->splashRadius) {
                            float splash = glm::smoothstep( ss.ammo->fatalRadius, ss.ammo->splashRadius, hitDist );
                            int dam = int( glm::round(1.0 + splash*3 ) );
                            printf("SPLASH Damage: %d\n", dam );
                            cc.health = glm::max( 0, cc.health - dam );
                        }
                        
                        if (cc.health < origHealth) {
                            cc.showDamageTimer = 0.5;
                        }
                    }
                }
                
                // FIXME: sloppy and might break stuff, removing while iterating
                shots.EraseSwapBack( i );
            }
            
            // Split?
            if ((!ss.splitDone) && (ss.age > ss.ammo->splitAge)) {
                
                printf("SPLIT!!");
                ss.splitDone = true;
                
                // Split!
                for (int i=0; i < ss.ammo->splitNum; i++) {
                    
                    SceneObject *objShot = scene->addObject( ss.ammo->meshName, ss.ammo->textureName );
                    
                    Shot shot( objShot, ss.ammo, ss.objShot->pos, ss.vel );
                    shot.vel += glm::ballRand( 1500.0 );
                    shot.splitDone = true;
                    shot.age = ss.age + glm::linearRand( 0.0f, 0.1f );
                    shots.Add( shot );
                }
            }
            
        }
        
        if (shouldRebuild) {
            if (planetNeedsRebuild==0) {
                
                // Frames until rebuild. Give a few frames in case other shots hit nearby, and so
                // we can draw the explosion a few times
                planetNeedsRebuild = 5;
            }
            //planet.Rebuild( scene );
        }
        
        
        // Is this the end of the turn
        if (isFiring && (shots.Size() == 0) && (nextTurnTimer < 0.0)) {
            // Finish turn
            nextTurnTimer = 1.5;
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
        
        
        glm::mat4 modelScale = glm::scale(glm::mat4(), obj->scale );
        glm::mat4 modelTform = glm::translate(glm::mat4(), obj->pos);
        modelTform = modelTform * glm::mat4_cast( obj->rot ) * modelScale;

        
        
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
    
    ctx->style.window.fixed_background.data.color.a = 200;
    ctx->style.button.normal.data.color.a = 200;
    
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
    if (fontValid) {
        DoGameUI( ctx );
        NKUI::Draw();
    }
#endif
    
    Gfx::EndPass();
    
    Gfx::CommitFrame();
    
    gameState = nextGameState;
    
    // continue running or quit?
    return (Gfx::QuitRequested() || quitRequested) ? AppState::Cleanup : AppState::Running;
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
    nextGameState = gameState;
    nextTurnTimer = -1.0;
    
    
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("CWD is: %s\n", cwd );
    
    // Dev hack to ensure we're running from the right place...
#if 0
    chdir("/Users/joeld/Projects/ld48jovoc/ld38_smallworld/smallworld");
    FILE *fp = fopen( "./gamedata/tree_062.omsh", "rt");
    if (!fp) {
        printf("Error! Not running from the right path...??\n");
        exit(1);
    }
    fclose(fp);
#endif
    
    CFBundleRef mainBundle;
    
    // Get the main bundle for the app
    mainBundle = CFBundleGetMainBundle();
    
    
    CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
    CFStringRef pathStr = CFURLGetString ( mainBundleURL );
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
    const char *path = CFStringGetCStringPtr(pathStr, encodingMethod);
    
    printf("PATH is : %s\n", path);
    
    useDebugCamera = false;
    animT = 0.0f;
    
    char resPath[1024];
    sprintf( resPath, "%sContents/Resources/", path+7 );
    
    printf("RESPATH is : %s\n", resPath);
    chdir(resPath);
    
    // set up IO system
    IOSetup ioSetup;
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
    ioSetup.Assigns.Add("msh:", "cwd:gamedata/");
    ioSetup.Assigns.Add("tex:", "cwd:gamedata/");
    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
//        ioSetup.Assigns.Add("msh:", resPath );
//        ioSetup.Assigns.Add("tex:", resPath );
//        ioSetup.Assigns.Add("data:", resPath );
    
    
    
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
    boom = scene->addObject( "msh:boom.omsh", "tex:explode.dds");
    //boom->pos = glm::vec3(500.0, 0.0, 0.0 );
    //boom->scale = glm::vec3( 12.0 );
    boom->enabled = false;
    
    // Setup planet
    planet.planetTexture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "tex:ground1.dds",
                                                                                          scene->texSetup)));
    planet.Setup( &(scene->gfxSetup), noiseCtx );
    
    // Setup UI
    NKUI::Setup();

    Memory::Clear(&g_uiMedia, sizeof(g_uiMedia));

    
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
    load_icon( "data:farmageddon_gameover.png", &(g_uiMedia.farmageddon_gameover));
    
    // Initialize teams and weapons
    MakeDefaultTeams( teams );
    MakeDefaultAmmos( ammos );
    
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
    if (noiseCtx) {
        open_simplex_noise_free(noiseCtx);
    }
    Oryol::TimePoint tp = Clock::Now();
    srand( tp.getRaw() );
    open_simplex_noise( tp.getRaw(), &noiseCtx );
    planet._noiseCtx = noiseCtx;
    planet.surfBuilder.noiseCtx = noiseCtx;

    
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
    // Init stuff
    nextTurnTimer = -1.0;
    isFiring = false;
    
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

    assert( numActiveTeams > 0 );
    
    int numCannonsPerTeam = 7 - numActiveTeams;
    int numTotalCannons = numActiveTeams * numCannonsPerTeam;
    
    // Add cannons
    while (cannons.Size() < numTotalCannons) {
        
        glm::vec3 p = glm::sphericalRand( planet.planetApproxRadius  );
        glm::vec3 upDir = planet.surfBuilder.evalNormal( p );
        Cannon cc( scene, &(teams[assignIndex]), p, upDir );
        
        cannons.Add(cc);
        
        do {
            assignIndex++;
            if (assignIndex >= teams.Size()) {
                assignIndex = 0;
            }
        } while (teams[assignIndex].playerType == Player_NONE);
    }
    
    activeCannon = 0;
    lookAtCannon( cannons[activeCannon] );
    
    beginTurn();    
}

void
TestApp::StartGame()
{
    nextGameState = GameState_GAME;
    selectedAmmo = 0;
    SpawnCannons();
    
    for (int i=0; i < teams.Size(); i++) {
        teams[i].selectedWeapon = 0;
        
        for (int j=0; j < ammos.Size(); j++) {
            teams[i].ammoSupply[j] = ammos[j].defaultSupply;
        }
    }
    
}

void
TestApp::RestartGame()
{
    for (int i=0; i < cannons.Size(); i++) {
        scene->removeObject( cannons[i].objBase );
        scene->removeObject( cannons[i].objBarrel );
        scene->removeObject( cannons[i].objBushing );
    }
    cannons.Clear();
    BuildPlanet();
    nextGameState = GameState_TITLE;
}

void
TestApp::beginTurn()
{
    Cannon &cc = cannons[activeCannon];
    
    if (cc.team->playerType == Player_CPU) {
        // pick a random angle and stuff
        aiTargHeading = glm::linearRand( -180.0f, 180.0f );
        aiTargAngle = glm::linearRand( 0.0f, 180.0f );
        aiChargeTime = glm::linearRand( 0.5f, 3.0f );
        aiFinished = false;
        
        // Pick a random weapon
        int availWeapons[20];
        availWeapons[0] = 0;
        int nAvailWeapons = 1;
        for (int i=1; i < ammos.Size(); i++) {
            if ( (ammos[i].defaultSupply != -1) && (cc.team->ammoSupply[i]) ) {
                availWeapons[nAvailWeapons++] = i;
            }
        }
        
        selectAmmo( availWeapons[ rand() % nAvailWeapons ]);
    }
    
}

void
TestApp::finishTurn()
{
    
    isFiring = false;
    
    // check if there's only one team with cannons remaining
    // FIXME: handle single player mode
    int teamsAlive = 0;
    int cannonsAlive = 0;
    for (int tndx=0; tndx < teams.Size(); tndx++) {
        
        for (int i=0; i < cannons.Size(); i++ ) {
            if ((cannons[i].team == &(teams[tndx])) && (cannons[i].health > 0)) {
                // found a cannon on this team
                teamsAlive++;
                cannonsAlive++;
                winningCannon = &(cannons[i]);
                winningTeam = winningCannon->team;
                if (playerCount > 1) {
                    // don't need to count the cannons in multiplayer games
                    break;
                }
            }
        }
    }
    
    printf("FINISH TURN: PlayerCount %d -- teams Alive: %d cannons Alive %d\n",
           playerCount, teamsAlive, cannonsAlive );
    
    if ( ((playerCount==1) && (cannonsAlive==1)) ||
        ((playerCount>1) && (teamsAlive==1)) ) {
    
        nextGameState = GameState_GAME_OVER;
    }
    
    do {
        
        activeCannon++;
        if (activeCannon == cannons.Size()) {
            activeCannon = 0;
        }
        
    } while (cannons[activeCannon].health == 0);
    
    
    // Move camera to active cannon
    lookAtCannon( cannons[activeCannon] );
    
    // restore ammo selection for team
    selectAmmo( cannons[activeCannon].team->selectedWeapon );
    
    if (nextGameState == GameState_GAME) {
        beginTurn();
    }
}

void TestApp::selectAmmo( int ammoIndex )
{
    Cannon &cc = cannons[activeCannon];
    cc.team->selectedWeapon = ammoIndex;
    selectedAmmo = ammoIndex;
}

void
TestApp::fireActiveCannon()
{
    Cannon &cc = cannons[activeCannon];
    // Make sure there's not already a shot in-flight...
    if (!isFiring)
    {
        
        AmmoInfo &ammo = ammos[selectedAmmo];
        SceneObject *objShot = scene->addObject( ammo.meshName, ammo.textureName );
        Shot shot( objShot, &ammo, cc._shootyPoint, cc.calcProjectileVel() );
        shots.Add( shot );
        
        if (ammo.defaultSupply != -1) {
            cc.team->ammoSupply[ selectedAmmo ]--;
        }
        
        cc.power = 0.0;
        isFiring = true;
    }
}

void TestApp::fireCannonDown()
{
    if (!isFiring) {
        fireCharging = true;
        Cannon &cc = cannons[activeCannon];
        cc.power = 0.0;
        chargingRawPower = 0.0;
    }
}

void TestApp::fireCannonUp()
{
    if (fireCharging) {
        
        fireCharging = false;
        // BLAM!
        fireActiveCannon();
    }
}


void
TestApp::lookAtCannon( Cannon &cc )
{
    lookAtPos( cc.objBase->pos, false );
}

void
TestApp::lookAtPos( glm::vec3 pos, bool update )
{
    
    glm::vec3 front(0.0f,0.0f,1.0f);
    glm::vec3 cannonDir = glm::normalize( pos );
    
    animCamTarg = glm::quat( front, cannonDir );
    
    if ((!update) || (!doAnimCam)) {
        animCamStart = gameCamera.Rotq;
        animCamTimer = 0.0;
    }
    doAnimCam = true;
}

//------------------------------------------------------------------------------
void
TestApp::handle_input( float dt )
{
    
    // Update active
    animT += dt;
    
    // Charging to fire??
    if (fireCharging) {
        chargingRawPower += dt;
        if (activeCannon < cannons.Size()) {
            Cannon &cc = cannons[activeCannon];
            cc.power = fabs( fmodf(1.0f + chargingRawPower, 2.0) - 1.0f);
        }
    }
    
    // Rebuild planet??
    if (planetNeedsRebuild > 0) {
        planetNeedsRebuild--;
        if (planetNeedsRebuild==0) {
            planet.Rebuild( scene );
        }
    }
    
    // turn timer
    if (nextTurnTimer >= 0.0) {
        nextTurnTimer -= dt;
        if (nextTurnTimer < 0.0 ) {
            finishTurn();
            nextTurnTimer = -1.0;
        }
    }
    
    
    if ((activeCannon < cannons.Size()) && (cannons[activeCannon].team->ammoSupply[selectedAmmo]==0)) {
        // Out of this ammo, switch to pea shooter
        selectAmmo(0);
    }
    
    // Handle AI
    if (activeCannon < cannons.Size()) {
        Cannon &cc = cannons[activeCannon];
        if ((cc.team->playerType == Player_CPU) && (!aiFinished)) {
            float turnSpeed = 40.0;
            float diffHead = aiTargHeading - cc.aimHeading;
            
            float turnAmount = glm::sign(diffHead) * turnSpeed * dt;
            if (fabs(turnAmount) > fabs(diffHead)) {
                turnAmount = diffHead;
            }
            
            
            cc.aimHeading = cc.aimHeading + turnAmount;
            
            float nearVal = fabsf(cc.aimHeading - aiTargHeading);
            if (nearVal< 1.0f) {

                // heading is aimed...
                float diffTilt = aiTargAngle - cc.cannonAngle;
                
                float tiltAmount = glm::sign(diffTilt) * turnSpeed * dt;
                if (fabs(tiltAmount) > fabs(diffTilt)) {
                    tiltAmount = diffTilt;
                }
                
                cc.cannonAngle += tiltAmount;
                if (fabsf(cc.cannonAngle - aiTargAngle) < 1.0) {
                    // tilt is aimed..
                    //printf("Ready to fire...\n");
                    if (!fireCharging) {
                        fireCannonDown();
                    } else {
                        aiChargeTime -= dt;
                        if (aiChargeTime < 0.0f) {
                            fireCannonUp();
                            aiFinished = true;
                        }
                    }
                }
//                else {
//                    printf( "diffTilt %f cc.cannonAngle %f aiTargAngle %f tiltAmount %f\n",
//                           diffTilt, cc.cannonAngle, aiTargAngle, tiltAmount );
//                }
            }
//            else {
//                printf( "nearVal %f diffHead %f cc.aimHeading %f aiTargHeading %f float turnAmount %f\n",
//                       nearVal, diffHead, cc.aimHeading, aiTargHeading, turnAmount );
//            }
        }
        
        cc.updatePlacement();
    }

    
    glm::vec3 move;
    glm::vec2 rot;
    float vel = 3.5f;
    float angSpeed = 1.0f;
    
    float powSpeed = 0.01f;
    
    if (Input::KeyboardAttached() ) {
        
        if (Input::KeyPressed( Key::LeftShift)) {
            vel *= 10.0;
            angSpeed *= 10.0f;
            powSpeed *= 2.0f;
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
            
            // Heading
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
            
            // Tilt
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
            
            if (Input::KeyPressed(Key::E) || Input::KeyPressed(Key::Period)) {
                cc.power += powSpeed;
                if (cc.power > 1.0) {
                    cc.power = 1.0;
                }
            }
            if (Input::KeyPressed(Key::Q) || Input::KeyPressed(Key::Comma)) {
                cc.power -= powSpeed;
                if (cc.power < 0.0) {
                    cc.power = 0.0;
                }
            }
            
            
            cc.updatePlacement();
            
            

            // FIRE?
            if (cc.team->playerType == Player_HUMAN) {
                if (Input::KeyDown(Key::Space) || Input::KeyDown(Key::Z) || Input::KeyDown(Key::X)) {
                    fireCannonDown();
                }
                
                if (Input::KeyUp(Key::Space) || Input::KeyUp(Key::Z) || Input::KeyUp(Key::X)) {
                    fireCannonUp();
                }
            }
            
            
#if CHEATS_ENABLED
            // DBG: kill current cannon
            if (Input::KeyDown( Key::K )) {
                nextTurnTimer = 1.5;
                cc.showDamageTimer = 0.5;
                cc.health = 0;
            }
            
            // DBG: end game
            if (Input::KeyDown( Key::J )) {
                winningCannon = &(cannons[activeCannon]);
                winningTeam = winningCannon->team;

                nextGameState = GameState_GAME_OVER;
            }
            
            // DBG
            if (Input::KeyDown(Key::N)) {
                finishTurn();
            }
            
            if (Input::KeyDown(Key::B)) {
                printf("Build ...\n");
                if (scene->didSetupPipeline) {
                    BuildPlanet();
                }
            }
#endif
        }
        
        
        
        if (Input::KeyDown(Key::GraveAccent)) {
            tmpUI = !tmpUI;
        }
        
        if (Input::KeyDown(Key::Tab)) {
            useDebugCamera = !useDebugCamera;
        }
    }
    
    // Not sure how to filter out events when NKUI handles them so this workaround
    bool haveMouse = true;
    glm::vec2 mousePos = Input::MousePosition();
    if ((mousePos.x < 220) || (mousePos.y > fbWidth-220)) {
        haveMouse = false;
    }
    
    // For mouse button-firing
    if ((fireCharging) && Input::MouseButtonUp( MouseButton::Left )) {
        fireCannonUp();
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
                
                if ((haveMouse) && (Input::MouseButtonPressed(MouseButton::Left)) ) {
                    
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
void UIApplyTeamColorToHeader( nk_context *ctx, TeamInfo *team )
{
    struct nk_style_item hc;
    hc.type = NK_STYLE_ITEM_COLOR;
    hc.data.color.r = (int)(team->teamColor.r * 255.0);
    hc.data.color.g = (int)(team->teamColor.g * 255.0);
    hc.data.color.b = (int)(team->teamColor.b * 255.0);
    hc.data.color.a = 240;
    
    ctx->style.window.header.normal = hc;
    ctx->style.window.header.hover = hc;
    ctx->style.window.header.active = hc;
    
    ctx->style.window.header.label_normal = { 255, 255, 255, 255 };
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
    playerCount = 0;
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
//        struct nk_style_item hc;
//        hc.type = NK_STYLE_ITEM_COLOR;
//        hc.data.color.r = (int)(team.teamColor.r * 255.0);
//        hc.data.color.g = (int)(team.teamColor.g * 255.0);
//        hc.data.color.b = (int)(team.teamColor.b * 255.0);
//        hc.data.color.a = 240;
//        
//        ctx->style.window.header.normal = hc;
//        ctx->style.window.header.hover = hc;
//        ctx->style.window.header.active = hc;
//        
//        ctx->style.window.header.label_normal = { 255, 255, 255, 255 };
        UIApplyTeamColorToHeader( ctx, &team );
        
        
        // This is a workaround for a "feature" of nkui, it stores panel sizes (for resizing) so
        // we can't use the same name as the in-game team panel, this avoids the problem... :_(
        char buff[45];
        sprintf(buff, " %s", team.teamName );
        if (nk_begin(ctx, &layout, buff, nk_rect(col, row, 220, fbHeight/2),
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
                playerCount++;
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
        
        // HACK: there's some crash bug that seems to be related to loading UI
        // fonts or resources, it seems to happen less if you wait to start the
        // game so delay the game start a few seconds
//        if (animT < 3.0) {
//            gameReady = false;
//        }
        
        
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
        
        //const char *notReadyMessage = (animT < 3.0)?"Get Ready...":"No Players Enabled";
        const char *notReadyMessage = "No Players Enabled";
        nk_label(ctx, (gameReady?"":notReadyMessage), NK_TEXT_CENTERED );
        
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
    struct nk_panel layout;
    struct nk_style_window_header oldHeaderStyle = ctx->style.window.header;
    
    // Game Over and winnner
    nk_color oldWindowBG = ctx->style.window.background;
    nk_color oldWindowFixedBG = ctx->style.window.fixed_background.data.color;
    
    ctx->style.window.background = { 0, 0, 0, 0 };
    ctx->style.window.fixed_background.data.color = { 0, 0, 0, 0 };
    
    if (nk_begin(ctx, &layout, "GameOver", nk_rect((fbWidth/2) - 300, 0,  600, fbHeight), 0)) {
        
        nk_layout_row_dynamic( ctx, 420, 1);
        //nk_label(ctx, "Image Goes Here.", NK_TEXT_CENTERED );
        nk_image( ctx, g_uiMedia.farmageddon_gameover );
        
        nk_layout_row_dynamic( ctx, 20, 1);
        nk_label( ctx, "Winner:", NK_TEXT_CENTERED );
        
        nk_style_set_font(ctx, &(g_uiMedia.font_24->handle));
        nk_layout_row_dynamic( ctx, 30, 1);

        glm::vec4 tc = winningTeam->teamColor;
        nk_label_colored(ctx, (playerCount==1)?winningCannon->name:winningTeam->teamName,
                         NK_TEXT_CENTERED, nk_rgb(int(tc.r*255.0),int(tc.g*255.0),int(tc.b*255.0) ));

        
        nk_layout_row_dynamic( ctx, 20, 1);
        
        
        // Start Game button
        nk_layout_row_dynamic( ctx, 50, 3);
        struct nk_style_button button_style = ctx->style.button;
        ctx->style.button.normal = mkColorStyleItem( "#84db45" );
        ctx->style.button.hover = mkColorStyleItem( "#b0e787" );
        ctx->style.button.active = mkColorStyleItem( "#6da247" );
        ctx->style.button.border_color = mkColorStyleItem( "#f5e458" ).data.color;
        ctx->style.button.text_normal = mkColorStyleItem( "#631e0c" ).data.color;
        ctx->style.button.text_hover = mkColorStyleItem( "#bb8e36" ).data.color;;
        ctx->style.button.text_active = mkColorStyleItem( "#ff0000" ).data.color;;
        
        nk_spacing(ctx, 1);
        if (nk_button_label(ctx, "Play Again!")) {
            RestartGame();
        }
        
        nk_layout_row_dynamic( ctx, 50, 3);
        nk_spacing(ctx, 1);
        nk_style_set_font(ctx, &(g_uiMedia.font_18->handle));
        ctx->style.button = button_style;
        
        if (nk_button_label(ctx, "quit")) {
            quitRequested = true;
        }
        
        nk_end(ctx);
    }
    
    ctx->style.window.background = oldWindowBG;
    ctx->style.window.fixed_background.data.color = oldWindowFixedBG;
}


void
TestApp::DoGameUI_Gameplay( nk_context* ctx )
{
    Cannon &cc = cannons[activeCannon];
    
    struct nk_panel layout;
    
    UIApplyTeamColorToHeader( ctx, cc.team );
    
    if (!strcmp(cc.team->teamName, "Dept. of Agriculture")) {
        // Special case "Dept. of Agriculture" so it fits
        nk_style_set_font(ctx, &(g_uiMedia.font_14->handle));
    } else {
        nk_style_set_font(ctx, &(g_uiMedia.font_20->handle));
    }

    struct nk_style_button button_style = ctx->style.button;
    
    
    if (nk_begin(ctx, &layout, cc.team->teamName, nk_rect(0, 0, 220, fbHeight),
                NK_WINDOW_BORDER|NK_WINDOW_BORDER_HEADER|NK_WINDOW_TITLE)) {
        
        nk_style_set_font(ctx, &(g_uiMedia.font_24->handle));
        nk_layout_row_dynamic(ctx, 24, 1);
        
        glm::vec4 tc = cc.team->teamColor;
        nk_label_colored(ctx, cc.name, NK_TEXT_CENTERED, nk_rgb(int(tc.r*255.0),int(tc.g*255.0),int(tc.b*255.0) ));
        
        nk_style_set_font(ctx, &(g_uiMedia.font_14->handle));
        
        nk_layout_row_dynamic(ctx, 20, 2);
        nk_label_colored( ctx, "Health:", NK_TEXT_ALIGN_RIGHT, nk_rgb(int(0.98*255),int(0.51*255),int(255*0.52)) );
        
        char buff[10];
        buff[7]=0;
        for (int z=0; z < 6; z++) {
            buff[z] = (z > cc.health)?'-':'*';
        }
        // TODO: replace with heart image
        nk_label_colored( ctx, buff, NK_TEXT_ALIGN_RIGHT, nk_rgb(int(0.98*255),int(0.51*255),int(255*0.52)) );
        
        nk_layout_row_dynamic(ctx, 20, 1);
        if (nk_button_label(ctx, "Locate")) {
            lookAtCannon(cc);
        }
        
        nk_style_set_font(ctx, &(g_uiMedia.font_18->handle));
        ctx->style.progress.cursor_normal = mkColorStyleItem( "#dfb801" );
        ctx->style.progress.cursor_hover = mkColorStyleItem( "#ceaa01" );
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Aim", NK_TEXT_LEFT );
        static nk_size progAim;
        progAim = nk_size(cc.aimHeading + 180.0);
        if (nk_progress(ctx, &progAim, 360, true )) {
            cc.aimHeading = float(progAim) - 180.0;
        }
        
        nk_layout_row_dynamic(ctx, 20, 1);
        static nk_size progTilt;
        nk_label(ctx, "Tilt", NK_TEXT_LEFT );
        progTilt = nk_size( cc.cannonAngle );
        if (nk_progress(ctx, &progTilt, 180, true )) {
            cc.cannonAngle = progTilt;
        }
        
        nk_layout_row_dynamic(ctx, 20, 1);
        static nk_size progPower;
        nk_label(ctx, "Power", NK_TEXT_LEFT );
        progPower = nk_size( cc.power * 1000.0 );
        //nk_slider_float(ctx, 0.0f, &(cc.power), 1.0f, 0.001f);
        nk_progress(ctx, &progPower, 1000, false );
        
        nk_layout_row_dynamic(ctx, 30, 1);
        
        bool canFire = ( (shots.Size() == 0) &&
                        (!isFiring) &&
                        (cc.team->playerType == Player_HUMAN) );
    
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
            
            // Begin firing sequence
            fireCannonDown();
        }
        
        nk_style_set_font(ctx, &(g_uiMedia.font_14->handle));
        
        ctx->style.button = button_style;
        
        nk_layout_row_dynamic(ctx, 14, 1);
        nk_label(ctx, "Drag AIM and TILT", NK_TEXT_ALIGN_LEFT  );
        
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Hold FIRE to shoot.", NK_TEXT_ALIGN_LEFT  );
        
        nk_layout_row_dynamic(ctx, 14, 1);
        nk_label(ctx, "WASD - Aim & Tilt", NK_TEXT_ALIGN_LEFT );
        nk_layout_row_dynamic(ctx, 14, 1);
        nk_label(ctx, "SPACE,Z - Hold to shoot", NK_TEXT_ALIGN_LEFT );
        
        nk_style_set_font(ctx, &(g_uiMedia.font_18->handle));
        
        nk_end(ctx);
    }
    
    
    // ----- Weapons panel
    
    if (nk_begin(ctx, &layout, "Weapons", nk_rect(fbWidth-220, 0, 220, fbHeight),
                 NK_WINDOW_BORDER|NK_WINDOW_BORDER_HEADER|NK_WINDOW_TITLE)) {
    
        nk_style_set_font(ctx, &(g_uiMedia.font_14->handle));
        
//        if (nk_group_begin(ctx, &layout, "Group",
//                           NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {

        
        
        char buttonLabel[30];
        for (int i = 0; i < ammos.Size(); i++)
        {
            nk_layout_row_dynamic( ctx, 44.0, 1 );
            
            
            //nk_button_label( ctx, number );
            //nk_label( ctx, weaponNames[i], NK_TEXT_RIGHT );
            AmmoInfo &ammo = ammos[i];
            if (ammo.defaultSupply == -1) {
                strcpy( buttonLabel, ammo.name );
            } else {
                sprintf( buttonLabel, "%s (%d)", ammo.name, cc.team->ammoSupply[i] );
            }
            if (i==selectedAmmo) {

                ctx->style.button.normal = mkColorStyleItem( "#dfb801" );
                ctx->style.button.hover = mkColorStyleItem( "#ceaa01" );
                
                ctx->style.button.text_normal = mkColorStyleItem( "#2d2d2d" ).data.color;
                ctx->style.button.text_hover = ctx->style.button.text_normal;
                
            }
            if (nk_button_symbol_label(ctx,
                                   (i==selectedAmmo)?NK_SYMBOL_CIRCLE:NK_SYMBOL_CIRCLE_FILLED,
                                       buttonLabel, NK_TEXT_RIGHT) ) {
                if (cc.team->playerType == Player_HUMAN) {
                    selectAmmo( i );
                }
            }
            
            if (i==selectedAmmo) {
                ctx->style.button = button_style;
            }
            
            
            nk_layout_row_end(ctx);
        }
//
//            nk_group_end(ctx);
//        }
        
        
        
        nk_end(ctx);
    }
    
    
    // Cannons panel
    struct nk_style_window oldWindowStyle = ctx->style.window;
    struct nk_style_button oldButtonStyle = ctx->style.button;
    
    ctx->style.window.fixed_background.data.color = nk_rgba(0,0,0,0);
    ctx->style.window.footer_padding = nk_vec2(0, 0);
    ctx->style.window.padding = nk_vec2(0, 0);
    ctx->style.window.group_padding = nk_vec2(0, 0);
    
    ctx->style.button.padding  = nk_vec2(0,0 );
    ctx->style.button.rounding = 0.0;
    
    if (nk_begin(ctx, &layout, "Cannons", nk_rect(224, 0, fbWidth - 444, 110.0), 0)) {
        
        int cols = cannons.Size();
        int rows = 1;
        if ( cols > 6 ) {
            cols /= 2;
            rows += 1;
        }
        
        int ndx=0;

        struct nk_style_item activeBorder = mkColorStyleItem( "#dfb801" );
        
        nk_color blankColor = ctx->style.progress.normal.data.color;
        nk_color healthColor = nk_rgb(int(0.98*255),int(0.51*255),int(255*0.52));
        ctx->style.progress.padding = {0, 0};
        
        for (int j=0; j < rows; j++) {
            
            nk_layout_row_dynamic( ctx, 5, cols );
            nk_layout_row_end(ctx);
            
            int ndx2 = ndx;
            for (int i=0; i < cols; i++) {
                if (ndx2 < cannons.Size()) {
                    Cannon &cc = cannons[ndx2];
                    //nk_progress( ctx, &cc.health, 6, false );
                    if (cc.health > 0) {
                        ctx->style.progress.cursor_normal.data.color = healthColor;
                        ctx->style.progress.cursor_hover.data.color = healthColor;
                    } else {
                        ctx->style.progress.cursor_normal.data.color = blankColor;
                        ctx->style.progress.cursor_hover.data.color = blankColor;

                    }
                    nk_size sz = nk_size(cc.health);
                    nk_progress(ctx, &sz, nk_size(6), true );

                }
                ndx2++;
            }
            
            
            nk_layout_row_dynamic( ctx, 35, cols );
            for (int i=0; i < cols; i++) {
                if (ndx < cannons.Size()) {
                    Cannon &cc = cannons[ndx];
                    
                    glm::vec4 tc = cc.team->teamColor;
                    if (cc.health > 0) {
                        ctx->style.button.text_normal = nk_rgb(int(tc.r*255.0),int(tc.g*255.0),int(tc.b*255.0));
                    } else {
                        // Dead
                        ctx->style.button.text_normal = nk_rgb( 80.0, 80.0, 80.0 );
                    }
                    
                    ctx->style.button.border_color = (ndx==activeCannon)?activeBorder.data.color:oldButtonStyle.border_color;
                    
                    if (nk_button_label( ctx, (cc.health > 0)?cc.name:"R.I.P" )) {
                        lookAtCannon( cc );
                    }
                }
                ndx++;
            }
            nk_layout_row_end(ctx);
        }
        
        nk_end(ctx);
    }
    ctx->style.window = oldWindowStyle;
    ctx->style.button = oldButtonStyle;
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


