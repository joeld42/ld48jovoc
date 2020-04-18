#ifndef RENDERIZER_H
#define RENDERIZER_H

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "UIAssets.h"
#include "DebugDraw.h"
#include "Ray.h"
#include "Camera.h"

#include "shadow_shaders.h"
#include "postproc.h"
#include "background.h"

namespace Tapnik
{    
    struct Renderizer {
        
        Renderizer( const Oryol::VertexLayout &meshLayout, Oryol::GfxSetup *gfxSetup, int mainRenderSampleCount  );
        
        void renderScene( Tapnik::Scene *scene, Tapnik::UIAssets *uiAssets );
		void finishMainPass();
		void finishRender( Tapnik::UIAssets *uiAssets );
        
		void testShadowStuff(float shadNear, float shadFar);

		float sceneTime;

		// Background (SDF Skybox) pass
		Oryol::TextureSetup backgroundRenderSetup;
		Oryol::Id backgroundRenderTarget;
		Oryol::Id backgroundRenderPass;
		Oryol::DrawState backgroundDrawState;
		BackgroundShader::vsParams backgroundVSparams;
		BackgroundShader::fsParams backgroundFSparams;
		Oryol::DrawState bgquadDrawState;
		BGQuadShader::vsParams bgquadFSParams;		

        // Main render stuff
        Oryol::PassAction passAction;
		Camera* activeCamera;
        
        // Shadow stuff
        const int shadowMapSize = 2048;
        Tapnik::Camera shadowCamera;
        Oryol::Id shadowMap;
        Oryol::Id shadowPass;
        Oryol::Id shadowShader;
        Oryol::DrawState shadowDrawState;
        bool debugDrawShadowMap = false;
		bool drawMainScene = true;
        
        Oryol::DrawState shadowDebugDrawState;
        DebugShadowShader::vsParams shadowDebugFSparams;

		// Offscreen render pass (pre-color-corrected)
		Oryol::TextureSetup mainRenderSetup;
		Oryol::TextureSetup gbuffRenderSetup;
		Oryol::Id gbuffRenderTarget;
		Oryol::Id mainRenderTarget;
		Oryol::Id mainRenderPass;
		
		// Post draw (color grade, etc)
		Oryol::DrawState postProcDrawState;
		PostProcShader::vsParams postProcFSparams;

        
    };
} // namespace Tapnik
#endif
