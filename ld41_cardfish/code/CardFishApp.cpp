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
#include "shaders.h"


#include "IO/IO.h"
#if ORYOL_EMSCRIPTEN
#include "HttpFS/HTTPFileSystem.h"
#else
#include "LocalFS/LocalFileSystem.h"
#endif

#include "Camera.h"
#include "SceneObject.h"
#include "CardFishApp.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

using namespace Oryol;
using namespace Tapnik;

#define DRAW_ANIM_TIME (0.5f)
#define BIG_CARD_SIZE (2.5f)

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
    //ioSetup.Assigns.Add("gamedata:", "root:../Resources/gamedata/");
    ioSetup.Assigns.Add("gamedata:", "cwd:gamedata/");
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
    
    IO::Load("gamedata:irongame.ogg", [this](IO::LoadResult loadResult) {
        this->musicData = std::move(loadResult.Data);
        music.loadMem(musicData.Data(), musicData.Size(), true, false );
        music.setLooping(true);
//        soloud.play( music );
//        musicPlaying = 1;
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
    
    // setup clear states
    this->passAction.Color[0] = glm::vec4( 0.2, 0.2, 0.2, 1.0 );
    
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
    
    dd::cross(glm::value_ptr(groundCursor), 2.0f );
    
    dbgDraw->debugDrawMVP = activeCamera->ViewProj;
    dbgDraw->debugDrawOrthoMVP = glm::ortho(0.0f, uiAssets->fbWidth, uiAssets->fbHeight, 0.0f );
    //Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    
    if (debugMode)
    {
        this->handleInputDebug();
    }
    
    // render one frame
    Gfx::BeginPass(this->passAction);
    
    this->draw();
    
    
    //    const ddVec3 boxColor  = { 0.0f, 0.8f, 0.8f };
    //    const ddVec3 boxCenter = { 0.0f, 0.0f, 0.0f };
    //    float boxSize = 1.0f;
    //    dd::box(boxCenter, boxColor, boxSize, boxSize, boxSize );
    //    dd::cross(boxCenter, 1.0f);
    
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
                obj->vsParams.tintColor = glm::vec4( 1,1,1,1 );
            } else {
                obj->vsParams.tintColor = glm::vec4( 1 );
                obj->interaction->mouseHovering = false;
            }
            obj->interaction->hoverDirty = false;
        }
        
        //dbgDrawBBox( obj, dd:: );
        // TODO: if it's a tile, hover it
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

// =======================================================================================
void CardFishApp::onSceneLoaded()
{
    SceneCamera cam = gameScene->findNamedCamera( "ShoreCam" );
    if (cam.index >=0) {
        activeCameraIndex = cam.index;
    }
    gameCamera.UpdateModel( cam.mat );
    gameCamera.UpdateProj(glm::radians(45.0f), uiAssets->fbWidth, uiAssets->fbHeight, 0.01f, 1000.0f);
    
    deckTablePos = glm::vec2( 2.86f, -2.1f );
    currCardTablePos = glm::vec2(0.0f, 0.35f);
    
    /*
    for (int i=0; i < 5; i++) {
        SceneObject *obj = gameScene->spawnObjectWithMeshNamed( (i&0x1)?"card0":"card1");
        obj->isCard = true;
        obj->makeInteractable();
        obj->xform = glm::translate(glm::mat4(), glm::vec3( ((float)i-2.5) * 1.2f, -2.0f, 0.0f ));
        testCards.Add( obj );
    }
    */
    
    game.resetGame();
    prepareNextDrawCard();
    drawNextCard();
}
void CardFishApp::fixedUpdate( Oryol::Duration fixedDt )
{
    gameTime += fixedDt;
    float dt = fixedDt.AsSeconds();
    
    // Need to animate the current card??
    if (currentCard.drawAnimTimer > 0.0f) {
        currentCard.drawAnimTimer -= dt;
        if (currentCard.drawAnimTimer < 0.0f) {
            currentCard.drawAnimTimer = 0.0f;
        }
        
        float t = currentCard.drawAnimTimer / DRAW_ANIM_TIME;
        currentCard.sceneObj->interaction->tablePos = glm::mix( this->currCardTablePos, this->deckTablePos, t );
        currentCard.sceneObj->interaction->flipAmount = t;
        currentCard.sceneObj->interaction->cardSize = glm::mix( BIG_CARD_SIZE, 1.0f, t );
    } else {
        if ((currentCard.sceneObj) && (currentCard.sceneObj->interaction)) {
            if (isDraggingCard) {
                if (activeDropZone == DropZone_LAKE) {
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
                currentCard.sceneObj->interaction->tablePos = glm::mix( currentCard.sceneObj->interaction->tablePos,
                                                                       this->currCardTablePos, 0.3f );
            }
        }
    }
    

}

void CardFishApp::dynamicUpdate( Oryol::Duration frameDt )
{
    // Update mouse ray and ground cursor
    glm::vec2 mousePos = Input::MousePosition();
    mouseRay = gameCamera.getCameraRay(mousePos.x, mousePos.y );
    
    // find where the ray hits the ground plane
    float t = mouseRay.pos.z / -mouseRay.dir.z;
    groundCursor = mouseRay.pos + ( mouseRay.dir * t );
    
    mouseCardsRay = cardCamera.getCameraRay( mousePos.x, mousePos.y );

    float t2 = mouseCardsRay.pos.z / -mouseCardsRay.dir.z;
    tableCursor = mouseCardsRay.pos + ( mouseCardsRay.dir * t2 );

//    Dbg::PrintF( "CardsRay: %3.2f %3.2f %3.2f\r\n",
//                mouseCardsRay.dir.x,
//                mouseCardsRay.dir.y,
//                mouseCardsRay.dir.z );
    
    // TODO: Touch screens
    if (Input::MouseButtonDown(MouseButton::Left)) {
        if (currentCard.sceneObj->interaction->mouseHovering) {
            isDraggingCard = true;
            currentCard.sceneObj->interaction->cardSize = 0.8f;
        }
    }
    if (Input::MouseButtonUp(MouseButton::Left)) {
        isDraggingCard = false;
        
        // Handle drop
        if ((activeDropZone == DropZone_TACKLE) && (game.tackleCards.Size() < 5)) {

            // ADD TACKLE CARD
            currentCard.sceneObj->isCard = true;
            currentCard.sceneObj->interaction->cardSize = 1.0f;
            currentCard.sceneObj->interaction->tablePos = glm::vec2( -2.86 + 1.0*game.tackleCards.Size(), -2.1f);
            game.tackleCards.Add( currentCard );
            
            drawNextCard();
        } else {
            // Fly back to table center
            currentCard.sceneObj->isCard = true;
            currentCard.sceneObj->interaction->cardSize = BIG_CARD_SIZE;
            currentCard.sceneObj->interaction->tablePos = glm::vec2( tableCursor );
        }
        
    }
    
    activeDropZone = DropZone_LAKE;
    //Dbg::PrintF("MouseY %f\n\r", mousePos.y );
    if (mousePos.y > (uiAssets->fbHeight - 200) ) {
        activeDropZone = DropZone_TACKLE;
    }
}

void CardFishApp::draw()
{
    gameScene->drawScene();
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

void CardFishApp::prepareNextDrawCard()
{
    if (game.deck.Size() > 0) {
        Card &card = game.deck.Back();
        
        SceneObject *obj = gameScene->spawnObjectWithMeshNamed( card.cardId );
        obj->isCard = true;
        obj->makeInteractable();
        obj->interaction->flipAmount = 1.0f;
        obj->interaction->cardSize = 1.0f;
        obj->interaction->tablePos = deckTablePos;
        //obj->xform = glm::translate(glm::mat4(), glm::vec3( 2.9f, -2.1f, 0.0f ));
        
        card.sceneObj = obj;
    }
}

void CardFishApp::drawNextCard()
{
    if (game.deck.Size() > 0) {
        currentCard = game.deck.PopBack();
        currentCard.drawAnimTimer = DRAW_ANIM_TIME;
        
        prepareNextDrawCard();
    }
}

// ------------------------------------------------

void CardFishApp::interfaceTitle( nk_context* ctx )
{
    
    struct nk_panel layout;
    //static nk_flags window_flags = NK_WINDOW_BORDER;
    static nk_flags window_flags = 0;
    
    float titleW = 800;
    float titleH = 175;
    //    float titleMarg = (uiAssets->fbWidth - titleW) / 2;
    //    if (nk_begin(ctx, &layout, "title_card", nk_rect( titleMarg, 30, titleW, 175), window_flags))
    //    {
    //        nk_layout_row_dynamic( ctx, titleH, 1);
    //        nk_image( ctx, uiAssets->img_title );
    //    }
    //    nk_end(ctx);
    
    //float menuW = 223;
    
    // Button image width is 223, not sure where the extra space is coming from
    float menuW = 230;
    float menuMarg = (uiAssets->fbWidth - menuW) / 2;
    
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    
    nk_style_set_font(ctx, &(uiAssets->font_30->handle));
    
    uiAssets->buttonStyleNormal(ctx);
    
    if (nk_begin(ctx, &layout, "main_menu", nk_rect( menuMarg, 30 + titleH + 80,
                                                    menuW, 300), window_flags))
    {
        nk_layout_row_dynamic( ctx, 93, 1);
        if (nk_button_label(ctx, "Next Camera")) {
            nextCamera();
        }
        
        if (nk_button_label(ctx, "SFX Jump")) {
            printf("Button 2 pressed...\n");
            soloud.play( sfxJump );
        }
        
        nk_layout_row_dynamic( ctx, 50, 1);
        if (nk_checkbox_label(ctx, "Music", &musicPlaying ) ) {
            printf("Music Toggled: %s\n", musicPlaying?"ON":"OFF");
            if (!musicPlaying) {
                music.stop();
            } else {
                soloud.play( music );
            }
        }
        
    }
    nk_end(ctx);

}

void CardFishApp::interfaceGame( nk_context* ctx )
{
    
    struct nk_panel layout;
    static nk_flags window_flags = 0;
    
    
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    
    nk_style_set_font(ctx, &(uiAssets->font_30->handle));
    ctx->style.text.color = nk_rgb(255,255,255);
    
    uiAssets->buttonStyleNormal(ctx);
    
    if (nk_begin(ctx, &layout, "game", nk_rect( uiAssets->fbWidth - 150,
                                                uiAssets->fbHeight - 110,
                                                    100, 100), window_flags))
    {
        // Draw pile
        nk_layout_row_dynamic( ctx, 40, 1);
        char buff[20];
        sprintf( buff, "%d", game.deck.Size() );
        nk_label(ctx, buff, NK_TEXT_CENTERED );
    }
    nk_end(ctx);

}

void CardFishApp::interfaceScreens( Tapnik::UIAssets *uiAssets )
{
    // Draw UI Layer
    nk_context* ctx = NKUI::NewFrame();
    
    ctx->style.window.background = { 0, 0, 0, 0 };
    ctx->style.window.fixed_background.data.color = { 0, 0, 0, 0 };
    
    //interfaceTitle( ctx );
    interfaceGame( ctx );
    
    NKUI::Draw();
    
    // Note: clear happens in draw, extra clear is to workaround a bug, need to upgrade nuklear in NKUI to get the fix
    nk_clear(ctx);
}



