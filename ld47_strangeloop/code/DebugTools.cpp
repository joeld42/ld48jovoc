#include "DebugTools.h"

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Dbg/Dbg.h"
#include "Input/Input.h"
#include "Core/Time/Clock.h"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"

#include "IMUI/IMUI.h"

#include "Gfx/private/gfxResourceContainer.h"


using namespace Oryol;
using namespace Tapnik;

//------------------------------------------------------------------------------
//  DebugTextureInfo
//------------------------------------------------------------------------------
struct DebugTextureInfo
{
public:
    DebugTextureInfo( const _priv::texture* _tex,
                     const ImTextureID _uiTexture ) :
    texture( _tex ), uiTexture( _uiTexture ) {}
    
    const _priv::texture* texture;
    const ImTextureID uiTexture;
};

//------------------------------------------------------------------------------
//  DebugTools
//------------------------------------------------------------------------------

DebugTools::DebugTools() :
    debugMode( true )
{
    
}

void
DebugTools::Setup( Tapnik::UIAssets *uiAssets )
{
    this->uiAssets = uiAssets;
    
    dbgCamera = {};
    dbgCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f),
                    uiAssets->fbWidth,
                    uiAssets->fbHeight,
                    1.0f, 100.0f);
    dbgCamera.Pos = glm::vec3( 11.22, -29.90, 34.06 );
    dbgCamera.Rot = glm::vec2( 0.28, 0.68 );
    dbgCamera.updateViewProjFrustum();

    IMUI::Setup();
    
}

void
DebugTools::ToggleDebugMode()
{
    debugMode = !debugMode;
	Log::Info("Debug Mode: %s\n", debugMode ? "ON" : "OFF");
    //NKUI::SetShouldHandleInput( !debugMode );
}

void
DebugTools::OnRunning( Oryol::Duration frameDt )
{
    IMUI::NewFrame(frameDt);
    if (debugMode) {
        this->DebugGui();
    }
}

void DebugTools::DebugRender()
{
	ImGui::Render();
}

void
DebugTools::handleInputDebug() {
    
    if (!debugMode) {
        return;
    }
    
    ImGuiIO &imguiIO = ImGui::GetIO();
    
    glm::vec3 move;
    glm::vec2 rot;
    float vel = 0.5f;
    if (Input::KeyboardAttached() && (!imguiIO.WantCaptureKeyboard)) {
        
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
            printf("dbgCamera.Pos = vec3( %3.2f, %3.2f, %3.2f );\n",
                   dbgCamera.Pos.x, dbgCamera.Pos.y, dbgCamera.Pos.z
                   );
            printf("dbgCamera.Rot = vec2( %3.2f, %3.2f );\n",
                   dbgCamera.Rot.x, dbgCamera.Rot.y
                   );
        }
        
        if (Input::KeyDown(Key::N1))
        {
            showStatusWindow = !showStatusWindow;
        }
        
        if (Input::KeyDown(Key::N2))
        {
            showResourceWindow = !showResourceWindow;
        }
        
        if (Input::KeyDown(Key::N4))
        {
            showGameDbgWindow = !showGameDbgWindow;
        }

        //            dbgCamera.Setup(glm::vec3(0.0, 0.0, 15.0), glm::radians(45.0f),
        //                            uiAssets->fbWidth,
        //                            uiAssets->fbHeight, 1.0f, 10.0f);
    }
    
    if (Input::MouseAttached() && (!imguiIO.WantCaptureMouse)) {
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


void
DebugTools::DebugGui()
{
    StatusWindow();
    ResourceWindow();
  
    GameDebugWindow();
 
}

void
DebugTools::ResourceWindow()
{
    static Oryol::Array<DebugTextureInfo> inspectTextures;
    
    if (showResourceWindow) {
        ImGui::SetNextWindowPos( ImVec2( 20.0f, 50.0f));
        ImGui::SetNextWindowSize( ImVec2( 300.0f, 200.0f ) );
        ImGui::Begin("Resources", &showResourceWindow );
        ImGui::Text("Resource Window");
        
        _priv::gfxResourceContainer *resCtr = Gfx::resource();
        
        if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen ))
        {
            ImGui::SetNextTreeNodeOpen( true );
            if (ImGui::TreeNode("Textures"))
            {
                ResourcePool<_priv::texture> &texturePool = resCtr->texturePool;
                ResourcePoolInfo poolInfo = texturePool.QueryPoolInfo();
                //printf("%d/%d texture slots used (%d free)\n", poolInfo.NumUsedSlots, poolInfo.NumSlots, poolInfo.NumFreeSlots );
                for (const _priv::texture& slot : texturePool.slots) {
                    if ((ResourceState::InvalidState != slot.State) && (slot.Id.IsValid()) ) {
                        
                        //                        const char *stateStr = "Unknown";
                        //                        switch (slot.State) {
                        //                                case ResourceState::Initial: stateStr = "Initial"; break;
                        //                                case ResourceState::Setup: stateStr = "Setup"; break;
                        //                                case ResourceState::Pending: stateStr = "Pending"; break;
                        //                                case ResourceState::Valid: stateStr = "Valid"; break;
                        //                                case ResourceState::Failed: stateStr = "Failed"; break;
                        //                        }
                        
                        char defaultName[200];
                        sprintf( defaultName, " UNKNOWN #%0llX", slot.Id.Value );
                        const char *texLocator = slot.Setup.Locator.Location().AsCStr();
                        if ((texLocator==NULL)||(strlen(texLocator)==0)) {
                            texLocator = defaultName;
                        }
                        // rewind locator to first path separator
                        const char *ch = texLocator + strlen(texLocator)-1;
                        while (ch > texLocator) {
                            if ((*ch=='/')||(*ch==':')) {
                                ch++; break;
                            } else {
                                ch--;
                            }
                        }
                        texLocator = ch;
                        
                        char buttonName[32];
                        sprintf(buttonName, "Inspect##%0llX", slot.Id.Value );
                        if (ImGui::Button( buttonName )) {
                            printf("INSPECT\n");
                            bool isInspecting = false;
                            for (int i=0; i < inspectTextures.Size(); i++) {
                                if (inspectTextures[i].texture->Id == slot.Id) {
                                    isInspecting = true;
                                    break;
                                }
                            }
                            if (!isInspecting) {
                                printf("Adding texture...\n");
                                
                                ImTextureID uiImg = IMUI::AllocImage();
                                IMUI::BindImage( uiImg, slot.Id );
                                
                                DebugTextureInfo dbgTex( &slot, uiImg );
                                
                                inspectTextures.Add( dbgTex );
                            }
                        }
                        ImGui::SameLine();
                        ImGui::Text( "[%dx%d] %s",
                                    slot.Setup.Width, slot.Setup.Height,
                                    texLocator );
                        //                        printf("%0llX [%s] -- (%dx%d) %s\n", slot.Id.Value, stateStr,
                        //                               slot.Setup.Width, slot.Setup.Height,
                        //                               slot.Setup.Locator.Location().AsCStr() );
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
        
        
        //printf("inspectTextures.size %d\n", inspectTextures.Size() );
        Oryol::Array<DebugTextureInfo> keepItems;
        for (int i=0; i < inspectTextures.Size(); i++) {
            bool showTexture = true;
            const DebugTextureInfo &dbgTex = inspectTextures[i];
            const _priv::texture *tex = dbgTex.texture;
            
            char windowName[32];
            sprintf(windowName, "Texture (%0llX)", tex->Id.Value );
            
            float texWidth = tex->Setup.Width;
            float texHeight = tex->Setup.Height;
            float texAspect = texHeight / texWidth;
            
            if (texWidth > 200) {
                texWidth = 200;
            }
            
            ImGui::SetNextWindowSize( ImVec2( texWidth+20.0f, texWidth+20.0f * texAspect ), ImGuiCond_Appearing );
            if (ImGui::Begin(windowName, &showTexture )) {
                
                const char *texLocator = tex->Setup.Locator.Location().AsCStr();
                ImGui::Text("%s", texLocator);
                for (int j=0; j < tex->numSlots; j++) {
                    
                    float winWidth = ImGui::GetWindowContentRegionWidth();
                    
                    if (texWidth < winWidth) {
                        texWidth = winWidth;
                    } else if (texWidth > winWidth) {
                        texWidth = winWidth;
                    }
                    texHeight = texWidth * texAspect;
                    
                    ImGui::Image( dbgTex.uiTexture,
                                 ImVec2(texWidth, texHeight),
                                 ImVec2(0,0), ImVec2(1,1),
                                 ImColor(255,255,255,255),
                                 ImColor(255,255,255,128));
                }
            }
            ImGui::End();
            
            if (showTexture) {
                keepItems.Add( dbgTex );
            } else {
                IMUI::FreeImage( dbgTex.uiTexture );
            }
        }
        inspectTextures = keepItems;
    }

}

bool
DebugTools::InputTextOryol( const char *label, Oryol::String &origText )
{
    char editableText[512];
    strcpy( editableText, origText.AsCStr());
    //ImGui::Text( "Current Map: %s", boardInfo->displayName.AsCStr());
    ImGui::InputText( label, editableText, 512 );
    if (origText != editableText ) {
        origText = editableText;
        return true;
    }
    return false;
}



void
DebugTools::GameDebugWindow()
{
    if (showGameDbgWindow) {
        
        ImGui::SetNextWindowPos( ImVec2( 40.0f, 50.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize( ImVec2( 400.0f, 480.0f ), ImGuiCond_FirstUseEver );
        
        ImGui::Begin( "Game Debug", &showGameDbgWindow, ImGuiWindowFlags_MenuBar );
                
        if (ImGui::CollapsingHeader("Cheats", ImGuiTreeNodeFlags_DefaultOpen )) {
            ImGui::Checkbox("Example Cheat 1", &(exampleCheat1));
        }
        
        
      
        
        ImGui::End();
    }
    
}


void
DebugTools::StatusWindow()
{
    if (showStatusWindow) {
        ImGui::SetNextWindowPos( ImVec2( 0, uiAssets->fbHeight - 60));
        ImGui::SetNextWindowSize(ImVec2( uiAssets->fbWidth, 60), ImGuiCond_Always );
        ImGui::Begin("Crossroads Status", &showStatusWindow,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse  );
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
}
