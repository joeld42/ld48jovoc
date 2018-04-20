#ifndef TUNNEL_DEBUG_DRAW_H
#define TUNNEL_DEBUG_DRAW_H

#ifndef DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw_impl.h"
#endif

#include "wireframe_shaders.h"

namespace Tapnik
{
    
class DebugDrawRenderer : public dd::RenderInterface
{
public:
    //
    // These are called by dd::flush() before any drawing and after drawing is finished.
    // User can override these to perform any common setup for subsequent draws and to
    // cleanup afterwards. By default, no-ops stubs are provided.
    //
    virtual void beginDraw();
    virtual void endDraw();
    
    //
    // Create/free the glyph bitmap texture used by the debug text drawing functions.
    // The debug renderer currently only creates one of those on startup.
    //
    // You're not required to implement these two if you don't care about debug text drawing.
    // Default no-op stubs are provided by default, which disable debug text rendering.
    //
    // Texture dimensions are in pixels, data format is always 8-bits per pixel (Grayscale/GL_RED).
    // The pixel values range from 255 for a pixel within a glyph to 0 for a transparent pixel.
    // If createGlyphTexture() returns null, the renderer will disable all text drawing functions.
    //
    virtual dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void * pixels);
    virtual void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex);
    
    //
    // Batch drawing methods for the primitives used by the debug renderer.
    // If you don't wish to support a given primitive type, don't override the method.
    //
    virtual void drawPointList(const dd::DrawVertex * points, int count, bool depthEnabled);
    virtual void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled);
    virtual void drawGlyphList(const dd::DrawVertex * glyphs, int count, dd::GlyphTextureHandle dummyGlyphTex);
    
    // User defined cleanup. Nothing by default.
    virtual ~DebugDrawRenderer();
    
    // ---------------------------------
    // Implementaiton stuff
    // ---------------------------------
    void Setup(const Oryol::GfxSetup& gfxSetup);    
    void Discard();
    
    glm::mat4x4 debugDrawMVP;
    glm::mat4x4 debugDrawOrthoMVP;
    
    WireframeShader::vsParams wireVSParams;    
    PointShader::vsParams pointVSParams;
    GlyphShader::vsParams glyphVSParams;
    
    Oryol::Id pipelineLines;
    Oryol::Id meshLines;
    
    Oryol::Id pipelinePoints;
    Oryol::Id meshPoints;
    
    Oryol::Id pipelineGlyphs;
    Oryol::Id meshGlyphs;
    
    Oryol::Id glyphTex;
    
    // draw calls for this frame
    int drawLineCount;
    int drawPointCount;
    int drawGlyphCount;
    
    Oryol::DrawState drawState;
    Oryol::ResourceLabel label;
    Oryol::ResourceLabel labelGlyphTex;
};

} // namespace Tapnik

#endif
