//------------------------------------------------------------------------------
//  LD45Nothing.cpp
//------------------------------------------------------------------------------

#ifdef __APPLE__
#include <unistd.h>
#endif

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
#include "LD45NothingApp.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

using namespace Oryol;
using namespace Tapnik;

// These get crated in LD45Nothing-Main
extern Oryol::Args OryolArgs;

//------------------------------------------------------------------------------
AppState::Code
LD45NothingApp::OnInit() {
    
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
    
	// Don't enable MSAA here because we're doing it in the main render texture pass

	const char* windowTitle = "LD45 Nothing";
	if (OryolArgs.HasArg("--fullscreen")) {
		gfxSetup = GfxSetup::Fullscreen(1280, 720, windowTitle);
	} else {
		gfxSetup = GfxSetup::Window(1280, 720, windowTitle );
	}
    Gfx::Setup(gfxSetup);

	renderPassMultisample = Gfx::QueryFeature(GfxFeature::MSAARenderTargets) ? 4 : 1;
	//renderPassMultisample = 8;

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

		//soloud.play( music );
		musicPlaying = 0;
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
    
    // Load the scenes
    gameScene = Memory::New<Scene>();
    gameScene->Setup( &gfxSetup, renderPassMultisample );
    
    gameScene->LoadScene( "ld45nothing",[this](bool success) {
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

	renderizer = Memory::New<Renderizer>( gameScene->meshLayout, &gfxSetup, renderPassMultisample);
    
    dbgDraw = new DebugDrawRenderer();
    dbgDraw->Setup( renderizer->mainRenderSetup );
    dd::initialize( dbgDraw );    
    
    this->lastTimePoint = Clock::Now();
    this->startTimePoint = this->lastTimePoint;
    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
LD45NothingApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}



//------------------------------------------------------------------------------
AppState::Code
LD45NothingApp::OnRunning() {
    
    // Update clocks
    frameDtRaw = Clock::LapTime(this->lastTimePoint);
    
    activeCamera = debugMode?&dbgCamera:&gameCamera;
    
	renderizer->activeCamera = activeCamera;

    //updateGameCamera();
    
    // Do update loop
    Oryol::Duration frameDt = Oryol::Duration::FromSeconds( 1.0/60.0 );
    
    
    leftoverTime += frameDtRaw;
    while ( leftoverTime > frameDt )
    {
        this->fixedUpdate( frameDt );
        leftoverTime -= frameDt;
    }
    
	this->dynamicUpdate(frameDtRaw);

    //this->lateUpdate();
        
    // === Finalize Transforms
    this->finalizeTransforms( activeCamera->ViewProj );
    
    dbgDraw->debugDrawMVP = activeCamera->ViewProj;
    dbgDraw->debugDrawOrthoMVP = glm::ortho(0.0f, uiAssets->fbWidth, uiAssets->fbHeight, 0.0f );
    //Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    
    if (debugMode)
    {
        this->handleInputDebug();
    }

	// Maybe move this to debug tools??
	if (Input::KeyDown(Key::P)) {
		renderizer->debugDrawShadowMap = !renderizer->debugDrawShadowMap;
	}
	else if (Input::KeyDown(Key::L)) {
		renderizer->drawMainScene = !renderizer->drawMainScene;
	}
    
	// render on frame
	this->draw();

        const ddVec3 boxColor  = { 0.0f, 0.8f, 0.8f };
        //const ddVec3 boxCenter = { 0.0f, 0.0f, 0.0f };
		glm::vec3 boxCenter = glm::vec3(0.0f);

		if (civGame) {
			boxCenter = civGame->groundCursor;
		}

        float boxSize = 1.0f;
        dd::box( glm::value_ptr(boxCenter), boxColor, boxSize, boxSize, boxSize );
        dd::cross( glm::value_ptr(boxCenter), 1.0f);

		const ddVec3 Up = { 0.0f, 0.0f, 1.0f };
		dd::circle(glm::value_ptr(boxCenter), Up, dd::colors::Cornsilk, 2.0f, 20);

		// debug hex ids
		if (civGame)
		{
			for (int j = 0; j < BOARD_SZ; j++) {
				for (int i = 0; i < BOARD_SZ; i++) {
					GameHex* hex = civGame->boardHex(i, j);
					if (hex) {
						char buff[10];
						sprintf(buff, "%d,%d", i, j);

						ddVec3 textPos2D;
						textPos2D[0] = hex->screenPos.x - 20;
						textPos2D[1] = hex->screenPos.y - 15;
						textPos2D[2] = 0.0f;
						
						dd::screenText(buff, textPos2D, dd::colors::White);
					}
				}
			}
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

	if (Input::KeyDown(Key::Escape)|| Input::KeyDown(Key::Q)) {
		Log::Info("Request Quit...\n");
		requestQuit();
	}

	// Flush debug draws (as part of the main pass)
	Oryol::Duration appTime = lastTimePoint.Since(startTimePoint);
	dd::flush(appTime.AsMilliSeconds());
			
	renderizer->finishMainPass();
    
    
    // Do UI
    if (this->uiAssets->fontValid) {
        this->interfaceScreens( uiAssets );
    }
    
    Dbg::DrawTextBuffer();
 
	renderizer->finishRender( uiAssets );

    Gfx::CommitFrame();
    
    // continue running or quit?
    return (App::quitRequested || Gfx::QuitRequested()) ? AppState::Cleanup : AppState::Running;
}

void
LD45NothingApp::handleInputDebug() {
    
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

// =======================================================================================
void LD45NothingApp::onSceneLoaded()
{
    SceneCamera cam = gameScene->findNamedCamera( "Camera" );
    if (cam.index >=0) {
        activeCameraIndex = cam.index;
    }
    gameCamera.UpdateModel( cam.mat );
    gameCamera.UpdateProj(glm::radians(45.0f), uiAssets->fbWidth, uiAssets->fbHeight, 0.01f, 100.0f);

	// TODO: title screen here
	StartGame();

}
void LD45NothingApp::fixedUpdate( Oryol::Duration fixedDt )
{
	if (civGame) {
		civGame->fixedUpdate(fixedDt, activeCamera );
	}
}

void LD45NothingApp::dynamicUpdate( Oryol::Duration frameDt )
{
	if (civGame) {
		civGame->dynamicUpdate(frameDt, activeCamera);
	}
}

void LD45NothingApp::finalizeTransforms( glm::mat4 matViewProj )
{
	glm::mat4 shadowMVP = renderizer->shadowCamera.ViewProj;
    gameScene->finalizeTransforms( matViewProj, shadowMVP );
}

void LD45NothingApp::draw()
{

    //gameScene->drawScene();
	renderizer->renderScene(gameScene, uiAssets);
}

void LD45NothingApp::nextCamera()
{
    activeCameraIndex++;
    
    if (activeCameraIndex >= gameScene->sceneCams.Size() ) {
        activeCameraIndex = 0;
    }
    
    SceneCamera cam = gameScene->sceneCams[activeCameraIndex];
    Log::Info("Camera: %s\n", cam.cameraName.AsCStr() );
    
    gameCamera.UpdateModel( cam.mat );
}

void
LD45NothingApp::StartGame()
{
	o_assert(civGame == NULL);
	civGame = Memory::New<CivGame>();
	civGame->uiAssets = uiAssets;
	civGame->SetupWithScene(gameScene);
}

void LD45NothingApp::interfaceScreens( Tapnik::UIAssets *uiAssets )
{
    // Draw UI Layer
    nk_context* ctx = NKUI::NewFrame();
    
    //ctx->style.button.normal.data.color = nk_rgb( 20, 80, 20 );
    //ctx->style.button.hover.data.color = nk_rgb( 100, 150, 100 );
    //ctx->style.window.fixed_background = nk_style_item_image( uiAssets->img_frame );
    
    ctx->style.window.background = { 0, 0, 0, 0 };
    ctx->style.window.fixed_background.data.color = { 0, 0, 0, 0 };
    
    
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
    //float menuMarg = (uiAssets->fbWidth - menuW) / 2;
	float menuMarg = 20;
    
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    
    nk_style_set_font(ctx, &(uiAssets->font_30->handle));
    
    uiAssets->buttonStyleNormal(ctx);
    
    if (nk_begin(ctx, //&layout,
                 "main_menu", nk_rect( menuMarg, 30 + titleH + 80,
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
    
    NKUI::Draw();
    
    // Note: clear happens in draw, extra clear is to workaround a bug, need to upgrade nuklear in NKUI to get the fix
    nk_clear(ctx);
}



