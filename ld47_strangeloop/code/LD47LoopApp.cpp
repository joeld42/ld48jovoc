//------------------------------------------------------------------------------
//  LD47LoopApp.cpp
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
#include "glm/gtc/random.hpp"
#include "glm/gtx/closest_point.hpp"
#include "shaders.h"


#include "IO/IO.h"
#if ORYOL_EMSCRIPTEN
#include "HttpFS/HTTPFileSystem.h"
#else
#include "LocalFS/LocalFileSystem.h"
#endif

#include "Assets/Gfx/TextureLoader.h"

#include "Camera.h"
#include "SceneObject.h"
#include "LD47LoopApp.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

using namespace Oryol;
using namespace Tapnik;

// These get crated in LD47Loop-Main
extern Oryol::Args OryolArgs;

//------------------------------------------------------------------------------
AppState::Code
LD47LoopApp::OnInit() {

	// Get settings from config file
	GameSettings defaultConfig = GameSettings::DefaultHighSpecSettings();
	cfg = GameSettings::FromConfigFileWithDefaults(defaultConfig, "config.ini");

	// set up IO system
	IOSetup ioSetup;
#if ORYOL_EMSCRIPTEN
	ioSetup.FileSystems.Add("http", HTTPFileSystem::Creator());
	ioSetup.Assigns.Add("gamedata:", "http://localhost:8000/gamedata/");
#else
	ioSetup.FileSystems.Add("file", LocalFileSystem::Creator());
	ioSetup.Assigns.Add("gamedata:", "root:../Resources/gamedata/");
	//ioSetup.Assigns.Add("gamedata:", "cwd:gamedata/");
#endif

	IO::Setup(ioSetup);
    
    gameStatus = GAME_TitleScreen;

	// Don't enable MSAA here because we're doing it in the main render texture pass

	const char* windowTitle = "LD47 Loop";
	if (OryolArgs.HasArg("--fullscreen")) {
		gfxSetup = GfxSetup::Fullscreen(1280, 720, windowTitle);
	}
	else {
		gfxSetup = GfxSetup::Window(1280, 720, windowTitle);
	}
	Gfx::Setup(gfxSetup);

	if (!Gfx::QueryFeature(GfxFeature::MSAARenderTargets)) {
		cfg.renderPassMultisample = 1;
	}

    this->uiAssets = Memory::New<UIAssets>();
    this->uiAssets->SetupUI();
    uiAssets->fbWidth = Gfx::DisplayAttrs().FramebufferWidth;
    uiAssets->fbHeight = Gfx::DisplayAttrs().FramebufferHeight;
    uiAssets->displayScale = uiAssets->fbWidth / 1280.0f;

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
		//soloud.setVolume(hmusic, settings->musicVolume);
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
    
    //dbgCamera = {};
    //dbgCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f),
    //                uiAssets->fbWidth,
    //                uiAssets->fbHeight, 0.01f, 100.0f);
    
    // Load the scenes
    gameScene = Memory::New<Scene>();
    gameScene->Setup( &gfxSetup, cfg.renderPassMultisample );
    
    gameScene->LoadScene( "ld47loop",[this](bool success) {
        Log::Info("Loaded game scene success: %s\n", success?"true":"false" );
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

	renderizer = Memory::New<Renderizer>( gameScene->meshLayout, &gfxSetup, cfg.renderPassMultisample );
    
    dbgDraw = new DebugDrawRenderer();
    dbgDraw->Setup( renderizer->mainRenderSetup );
    dd::initialize( dbgDraw );    

	// Setup debug mode tools
	debugTools = Memory::New<DebugTools>();
    
    this->lastTimePoint = Clock::Now();
    this->startTimePoint = this->lastTimePoint;

	debugTools->Setup(uiAssets );
	Log::Info("set up debugTool\n");
    
    
    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
LD47LoopApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}


//------------------------------------------------------------------------------
AppState::Code
LD47LoopApp::OnRunning() {
    
    // Update clocks
    frameDtRaw = Clock::LapTime(this->lastTimePoint);
    
	Camera* activeCamera = debugTools->debugMode ? &(debugTools->dbgCamera) : &gameCamera;
    
	renderizer->activeCamera = activeCamera;

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
    this->finalizeTransforms( activeCamera->ViewProj );
    
    dbgDraw->debugDrawMVP = activeCamera->ViewProj;
    dbgDraw->debugDrawOrthoMVP = glm::ortho(0.0f, uiAssets->fbWidth, uiAssets->fbHeight, 0.0f );
    //Gfx::ApplyDefaultRenderTarget(this->mainClearState);
    
	if ((debugTools) && (debugTools->debugMode))
	{
		debugTools->handleInputDebug();
    } else {
        handleGameInput();
    }

	// Maybe move this to debug tools??
	if (Input::KeyDown(Key::P)) {
		renderizer->debugDrawShadowMap = !renderizer->debugDrawShadowMap;
	}
    
//    if (Input::KeyDown(Key::V)) {
//        ClearCheckVal();
//    }
    
    
    if ((cheatsEnabled) && (Input::KeyDown(Key::B)) ) {
        for (int i=0; i < puzzleEdge.Size(); i++) {
            if (puzzleEdge[i].status != EDGE_Premarked) {
                puzzleEdge[i].status = puzzleEdge[i].loopEdge?EDGE_Marked : EDGE_Blank;
            }
        }
    }
    
    // Find the cell under the mouse
    FindHoverCell();
    
	// render on frame
	this->draw();

//        const ddVec3 boxColor  = { 0.0f, 0.8f, 0.8f };
//        const ddVec3 boxCenter = { 0.0f, 0.0f, 0.0f };
//        float boxSize = 1.0f;
//        dd::box(boxCenter, boxColor, boxSize, boxSize, boxSize );
//        dd::cross(boxCenter, 1.0f);
    
    if (Input::KeyDown(Key::Tab)) {
		Log::Info("Toggle debug mode...\n");
		debugTools->ToggleDebugMode();
        //Log::Info( "Debug Mode: %s\n", debugMode?"ON":"OFF" );
    }
    
    // Draw the loop edges
    if (dbgShowLine)
    {
        const ddVec3 loopColor  = { 1.0f, 0.2f, 1.0f };
        for (int i=0; i < puzzleEdge.Size(); i++) {
            //dd::cross(boxCenter, 1.0f);
            LoopEdge &edge = puzzleEdge[i];
            if (edge.loopEdge) {
                
                dd::line( glm::value_ptr( puzzlePos[ edge.pA ].pos * 1.05f ),
                          glm::value_ptr( puzzlePos[ edge.pB ].pos * 1.05f ),
                         loopColor );
            }
        }
    }

    if (cheatsEnabled) {
        ddVec3 textPos2D = { 0.0f, 20.0f, 0.0f };
        textPos2D[0] = uiAssets->fbWidth / 2.0;
        dd::screenText( "Cheats Enabled", textPos2D, dd::colors::Orange );
    }
    
	if (debugTools->debugMode) {
        ddVec3 textPos2D = { 0.0f, 20.0f, 0.0f };
        textPos2D[0] = uiAssets->fbWidth / 2.0;
        
        if (puzzleMeshes.Size() > 0)
        {
            //Oryol::String puzMeshName = puzzleMeshes[selectedPuzzleIndex]->meshName;
            //dd::screenText( puzMeshName.AsCStr(), textPos2D, dd::colors::Orange );
        }
    }

	if (Input::KeyDown(Key::Escape)|| Input::KeyDown(Key::Q)) {
		Log::Info("Request Quit...\n");
		requestQuit();
	}
    
    if (Input::KeyDown(Key::N0)) {
        cheatsEnabled = !cheatsEnabled;
    }
    
//    if (Input::KeyDown(Key::N9)) {
//        Log::Info("-----------------------\n");
//        if (GenPuzzleStep()) {
//        } else {
//            Log::Info("GEN STEP: Puzzle is compled...\n");
//            GenPuzzleFinish();
//        }
//        UpdateCellData();
//    }
//
//    if (Input::KeyDown(Key::N8)) {
//        Log::Info("----Gen Whole puzzle-------------------\n");
//        GenPuzzleStart();
//        bool moreSteps = true;
//        while (moreSteps) {
//            moreSteps = GenPuzzleStep();
//        }
//        GenPuzzleFinish();
//        UpdateCellData();
//        Log::Info("GENPuz: Puzzle is compled...\n");
//
//    }
    
//    if ((Input::KeyDown(Key::J)) && (objPuzzle == NULL) ) {
//
//        SetupPuzzle( puzzleMeshes[selectedPuzzleIndex ] );
//
//    }
    
    if ((Input::KeyDown(Key::H)) && (objPuzzle != NULL) && (hoverCell >= 0) )
    {
        Log::Info("DBG: hovercell %d\n", hoverCell );
        
        // Is this cell a neigbor of cell I?
        LoopCell &cell = puzzleCells[hoverCell];
        for (int j=0; j < puzzleCells.Size(); j++) {
            LoopCell &otherCell = puzzleCells[j];
            
            Log::Info("Nbrs says %d %d %d %d\n", cell.nbrAB, cell.nbrBD, cell.nbrAC, cell.nbrCD );
            Log::Info("Will check nbr %d %d --> %d %d %d %d\n",
                      cell.pA, cell.pB,
                      otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD );
            
//            if (checkNbr( cell.pA, cell.pB, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
//                cell.nbrAB = j;
//            } else if (checkNbr( cell.pB, cell.pD, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
//                cell.nbrBD = j;
//            } else if (checkNbr( cell.pA, cell.pC, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
//                cell.nbrAC = j;
//            } else if (checkNbr( cell.pC, cell.pD, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
//                cell.nbrCD = j;
//            }
        }
    }

    if (Input::KeyDown(Key::Y)) {
        selectedPuzzleIndex++;
        if (selectedPuzzleIndex >= puzzleMeshes.Size()) {
            selectedPuzzleIndex = 0;
        }
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
 
	if (debugTools) {
		debugTools->OnRunning(frameDt);

		debugTools->DebugRender();
	}

	renderizer->finishRender( uiAssets );

	
    Gfx::CommitFrame();
    
    // continue running or quit?
    return (App::quitRequested || Gfx::QuitRequested()) ? AppState::Cleanup : AppState::Running;
}

glm::vec2 remapTileST( glm::vec2 orig, int tileNdx )
{
    float tileScale = 1.0/8.0f;
    int tx = tileNdx % 8;
    int ty = tileNdx / 8;
    glm::vec2 tileSt = orig;
    return glm::vec2( float(tx) + tileSt.x, float(ty) + tileSt.y ) * tileScale;
}

float valForEdgeCode( LoopEdge edge )
{
    switch ( edge.status ) {
        case EDGE_Blank: return 0.0f; break;
        case EDGE_Marked: return 1.0f; break;
        case EDGE_Premarked:
            return edge.loopEdge?2.0f:3.0f;
            break;
        case EDGE_Forbid: return 3.0f; break;
        default: return 0.0f; break;
    }
}

void
LD47LoopApp::UpdateCellData()
{
    cellDataNeedsUpdate = true;
}

void
LD47LoopApp::doUpdateCellData()
{
    if (!cellDataNeedsUpdate) {
        return;
    }
    cellDataNeedsUpdate = false;
    
    // Don't do this until the puzzle object has been initialized
    if (!objPuzzle) return;
    
    // Test update mesh
    for (int i=0; i < puzzleCells.Size(); i++) {
//        glm::vec4 cellCode = glm::vec4( glm::linearRand(0.0f, 0.1f), glm::linearRand(0.0f, 0.1f),
//                                        glm::linearRand(0.0f, 0.1f), glm::linearRand(0.0f, 0.1f)  );        
        LoopCell &cell = puzzleCells[i];
        
        glm::vec4 cellCode( 0,0,0,0 );
        if (cell.edgeAB >=0) {
            cellCode.x = valForEdgeCode( puzzleEdge[cell.edgeAB] );
        }
        if (cell.edgeAC >=0) {
            cellCode.y = valForEdgeCode( puzzleEdge[cell.edgeAC] );
        }
        if (cell.edgeCD >=0) {
            cellCode.z = valForEdgeCode( puzzleEdge[cell.edgeCD] );
        }
        if (cell.edgeBD >=0) {
            cellCode.w = valForEdgeCode( puzzleEdge[cell.edgeBD] );
        }
        
        
//        if (cell.fillStatus == FILL_Outside) {
//            cellCode = glm::vec2( 0.0f, 0.0f );
////        } else if (cell.fillStatus == FILL_Grey) {
////            cellCode = glm::vec2( 0.3f, 0.3f );
//        } else if (cell.fillStatus == FILL_Inside) {
//            cellCode = glm::vec2( 1.0f, 1.0f );
//        }
        
        for (int j=0; j < puzzleCells[i].numTri; j++) {
            uint16_t *tri = objPuzzle->mesh->meshIndex + (puzzleCells[i].tri[j]*3);
            LDJamFileVertex *vertA = objPuzzle->mesh->meshVertData + tri[0];
            LDJamFileVertex *vertB = objPuzzle->mesh->meshVertData + tri[1];
            LDJamFileVertex *vertC = objPuzzle->mesh->meshVertData + tri[2];
            
            vertA->edgemask = cellCode;
            vertB->edgemask = cellCode;
            vertC->edgemask = cellCode;
            
            // Update the base STs based on the neighbor count
            int tileNdx = puzzleCells[i].hidden?4:puzzleCells[i].loopEdgeCount;
            
            
            // TODO: make count 4 if cell is "hidden"
            vertA->m_st0 = remapTileST( origMeshST[ tri[0] ], tileNdx );
            vertB->m_st0 = remapTileST( origMeshST[ tri[1] ], tileNdx );
            vertC->m_st0 = remapTileST( origMeshST[ tri[2] ], tileNdx );
            
            glm::vec2 checkVal2 = glm::vec2( cell.checkVal, cell.checkVal );
            vertA->m_st1 = checkVal2;
            vertB->m_st1 = checkVal2;
            vertC->m_st1 = checkVal2;
        }
        
        
    }
    Gfx::UpdateVertices( objPuzzle->mesh->mesh, objPuzzle->mesh->meshVertData,
                        sizeof(LDJamFileVertex) * objPuzzle->mesh->numVerts );
}

float cross2d( const glm::vec2 &a, const glm::vec2 &b )
{
    return (a.x*b.y) - (a.y*b.x);
}

bool pointInTri( glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c )
{
    glm::vec2 ab = b - a;
    glm::vec2 bc = c - b;
    glm::vec2 ca = a - c;
    float cp1 = cross2d( ab, p - a );
    float cp2 = cross2d( bc, p - b );
    float cp3 = cross2d( ca, p - c );
    if ((cp1 <= 0.0f) && (cp2 <= 0.0f) && (cp3 <= 0.0f)) {
        return true;
    }
    return false;
}

int
LD47LoopApp::FindOrCreatePoint( glm::vec3 p )
{
    for (int i=0; i < puzzlePos.Size(); i++ )
    {
        float d = glm::distance( p, puzzlePos[i].pos );
        if ( d < 0.01) {
            return i;
        }
    }
    
    // Didn't find it, add it
    int addNdx = puzzlePos.Size();
    LoopPoint lp = {};
    lp.pos = p;
    puzzlePos.Add( lp );
    
    return addNdx;
}

int
LD47LoopApp::FindOrCreateEdge( int pA, int pB )
{
    // pA is lowest point ndx
    if (pA > pB) {
        int t = pA;
        pA = pB;
        pB = t;
    }
    
    int edgeNdx = FindEdge( pA, pB );
    if (edgeNdx == -1) {
        LoopEdge edge = {};
        edge.pA = pA;
        edge.pB = pB;
        edge.status = EDGE_Blank;
        edgeNdx = puzzleEdge.Size();
        puzzleEdge.Add( edge );
    }
    
    return edgeNdx;
}

int
LD47LoopApp::FindEdge( int pA, int pB )
{
    // pA is lowest point ndx
    if (pA > pB) {
        int t = pA;
        pA = pB;
        pB = t;
    }
    
    for (int i=0; i < puzzleEdge.Size(); i++) {
        LoopEdge &edge = puzzleEdge[i];
        if ((edge.pA == pA) && (edge.pB == pB))
        {
            return i;
        }
    }
    return -1;
}

void
LD47LoopApp::FindHoverCell()
{
    glm::vec2 mousePos = Input::MousePosition()  * uiAssets->displayScale;
    
    ddVec3 textPos2D = { 0.0f, 20.0f, 0.0f };
    textPos2D[0] = mousePos.x;
    textPos2D[1] = mousePos.y;
    //textPos2D[0] = uiAssets->fbWidth / 2.0;
//    dd::screenText("MOUSE", textPos2D, dd::colors::Orange );
                         
    int foundHoverCell = -1;
    float bestDist = 5000.0f;
    
    for (int i=0; i < puzzleCells.Size(); i++) {
        for (int j=0; j < puzzleCells[i].numTri; j++) {
            uint16_t *tri = objPuzzle->mesh->meshIndex + (puzzleCells[i].tri[j]*3);
            LDJamFileVertex *vertA = objPuzzle->mesh->meshVertData + tri[0];
            LDJamFileVertex *vertB = objPuzzle->mesh->meshVertData + tri[1];
            LDJamFileVertex *vertC = objPuzzle->mesh->meshVertData + tri[2];

            // this ignores the puzzle object's transform, which is OK because it's zero
            glm::vec3 screenA = renderizer->activeCamera->worldPointToScreen2( vertA->m_pos );
            glm::vec3 screenB = renderizer->activeCamera->worldPointToScreen2( vertB->m_pos );
            glm::vec3 screenC = renderizer->activeCamera->worldPointToScreen2( vertC->m_pos );
            
            float triAvgZ = (screenA.z + screenB.z + screenC.z) / 3.0f;
            
            if (pointInTri( mousePos, screenA, screenB, screenC) && (triAvgZ < bestDist)) {
                bestDist = triAvgZ;
                foundHoverCell = i;
            }
        }
    }
    
    if (foundHoverCell != hoverCell) {
        Log::Info("hover changed, was %d now %d\n", hoverCell, foundHoverCell );
        hoverCell = foundHoverCell;
        
        if (hoverCell >= 0) {
            LoopCell &cell = puzzleCells[hoverCell];
            //printf("neighbors %d %d %d %d\n", cell.nbrAB, cell.nbrAC, cell.nbrBD, cell.nbrCD );
            Log::Info("Cell has %d loop edges\n", cell.loopEdgeCount );
        }
        
        UpdateCellData();
    } else {
        // hover did not change
        //printf("No hover, closest dist was %f\n", bestDist );
    }
}

void LD47LoopApp::ClearCheckVal()
{
    for (int i=0; i < puzzleCells.Size(); i++) {
        puzzleCells[i].checkVal = 0.0f;
    }
    UpdateCellData();
}

float float_min( float a, float b)
{
    return (a<b)?a:b;
}

bool checkSt( glm::vec2 st, float ss, float tt )
{
    const float e = 0.01f;
    if ( (fabs(st.x - ss) < e) && (fabs(st.y - tt) < e)) {
        return true;
    } else {
        return false;
    }
}

bool checkNbrEdge( int a1, int b1, int a2, int b2)
{
    if ( ((a1==a2) && (b1==b2)) ||
         ((a1==b2) && (b1==a2)) ) {
        return true;
    }
    return false;
}

bool checkNbr( int a, int b, int a2, int b2, int c2, int d2 )
{
    if (checkNbrEdge( a, b, a2, b2) ||
        checkNbrEdge( a, b, b2, d2) ||
        checkNbrEdge( a, b, c2, d2) ||
        checkNbrEdge( a, b, a2, c2) ) {
        return true;
    } else {
        return false;
    }
}

void
LD47LoopApp::SetupPuzzle( Tapnik::SceneMesh *mesh )
{
    
    
    
    // Make a puzzle object
    objPuzzle = gameScene->spawnObject( mesh );
    
    // Make a copy of the mesh STs to refer to after we modify them
    origMeshST.Clear();
    origMeshST.Reserve( objPuzzle->mesh->numVerts );
    for (int i=0; i < objPuzzle->mesh->numVerts; i++ ) {
        LDJamFileVertex *vert = objPuzzle->mesh->meshVertData + i;
        origMeshST.Add( vert->m_st0 );
    }
    
    // Initialize puzzle cells
    puzzleCells.Clear();
    puzzleCells.Reserve( 1000 );
    for (uint32_t i=0; i < objPuzzle->mesh->numTris; i++) {
        uint16_t *tri = objPuzzle->mesh->meshIndex + (i*3);
        LDJamFileVertex *vertA = objPuzzle->mesh->meshVertData + tri[0];
        LDJamFileVertex *vertB = objPuzzle->mesh->meshVertData + tri[1];
        LDJamFileVertex *vertC = objPuzzle->mesh->meshVertData + tri[2];
        
        // Average the STs to find their Cell
        float stA = vertA->m_st0.x;
        float stB = vertB->m_st0.x;
        float stC = vertC->m_st0.x;
        float stAvg = (stA + stB + stC) / 3.0f;
        
        if (stAvg < 0.0) continue;
        
        int cellNdx = floor( stAvg );
        assert( cellNdx < 1000 );
        //Log::Info("Cell is %d\n" , cellNdx );
        LoopCell dummy = {};
        while (puzzleCells.Size() < cellNdx+1) {
            puzzleCells.Add(dummy);
        }
        LoopCell &cell = puzzleCells[cellNdx];
        if (cell.numTri < MAX_TRIS_FOR_CELL)
        {
            assert( cell.numTri < MAX_TRIS_FOR_CELL);
            cell.tri[cell.numTri++] = i;
        }
    }
    
    // Now we've collected the triangles into cells. Go through them and find the corners
    for (int i=0; i < puzzleCells.Size(); i++) {
        //printf("Cell %d has %d tris\n", i, puzzleCells[i].numTri );
        LoopCell &cell = puzzleCells[i];
        
        // First, find the minimum ST in the cell (really we only need the S)
        glm::vec2 minSt = glm::vec2( 999999.0f, 9999999.0f );
        for (int j=0; j < cell.numTri; j++) {
            for (int k=0; k < 3; k++) {
                uint16_t *tri = objPuzzle->mesh->meshIndex + (cell.tri[j]*3+k);
                LDJamFileVertex *vert = objPuzzle->mesh->meshVertData + (*tri);
                
                //printf("v %f %f\n", vert->m_st0.x, vert->m_st0.y );
                minSt = glm::vec2( float_min( minSt.x, vert->m_st0.x), float_min( minSt.y, vert->m_st0.y) );
            }
        }
        
        // Now find the corners
        for (int j=0; j < cell.numTri; j++) {
            for (int k=0; k < 3; k++) {
                int ndx = (cell.tri[j]*3+k);
                uint16_t *tri = objPuzzle->mesh->meshIndex + ndx;
                LDJamFileVertex *vert = objPuzzle->mesh->meshVertData + (*tri);
                
                glm::vec2 cellSt = vert->m_st0 - minSt;
                origMeshST[ *tri ] = cellSt; // HACK save the modified ST so we can remap it
                
                if ( checkSt( cellSt, 0.0, 0.0) ) {
                    cell.pA = FindOrCreatePoint( vert->m_pos );
                } else if ( checkSt( cellSt, 1.0, 0.0) ) {
                    cell.pB = FindOrCreatePoint( vert->m_pos );
                } else if ( checkSt( cellSt, 0.0, 1.0) ) {
                    cell.pC = FindOrCreatePoint( vert->m_pos );
                } else if ( checkSt( cellSt, 1.0, 1.0) ) {
                    cell.pD = FindOrCreatePoint( vert->m_pos );
                }
            }
        }
        
        //Log::Info("Cell %d (%d) corners %d %d %d %d minSt %f %f\n", i, cell.numTri, cell.pA, cell.pB, cell.pC, cell.pD, minSt.x, minSt.y );
    }
    
    // Find the neighbors for each cell
    for (int i=0; i < puzzleCells.Size(); i++) {
        LoopCell &cell = puzzleCells[i];
    
        cell.nbrAB = -1;
        cell.nbrBD = -1;
        cell.nbrAC = -1;
        cell.nbrCD = -1;
        
        for (int j=0; j < puzzleCells.Size(); j++) {
            if (i==j) continue;
            
            // Is this cell a neigbor of cell I?
            LoopCell &otherCell = puzzleCells[j];
            if (checkNbr( cell.pA, cell.pB, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
                cell.nbrAB = j;
            } else if (checkNbr( cell.pB, cell.pD, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
                cell.nbrBD = j;
            } else if (checkNbr( cell.pA, cell.pC, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
                cell.nbrAC = j;
            } else if (checkNbr( cell.pC, cell.pD, otherCell.pA, otherCell.pB, otherCell.pC, otherCell.pD )) {
                cell.nbrCD = j;
            }
        }
        
    }
    
    // Now add the edges
    puzzleEdge.Clear();
    for (int i=0; i < puzzleCells.Size(); i++) {
        LoopCell &cell = puzzleCells[i];
        FindOrCreateEdge( cell.pA, cell.pB );
        FindOrCreateEdge( cell.pA, cell.pC );
        FindOrCreateEdge( cell.pB, cell.pD );
        FindOrCreateEdge( cell.pC, cell.pD );
    }
    
    // Setup the game camera for the new mesh
    float meshRadius = 0.1f;
    for (uint32_t i=0; i < objPuzzle->mesh->numVerts; i++) {
        LDJamFileVertex *vert = objPuzzle->mesh->meshVertData + i;
        float d = glm::length( vert->m_pos);
        if (d > meshRadius) {
            meshRadius = d;
        }
    }
    printf("mesh radius is %f\n", meshRadius );
    glm::vec3 camPos = glm::vec3(0.0, -meshRadius * 3, 0.0);
    gameCamera.Setup( camPos, glm::radians(45.0f),
                     uiAssets->fbWidth, uiAssets->fbHeight, 0.1f, 1000.0f);
    
    glm::mat4 modelCam = glm::translate(glm::mat4(), camPos);
    glm::mat4 modelCam2 = glm::rotate( modelCam, glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ));
    gameCamera.UpdateModel( modelCam2 );
    
    arcball.setup( camPos, glm::vec3( 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ));
    debugTools->debugMode = false;
    
}

void
LD47LoopApp::GenPuzzleStart()
{
    for (int i=0; i < puzzlePos.Size(); i++) {
        puzzlePos[i].faceCount = 0;
    }
    
    for (int i=0; i < puzzleCells.Size(); i++) {
        puzzleCells[i].fillStatus = FILL_Outside;
        puzzleCells[i].loopEdgeCount = 0;
    }
    
    for (int i=0; i < puzzleEdge.Size(); i++) {
        puzzleEdge[i].loopEdge = false;
    }
    
    // Now pick a random face to be "inside"
    int ndx = glm::linearRand( 0, puzzleCells.Size()-1 );
    puzzleCells[ndx].fillStatus = FILL_Inside;
    genPuzzleMarkCell(ndx);
}

// dumb
bool isSafe( int num, int safe1, int safe2, int safe3, int safe4 )
{
    return ((num==safe1)||(num==safe2)||(num==safe3)||(num==safe4));
}

bool
LD47LoopApp::checkFaceCount( int ndx, int safe1, int safe2, int safe3, int safe4 )
{
//    if  (!( isSafe(ndx, safe1, safe2, safe3, safe4) || (puzzlePos[ndx].faceCount==0)) ) {
//        printf("checkFaceCount failed, ndx %d (fc %d)\n", ndx, puzzlePos[ndx].faceCount );
//    }
    return ( isSafe(ndx, safe1, safe2, safe3, safe4) || (puzzlePos[ndx].faceCount==0));
}

// Count this as a neighbor?
int
LD47LoopApp::genPuzzleCheckNbr( int nbrNdx, int ourCellNdx )
{
    if (nbrNdx == -1) return 0;
    LoopCell &nbrCell = puzzleCells[nbrNdx];
    if (puzzleCells[nbrNdx].fillStatus == FillStatus::FILL_Inside) {

        // Now see if the neighbor cell has any corners in use
        // but not by the from cell because we came from there
        LoopCell &ourCell = puzzleCells[ourCellNdx];
        if ( (!checkFaceCount( ourCell.pA,  nbrCell.pA, nbrCell.pB, nbrCell.pC, nbrCell.pD )) ||
             (!checkFaceCount( ourCell.pB,  nbrCell.pA, nbrCell.pB, nbrCell.pC, nbrCell.pD )) ||
             (!checkFaceCount( ourCell.pC,  nbrCell.pA, nbrCell.pB, nbrCell.pC, nbrCell.pD )) ||
             (!checkFaceCount( ourCell.pD,  nbrCell.pA, nbrCell.pB, nbrCell.pC, nbrCell.pD )) ) {

//            printf("checkFace failed into %d, pndx %d %d %d %d (safes %d %d %d %d)\n",
//                   ourCellNdx,
//                   nbrCell.pA, nbrCell.pB,  nbrCell.pC, nbrCell.pD,
//                   ourCell.pA, ourCell.pB, ourCell.pC, ourCell.pD );

            // Uh oh, this corner is already used (and not by where we're coming from), reject it
            return 2; // we're looking for exactly one total, so this will fail it
        }

        return 1;
    }
    return 0;
}

//int
//LD47LoopApp::genPuzzleCheckNbr( int nbrNdx )
//{
//    if (nbrNdx == -1) return 0;
//    LoopCell &nbrCell = puzzleCells[nbrNdx];
//    if (puzzleCells[nbrNdx].fillStatus == FillStatus::FILL_Inside) {
//        return 1;
//    }
//    return 0;
//}

void
LD47LoopApp::genPuzzleMarkCell( int cellNdx )
{
    LoopCell &cell = puzzleCells[cellNdx];
    
//    printf("Marking cell %d, ps %d %d %d %d\n",
//           cellNdx,
//           cell.pA, cell.pB, cell.pC, cell.pD );
    
    puzzlePos[ cell.pA ].faceCount++;
    puzzlePos[ cell.pB ].faceCount++;
    puzzlePos[ cell.pC ].faceCount++;
    puzzlePos[ cell.pD ].faceCount++;
}

bool
LD47LoopApp::GenPuzzleStep()
{
    Oryol::Array<int> candidateSteps;

    // Choose a random black cell with exactly 1 white cell
    for (int i=0; i < puzzleCells.Size(); i++) {
        LoopCell &cell = puzzleCells[i];
        if (cell.fillStatus == FillStatus::FILL_Inside) {
            continue;
        }
            
        // cell is not filled, see if it has exactly 1 neighbor
        int nbrCount1 = genPuzzleCheckNbr(cell.nbrAB, i );
//        printf("Count going into %d from %d is %d\n", cell.nbrAB, i, nbrCount1 );
        
        int nbrCount2 = genPuzzleCheckNbr(cell.nbrCD, i );
//        printf("Count going into %d from %d is %d\n", cell.nbrCD, i, nbrCount2 );
        
        int nbrCount3 = genPuzzleCheckNbr(cell.nbrBD, i );
//        printf("Count going into %d from %d is %d\n", cell.nbrBD, i, nbrCount3 );
        
        int nbrCount4 = genPuzzleCheckNbr(cell.nbrAC, i );
//        printf("Count going into %d from %d is %d\n", cell.nbrAC, i, nbrCount4 );
        
        int nbrCount = nbrCount1 +  nbrCount2 +  nbrCount3 +  nbrCount4;
        if (nbrCount == 1) {
//            printf("Adding %d as a possible step\n", i );
            candidateSteps.Add( i );
        }
    }
    
    if (candidateSteps.Size() == 0) {
        return false;
    }
//    Log::Info("%d Candidates\n", candidateSteps.Size() );
    
    int ndx = glm::linearRand( 0, candidateSteps.Size()-1 );
    int addCell = candidateSteps[ndx];
    puzzleCells[ addCell ].fillStatus = FILL_Inside;
    genPuzzleMarkCell( addCell );
    
    return true;
}

void
LD47LoopApp::CheckNbrEdge( int cellNdx, int nbrIndex, int eA, int eB )
{
    if (nbrIndex == -1) return;
    
    LoopCell &cell = puzzleCells[cellNdx];
    LoopCell &nbrCell = puzzleCells[nbrIndex];
    
    // Cells are inside/outside
    if ((nbrCell.fillStatus != cell.fillStatus))
    {
        int edgeNdx = FindEdge( eA, eB );
        assert( edgeNdx != -1 ); // should have an edge if there's a neighbor
        LoopEdge &edge = puzzleEdge[edgeNdx];
        edge.loopEdge = true;
        
        // TODO: Cache edgemask on cell
        cell.loopEdgeCount++;
    }
    
}

void
LD47LoopApp::revealEdge( int edgeNdx )
{
    if (edgeNdx >= 0) {
        puzzleEdge[edgeNdx].status = EDGE_Premarked;
    }
}

void
LD47LoopApp::GenPuzzleFinish()
{
    // go through the cells and mark the loop edges
    for (int i=0; i < puzzleCells.Size(); i++) {
        
        LoopCell &cell = puzzleCells[i];
        CheckNbrEdge( i, cell.nbrAB, cell.pA, cell.pB );
        CheckNbrEdge( i, cell.nbrBD, cell.pB, cell.pD );
        CheckNbrEdge( i, cell.nbrAC, cell.pA, cell.pC );
        CheckNbrEdge( i, cell.nbrCD, cell.pC, cell.pD );
        
        // Now cache the edge indices
        cell.edgeAB = FindEdge( cell.pA, cell.pB );
        cell.edgeBD = FindEdge( cell.pB, cell.pD );
        cell.edgeAC = FindEdge( cell.pA, cell.pC );
        cell.edgeCD = FindEdge( cell.pC, cell.pD );
        
        cell.checkVal = 0.0f;
    }
    
    // Initially clear the edge marks
    for (int i=0; i < puzzleEdge.Size(); i++) {
        puzzleEdge[i].status = EDGE_Blank;
        
        // (DBG: set them to loop edge)
        //puzzleEdge[i].marked = puzzleEdge[i].loopEdge;
    }
    
    // Hide some of the cells to make the puzzle harder
    float hidePercent = 0.25;
    if (puzdif == 1) {
        hidePercent = 0.3;
    } else if (puzdif == 2) {
        hidePercent = 0.4;
    }
    Oryol::Array<int> hideCells;
    for (int i=0; i < puzzleCells.Size(); i++) {
        hideCells.Add( i );
        puzzleCells[i].hidden = false;
    }
        
    int hideCutoff = (int)(hidePercent * (float)hideCells.Size());
    while (hideCells.Size() > hideCutoff) {
        int ndx = glm::linearRand( 0, hideCells.Size()-1 );
        hideCells.EraseSwapBack( ndx );
    }
    for (int i=0; i < hideCells.Size(); i++) {
        puzzleCells[ hideCells[i] ].hidden = true;
    }
    
    // Reveal some of the cells to make the puzzle easier
    if (puzdif < 2)
    {
        float revealPercent = 0.15;
        if (puzdif >= 1) {
            revealPercent = 0.05;
        }
        Oryol::Array<int> revealCells;
        for (int i=0; i < puzzleCells.Size(); i++) {
            if (!puzzleCells[i].hidden) {
                revealCells.Add( i );
            }
        }
            
        int revealCuttoff = (int)(revealPercent * (float)puzzleCells.Size());
        while (revealCells.Size() > revealCuttoff) {
            int ndx = glm::linearRand( 0, revealCells.Size()-1 );
            revealCells.EraseSwapBack( ndx );
        }
        for (int i=0; i < revealCells.Size(); i++) {
            
            int ndx = revealCells[i];
            LoopCell &cell = puzzleCells[ ndx ];
            
            cell.hidden = true;
            revealEdge( cell.edgeAB );
            revealEdge( cell.edgeAC );
            revealEdge( cell.edgeBD );
            revealEdge( cell.edgeCD );
            
            //cell.checkVal = 1.0f;
        }
    }
}

bool
LD47LoopApp::checkEdgeCorrect( int edgeNdx )
{
    // do we even care about this edge?
    if (edgeNdx < 0) return true;
    
    LoopEdge &edge = puzzleEdge[edgeNdx];
    
    // Is this a given edge?
    if (edge.status == EDGE_Premarked) return true;
    
    
    if (edge.loopEdge) {
        return (edge.status == EDGE_Marked);
    } else {
        return (edge.status != EDGE_Marked);
    }
}

bool
LD47LoopApp::CheckPuzzle()
{
    bool puzzleSolved = true;
    wrongCells = 0;
    
    // This checks some edges more than once but that's OK, need
    // to do it this way so we can identify/mark the cells
    for (int i=0; i < puzzleCells.Size(); i++) {
        LoopCell &cell = puzzleCells[i];
        
        if ( (!checkEdgeCorrect(cell.edgeAB)) ||
            (!checkEdgeCorrect(cell.edgeAC)) ||
            (!checkEdgeCorrect(cell.edgeBD)) ||
            (!checkEdgeCorrect(cell.edgeCD)) ) {
            puzzleSolved = false;
            cell.checkVal = 1.0f;
            wrongCells += 1;
        } else {
            cell.checkVal = 0.0f;
        }
    }
    
    return puzzleSolved;
}

void
LD47LoopApp::handleGameInput() {
    glm::vec2 mousePos = Input::MousePosition() * uiAssets->displayScale;

    glm::vec2 ndcScreen = glm::vec2( ((mousePos.x / uiAssets->fbWidth) * 2.0f) - 1.0f,
                                     -(((mousePos.y / uiAssets->fbHeight) * 2.0f) - 1.0f) );
    //printf("NDC screen %3.2f %3.2f\n", ndcScreen.x, ndcScreen.y );
    glm::vec2 arcballNdcCurrMouse = ndcScreen;
    if (Input::MouseButtonPressed( MouseButton::Left ))
    {
        
        arcball.rotate( arcballNdcPrevMouse, arcballNdcCurrMouse );
    }
    arcballNdcPrevMouse = arcballNdcCurrMouse;
    
    gameCamera.UpdateModel( arcball.inv_transform() );
    
    if (Input::MouseButtonDown( MouseButton::Left )) {
        mouseDownTime = Clock::Now();
    }
    if (Input::MouseButtonUp( MouseButton::Left ))
    {
        Oryol::TimePoint mouseUpTime = Clock::Now();
        Oryol::Duration clickTime = mouseUpTime - mouseDownTime;
        if (clickTime.AsMilliSeconds() < 200.0f) {
            Log::Info("Click Time %f ms\n", clickTime.AsMilliSeconds() );
            // Handle Edge Click
            handleMouseClick( mousePos );
        }
    }
    
    if ((cheatsEnabled) && (Input::KeyDown(Key::D)) ) {
        dbgShowLine = !dbgShowLine;
    }
}

void
LD47LoopApp::handleMouseClick( glm::vec2 mousePos )
{
    
    // Only search the hover cell
    Oryol::Array<int> validEdges;
    if (hoverCell < 0) return;
    
    LoopCell &cell = puzzleCells[hoverCell];
    validEdges.Add( cell.edgeAB );
    validEdges.Add( cell.edgeAC );
    validEdges.Add( cell.edgeBD );
    validEdges.Add( cell.edgeCD );
    
    // See which edge we clicked closest to
    float bestEdgeDist;
    int bestEdgeNdx = 0;
    for (int ei=0; ei < validEdges.Size(); ei++)
    {
        int i = validEdges[ei];
        if (i==-1) continue;
        
        LoopEdge &edge = puzzleEdge[i];
        glm::vec2 pAScr = renderizer->activeCamera->worldPointToScreen( puzzlePos[ edge.pA ].pos );
        glm::vec2 pBScr = renderizer->activeCamera->worldPointToScreen( puzzlePos[ edge.pB ].pos );
        
        glm::vec2 p = glm::closestPointOnLine( mousePos, pAScr, pBScr );
        float d = glm::distance( p, mousePos );
        printf("Edge %d dist %f\n", i, d );
        
        if ((ei==0) || (d < bestEdgeDist)) {
            bestEdgeDist = d;
            bestEdgeNdx = i;
        }
    }
    
    Log::Info("Best Edge %d dist %f\n", bestEdgeNdx, bestEdgeDist );
    if (bestEdgeDist < 20.0f) {
        printf("Toggle Edge %d\n", bestEdgeNdx );
        LoopEdge &bestEdge = puzzleEdge[ bestEdgeNdx ];
        //bestEdge.marked = !bestEdge.marked;
        if ((Input::KeyPressed( Key::LeftShift) ) || (Input::KeyPressed( Key::RightShift) ))
        {
            // shift is pressed, toggle forib
            if (bestEdge.status == EDGE_Blank) {
                bestEdge.status = EDGE_Forbid;
            } else if (bestEdge.status == EDGE_Marked) {
                bestEdge.status = EDGE_Blank;
            } else if (bestEdge.status == EDGE_Forbid) {
                bestEdge.status = EDGE_Blank;
            }
        } else {
            // Normal -- cycle through edge states
            if (bestEdge.status == EDGE_Blank) {
                bestEdge.status = EDGE_Marked;
            } else if (bestEdge.status == EDGE_Marked) {
                bestEdge.status = EDGE_Forbid;
            } else if (bestEdge.status == EDGE_Forbid) {
                bestEdge.status = EDGE_Blank;
            }
        }
        UpdateCellData();
    }
}
void
LD47LoopApp::handleInputDebug() {
    
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
            
            debugTools->dbgCamera.Setup(glm::vec3(0.0, 0.0, 15.0), glm::radians(45.0f),
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
    //dbgCamera.MoveRotate(move, rot);
}

// =======================================================================================
void LD47LoopApp::onSceneLoaded()
{
    SceneCamera cam = gameScene->findNamedCamera( "MonkeyCam" );
    if (cam.index >=0) {
        activeCameraIndex = cam.index;
    }
    gameCamera.UpdateModel( cam.mat );
    gameCamera.UpdateProj(glm::radians(45.0f), uiAssets->fbWidth, uiAssets->fbHeight, 0.01f, 100.0f);
    
//    objPuzzle = gameScene->FindNamedObject( "ball" );
//    if (objPuzzle) {
//        Log::Info("Puzzle Object found: %zu mesh data\n", objPuzzle->mesh->meshDataSize );
//    }
    
    // Remove all the puzzle meshes
    for (int i=0; i < gameScene->sceneMeshes.Size(); i++) {
        puzzleMeshes.Add( gameScene->sceneMeshes[i] );
        //if ( gameScene)
//        Log::Info("game mesh %s\n", gameScene->sceneMeshes[i]->meshName.AsCStr() );
        if (gameScene->sceneMeshes[i]->meshName == "ball_mesh") {
            selectedPuzzleIndex = i;
        }
    }    
    gameScene->sceneObjs.Clear();        
}
void LD47LoopApp::fixedUpdate( Oryol::Duration fixedDt )
{
    
}

void LD47LoopApp::dynamicUpdate( Oryol::Duration frameDt )
{
    renderizer->sceneTime += frameDt.AsSeconds();
}

void LD47LoopApp::finalizeTransforms( glm::mat4 matViewProj )
{
	glm::mat4 shadowMVP = renderizer->shadowCamera.ViewProj;
    gameScene->finalizeTransforms( matViewProj, shadowMVP );
}

void LD47LoopApp::draw()
{

    glm::vec3 lightDir = glm::normalize( glm::vec3( 1.0, 0.4, 0.4 ) );
    gameScene->lightDir = glm::vec4(lightDir, 0.0) * gameCamera.Model;
    
    //gameScene->drawScene();
    doUpdateCellData();
	renderizer->renderScene(gameScene, uiAssets);
}

void LD47LoopApp::nextCamera()
{
    activeCameraIndex++;
    
    if (activeCameraIndex >= gameScene->sceneCams.Size() ) {
        activeCameraIndex = 0;
    }
    
    SceneCamera cam = gameScene->sceneCams[activeCameraIndex];
//    Log::Info("Camera: %s\n", cam.cameraName.AsCStr() );
    
    gameCamera.UpdateModel( cam.mat );
}

void LD47LoopApp::interfaceScreens( Tapnik::UIAssets *uiAssets )
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
    
    if (gameStatus == GAME_TitleScreen)
    {
        float titleW = 800;
        float titleH = 300;
        float titleMarg = (uiAssets->fbWidth - titleW) / 2;
        if (nk_begin(ctx, "title_card", nk_rect( titleMarg, 30, titleW, 700), window_flags))
        {
            nk_layout_row_dynamic( ctx, 175, 1);
            nk_image( ctx, uiAssets->img_title_card );
            
            // ----- Puzzle Difficulty
            nk_layout_row_dynamic( ctx, 50, 1);
            ctx->style.text.color = nk_rgb(10,100,100);
            nk_label( ctx, "Difficulty", NK_TEXT_ALIGN_CENTERED );
                        
            nk_layout_row_dynamic( ctx, 40, 3);
            ctx->style.option.cursor_normal = nk_style_item_color( nk_rgb( 255, 255, 0) );
            ctx->style.option.cursor_hover = nk_style_item_color( nk_rgb( 255, 255, 100) );
            ctx->style.option.text_normal = nk_rgb( 10, 20, 50) ;
            ctx->style.option.text_hover = nk_rgb( 10, 40, 50) ;
            
            if (nk_option_label(ctx, "Normal", puzdif == 0)) puzdif = 0;
            if (nk_option_label(ctx, "Tricky", puzdif == 1)) puzdif = 1;
            if (nk_option_label(ctx, "Hard", puzdif == 2)) puzdif = 2;
            
            // ----- Puzzle Shape
            nk_layout_row_dynamic( ctx, 50, 1);
            ctx->style.text.color = nk_rgb(10,100,100);
            nk_label( ctx, "Shape", NK_TEXT_ALIGN_CENTERED );
            
            static int puzshape = 4;
            nk_layout_row_dynamic( ctx, 40, 3);
            //ctx->style.option.cursor_normal = nk_style_item_color( nk_rgb( 255, 255, 0) );
            //ctx->style.option.cursor_hover = nk_style_item_color( nk_rgb( 255, 255, 100) );
            //ctx->style.option.text_active = nk_rgb( 255, 255, 0) ;
            
            for (int i=0; i < puzzleMeshes.Size(); i++ ) {
                char buff[40];
                strcpy( buff, puzzleMeshes[i]->meshName.AsCStr() );
                char *ch = strstr( buff, "_mesh");
                if (ch) {
                    *ch = '\0';
                }
                
                if (nk_option_label(ctx, buff, puzshape == i)) { puzshape = i; }
            }
            
//            if (nk_option_label(ctx, "Big Ball", puzshape == 1)) puzshape = 1;
//            if (nk_option_label(ctx, "Torus", puzshape == 2)) puzshape = 2;
//            if (nk_option_label(ctx, "Blob", puzshape == 3)) puzshape = 3;
            
            nk_style_set_font(ctx, &(uiAssets->font_30->handle));
        //    nk_layout_row_dynamic( ctx, 20, 1);
            uiAssets->buttonStyleBigDots(ctx);
            nk_layout_row_dynamic( ctx, 93, 1);
            if (nk_button_label(ctx, "Instructions")) {
                // On mac: system("open file...");
//                ShellExecute(NULL, "open", "file://README.html", NULL, NULL, SW_SHOWNORMAL);
                system("open README.html");
            }
            
            if (nk_button_label(ctx, "Start Game")) {
            
                printf("puz shape is %d\n", puzshape );
                SetupPuzzle( puzzleMeshes[ puzshape ] );
                
               GenPuzzleStart();
               bool moreSteps = true;
               while (moreSteps) {
                   moreSteps = GenPuzzleStep();
               }
               GenPuzzleFinish();
               UpdateCellData();
               Log::Info("GENPuz: Puzzle is compled...\n");
                
                gameStatus = GAME_Solving;
            }
        
        }
        nk_end(ctx);
    }
    
    else if (gameStatus == GAME_Solving)
    {
        
        //float menuW = 223;
        
        // Button image width is 223, not sure where the extra space is coming from
        float menuW = 300;
        //float menuMarg = (uiAssets->fbWidth - menuW) / 2;
        float menuMarg = 20;
        
        ctx->style.window.padding = nk_vec2(0,0);
        ctx->style.window.padding = nk_vec2(0,0);
        
        nk_style_set_font(ctx, &(uiAssets->font_30->handle));
        
        uiAssets->buttonStyleBigDots(ctx);
        float center = uiAssets->fbWidth / 2.0;
        
        if (nk_begin(ctx, //&layout,
                     "main_menu", nk_rect( menuMarg, uiAssets->fbHeight - (100+menuMarg),
                                                        menuW, 200), window_flags))
        {
//            nk_layout_row_dynamic( ctx, 50, 1);
//            if (nk_checkbox_label(ctx, "Music", &musicPlaying ) ) {
//                printf("Music Toggled: %s\n", musicPlaying?"ON":"OFF");
//                if (!musicPlaying) {
//                    music.stop();
//                } else {
//                    soloud.play( music );
//                }
//            }

            nk_layout_row_dynamic( ctx, 93, 1);
            if (nk_button_label(ctx, "Check")) {
                puzzleSolved = CheckPuzzle();
                UpdateCellData();
                
                gameStatus = GAME_Review;
            }
            
    //        if (nk_button_label(ctx, "SFX Jump")) {
    //            printf("Button 2 pressed...\n");
    //            soloud.play( sfxJump );
    //        }
            
            
        }
        nk_end(ctx);
    }
    else if (gameStatus == GAME_Review) {
        
        float titleW = 800;
        float titleH = 300;
        float titleMarg = (uiAssets->fbWidth - titleW) / 2;
        if (nk_begin(ctx, "review", nk_rect( titleMarg, 30, titleW, 200), window_flags))
        {
            nk_layout_row_dynamic( ctx, 40, 1);
            nk_style_set_font(ctx, &(uiAssets->font_30->handle));
            
            if (puzzleSolved) {
                ctx->style.text.color = nk_rgb(200,255,200);
                nk_label( ctx, "You Win!", NK_TEXT_ALIGN_CENTERED );
                
                uiAssets->buttonStyleNormal(ctx);
                nk_layout_row_dynamic( ctx, 100, 1);
                if (nk_button_label(ctx, "Yay!")) {
                    gameStatus = GAME_TitleScreen;
                    gameScene->sceneObjs.Clear();
                }
                
            } else {
                ctx->style.text.color = nk_rgb(255,50,50);
                nk_label( ctx, "Some Cells Incorrect", NK_TEXT_ALIGN_CENTERED );
                    
                uiAssets->buttonStyleNormal(ctx);
                nk_layout_row_dynamic( ctx, 100, 2);
                if (nk_button_label(ctx, "Keep Going")) {
                        gameStatus = GAME_Solving;
                        ClearCheckVal();
                }
                if (nk_button_label(ctx, "Give Up")) {
                        gameStatus = GAME_TitleScreen;
                        gameScene->sceneObjs.Clear();
                }
            }
        
        }
        nk_end(ctx);
    }
    
    NKUI::Draw();
    
    // Note: clear happens in draw, extra clear is to workaround a bug, need to upgrade nuklear in NKUI to get the fix
    nk_clear(ctx);
}



