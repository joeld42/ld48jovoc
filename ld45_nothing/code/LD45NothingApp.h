#ifndef LD45_NOTHING_APP_H
#define LD45_NOTHING_APP_H
//------------------------------------------------------------------------------
//  LD45NothingApp.h
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
#include "DebugDraw.h"
#include "UIAssets.h"
#include "CivGame.h"

class LD45NothingApp : public Oryol::App {
    
public:
    Oryol::AppState::Code OnRunning();
    Oryol::AppState::Code OnInit();
    Oryol::AppState::Code OnCleanup();
    
private:
    void handleInputDebug();
    void mainMenuUI();

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
    
	int renderPassMultisample;
    Oryol::GfxSetup gfxSetup;
    Oryol::PassAction passAction;
        
    // Debug mode stuff
    bool debugMode;
    Tapnik::Camera gameCamera;
    Tapnik::Camera dbgCamera;
    Tapnik::DebugDrawRenderer *dbgDraw;
	Tapnik::Camera* activeCamera;
	int activeCameraIndex;
    
	// Audio Stuff
	SoundMaker sfx;

    // Gameplay stuff
    Oryol::TimePoint startTimePoint;
    Oryol::TimePoint lastTimePoint;
    Oryol::Duration frameDtRaw;
    Oryol::Duration leftoverTime;

    // Game stuff
    // Note to self -- split these into another file if not messing with the main game loop structure much
	CivGame *civGame;
	void StartGame();

	Tapnik::Scene *gameScene;
    void onSceneLoaded();
    void fixedUpdate( Oryol::Duration fixedDt );
    void dynamicUpdate( Oryol::Duration frameDt );
    void finalizeTransforms( glm::mat4 matViewProj );
    void draw();
    void interfaceScreens( Tapnik::UIAssets *uiAssets );

	Tapnik::Renderizer* renderizer;
    
    Tapnik::UIAssets *uiAssets;
    
  
};
#endif
