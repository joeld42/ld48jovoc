#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"

#include "shaders.h"
#include "shadow_shaders.h"
#include "postproc.h"

#include "SceneObject.h"
#include "Renderizer.h"

#include "glm/glm.hpp"

using namespace Oryol;
using namespace Tapnik;

Renderizer::Renderizer(  const Oryol::VertexLayout &meshLayout, 
	Oryol::GfxSetup *gfxSetup, int mainRenderSampleCount )
{
	sceneTime = 0.0f;

    // Main render setup
    this->passAction.Color[0] = glm::vec4( 0.2, 0.2, 0.2, 1.0 );

    // Shadow Setup
    this->shadowShader = Gfx::CreateResource(ShadowShader::Setup());
    
    // TODO get from scene or something
    shadowCamera = {};

    shadowCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f),
                    shadowMapSize, shadowMapSize, 10.0f, 60.0f);
   

    // Doesn't quite work
    //shadowCamera.SetupShadow(glm::vec3(0.0, 0.0, 25.0), 20.0f, shadowMapSize, shadowMapSize, 1.0f, 100.0f );
    
	shadowCamera.Pos = glm::vec3(9.22, -29.90, 34.06) * 0.9f;
    //shadowCamera.Pos = glm::vec3( 11.22, -29.90, 34.06 );
    shadowCamera.Rot = glm::vec2( 0.28, 0.68 );
	
    
    // This applies the pos/rot
    shadowCamera.MoveRotate( glm::vec3(0.0f), glm::vec2(0.0f));


    TextureSetup shadowMapSetup = TextureSetup::RenderTarget2D(shadowMapSize, shadowMapSize, PixelFormat::RGBA8, PixelFormat::DEPTH);
	shadowMapSetup.Sampler.MinFilter = TextureFilterMode::Linear;
	shadowMapSetup.Sampler.MagFilter = TextureFilterMode::Linear;
	this->shadowMap = Gfx::CreateResource(shadowMapSetup);
    
    PassSetup shadowPassSetup = PassSetup::From(this->shadowMap,  // color target
                                                this->shadowMap); // depth target
    
    shadowPassSetup.DefaultAction = PassAction::Clear(glm::vec4(1.0f), 1.0f, 0);
    this->shadowPass = Gfx::CreateResource(shadowPassSetup);
    //this->ColorDrawState.FSTexture[0] = this->shadowMap;
    
    // create shadow pass pipeline state
    auto ps = PipelineSetup::FromLayoutAndShader( meshLayout,this->shadowShader);
    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    ps.RasterizerState.CullFaceEnabled = true;
    ps.RasterizerState.CullFace = Face::Code::Front;
    ps.RasterizerState.SampleCount = 1;
    ps.BlendState.ColorFormat = shadowMapSetup.ColorFormat;
    ps.BlendState.DepthFormat = shadowMapSetup.DepthFormat;
    this->shadowDrawState.Pipeline = Gfx::CreateResource(ps);
    
    // Setup the debug visualization
	bool flipQuad = false;
// flip on Metal targets
#ifdef ORYOL_OSX
	flipQuad = true;
#endif
    auto quadSetup = MeshSetup::FullScreenQuad( flipQuad );
	Oryol::Id quadMesh = Gfx::CreateResource(quadSetup);
	this->shadowDebugDrawState.Mesh[0] = quadMesh;
    Id shd = Gfx::CreateResource(DebugShadowShader::Setup());
    auto psShadowDebug = PipelineSetup::FromLayoutAndShader(quadSetup.Layout, shd);
    psShadowDebug.RasterizerState.SampleCount = gfxSetup->SampleCount;
    this->shadowDebugDrawState.Pipeline = Gfx::CreateResource(psShadowDebug);

	// Setup the post-process pass
	{
		this->postProcDrawState.Mesh[0] = quadMesh;
		Id postProcShd = Gfx::CreateResource(PostProcShader::Setup());
		auto psPostProc = PipelineSetup::FromLayoutAndShader(quadSetup.Layout, postProcShd);
		psPostProc.RasterizerState.SampleCount = gfxSetup->SampleCount;
		this->postProcDrawState.Pipeline = Gfx::CreateResource(psPostProc);
	}

	// Set up the offscreen render target
	float renderDownscale = 1.0f;
	int mainRenderWidth = (int)(1280 * renderDownscale);
	int mainRenderHeight = (int)(720 * renderDownscale);
		
		mainRenderSetup = TextureSetup::RenderTarget2D(
			mainRenderWidth, mainRenderHeight, 
			PixelFormat::RGBA16F, PixelFormat::DEPTHSTENCIL );
		mainRenderSetup.SampleCount = mainRenderSampleCount;

		mainRenderSetup.Sampler.MinFilter = TextureFilterMode::Linear;
		mainRenderSetup.Sampler.MagFilter = TextureFilterMode::Linear;
		mainRenderSetup.Sampler.WrapU = TextureWrapMode::Repeat;
		mainRenderSetup.Sampler.WrapV = TextureWrapMode::Repeat;

		this->mainRenderTarget = Gfx::CreateResource(mainRenderSetup);


		// Render Target for depth and Normal info
		gbuffRenderSetup = TextureSetup::RenderTarget2D(
			mainRenderWidth, mainRenderHeight,
			PixelFormat::RGBA16F, PixelFormat::None);
		gbuffRenderSetup.SampleCount = mainRenderSampleCount;

		this->gbuffRenderTarget = Gfx::CreateResource(gbuffRenderSetup);


		//PassSetup mainRenderPassSetup = PassSetup::From(
		//	this->mainRenderTarget,  // color target
		//	this->mainRenderTarget); // depth target

		//PassSetup::From({ rt0, rt1, rt2 }, rt0);
		PassSetup mainRenderPassSetup = PassSetup::From(
			{ this->mainRenderTarget, this->gbuffRenderTarget },
			this->mainRenderTarget // Use depth attachment in main target
		);

		mainRenderPassSetup.DefaultAction = PassAction::Clear(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f, 0);
		this->mainRenderPass = Gfx::CreateResource(mainRenderPassSetup);
	


	// Set up the background render target
	{
		float bgRenderDownscale = 0.25;
		backgroundRenderSetup = TextureSetup::RenderTarget2D(
			(int)(1280 * bgRenderDownscale),
			(int)(720 * bgRenderDownscale), PixelFormat::RGBA16F, PixelFormat::None);
		backgroundRenderSetup.SampleCount = 1;
		//backgroundRenderSetup.SampleCount = mainRenderSampleCount;

		backgroundRenderSetup.Sampler.MinFilter = TextureFilterMode::Linear;
		backgroundRenderSetup.Sampler.MagFilter = TextureFilterMode::Linear;
		backgroundRenderSetup.Sampler.WrapU = TextureWrapMode::Repeat;
		backgroundRenderSetup.Sampler.WrapV = TextureWrapMode::Repeat;

		this->backgroundRenderTarget = Gfx::CreateResource(backgroundRenderSetup);

		PassSetup backgroundRenderPassSetup = PassSetup::From(
			this->backgroundRenderTarget,  // color target
			Id::InvalidId() ); // depth target

		backgroundRenderPassSetup.DefaultAction = PassAction::Clear(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f, 0);
		this->backgroundRenderPass = Gfx::CreateResource(backgroundRenderPassSetup);

		this->backgroundDrawState.Mesh[0] = quadMesh;
		Id shaderBG = Gfx::CreateResource(BackgroundShader::Setup());
		auto psBackgroundRndr = PipelineSetup::FromLayoutAndShader(quadSetup.Layout, shaderBG);
		psBackgroundRndr.BlendState.ColorFormat = backgroundRenderSetup.ColorFormat;
		psBackgroundRndr.BlendState.DepthFormat = backgroundRenderSetup.DepthFormat;
		psBackgroundRndr.RasterizerState.SampleCount = 1;
		//psBackgroundRndr.RasterizerState.SampleCount = gfxSetup->SampleCount;
		this->backgroundDrawState.Pipeline = Gfx::CreateResource(psBackgroundRndr);

		this->bgquadDrawState.Mesh[0] = quadMesh;
		Id bgquadShd = Gfx::CreateResource(BGQuadShader::Setup());
		auto psBGQuad = PipelineSetup::FromLayoutAndShader(quadSetup.Layout, bgquadShd);
		psBGQuad.RasterizerState.SampleCount = mainRenderSetup.SampleCount;
		psBGQuad.BlendState.ColorFormat = mainRenderSetup.ColorFormat;
		psBGQuad.BlendState.DepthFormat = mainRenderSetup.DepthFormat;
		psBGQuad.BlendState.MRTCount = 2;
		this->bgquadDrawState.Pipeline = Gfx::CreateResource(psBGQuad);
	}

}

void Renderizer::testShadowStuff(float shadNear, float shadFar)
{
	shadowCamera.Setup(glm::vec3(0.0, 0.0, 25.0), glm::radians(45.0f),
		shadowMapSize, shadowMapSize, shadNear, shadFar );

	//shadowCamera.SetupShadow(glm::vec3(0.0, 0.0, 25.0), 20.0f, shadowMapSize, shadowMapSize, shadNear, shadFar );

	shadowCamera.Pos = glm::vec3(9.22, -29.90, 34.06) *0.9f;
	shadowCamera.Rot = glm::vec2(0.28, 0.68);

	// This applies the pos/rot
	shadowCamera.MoveRotate(glm::vec3(0.0f), glm::vec2(0.0f));

	
}

void Renderizer::renderScene(Tapnik::Scene* scene, Tapnik::UIAssets* uiAssets)
{
	// the shadow pass
	// this->shadowVSParams.mvp = this->lightProjView;
	if (scene) {
		Gfx::BeginPass(shadowPass);

		// this->shapeRenderer.DrawShadows(this->shadowVSParams);
		scene->drawShadowPass(shadowDrawState);
		Gfx::EndPass();
	}

	// Draw the background pass itself
	Gfx::BeginPass(this->backgroundRenderPass);

	backgroundVSparams.size = glm::vec2(1.0f, 1.0f);
	backgroundVSparams.offs = glm::vec2(0.0, 0.0);
	if (activeCamera) {

		glm::mat4x4 camXform = activeCamera->Model;
		
		//backgroundFSparams.cameraXform = glm::inverse(camXform);
		backgroundFSparams.cameraXform = camXform;
		backgroundFSparams.aspect = uiAssets->fbWidth / uiAssets->fbHeight;
		backgroundFSparams.aTime = sceneTime;
	}

	//this->backgroundVSparams.FSTexture[PostProcShader::tex] = mainRenderTarget;
	Gfx::ApplyDrawState(this->backgroundDrawState);
	Gfx::ApplyUniformBlock(this->backgroundVSparams);
	Gfx::ApplyUniformBlock(this->backgroundFSparams);
	Gfx::Draw();
	Gfx::EndPass();

	// Draw the scene into the offscreen buffer
	Gfx::BeginPass(this->mainRenderPass);

	// Draw the background quad	
	bgquadFSParams.size = glm::vec2(1.0f, 1.0f);
	bgquadFSParams.offs = glm::vec2(0.0, 0.0);

	this->bgquadDrawState.FSTexture[BGQuadShader::tex] = backgroundRenderTarget;
	//this->bgquadDrawState.FSTexture[BGQuadShader::tex] = backgroundRenderTarget;
	Gfx::ApplyDrawState(this->bgquadDrawState);
	Gfx::ApplyUniformBlock(this->bgquadFSParams);
	Gfx::Draw();

	// Draw the actual scene
	if ((scene)&&(drawMainScene)) {
		scene->drawScene(shadowMap);
	}
}

void Renderizer::finishMainPass()
{
	Gfx::EndPass();
  
	// Start the main render pass with post-proc 
	// (for debug text and stuff)
	Gfx::BeginPass(this->passAction);

	// Draw the post-processes quad
	postProcFSparams.size = glm::vec2(1.0f, 1.0f);
	postProcFSparams.offs = glm::vec2(0.0, 0.0);
	this->postProcDrawState.FSTexture[PostProcShader::tex] = mainRenderTarget;
	this->postProcDrawState.FSTexture[PostProcShader::depthTex] = gbuffRenderTarget;
	Gfx::ApplyDrawState(this->postProcDrawState);
	Gfx::ApplyUniformBlock(this->postProcFSparams);
	Gfx::Draw();
}

void Renderizer::finishRender(Tapnik::UIAssets* uiAssets)
{
    // Draw the shadow debug
    if (debugDrawShadowMap) {
		float aspect = uiAssets->fbWidth / uiAssets->fbHeight;
        shadowDebugFSparams.size = glm::vec2( 0.5f, 0.5f * aspect );
        shadowDebugFSparams.offs = glm::vec2( -0.42, 0.42 );
        this->shadowDebugDrawState.FSTexture[ DebugShadowShader::tex ] = shadowMap;
        Gfx::ApplyDrawState(this->shadowDebugDrawState);    
        Gfx::ApplyUniformBlock(this->shadowDebugFSparams);
        Gfx::Draw();
    }
    
	Gfx::EndPass();
}
