#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <GL/gl.h>

#include <luddite/handle.h>
#include <luddite/resource.h>

namespace Luddite
{

// ===========================================================================
struct TextureGL
{
    std::string m_texName;
	GLuint m_width;    // width bumped to pow2
	GLuint m_height;
	GLuint m_origWidth;  // orig width
	GLuint m_origHeight;
    GLuint m_texId;
};

// Typedefs
struct tag_TEXTURE {};
typedef Handle<tag_TEXTURE> HTexture;
typedef HandleMgr<TextureGL,HTexture> HTextureMgr;

// ===========================================================================
typedef ResourceMgr<TextureGL, HTexture> BaseTextureDB;

// All the template funkiness is hidden now. :)
class TextureDB : public BaseTextureDB
{
public:

    // Getting/releasing of textures
    HTexture getTexture( const char *name );
    void freeTexture( HTexture hTex );

    // Access to texture
    // TODO: needs to be reconsiled with the 
    // GraphicsES1/GraphicsES2 code
    GLuint getTextureId( HTexture hTex );    

    // Access to texture name
    const std::string &getTextureName( HTexture hTex );
	
	// Access to texture size
	// todo
	
	// Print texture usage report
	void reportUsage();
    
};

}; // namespace 'Luddite'
    
// Load/unload functions to use. Thesea are called by the
// TextureDB, and you probably don't need to call them 
// directly
bool loadResource( const char *filename, Luddite::TextureGL *texture );
void unloadResource( Luddite::TextureGL *texture );

#endif

