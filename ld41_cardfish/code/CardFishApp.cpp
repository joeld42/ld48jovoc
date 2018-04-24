//------------------------------------------------------------------------------
//  CardFishApp.cpp
//------------------------------------------------------------------------------
#include <unistd.h>

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Dbg/Dbg.h"
#include "Input/Input.h"
#include "Core/Time/Clock.h"

//#include "Sound/Sound.h"
//#include "Sound/SoundGen.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "Assets/Gfx/ShapeBuilder.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/random.hpp"

#include "shaders.h"

#include "IO/IO.h"
#if ORYOL_EMSCRIPTEN
#include "HttpFS/HTTPFileSystem.h"
#else
#include "LocalFS/LocalFileSystem.h"
#endif

#include "glm/gtx/matrix_interpolation.hpp"

#include "Camera.h"
#include "SceneObject.h"
#include "CardFishApp.h"

#include "par_easings.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

using namespace Oryol;
using namespace Tapnik;

#define DRAW_ANIM_TIME (0.5f)
#define BIG_CARD_SIZE (2.5f)
#define REEL_TIMEOUT (0.5f)

#define MESSAGE_TIME (0.75f)
#define MESSAGE_FADE_TIME (0.2f)
#define ANIM_CAMERA_TIME (2.5f)

//------------------------------------------------------------------------------
AppState::Code
CardFishApp::OnInit() {
    
    // set up IO system
    IOSetup ioSetup;
#if ORYOL_EMSCRIPTEN
    ioSetup.FileSystems.Add("http", HTTPFileSystem::Creator());
    ioSetup.Assigns.Add("gamedata:", "http://localhost:8000/gamedata/");
#else
    ioSetup.FileSystems.Add( "file", LocalFileSystem::Creator() );
    ioSetup.Assigns.Add("gamedata:", "root:../Resources/gamedata/");
    //ioSetup.Assigns.Add("gamedata:", "cwd:gamedata/");
#endif
    
    IO::Setup(ioSetup);    
    
    
    gfxSetup = GfxSetup::WindowMSAA4(800, 600, "LD41 CardFish");
    Gfx::Setup(gfxSetup);
    
    this->uiAssets = Memory::New<UIAssets>();
    this->uiAssets->SetupUI();
    uiAssets->fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    uiAssets->fbHeight = Gfx::DisplayAttrs().FramebufferHeight;

    srand( Clock::Now().getRaw() );
    
    Dbg::Setup();

    Input::Setup();
    
    
    
    // Initialize SoLoud (automatic back-end selection)
    soloud.init();
    
    IO::Load("gamedata:jump.wav", [this](IO::LoadResult loadResult) {
        this->sfxJumpData = std::move(loadResult.Data);
        sfxJump.loadMem(sfxJumpData.Data(), sfxJumpData.Size(), true, false );
        printf("SFX Jump loaded...\n");
    });
    
    IO::Load("gamedata:ByTheLake.ogg", [this](IO::LoadResult loadResult) {
        this->musicData = std::move(loadResult.Data);
        music.loadMem(musicData.Data(), musicData.Size(), true, false );
        music.setLooping(true);
        soloud.play( music );
        musicPlaying = 1;
    });
    /*
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
     */
    
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
    //Memory::Clear(&g_uiMedia, sizeof(g_uiMedia));
    
    Log::Info("---- setup NKUI ---\n");
    isAnimCamera = false;
    
    // setup clear states
    this->passAction.Color[0] = glm::vec4( 156.0f/255.0f, 221.0f/255.0f, 206.0f/255.0f, 1.0 );

    gameCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f),
                    uiAssets->fbWidth,
                    uiAssets->fbHeight, 0.01f, 100.0f);

    dbgCamera = {};
    dbgCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f),
                    uiAssets->fbWidth,
                    uiAssets->fbHeight, 0.01f, 100.0f);
    
    cardCamera.Setup(glm::vec3(0.0, 0.0, 7.0), glm::radians(45.0f),
                    uiAssets->fbWidth,
                    uiAssets->fbHeight, 1.0f, 1000.0f);

    
    // Load the scenes
    gameScene = Memory::New<Scene>();
    gameScene->Setup( &gfxSetup );
    
    game.gameStarted = false;
    game.SetupCardDefs();
    for (int i=0; i < game.cardDefs.Size(); i++) {
        gameScene->cardIds.Add( game.cardDefs[i].cardId );
    }
    
    gameScene->LoadScene( "cardfish",[this](bool success) {
        onSceneLoaded();
    });
    
    // create a donut mesh, shader and pipeline object
    ShapeBuilder shapeBuilder;
    shapeBuilder.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Normal, VertexFormat::Byte4N }
    };
    shapeBuilder.Torus(0.8f, 1.5f, 20, 36);
    SetupAndData<MeshSetup> meshSetup = shapeBuilder.Build();
    this->mainDrawState.Mesh[0] = Gfx::CreateResource( meshSetup );
    
    Id shd = Gfx::CreateResource(WorldShader::Setup());
    
    auto ps = PipelineSetup::FromLayoutAndShader( shapeBuilder.Layout, shd);
    ps.RasterizerState.CullFaceEnabled = true;
    ps.RasterizerState.CullFace = Face::Code::Front;
    //ps.RasterizerState.SampleCount = coordinator->gfxSetup.SampleCount;
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
    dbgDraw->Setup( gfxSetup );
    dd::initialize( dbgDraw );    
    
    this->lastTimePoint = Clock::Now();
    this->startTimePoint = this->lastTimePoint;
    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
CardFishApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}


//------------------------------------------------------------------------------
AppState::Code
CardFishApp::OnRunning() {
    
    // Update clocks
    frameDtRaw = Clock::LapTime(this->lastTimePoint);
    
    Camera *activeCamera = debugMode?&dbgCamera:&gameCamera;
    
    //updateGameCamera();
    
    // Do update loop
    Oryol::Duration frameDt = Oryol::Duration::FromSeconds( 1.0/60.0 );
    
    this->dynamicUpdate( frameDtRaw );
    
    leftoverTime += frameDtRaw;
    while ( leftoverTime > frameDt )
    {
        this->fixedUpdate( frameDt );
        leftoverTime -= frameDt;
    }
    
    //this->lateUpdate();
        
    // === Finalize Transforms
    this->updateCards();
    
    gameScene->finalizeTransforms( activeCamera->ViewProj, cardCamera.ViewProj );
    this->updatePicking();
    
    //dd::cross(glm::value_ptr(groundCursor), 2.0f );
    
    dbgDraw->debugDrawMVP = activeCamera->ViewProj;
    dbgDraw->debugDrawOrthoMVP = glm::ortho(0.0f, uiAssets->fbWidth, uiAssets->fbHeight, 0.0f );
    //Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    
    if (debugMode)
    {
        this->handleInputDebug();
    }
    
    // render one frame
    Gfx::BeginPass(this->passAction);
    
    gameScene->drawSceneLayer( gameScene->sceneDrawState, false, false );
    gameScene->drawSceneLayer( gameScene->lakeDrawState, false, true );
    
    //    const ddVec3 boxColor  = { 0.0f, 0.8f, 0.8f };
    //    const ddVec3 boxCenter = { 0.0f, 0.0f, 0.0f };
    //    float boxSize = 1.0f;
    //    dd::box(boxCenter, boxColor, boxSize, boxSize, boxSize );
    //    dd::cross(boxCenter, 1.0f);
    
    // Toggle vis for screenshots
    if (Input::KeyDown(Key::Z)) {
        if (currentCard.sceneObj) {
            currentCard.sceneObj->hidden = !currentCard.sceneObj->hidden;
        }
    }
    
    // Toggle music
    if (Input::KeyDown(Key::M)) {
        musicPlaying = !musicPlaying;
        if (!musicPlaying) {
            music.stop();
        } else {
            soloud.play( music );
        }
    }
    
    if (Input::KeyDown(Key::X)) {
        disposeCard(currentCard);
        drawNextCard();
    }
    
    if (Input::KeyDown(Key::Tab)) {
        debugMode = !debugMode;
        Log::Info( "Debug Mode: %s\n", debugMode?"ON":"OFF" );
    }
    if (debugMode) {
        ddVec3 textPos2D = { 0.0f, 20.0f, 0.0f };
        textPos2D[0] = uiAssets->fbWidth / 2.0;
        dd::screenText("Debug Mode", textPos2D, dd::colors::Orange );
    }
    
    // Flush debug draws
    Oryol::Duration appTime = lastTimePoint.Since( startTimePoint );
    dd::flush( appTime.AsMilliSeconds() );
    
    // Do UI
    if (this->uiAssets->fontValid) {
        this->interfaceScreens( uiAssets );
    }
    
    // Draw the table cards on top
    if ((game.gameStarted) && (!game.gameOver)) {
        gameScene->drawSceneLayer( gameScene->cardDrawState, true, false );
    }
    
    Dbg::DrawTextBuffer();
    
    Gfx::EndPass();
    Gfx::CommitFrame();
    
    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

void
CardFishApp::handleInputDebug() {
    
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
        
        if (Input::KeyDown(Key::C)) {
            
            dbgCamera.Setup(glm::vec3(0.0, 0.0, 15.0), glm::radians(45.0f),
                            uiAssets->fbWidth,
                            uiAssets->fbHeight, 1.0f, 1000.0f);
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

void CardFishApp::updatePicking()
{
    // Only update hovering if the mouse has moved
    const glm::vec2 &mouseMove = Input::MouseMovement();
    bool didMouseMove = glm::length( mouseMove ) > 0.01f;
    
    for (int i=0; i < gameScene->sceneObjs.Size(); i++) {
        SceneObject *obj = gameScene->sceneObjs[i];
        
        if (!obj->interaction) {
            continue;
        }
        
        if ((didMouseMove) || (obj->interaction->hoverDirty)) {
            
            if (RayHitObject( obj, obj->isCard?mouseCardsRay:mouseRay) ) {
                obj->interaction->mouseHovering = true;
                //obj->vsParams.tintColor = glm::vec4( 1,1,1,1 );
            } else {
                //obj->vsParams.tintColor = glm::vec4( 1 );
                obj->interaction->mouseHovering = false;
            }
            obj->interaction->hoverDirty = false;
        }
        
        //dbgDrawBBox( obj, dd:: );
        // TODO: if it's a tile, hover it
    }
    
    // special handling.. if the trash can is hovered then don't hover the lake
    if (lakeObj && trashCanObj) {
        if (trashCanObj->interaction->mouseHovering) {
            lakeObj->interaction->mouseHovering = false;
        }
    }
    
    if (lakeObj) {
        if ((lakeObj->interaction->mouseHovering) && (isDraggingCard)) {
            lakeObj->vsParams.tintColor = glm::vec4( 1 );
            lakeObj->fsParamsLake.highlight = 1.0f;
        } else {
            lakeObj->vsParamsLake.tintColor = glm::vec4( 0.6f, 0.6f, 0.6f, 1.0f );
            lakeObj->fsParamsLake.highlight = 0.0f;
        }
        lakeObj->fsParamsLake.tval = gameTime.AsSeconds() * 0.012f;
    }
    
    
    
    // For testing...
    /*
    for (int i=0; i < testCards.Size(); i++) {
        SceneObject *obj = testCards[i];
        if (obj->interaction->mouseHovering) {
            obj->interaction->flipAmount += 0.05f;
        } else {
            obj->interaction->flipAmount -= 0.08f;
        }
        obj->interaction->flipAmount = glm::clamp( obj->interaction->flipAmount, 0.0f, 1.0f);
        
        
        glm::mat4 xlate = glm::translate( glm::mat4(), glm::vec3( ((float)i-2.5) * 1.2f, -2.0f, 0.0f ));
        glm::mat4 rot = glm::rotate( xlate,
                                    obj->interaction->flipAmount * glm::pi<float>(),
                                    glm::vec3(0.0f, 1.0f, 0.0f)  );
        obj->xform = rot;
    }
     */
}

// TODO: interpolate
void CardFishApp::switchToNamedCamera( Oryol::String cameraName )
{
    SceneCamera cam = gameScene->findNamedCamera( cameraName );
    if (cam.index >=0) {
        activeCameraIndex = cam.index;
    }
    
    startCamera = glm::mat4( gameCamera.Model );
    endCamera = glm::mat4( cam.mat );
    isAnimCamera = true;
    switchCameraAnim = 0.0f;
    
    //gameCamera.UpdateModel( cam.mat );
    gameCamera.UpdateProj(glm::radians(45.0f), uiAssets->fbWidth, uiAssets->fbHeight, 0.01f, 1000.0f);

}

// =======================================================================================
void CardFishApp::onSceneLoaded()
{
    ready = true;
    printf(".... OnSceneLoaded... ready \n");
    
    switchToNamedCamera( "titleCam" );
    
    deckTablePos = glm::vec2( 2.86f, -2.1f );
    currCardTablePos = glm::vec2(0.0f, 0.35f);
    
    lakeObj = gameScene->FindNamedObject( "Lake" );
    lakeObj->lake = true;
    lakeObj->makeInteractable();
    
    // TODO: multiple trash cans
    trashCanObj = gameScene->FindNamedObject( "TrashCan" );
    trashCanObj->makeInteractable();
    
    /*
    for (int i=0; i < 5; i++) {
        SceneObject *obj = gameScene->spawnObjectWithMeshNamed( (i&0x1)?"card0":"card1");
        obj->isCard = true;
        obj->makeInteractable();
        obj->xform = glm::translate(glm::mat4(), glm::vec3( ((float)i-2.5) * 1.2f, -2.0f, 0.0f ));
        testCards.Add( obj );
    }
    */
    
}

void CardFishApp::startGame()
{
    game.gameStarted = true;
    game.gameOver = false;
    game.resetGame();
    prepareNextDrawCard();
    drawNextCard();
    
    switchToNamedCamera( "lake1Cam" );
    
    messageTimeout = MESSAGE_TIME + MESSAGE_FADE_TIME;
    message( "Ready to Fish!", glm::vec4( 0.5f, 1.0f, 0.5f, 1.0f ));

}

void CardFishApp::fixedUpdate( Oryol::Duration fixedDt )
{
    gameTime += fixedDt;
    float dt = fixedDt.AsSeconds();
    
    // Camera stuff
    if (isAnimCamera) {
        switchCameraAnim += dt;
        float t = switchCameraAnim / ANIM_CAMERA_TIME;
        if (t > 1.0f) {
            t = 1.0f;
            isAnimCamera = false;
            switchCameraAnim = 0.0f;
        }
        
        gameCamera.UpdateModel( glm::interpolate( startCamera, endCamera, t ) );
        
    }
    
    
    // Our input handling is crap so give it a few frames
    // between pressing play before starting game
    if (!game.gameStarted) {
        if (doStartGame > 0) {
            doStartGame--;
            if (doStartGame <= 0) {
                doStartGame = -1;
                startGame();
            }
        }
        return;
    }

    if (mouseDownTimeout > 0) {
        mouseDownTimeout -= dt;
    }
    
    // Need to animate the current card??
    if (currentCard.drawAnimTimer > 0.0f) {
        currentCard.drawAnimTimer -= dt;
        if (currentCard.drawAnimTimer < 0.0f) {
            currentCard.drawAnimTimer = 0.0f;
        }
        
        float t = currentCard.drawAnimTimer / DRAW_ANIM_TIME;
        if (currentCard.sceneObj) {
            currentCard.sceneObj->interaction->tablePos = glm::mix( this->currCardTablePos, this->deckTablePos, t );
            currentCard.sceneObj->interaction->flipAmount = t;
            currentCard.sceneObj->interaction->cardSize = glm::mix( BIG_CARD_SIZE, 1.0f, t );
        }
    } else {
        if ((currentCard.sceneObj) && (currentCard.sceneObj->interaction)) {
            if (isDraggingCard) {
                if (activeDropZone == DropZone_TRASH) {
                    currentCard.sceneObj->isCard = false;
                    
                    glm::mat4 xlate = glm::translate( trashCanObj->xform,
                                        glm::vec3(0.0f, 0.0f, 0.03f + fabs(sin(gameTime.AsSeconds() * 3.0f) )) );
                    glm::mat4 rot1 = glm::rotate( xlate, glm::radians( 90.0f),
                                                 glm::vec3(1.0f, 0.0f, 0.0f)  );
                    glm::mat4 rot2 = glm::rotate( rot1,
                                                 float(gameTime.AsSeconds() * glm::pi<float>()),
                                                 glm::vec3(0.0f, 1.0f, 0.0f)  );
                    glm::mat4 scl = glm::scale( rot2, glm::vec3( 0.9f ));
                    currentCard.sceneObj->xform = scl;

                    
                } else if (activeDropZone == DropZone_LAKE) {
                    currentCard.sceneObj->isCard = false;
                    
                    //currentCard.sceneObj->xform = glm::translate( glm::mat4(), groundCursor );
                    
                    glm::mat4 xlate = glm::translate( glm::mat4(), groundCursor );
                    glm::mat4 rot1 = glm::rotate( xlate, glm::radians( 90.0f),
                                                glm::vec3(1.0f, 0.0f, 0.0f)  );
                    glm::mat4 rot2 = glm::rotate( rot1,
                                                float(gameTime.AsSeconds() * glm::pi<float>()),
                                                glm::vec3(0.0f, 1.0f, 0.0f)  );
                    glm::mat4 scl = glm::scale( rot2, glm::vec3( 5.0f ));
                    currentCard.sceneObj->xform = scl;
                    
                } else {
                    currentCard.sceneObj->isCard = true;
                    currentCard.sceneObj->interaction->tablePos = glm::vec2( tableCursor );
                }
            } else {
                // Fly card back to table center
                if (currentCard.sceneObj) {
                    currentCard.sceneObj->interaction->tablePos = glm::mix( currentCard.sceneObj->interaction->tablePos,
                                                                           this->currCardTablePos, 0.3f );
                }
            }
        }
    }
    
    // Update lake fish
    // TODO: Flocking sim...
    for (int i=0; i < game.lakeFish.Size(); i++) {
        glm::mat4 xlate = glm::translate( glm::mat4(),
                            game.lakeFish[i].lakePos + glm::vec3( 0.0f, 0.0f,
                                                sin(gameTime.AsSeconds() + (game.lakeFish[i].seed * 100.0) )) );
        glm::mat4 rot1 = glm::rotate( xlate, glm::radians( 90.0f),
                                     glm::vec3(1.0f, 0.0f, 0.0f)  );
        glm::mat4 rot2 = glm::rotate( rot1,
                                     float( (game.lakeFish[i].seed * 100.0) + gameTime.AsSeconds() * 0.5f * glm::pi<float>()),
                                     glm::vec3(0.0f, 1.0f, 0.0f)  );
        glm::mat4 scl = glm::scale( rot2, glm::vec3( 1.0f ));
        game.lakeFish[i].sceneObj->xform = scl;
    }
    
    // Reel in the fishy
    if (reelPowerRemaining > 0) {
        reelTimeout -= dt;
        if (reelTimeout < 0.0f) {
            reelPowerRemaining -= 1;
            game.reelDistance -= 1;
            
            if (game.reelCard.cardId != "dead_fish") {
                game.reelTension += 1;
            }
            
            reelTimeout = REEL_TIMEOUT;
        
            if (game.reelDistance <= 0) {
                catchReelFish();
            } else {
                updateTension();
            }
        }
    }
    
    // Update the message queue
    if (messageQueue.Size() > 0 ) {
        messageTimeout -= dt;
        if (messageTimeout < 0.0f) {
            messageQueue.PopFront();
            messageTimeout = MESSAGE_TIME + MESSAGE_FADE_TIME;
        }
    }
}

void CardFishApp::message( Oryol::String messageText, glm::vec4 color )
{
    Message msg = {};
    msg.messageText = messageText;
    msg.color = color;
    
    messageQueue.Add( msg );
}

void CardFishApp::dynamicUpdate( Oryol::Duration frameDt )
{
    
    // Don't update if we're animating
    if (currentCard.drawAnimTimer > 0.0f) {
        return;
    }
    
    // Update mouse ray and ground cursor
    glm::vec2 mousePos = Input::MousePosition();
    mouseRay = gameCamera.getCameraRay(mousePos.x, mousePos.y );
    
    // find where the ray hits the ground plane
    float t = mouseRay.pos.z / -mouseRay.dir.z;
    groundCursor = mouseRay.pos + ( mouseRay.dir * t );
    
    mouseCardsRay = cardCamera.getCameraRay( mousePos.x, mousePos.y );

    float t2 = mouseCardsRay.pos.z / -mouseCardsRay.dir.z;
    tableCursor = mouseCardsRay.pos + ( mouseCardsRay.dir * t2 );

    if ((!game.gameStarted)||(game.gameOver)) {
        return;
    }

    
//    Dbg::PrintF( "CardsRay: %3.2f %3.2f %3.2f\r\n",
//                mouseCardsRay.dir.x,
//                mouseCardsRay.dir.y,
//                mouseCardsRay.dir.z );
    
    // TODO: Touch screens
    if (Input::MouseButtonDown(MouseButton::Left)) {
        if (currentCard.sceneObj && currentCard.sceneObj->interaction->mouseHovering) {
            isDraggingCard = true;
            currentCard.sceneObj->interaction->cardSize = 0.8f;
        }
        mouseDownTimeout = 0.2f;
    }
    if (Input::MouseButtonUp(MouseButton::Left)) {
        bool wasDragging = isDraggingCard;
        isDraggingCard = false;
        
        if (mouseDownTimeout > 0.0f) {
            // Just cancel drag, don't process
            if (currentCard.sceneObj) {
                currentCard.sceneObj->isCard = true;
                currentCard.sceneObj->interaction->cardSize = BIG_CARD_SIZE;
            }
            return;
        }
        
        // Handle drop
        bool handledCard = false;
        if (activeDropZone == DropZone_TRASH) {
            if ((wasDragging) && (currentCard.cardId != "shoe")) {
                trashCard();
                handledCard = true;
            }
        } else if (activeDropZone == DropZone_TACKLE) {
            if ((game.tackleCards.Size() < 5) && (currentCard.cardType==CardType_TACKLE)) {
                // ADD TACKLE CARD
                if (currentCard.sceneObj) {
                    currentCard.sceneObj->isCard = true;
                    currentCard.sceneObj->interaction->cardSize = 1.0f;
                    currentCard.sceneObj->interaction->tablePos = glm::vec2( -2.86 + 1.0*game.tackleCards.Size(), -2.1f);
                }
                game.PlayTackleCard( currentCard );
                updateSlack();
                
                handledCard = true;
                drawNextCard();
            }
        } else if (activeDropZone == DropZone_REEL) /* && (game.reelDistance > 0) */ {
            
            if (game.reelDistance > 0) {
                
                reelTimeout = REEL_TIMEOUT;
                reelPowerRemaining +=  game.calcReelPower( currentCard );

                // Consume the card used to reel the fish
                disposeCard( currentCard );
                drawNextCard();
                handledCard = true;

            }
        } else if (activeDropZone == DropZone_LAKE)  {
            
            // Really could combine Cast and Action
            if ((currentCard.cardType == CardType_CAST) ||
                (currentCard.cardType == CardType_FISH) ||
                (currentCard.cardType == CardType_ACTION)) {
                
                if (currentCard.cardType == CardType_CAST) {
                    
                    // Consume the cast card
                    disposeCard( currentCard );
                    doCast();
                    
                } else if (currentCard.cardType == CardType_ACTION) {
                    // Consume the action card
                    disposeCard( currentCard );
                    doAction();
                    updateSlack();
                    
                } else {
                    // CardType_FISH
                    doLake();
                }
                handledCard = true;
                drawNextCard();
            }
        }
        
        if (!handledCard) {
            // Fly back to table center
            if (currentCard.sceneObj) {
                currentCard.sceneObj->isCard = true;
                currentCard.sceneObj->vsParams.tintColor = glm::vec4( 1.0 );
                currentCard.sceneObj->interaction->cardSize = BIG_CARD_SIZE;
                currentCard.sceneObj->interaction->tablePos = glm::vec2( tableCursor );
            }
        }
        
    }
    
    // Update drop zone
    activeDropZone = DropZone_LAKE;
    //Dbg::PrintF("MouseY %f\n\r", mousePos.y );
    if (trashCanObj) {
        trashCanObj->vsParams.tintColor = glm::vec4( 1.0f);
    }
    if (trashCanObj && trashCanObj->interaction->mouseHovering) {
        activeDropZone = DropZone_TRASH;
        if ((isDraggingCard) && (currentCard.sceneObj)) {
            currentCard.sceneObj->vsParams.tintColor = glm::vec4( 0.6, 0.6, 1.0, 1.0 );
        }
        trashCanObj->vsParams.tintColor = glm::vec4( 0.5f, 1.0f, 1.0f, 1.0f );
    } else if (mousePos.y < 100 ) {
        activeDropZone = DropZone_REEL;
    } else if (mousePos.y > (uiAssets->fbHeight - 200) ) {
        activeDropZone = DropZone_TACKLE;
        if ((currentCard.sceneObj) && (isDraggingCard)) {
            if ((game.tackleCards.Size() < 5) && (currentCard.cardType==CardType_TACKLE)) {
                currentCard.sceneObj->vsParams.tintColor = glm::vec4( 1.0 );
            } else {
                currentCard.sceneObj->vsParams.tintColor = glm::vec4( 1.0, 0.6, 0.6, 1.0 );
            }
        }
    } else {
        if (currentCard.sceneObj) {
            currentCard.sceneObj->vsParams.tintColor = glm::vec4( 1.0 );
        }
    }
}

void CardFishApp::disposeCard( Card &card ) {
    
    // TODO: ref-count scene objs? ugh?
    if (card.sceneObj != NULL) {
        gameScene->destroyObject( card.sceneObj );
        card.sceneObj = NULL;
    }
}

void CardFishApp::catchReelFish()
{
    reelPowerRemaining = 0;
    game.reelDistance = 0;
    
    StringBuilder strBuilder;
    strBuilder.Format( 200, "Yay, caught %s and earned %d fishpoints...",
                      game.reelCard.title.AsCStr(),
                      game.reelCard.fishPoints );
    game.fishPoints += game.reelCard.fishPoints;
    message( strBuilder.GetString(), glm::vec4(0.86f,0.26f,0.18f, 1.0f) );
    soloud.play( sfxJump );
    
    disposeCard( game.reelCard );
}

void CardFishApp::doAction()
{
    if (currentCard.cardId=="power_reel") {
        if (game.reelDistance > 0) {
            // Insta-catch
            catchReelFish();
        } else {
            message( "No fish on line, Power Reel does nothing.", glm::vec4(0.86f,0.26f,0.18f, 1.0f) );
        }


    } else if (currentCard.cardId=="zap_line") {
        
        if (game.reelDistance > 0) {
            game.reelCard = transmuteCard( game.reelCard, "dead_fish" );
        } else {
            message( "No fish on line, Zap Line does nothing.", glm::vec4(0.86f,0.26f,0.18f, 1.0f) );
        }

    } else if (currentCard.cardId=="dynamite") {
    
        int count = 0;
        for (int i=0; i < game.lakeFish.Size(); i++) {
            game.lakeFish[i] = transmuteCard( game.lakeFish[i], "dead_fish" );
            count++;
        }
        if (game.reelDistance > 0) {
            game.reelCard = transmuteCard( game.reelCard, "dead_fish" );
            count++;
        }
        StringBuilder strBuilder;
        strBuilder.Format( 200, "BOOM! Dynamite killed %d fish.", count );
        message( strBuilder.GetString(), glm::vec4(0.39f,0.99f,0.50f,1.0f) );
    
    } else if (currentCard.cardId=="capsize") {
        
        for (int i = 0; i < game.tackleCards.Size(); i++) {
            disposeCard( game.tackleCards[i] );
            game.returnCardToDeck( game.tackleCards[i] );
        }
        
        game.tackleCards.Clear();
        
    } else if (currentCard.cardId=="scoop_net") {
        
        int totalFishPoints = 0;
        for (int i=game.lakeFish.Size()-1; i >=0; i--) {
            
            if (game.lakeFish[i].cardId=="dead_fish") {
                totalFishPoints += game.lakeFish[i].fishPoints;
                disposeCard( game.lakeFish[i] );
                game.lakeFish.EraseSwapBack( i );
            }
        }
        
        if ((game.reelDistance > 0) && (game.reelCard.cardId == "dead_fish" )) {
            totalFishPoints += game.reelCard.fishPoints;
            disposeCard( game.reelCard );
            game.reelDistance = 0;
        }
        
        game.fishPoints += totalFishPoints;
        
        StringBuilder strBuilder;
        strBuilder.Format( 200, "Scooped up %d points worth of dead fish.", totalFishPoints );
        message( strBuilder.GetString(), glm::vec4(0.39f,0.99f,0.50f,1.0f) );

        
    } else  {
        printf("MISSING action for card '%s'\n", currentCard.cardId.AsCStr() );
    }

}

void CardFishApp::doLake()
{
    // Add fish to lake
    currentCard.lakePos = groundCursor;
    currentCard.seed = glm::linearRand(0.0f, 1.0f);
    
    
    if (currentCard.cardId=="minnows") {
        Card minnow = game.FindCardDef("minnow");
        for (int i=0; i < 3; i++) {
            Card cardClone = minnow;
            cardClone.sceneObj = NULL;
            spawnCardObject( cardClone );
            cardClone.sceneObj->isCard = false;
            cardClone.lakePos = glm::gaussRand( groundCursor, glm::vec3(1.0f));            
            cardClone.seed = glm::linearRand(0.0f, 1.0f);
            game.PlayCardToLake( cardClone );
        }
        
        disposeCard( currentCard );
        
    } else {
        game.PlayCardToLake( currentCard );
    }
    
    updateSlack();
}

void CardFishApp::doCast()
{
    // Special casts?
    if ((currentCard.cardId == "cast2") && (currentCard.charges>0)) {
        currentCard.charges--;
        game.returnCardToDeck( currentCard );
    }

    // If we're already after a fish, let it go
    if (game.reelDistance > 0) {
        StringBuilder strBuilder;
        strBuilder.Format( 200, "You let %s go ...",
                          game.reelCard.title.AsCStr() );
        message( strBuilder.GetString(), glm::vec4(0.86f,0.26f,0.18f, 1.0f) );
    
        disposeCard( game.reelCard);
        game.reelDistance = 0;
    }
    
    if (game.lakeFish.Size() > 0) {
        Card lakeCard = game.castLakeCard();
        lakeCard.sceneObj->isCard = true;
        lakeCard.sceneObj->interaction->cardSize = 0.8f;
        lakeCard.sceneObj->interaction->tablePos = glm::vec2( 3.0f, 2.3f);
        
        game.reelCard = lakeCard;
        game.reelTension = 6;
        game.reelDistance = lakeCard.depth;
        printf("Reeldistance is %d\n", game.reelDistance );
        
        // transfer ownership of card to reelCard
        currentCard.sceneObj = NULL;
        
        StringBuilder strBuilder;
        strBuilder.Format( 200, "Hooked %s, now reel it in...",
                          game.reelCard.title.AsCStr() );
        message( strBuilder.GetString(), glm::vec4(0.39f,0.99f,0.50f,1.0f) );
        
    } else {

        message( "Didn't even get a nibble...", glm::vec4(0.86f,0.26f,0.18f, 1.0f) );
    }
    
}
                
void CardFishApp::nextCamera()
{
    activeCameraIndex++;
    
    if (activeCameraIndex >= gameScene->sceneCams.Size() ) {
        activeCameraIndex = 0;
    }
    
    SceneCamera cam = gameScene->sceneCams[activeCameraIndex];
    Log::Info("Camera: %s\n", cam.cameraName.AsCStr() );
    
    gameCamera.UpdateModel( cam.mat );
    
    dbgPrintMatrix( "gameCamera model", gameCamera.Model );
}

void CardFishApp::updateCards()
{
    for (int i=0; i < gameScene->sceneObjs.Size(); i++) {
        SceneObject *obj = gameScene->sceneObjs[i];
        if (obj->hidden) continue;
        if ((obj->isCard) && (obj->interaction)) {
            
            obj->interaction->flipAmount = glm::clamp( obj->interaction->flipAmount, 0.0f, 1.0f);
            
            glm::vec2 tablePos = obj->interaction->tablePos;
            
            glm::mat4 xlate = glm::translate( glm::mat4(), glm::vec3( tablePos.x, tablePos.y, 0.0f ));
            glm::mat4 rot = glm::rotate( xlate,
                                        obj->interaction->flipAmount * glm::pi<float>(),
                                        glm::vec3(0.0f, 1.0f, 0.0f)  );
            glm::mat4 scl = glm::scale( rot, glm::vec3( obj->interaction->cardSize));
            obj->xform = scl;
        }
    }

}

void CardFishApp::spawnCardObject( Card &card ) {
    if (card.sceneObj != NULL) {
        printf("WARN: spawnCardObject called for card '%s' but it already has a sceneObj.\n" , card.cardId.AsCStr() );
    }
    
    SceneObject *obj = gameScene->spawnObjectWithMeshNamed( card.cardId );
    obj->isCard = true;
    obj->makeInteractable();
    obj->interaction->flipAmount = 1.0f;
    obj->interaction->cardSize = 1.0f;
    obj->interaction->tablePos = deckTablePos;
    //obj->xform = glm::translate(glm::mat4(), glm::vec3( 2.9f, -2.1f, 0.0f ));
    
    card.sceneObj = obj;

}

Card CardFishApp::transmuteCard( Card origCard, Oryol::String newCardId )
{
    Card newCard = game.FindCardDef( newCardId );
    newCard.charges = origCard.charges;
    newCard.seed = origCard.seed;
    newCard.drawAnimTimer = origCard.drawAnimTimer;
    newCard.lakePos = origCard.lakePos;
    
    spawnCardObject( newCard );
    if (origCard.sceneObj) {
        newCard.sceneObj->xform = origCard.sceneObj->xform;
        newCard.sceneObj->isCard = origCard.sceneObj->isCard;
        
        if (origCard.sceneObj->interaction) {
            *(newCard.sceneObj->interaction) = *(origCard.sceneObj->interaction );
        }
        
        disposeCard( origCard );
    }

    return newCard;
}

void CardFishApp::prepareNextDrawCard()
{
    if (game.deck.Size() > 0) {
        Card &card = game.deck.Back();
        spawnCardObject( card );
    }
}

void CardFishApp::drawNextCard()
{
    printf("... draw next card...\n");
    if (game.deck.Size() > 0) {
        currentCard = game.deck.PopBack();
        currentCard.drawAnimTimer = DRAW_ANIM_TIME;
        
        prepareNextDrawCard();
    } else {
        // Tried to draw a card but the deck was empty ... game over man!
        game.gameOver = true;
        
        switchToNamedCamera( "gameOverCam" );
        
        cleanupCards( game.tackleCards );
        cleanupCards( game.lakeFish );
        cleanupCards( game.deck );
        
        // BUG??
        //disposeCard( currentCard );
        currentCard.sceneObj = NULL;
        disposeCard( game.reelCard );
    }
}



void CardFishApp::cleanupCards( Oryol::Array<Card> &cards )
{
    for (int i=0; i < cards.Size(); i++) {
        disposeCard( cards[i] );
    }
    cards.Clear();
}


void CardFishApp::trashCard()
{
    // Discard card
    disposeCard( currentCard );
    updateSlack();
    drawNextCard();
}

void CardFishApp::updateTension()
{
    printf("Udate tension reelDist %d tension %d (%d, %d)\n",
           game.reelDistance, game.reelTension, game.reelTensionMax, game.reelSlackMin );
    
    // Note: Reel tension max has some one-off problem
    if (game.reelTension > game.reelTensionMax+1) {
        
        StringBuilder strBuilder;
        strBuilder.Format( 200, "Line Broke! %s got away...",
                          game.reelCard.title.AsCStr() );
        message( strBuilder.GetString(), glm::vec4(0.86f,0.26f,0.18f, 1.0f) );

        disposeCard( game.reelCard );
        game.reelDistance = 0;
    }

}

void CardFishApp::updateSlack()
{
    // If there's a fish on the line, add 1 slack
    if (game.reelDistance > 0) {
        
        if (game.reelCard.cardId != "dead_fish") {
            game.UpdateLineTension();
        }
        
        if (game.reelTension <= game.reelSlackMin) {
            StringBuilder strBuilder;
            strBuilder.Format( 200, "Lost 'em! %s got away...",
                              game.reelCard.title.AsCStr() );
            message( strBuilder.GetString(), glm::vec4(0.86f,0.26f,0.18f, 1.0f) );
            
            disposeCard( game.reelCard );
            game.reelDistance = 0;
        }
    }
}

// ------------------------------------------------

void CardFishApp::interfaceTitle( nk_context* ctx )
{
    
    struct nk_panel layout;
    //static nk_flags window_flags = NK_WINDOW_BORDER;
    static nk_flags window_flags = 0;
    
    // TODO: Need a title card
    float titleW = 800;
    float titleH = 175;

    float titleMarg = (uiAssets->fbWidth - titleW) / 2;
    if (nk_begin(ctx, &layout, "title_card", nk_rect( titleMarg, 0, titleW, 175), NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic( ctx, titleH, 1);
        nk_image( ctx, uiAssets->img_title_card );
    }
    nk_end(ctx);
    
    //float menuW = 223;
    
    // Button image width is 223, not sure where the extra space is coming from
    float menuW = 230;
    float menuMarg = (uiAssets->fbWidth - menuW) / 2;
    
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    
    ctx->style.text.color = nk_rgb( 255, 255, 255 );
    nk_style_set_font(ctx, &(uiAssets->font_30->handle));
    
    uiAssets->buttonStyleNormal(ctx);
    
    if (nk_begin(ctx, &layout, "main_menu", nk_rect( menuMarg, 30 + titleH + 170,
                                                    menuW, 300), window_flags))
    {
        nk_layout_row_dynamic( ctx, 93, 1);
        
        char loadSpinner[12];
        strcpy(loadSpinner,"Loading..." );
        loadSpinner[10]="|/-\\"[int(gameTime.AsSeconds()*20.0f)%4];
        if (nk_button_label(ctx, ready?"Play!":loadSpinner )) {
            if (ready) {
                doStartGame = 10;
            }
        }
        nk_layout_row_dynamic( ctx, 50, 1);
        if (nk_checkbox_label(ctx, musicPlaying?"Music: ON":"Music: OFF", &musicPlaying ) ) {
            if (!musicPlaying) {
                music.stop();
            } else {
                soloud.play( music );
            }
        }
        
    }
    nk_end(ctx);

}

float CardFishApp::calcReelDiagramXPos( float reelDistance )
{
    return 28 + 52*reelDistance;
}

float CardFishApp::calcReelDiagramYPos( float tension )
{
    return (10*(10.0 - tension)) - 4.0;
}


void CardFishApp::interfaceGame( nk_context* ctx )
{
    
    struct nk_panel layout;
    //static nk_flags window_flags = 0;
    static nk_flags window_flags = NK_WINDOW_NO_SCROLLBAR;
    
    
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    
    nk_style_set_font(ctx, &(uiAssets->font_20->handle));
    ctx->style.text.color = nk_rgb(255,255,255);
    
    uiAssets->buttonStyleNormal(ctx);
    
    
    if (!game.gameOver)
    {
        ctx->style.window.padding = nk_vec2(0,0);
        ctx->style.window.fixed_background = nk_style_item_image( uiAssets->img_line_tension );
        if (nk_begin(ctx, &layout, "reelWidgets", nk_rect( 0, 0, uiAssets->fbWidth, 100), window_flags))
        {
            nk_layout_space_begin(ctx, NK_STATIC, 100, 10000);
            
            float highTensionY = calcReelDiagramYPos( game.reelTensionMax+1 );
            nk_layout_space_push(ctx, nk_rect(47,0,uiAssets->fbWidth - 47,highTensionY));
            nk_image( ctx, uiAssets->img_tension_high );
            
            nk_layout_space_push(ctx, nk_rect(47,-10,uiAssets->fbWidth - 47,50));
            nk_text( ctx, "Too Much Tension", 16, NK_TEXT_ALIGN_TOP|NK_TEXT_ALIGN_CENTERED );

            float lowTensionY = calcReelDiagramYPos( game.reelSlackMin );
            nk_layout_space_push(ctx, nk_rect(47,lowTensionY,uiAssets->fbWidth - 47, 100 - lowTensionY));
            nk_image( ctx, uiAssets->img_tension_low );
            
            nk_layout_space_push(ctx, nk_rect(47,60,uiAssets->fbWidth - 47,50));
            nk_text( ctx, "Too Slack", 9, NK_TEXT_ALIGN_TOP|NK_TEXT_ALIGN_CENTERED );
            
            for (int i=1; i < 15; i++) {
                char buff[10];
                sprintf(buff, "%d", i );
                nk_layout_space_push(ctx, nk_rect( calcReelDiagramXPos(i),73, 80, 50));
                nk_text( ctx, buff, strlen(buff), NK_TEXT_ALIGN_TOP|NK_TEXT_ALIGN_LEFT );
            }
            
            
            if (game.reelDistance > 0) {
                
                float reelOffs = 0.0;
                if (reelTimeout > 0) {
                    reelOffs = 1.0-(reelTimeout / REEL_TIMEOUT);
                }
                
                fishDiagramPos = glm::vec2(calcReelDiagramXPos(game.reelDistance - par_easings_in_out_cubic( reelOffs ) ),
                                           calcReelDiagramYPos(game.reelTension) );
                
                nk_layout_space_push(ctx, nk_rect( 47, 0, fishDiagramPos.x-47 + 4, fishDiagramPos.y + 4));
                nk_image( ctx, uiAssets->img_fishing_line );
                
                nk_layout_space_push(ctx, nk_rect(fishDiagramPos.x, fishDiagramPos.y-20,100,50));
                nk_image( ctx, uiAssets->img_fish_icon );
            }
        }
        nk_end(ctx);
    }
    nk_style_set_font(ctx, &(uiAssets->font_30->handle));
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba( 0, 0, 0, 0));
    if (nk_begin(ctx, &layout, "game", nk_rect( uiAssets->fbWidth - 150,
                                                uiAssets->fbHeight - 180,
                                                    100, 100), window_flags))
    {
        // Draw pile
        nk_layout_row_dynamic( ctx, 40, 1);
        char buff[20];
        sprintf( buff, "%d", game.deck.Size() );
        nk_label(ctx, buff, NK_TEXT_CENTERED );
    }
    nk_end(ctx);
    
    if (!game.gameOver) {
        if (nk_begin(ctx, &layout, "game_stats", nk_rect( 0, 100, 200, 150), window_flags))
        {
            char buff[50];
            nk_style_set_font(ctx, &(uiAssets->font_20->handle));
            
            nk_layout_row_dynamic( ctx, 22, 1);
            sprintf(buff, "Fish Points: %d", game.fishPoints );
            ctx->style.text.color = nk_rgb( 181, 100, 255 );
            nk_label(ctx, buff, NK_TEXT_LEFT );
        }
        nk_end(ctx);
    }
    
    // ---------------------------------------------
    //  Message Panel
    // ---------------------------------------------
    
    if (messageQueue.Size()) {
        const Message &currMessage = messageQueue.Front();
        
        nk_style_set_font(ctx, &(uiAssets->font_30->handle));
        
        float fade = 1.0f;
        if (messageTimeout < MESSAGE_FADE_TIME) {
            fade = messageTimeout / MESSAGE_FADE_TIME;
        }
        int fadeInt = 255 * currMessage.color.a * fade;
        ctx->style.text.color = nk_rgba( currMessage.color.r * 255,
                                         currMessage.color.g * 255,
                                         currMessage.color.b * 255,
                                         fadeInt);
        
        ctx->style.window.fixed_background = nk_style_item_color(nk_rgba( 0, 0, 0, 200 * fade));
        
        if (nk_begin(ctx, &layout, "message_hud", nk_rect( 100, 10, uiAssets->fbWidth-200, 80), 0)) {
            nk_layout_row_dynamic( ctx, 50, 1);
            nk_label( ctx, currMessage.messageText.AsCStr(), NK_TEXT_CENTERED );
        }
        nk_end(ctx);
    }
    
    // ---------------------------------------------
    //  Game Over Man
    // ---------------------------------------------
    if (game.gameOver)
    {
        ctx->style.window.background = nk_rgba(43,81,123, 200);
        ctx->style.window.border_color = nk_rgb( 56, 119, 249 );
        ctx->style.window.fixed_background.data.color = nk_rgba(43,81,123, 200);
        
        float ww = uiAssets->fbWidth-300;
        float sz = 1.0;
        //float hh =
        if (nk_begin(ctx, &layout, "game_end", nk_rect( 150, 130, ww, 400), NK_WINDOW_BORDER)) {
            
            ctx->style.text.color = nk_rgb( 255, 255, 255 );
            nk_style_set_font(ctx, &(uiAssets->font_30->handle));
            nk_layout_row_dynamic( ctx, 50, 1);
            nk_label( ctx, "--- Game Finished! ---", NK_TEXT_CENTERED );

            char buff[50];
            
            nk_layout_row_dynamic( ctx, 30, 1);
            sprintf(buff, "Fish Points: %d", game.fishPoints );
            ctx->style.text.color = nk_rgb( 181, 100, 255 );
            nk_label(ctx, buff, NK_TEXT_CENTERED );
            
            ctx->style.text.color = nk_rgb( 255, 255, 255 );
            nk_style_set_font(ctx, &(uiAssets->font_30->handle));
            
            const char *fishRating = "???";
            const char *ratingText = "???";
            if (game.fishPoints < 6) {
                fishRating = "Fish Friend";
                ratingText = "Maybe you don't need to catch anything, and "
                              "you just like drinking beer and hanging around lakes.";
            } else if (game.fishPoints < 12) {
                    fishRating = "Weekend Warrior";
                    ratingText = "A bad day of fishing is better than a great day at work, amirite?";
            } else if (game.fishPoints < 18) {
                fishRating = "Outdoorsey";
                ratingText = "Respectable showing. A fine kettle of fish. A basket of bream. A hellabit of halibut.";
            } else {
                fishRating = "Master Angler";
                ratingText = "You think like a fish. You drink pondwater and poop neon marshmallows.";
            }
            
            ctx->style.text.color = nk_rgb( 255, 255, 40 );
            nk_style_set_font(ctx, &(uiAssets->font_30->handle));
            nk_layout_row_dynamic( ctx, 40, 1);
            nk_label(ctx, fishRating, NK_TEXT_CENTERED );
            
            ctx->style.text.color = nk_rgb( 255, 255, 255 );
            nk_style_set_font(ctx, &(uiAssets->font_20->handle));
            nk_layout_row_dynamic( ctx, 120, 1);
            nk_label_wrap(ctx, ratingText );
            
            nk_style_set_font(ctx, &(uiAssets->font_30->handle));
            nk_layout_row_begin(ctx, NK_STATIC, 93*sz, 3);
            nk_layout_row_push(ctx, (ww - (230*sz))/2 );
            nk_label( ctx, "", NK_TEXT_CENTERED );
            nk_layout_row_push(ctx, 230);
            if (nk_button_label(ctx, "Play Again" )) {
                game.gameStarted = false;
                game.gameOver = false;
                game.resetGame();
                switchToNamedCamera("titleCam");
            }
            nk_layout_row_end(ctx);
            
            
            
        }
        nk_end(ctx);
    }


}

void CardFishApp::interfaceScreens( Tapnik::UIAssets *uiAssets )
{
    // Draw UI Layer
    nk_context* ctx = NKUI::NewFrame();
    
    ctx->style.window.background = { 0, 0, 0, 0 };
    ctx->style.window.border_color = { 0, 0, 0, 0 };
    ctx->style.window.fixed_background.data.color = { 0, 0, 0, 0 };
    
    if (!game.gameStarted)
    {
        interfaceTitle( ctx );
    } else {
        interfaceGame( ctx );
    }
    
    NKUI::Draw();
    
    // Note: clear happens in draw, extra clear is to workaround a bug, need to upgrade nuklear in NKUI to get the fix
    nk_clear(ctx);
}



