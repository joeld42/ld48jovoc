#define DEBUG_DRAW_IMPLEMENTATION

#include "Pre.h"
#include "Gfx/Gfx.h"
//#include "wireframe_shaders.h"

#include "debug_draw_impl.h"
#include "wireframe_shaders.h"

#include "DebugDraw.h"

using namespace Oryol;
using namespace Tapnik;
using namespace dd;

//------------------------------------------------------------------------------
// Interface to Debug_draw renderer
//------------------------------------------------------------------------------
DebugDrawRenderer::~DebugDrawRenderer()
{
    Discard();
}

void DebugDrawRenderer::beginDraw()
{
    drawLineCount = 0;
    drawPointCount = 0;
    drawGlyphCount = 0;
}

void DebugDrawRenderer::endDraw()
{
    if (drawLineCount > 1) {
        Log::Warn( "Tried to draw %d line lists..\n", drawLineCount);
    }
    if (drawPointCount > 1) {
        Log::Warn( "Tried to draw %d point lists..\n", drawPointCount);
    }
    if (drawGlyphCount > 1) {
        Log::Warn( "Tried to draw %d line lists..\n", drawGlyphCount);
    }
}

void DebugDrawRenderer::drawPointList(const dd::DrawVertex * points, int count, bool depthEnabled)
{
    drawPointCount++;
    if (drawPointCount > 1) return;
    
    this->drawState.Pipeline = pipelinePoints;
    this->drawState.FSTexture[GlyphShader::tex] = Id::InvalidId();
    this->drawState.Mesh[0] = meshPoints;
    
    Gfx::UpdateVertices(meshPoints, points, count*sizeof(dd::DrawVertex));
    
    Gfx::ApplyDrawState(this->drawState);
    
    pointVSParams.viewProj = this->debugDrawMVP;
    Gfx::ApplyUniformBlock(pointVSParams );
    Gfx::Draw({ 0, count });

}

void DebugDrawRenderer::drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled)
{
    drawLineCount++;
    if (drawLineCount > 1) return;
    
    this->drawState.Pipeline = pipelineLines;
    this->drawState.FSTexture[GlyphShader::tex] = Id::InvalidId();
    this->drawState.Mesh[0] = meshLines;
    
    Gfx::UpdateVertices(meshLines, lines, count*sizeof(dd::DrawVertex));
    
    Gfx::ApplyDrawState(this->drawState);    
    wireVSParams.viewProj = this->debugDrawMVP;
    Gfx::ApplyUniformBlock( wireVSParams );
    Gfx::Draw({ 0, count });
}

void DebugDrawRenderer::drawGlyphList(const DrawVertex * glyphs, int count, GlyphTextureHandle dummyGlyphTex)
{
    drawGlyphCount++;
    if (drawGlyphCount > 1) return;
    
    this->drawState.Pipeline = pipelineGlyphs;
    this->drawState.Mesh[0] = meshGlyphs;
    this->drawState.FSTexture[GlyphShader::tex] = this->glyphTex;
    
    Gfx::UpdateVertices(meshGlyphs, glyphs, count*sizeof(dd::DrawVertex));
    
    Gfx::ApplyDrawState(this->drawState);
    glyphVSParams.viewProj = this->debugDrawOrthoMVP;
    Gfx::ApplyUniformBlock( glyphVSParams );
    Gfx::Draw({ 0, count });

}

GlyphTextureHandle DebugDrawRenderer::createGlyphTexture(int width, int height, const void * pixels)
{
    Gfx::PushResourceLabel();
    
    auto glyphTexSetup = TextureSetup::FromPixelData2D( width, height, 1, PixelFormat::Code::L8 );
    glyphTexSetup.Sampler.MinFilter = TextureFilterMode::NearestMipmapLinear;
    glyphTexSetup.Sampler.MagFilter = TextureFilterMode::Linear;
    glyphTexSetup.ImageData.Sizes[0][0] = width*height;
    this->glyphTex = Gfx::CreateResource( glyphTexSetup, pixels, width*height );
                                                       
    this->labelGlyphTex = Gfx::PopResourceLabel();
    
    // We're saving the resource Id, so we just need to return nonzero
    return (GlyphTextureHandle)1;
}

void DebugDrawRenderer::destroyGlyphTexture(GlyphTextureHandle glyphTex)
{
    Gfx::DestroyResources(this->labelGlyphTex);
}



//------------------------------------------------------------------------------
// Implmentation
// Mostly adapted from Wireframe.h in Oryol samples...
//------------------------------------------------------------------------------
void DebugDrawRenderer::Setup(const GfxSetup& gfxSetup)
{
    Gfx::PushResourceLabel();
    
    // --- Line Render -----
    auto lineRenderSetup = MeshSetup::Empty( DEBUG_DRAW_VERTEX_BUFFER_SIZE, Usage::Stream);
    lineRenderSetup.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Color0, VertexFormat::Float4 }   // Really a float3, the extra float is padding bc of how dd::DrawVert
    };
    meshLines = Gfx::CreateResource(lineRenderSetup);
    
    Id wireShader = Gfx::CreateResource(WireframeShader::Setup());
    auto pipSetup = PipelineSetup::FromLayoutAndShader(lineRenderSetup.Layout, wireShader);
    pipSetup.RasterizerState.SampleCount = gfxSetup.SampleCount;
    pipSetup.BlendState.BlendEnabled = false; // JBD: fixme was true
    pipSetup.BlendState.SrcFactorRGB = BlendFactor::SrcAlpha;
    pipSetup.BlendState.SrcFactorAlpha = BlendFactor::Zero;
    pipSetup.BlendState.DstFactorRGB = BlendFactor::OneMinusSrcAlpha;
    pipSetup.BlendState.DstFactorAlpha = BlendFactor::One;
    pipSetup.BlendState.ColorFormat = gfxSetup.ColorFormat;
    pipSetup.BlendState.DepthFormat = gfxSetup.DepthFormat;
    
    pipSetup.RasterizerState.CullFaceEnabled = false;
    pipSetup.RasterizerState.CullFace = Face::Code::Front;
    pipSetup.DepthStencilState.DepthWriteEnabled = true;
    pipSetup.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    
    pipSetup.PrimType = PrimitiveType::Lines;
    this->pipelineLines = Gfx::CreateResource(pipSetup);
    
    // --- Point Render -----
    
    auto pointRenderSetup = MeshSetup::Empty( DEBUG_DRAW_VERTEX_BUFFER_SIZE, Usage::Stream);
    pointRenderSetup.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Color0, VertexFormat::Float4 },
    };
    this->meshPoints = Gfx::CreateResource(pointRenderSetup);
    
    Id pointShader = Gfx::CreateResource(PointShader::Setup());
    pipSetup = PipelineSetup::FromLayoutAndShader(pointRenderSetup.Layout, pointShader);
    pipSetup.RasterizerState.SampleCount = gfxSetup.SampleCount;
    pipSetup.BlendState.BlendEnabled = true;
    pipSetup.BlendState.SrcFactorRGB = BlendFactor::SrcAlpha;
    pipSetup.BlendState.SrcFactorAlpha = BlendFactor::Zero;
    pipSetup.BlendState.DstFactorRGB = BlendFactor::OneMinusSrcAlpha;
    pipSetup.BlendState.DstFactorAlpha = BlendFactor::One;
    pipSetup.BlendState.ColorFormat = gfxSetup.ColorFormat;
    pipSetup.BlendState.DepthFormat = gfxSetup.DepthFormat;
    
    pipSetup.RasterizerState.CullFaceEnabled = false;
    pipSetup.RasterizerState.CullFace = Face::Code::Front;
    pipSetup.DepthStencilState.DepthWriteEnabled = true;
    pipSetup.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    
    pipSetup.PrimType = PrimitiveType::Points;
    this->pipelinePoints = Gfx::CreateResource(pipSetup);
    
    // --- Glyph Render -----
    
    auto glyphRenderSetup = MeshSetup::Empty( DEBUG_DRAW_VERTEX_BUFFER_SIZE, Usage::Stream);
    glyphRenderSetup.Layout = {
        { VertexAttr::Position, VertexFormat::Float2 },
        { VertexAttr::TexCoord0, VertexFormat::Float2 },
        { VertexAttr::Color0, VertexFormat::Float3 },
    };
    this->meshGlyphs = Gfx::CreateResource(glyphRenderSetup);
    
    Id glyphShader = Gfx::CreateResource(GlyphShader::Setup());
    pipSetup = PipelineSetup::FromLayoutAndShader(glyphRenderSetup.Layout, glyphShader);
    pipSetup.RasterizerState.SampleCount = gfxSetup.SampleCount;
    pipSetup.BlendState.BlendEnabled = true;
    pipSetup.BlendState.SrcFactorRGB = BlendFactor::SrcAlpha;
    pipSetup.BlendState.SrcFactorAlpha = BlendFactor::Zero;
    pipSetup.BlendState.DstFactorRGB = BlendFactor::OneMinusSrcAlpha;
    pipSetup.BlendState.DstFactorAlpha = BlendFactor::One;
    pipSetup.BlendState.ColorFormat = gfxSetup.ColorFormat;
    pipSetup.BlendState.DepthFormat = gfxSetup.DepthFormat;
    
    pipSetup.RasterizerState.CullFaceEnabled = false;
    pipSetup.RasterizerState.CullFace = Face::Code::Front;
    pipSetup.DepthStencilState.DepthWriteEnabled = true;
    pipSetup.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    
    pipSetup.PrimType = PrimitiveType::Triangles;
    this->pipelineGlyphs = Gfx::CreateResource(pipSetup);
    

    // Done with this resource label
    this->label = Gfx::PopResourceLabel();
}

//------------------------------------------------------------------------------
void DebugDrawRenderer::Discard() {
    Gfx::DestroyResources(this->label);
}

////------------------------------------------------------------------------------
//void DebugDrawRenderer::Render() {
//    if (!this->vertices.Empty()) {
//        Gfx::UpdateVertices(this->drawState.Mesh[0], this->vertices.begin(), this->vertices.Size()*sizeof(Vertex));
//        Gfx::ApplyDrawState(this->drawState);
//        WireframeShader::vsParams vsParams;
//        vsParams.viewProj = this->ViewProj;
//        Gfx::ApplyUniformBlock(vsParams);
//        Gfx::Draw({ 0, this->vertices.Size() });
//        this->vertices.Reset();
//    }
//}

