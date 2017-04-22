#include "Planet.h"
#include "SceneObject.h"

using namespace Oryol;

Planet::Planet() : _built(false)
{
    
}

void Planet::Setup( GfxSetup *gfxSetup )
{
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
    // Build isosurf
    surfBuilder.Layout = surfLayout;
    meshIsosurf = Gfx::CreateResource( surfBuilder.Build() );
    
    // Borrow texture
    planetTexture = scene->sceneInfos[0]->texture;
    
    _built = true;
}

void Planet::UpdateCamera( Camera *camera )
{
    planetVSParams.ModelViewProjection = camera->ViewProj;
    
    planetFSParams.LightColor = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
    planetFSParams.MatDiffuse = glm::vec4( 0.9, 0.9, 1.0, 1.0 );
    planetFSParams.MatSpecular = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
    planetFSParams.LightDir = glm::vec3( 1.0, 0.0, 0.0 );
    planetFSParams.EyePos = camera->Pos;
    planetFSParams.MatSpecularPower = 1.0f;
    planetFSParams.GammaCorrect = 0;
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
