#include "Planet.h"
#include "SceneObject.h"

using namespace Oryol;

Planet::Planet() : _built(false), worldSize( 3000.0f )
{
}

// ==================================================================
//   Engine stuff
// ==================================================================

// FIXME: move this into planet and out of Isobuilder
//float Planet::evalSDF( glm::vec3 p )
//{
//    // Test SDF
//    return glm::length(p)-1.0;
//}

// ==================================================================
//   Engine stuff
// ==================================================================
void Planet::Setup( Oryol::GfxSetup *gfxSetup, struct osn_context *noiseCtx )
{
    _noiseCtx = noiseCtx;
    
    planetShader = Gfx::CreateResource( PlanetShader::Setup());
    
    surfLayout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Normal, VertexFormat::Float3 },
        { VertexAttr::TexCoord0, VertexFormat::Float2 },
        { VertexAttr::Color0, VertexFormat::Float4 }
    };
    
    auto ps = PipelineSetup::FromLayoutAndShader( surfLayout, planetShader );
    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    ps.RasterizerState.SampleCount = gfxSetup->SampleCount;
    planetDrawState.Pipeline = Gfx::CreateResource(ps);
}

void Planet::Rebuild( Scene *scene )
{
    
    if (_built) {
        // clean up mesh
        Gfx::DestroyResources( planetResource );
    }
    
    // Build isosurf
    surfBuilder.Layout = surfLayout;
    surfBuilder.noiseCtx = _noiseCtx;
    
    Gfx::PushResourceLabel();
    meshIsosurf = Gfx::CreateResource( surfBuilder.Build( worldSize ) );
    planetResource = Gfx::PopResourceLabel();
    
    planetApproxRadius = 3000.0f * 0.8f;
    
    _built = true;
    
    
}

void Planet::UpdateCamera( Camera *camera )
{
    planetVSParams.ModelViewProjection = camera->ViewProj;
    
    planetFSParams.LightColor = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
    planetFSParams.MatDiffuse = glm::vec4( 0.92,0.41,0.25, 1.0 );
    planetFSParams.MatSpecular = glm::vec4( 1.0, 0.8, 0.5, 1.0 ) * 3.0f;
    //planetFSParams.LightDir = glm::normalize( glm::vec3( -1.0, -0.3, 0.1 ) );
    planetFSParams.AmbientColor = glm::vec4( 0.42,0.36,0.71, 1.0) * 0.01f;
    planetFSParams.RimColor = glm::vec4( 0.82,0.74,0.95,1.0) * 0.2f;
    planetFSParams.RimPower = 10.0;
    planetFSParams.EyePos = camera->Pos;
    planetFSParams.MatSpecularPower = 50.0f;
    planetFSParams.GammaCorrect = 1;
}

void Planet::Draw()
{
    if (_built) {
        const auto resStateTex = Gfx::QueryResourceInfo( planetTexture ).State;
        const auto resStateMesh = Gfx::QueryResourceInfo( meshIsosurf ).State;
        
        if ((resStateTex == ResourceState::Valid) && (resStateMesh == ResourceState::Valid)) {
            //planetDrawState.FSTexture[Textures::Texture] = planetTexture;
            planetDrawState.Mesh[0] = meshIsosurf;
            
            Gfx::ApplyDrawState( planetDrawState );
            Gfx::ApplyUniformBlock( planetVSParams );
            Gfx::ApplyUniformBlock( planetFSParams );
            
            // Planet has 1 prim..
            Gfx::Draw(0);
        } else {
            printf("planet resources not valid?.\n");
        }

    }
}
