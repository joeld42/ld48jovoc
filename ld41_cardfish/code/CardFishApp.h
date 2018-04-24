#ifndef CARDFISH_APP_H
#define CARDFISH_APP_H
//------------------------------------------------------------------------------
//  CardFishApp.h
//------------------------------------------------------------------------------
#include <unistd.h>

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
#include "DebugDraw.h"
#include "UIAssets.h"
#include "CardFishGame.h"

struct Message {
    Oryol::String messageText;
    glm::vec4 color;
};

class CardFishApp : public Oryol::App {
    
public:
    Oryol::AppState::Code OnRunning();
    Oryol::AppState::Code OnInit();
    Oryol::AppState::Code OnCleanup();
    
private:
    void handleInputDebug();
    void mainMenuUI();
    void updatePicking();
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
    
    Oryol::GfxSetup gfxSetup;
    Oryol::PassAction passAction;
        
    // Debug mode stuff
    bool debugMode;
    Tapnik::Camera gameCamera;
    Tapnik::Camera cardCamera;
    Tapnik::Camera dbgCamera;
    Tapnik::DebugDrawRenderer *dbgDraw;
    int activeCameraIndex = 0;
    
    // Gameplay stuff
    Oryol::TimePoint startTimePoint;
    Oryol::TimePoint lastTimePoint;
    Oryol::Duration frameDtRaw;
    Oryol::Duration leftoverTime;

    // Game stuff
    // Note to self -- split these into another file if not messing with the main game loop structure much
    Tapnik::Scene *gameScene;
    void onSceneLoaded();
    void fixedUpdate( Oryol::Duration fixedDt );
    void dynamicUpdate( Oryol::Duration frameDt );
    void finalizeTransforms( glm::mat4 matViewProj );
    void draw();
    
    void interfaceScreens( Tapnik::UIAssets *uiAssets );
    void interfaceTitle( nk_context* ctx );
    void interfaceGame( nk_context* ctx );
    
    void message( Oryol::String messageText, glm::vec4 color );
    void switchToNamedCamera( Oryol::String camera );
    
    Tapnik::Ray mouseRay;
    Tapnik::Ray mouseCardsRay;
    glm::vec3 groundCursor;
    glm::vec3 tableCursor;
    Tapnik::SceneObject *lakeObj;
    Tapnik::SceneObject *trashCanObj;
    
    glm::vec2 fishDiagramPos;
    
    enum DropZone {
        DropZone_LAKE,
        DropZone_TACKLE,
        DropZone_REEL,
        DropZone_TRASH,
    };
    DropZone activeDropZone;
    
    Card currentCard = {};
    bool isDraggingCard;
    float reelTimeout;
    int reelPowerRemaining;
    
    float calcReelDiagramXPos( float reelDistance );
    float calcReelDiagramYPos( float tension );
    
    
    CardFishGame game;
    void startGame();
    void updateCards();
    void prepareNextDrawCard();
    void drawNextCard();
    void doCast();
    void doAction();
    void doLake();
    void catchReelFish();
    void updateTension();
    void updateSlack();
    void trashCard();
    void spawnCardObject( Card &card );
    Card transmuteCard( Card origCard, Oryol::String newCardId );
    void cleanupCards( Oryol::Array<Card> &cards );
    void disposeCard( Card &card );
    
    
    glm::vec2 deckTablePos;
    glm::vec2 currCardTablePos;
    Oryol::Duration gameTime;
    
    Oryol::Array<Message> messageQueue;
    float messageTimeout;
    
    Tapnik::UIAssets *uiAssets;
    SoLoud::Soloud soloud;
    
    // TODO: wrap up all this 
    Oryol::Buffer sfxJumpData;
    SoLoud::Wav sfxJump;
    
    int doStartGame = -1;
    
    int musicPlaying = 0;
    bool ready = false;
    Oryol::Buffer musicData;
    SoLoud::WavStream music;
};
#endif
