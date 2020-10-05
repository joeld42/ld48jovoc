#ifndef LD47LOOP_APP_H
#define LD47LOOP_APP_H
//------------------------------------------------------------------------------
//  DarkStarApp.h
//------------------------------------------------------------------------------
#ifdef __APPLE__
#include <unistd.h>
#endif

#include "Pre.h"

#include "Core/Main.h"
#include "Core/Types.h"
#include "Core/Time/Clock.h"

#include "Gfx/Gfx.h"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "Camera.h"
#include "SceneObject.h"
#include "Renderizer.h"
#include "GameSettings.h"
#include "DebugTools.h"
#include "DebugDraw.h"
#include "UIAssets.h"

#include "arcball_camera.h"

#define MAX_TRIS_FOR_CELL 32

struct LoopPoint
{
    glm::vec3 pos;
    int faceCount;
};

enum FillStatus {
    FILL_Inside,
    FILL_Outside
};

enum EdgeStatus {
    EDGE_Blank,     // Edge is not marked
    EDGE_Premarked, // Edge is pre-marked by the game
    EDGE_Marked,    // Edge is marked by user as part of the loop
    EDGE_Forbid     // User marked edge as not good
};

enum GameStatus {
    GAME_TitleScreen,
    GAME_Solving,
    GAME_Review
};

struct LoopCell
{
    int nbrAB, nbrBD, nbrAC, nbrCD;
    uint16_t pA, pB, pC, pD;
    
    // quick lookup for which edges, can be inferred
    // from pA, pB, etc..
    int edgeAB, edgeBD, edgeAC, edgeCD;
    
    uint8_t numTri;
    uint16_t tri[MAX_TRIS_FOR_CELL];
    
    FillStatus fillStatus;
    int loopEdgeCount;
    bool hidden;
    
    float checkVal; // for displaying if bad
};

struct LoopEdge
{
    uint16_t pA;
    uint16_t pB;
    EdgeStatus status;
    bool loopEdge; // is this part of the loop?    
};


class LD47LoopApp : public Oryol::App {
    
public:
    Oryol::AppState::Code OnRunning();
    Oryol::AppState::Code OnInit();
    Oryol::AppState::Code OnCleanup();
    
private:
    void handleInputDebug();
    void mainMenuUI();
    
    void handleGameInput();
    void handleMouseClick( glm::vec2 mousePos );
    
    float edgePixelDist( glm::vec2 p, int pA, int pB );

    void nextCamera();
    
    glm::mat4 computeMVP(const glm::mat4& proj,
                         float rotX, float rotY, const glm::vec3& pos);
    
    Oryol::Id renderPass;
    Oryol::DrawState offscrDrawState;
    Oryol::DrawState mainDrawState;
    //OffscreenShader::params offscrVSParams;
    //MainShader::params mainVSParams;
    glm::mat4 view;
    glm::mat4 offscreenProj;
    glm::mat4 displayProj;
    float angleX = 0.0f;
    float angleY = 0.0f;
    
	GameSettings cfg;
    Oryol::GfxSetup gfxSetup;
    Oryol::PassAction passAction;
        
    // Debug mode stuff
	DebugTools* debugTools;
    Tapnik::Camera gameCamera;
    //Tapnik::Camera dbgCamera;
    Tapnik::DebugDrawRenderer *dbgDraw;
    int activeCameraIndex = 0;
    bool dbgShowLine = false;
    
    bool draggingArcball;
    ArcballCamera arcball;
    glm::vec2 arcballNdcPrevMouse;
        
    Oryol::Array<Tapnik::SceneMesh*> puzzleMeshes;
    Tapnik::SceneObject *objPuzzle = NULL;
    int selectedPuzzleIndex = 0;
    
    Oryol::Array<LoopCell> puzzleCells;
    Oryol::Array<LoopPoint> puzzlePos;
    Oryol::Array<LoopEdge> puzzleEdge;
    Oryol::Array<glm::vec2> origMeshST;
    
    int hoverCell; // cell index under mouse
    int FindOrCreatePoint( glm::vec3 p );
    int FindOrCreateEdge( int pA, int pB );
    int FindEdge( int pA, int pB );
    void FindHoverCell();
    GameStatus gameStatus;
    bool wrongCells = 0;
    bool puzzleSolved = false;
    bool cheatsEnabled = false;
    
    void UpdateCellData();
    void doUpdateCellData();
    bool cellDataNeedsUpdate = false;

    
    void SetupPuzzle( Tapnik::SceneMesh *mesh );
    
    bool GenPuzzleStep();
    void GenPuzzleStart();
    void GenPuzzleFinish();
    
    bool CheckPuzzle();
    void ClearCheckVal();
    bool checkEdgeCorrect( int edgeNdx );
    
    int genPuzzleCheckNbr( int nbrNdx, int cellNdx );
    void genPuzzleMarkCell( int cellNdx );
    bool checkFaceCount( int ndx, int safe1, int safe2, int safe3, int safe4 );
    void CheckNbrEdge( int cellNdx, int nbrIndex, int eA, int eB );
    void revealEdge( int edgeNdx );
    
    // Gameplay stuff
    Oryol::TimePoint startTimePoint;
    Oryol::TimePoint lastTimePoint;
    Oryol::Duration frameDtRaw;
    Oryol::Duration leftoverTime;
    
    Oryol::TimePoint mouseDownTime;

    // Game stuff
    // Note to self -- split these into another file if not messing with the main game loop structure much
    Tapnik::Scene *gameScene;
    void onSceneLoaded();
    void fixedUpdate( Oryol::Duration fixedDt );
    void dynamicUpdate( Oryol::Duration frameDt );
    void finalizeTransforms( glm::mat4 matViewProj );
    void draw();
    void interfaceScreens( Tapnik::UIAssets *uiAssets );

	Tapnik::Renderizer* renderizer;
    
    Tapnik::UIAssets *uiAssets;
    SoLoud::Soloud soloud;
    
    // TODO: wrap up all this 
    Oryol::Buffer sfxJumpData;
    SoLoud::Wav sfxJump;
    int puzdif = 0;
    
    int musicPlaying = 0;
    Oryol::Buffer musicData;
    SoLoud::WavStream music;
};
#endif
