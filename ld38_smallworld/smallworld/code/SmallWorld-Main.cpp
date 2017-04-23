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
#include "Cannon.h"

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
    
    void BuildPlanet();
    
    ResourceLabel curMeshLabel;
//    MeshSetup curMeshSetup;
    ResourceLabel curMaterialLabel;
    int numMaterials = 0;

    bool paused;
    
    Id texture;
    DrawState mainDrawState;
    
    Planet planet;
  
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
    
    // Gameplay thinggys
    int activeCannon;
    Oryol::Array<Cannon> cannons;
    Oryol::Array<Shot> shots;
    void finishTurn();
    void fireActiveCannon();
    
    
    struct nk_style_item mkColorStyleItem( const char *hexColor );
    
    void DoGameUI( nk_context* ctx );
    
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
    double dt = frameDuration.AsSeconds();
    
    this->handle_input();
    
    if (!paused)
    {
        
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
                
                finishTurn();            
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
    
    // Dev hack to ensure we're running from the right place...
    chdir("/Users/joeld/Projects/ld48jovoc/ld38_smallworld/smallworld");
    FILE *fp = fopen( "./gamedata/tree_062.omsh", "rt");
    if (!fp) {
        printf("Error! Not running from the right path...??\n");
        exit(1);
    }
    fclose(fp);
    
    useDebugCamera = false;
    
    // set up IO system
    IOSetup ioSetup;
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
//    ioSetup.Assigns.Add("data:", "cwd:gamedata/");
    ioSetup.Assigns.Add("msh:", "cwd:gamedata/");
    ioSetup.Assigns.Add("tex:", "cwd:gamedata/");
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
    
    cannonBushing = scene->addObject( "msh:cannon_bushing.omsh", "tex:cannon_basecolor.dds");
    cannonBushing->pos = glm::vec3(0.0, 0.0, 0.0 );
    
    cannonBarrel = scene->addObject( "msh:cannon_barrel.omsh", "tex:cannon_basecolor.dds");
    cannonBarrel->pos = glm::vec3(0.0, 191.6, 0.0 );
    
    
    // Setup planet
    planet.planetTexture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile( "tex:ground1.dds",
                                                                                          scene->texSetup)));
    planet.Setup( &(scene->gfxSetup) );
    
    // Setup UI
    NKUI::Setup();
    
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
    
    Oryol::TimePoint tp = Clock::Now();
    srand( tp.getRaw() );

    // Start with some damage
    for (int i=0; i < 10; i++) {
        glm::vec3 p = glm::sphericalRand( 0.8 );
        planet.surfBuilder.addDamage( p, glm::linearRand( 0.1f, 0.2f) );
    }
    
    planet.Rebuild( scene );
    
    // Add cannons
    while (cannons.Size() < 12) {
        
        glm::vec3 p = glm::sphericalRand( planet.planetApproxRadius  );
        glm::vec3 upDir = planet.surfBuilder.evalNormal( p );
        Cannon cc( scene, p, upDir );
        cannons.Add(cc);
    }
}

void
TestApp::finishTurn()
{
    // HERE check if there's only one team with cannons remaining
    
    do {
        activeCannon++;
        if (activeCannon == cannons.Size()) {
            activeCannon = 0;
        }
    } while (cannons[activeCannon].health == 0);
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

//------------------------------------------------------------------------------
void
TestApp::handle_input() {
    
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
        } else {
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
//        if (fabs( arcballAngle ) < 0.01) {
//            arcballAxis = glm::vec3( 0.0, 1.0, 0.0 );
//            arcballAngle = glm::radians( 0.25 );
//            
//        }
        
        
        // Arcball (with momentum)
        if (fabs( arcballAngle ) > 0.001) {
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
        
        nk_layout_row_dynamic(ctx, 20, 1);
        
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
        if (nk_button_label(ctx, "Fire!") && canFire) {
            fireActiveCannon();
        }
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


