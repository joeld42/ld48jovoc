#ifndef DEBUG_TOOLS_H
#define DEBUG_TOOLS_H

#include "Pre.h"

#include "Core/Main.h"
#include "Core/Types.h"
#include "Core/Time/Clock.h"

#include "Gfx/Gfx.h"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"

#include "IMUI/IMUI.h"

#include "UIAssets.h"

#include "Camera.h"

//------------------------------------------------------------------------------
//  DebugTools
//------------------------------------------------------------------------------
class DebugTools
{
public:
    DebugTools();
    
    bool debugMode;
    Tapnik::UIAssets *uiAssets;
    Tapnik::Camera dbgCamera;
        
    void ToggleDebugMode();
    
    void Setup( Tapnik::UIAssets *uiAssets );
    void handleInputDebug();
    
    void OnRunning( Oryol::Duration frameDt );
    
    void DebugGui();
	void DebugRender();
    
    bool showStatusWindow = true;
    void StatusWindow();
    
    bool showResourceWindow = false;
    void ResourceWindow();
    
 
    // ==Game Mode Window stuff
    bool showGameDbgWindow = false;
    void GameDebugWindow();

	bool exampleCheat1 = false;
    
    
    
    // Helpers to wrap Oryol strings with IMGUI text
    bool InputTextOryol( const char *label, Oryol::String &origText );

};

#endif
