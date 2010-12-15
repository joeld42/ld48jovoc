#include <string.h>

#include "texture.h"
#include "debug.h"

#include "PNGLoader.h"

using namespace Luddite;

// adapted from
// http://graphics.stanford.edu/~seander/bithacks.html
bool isPow2(unsigned int v)
{    
    unsigned int c; // c accumulates the total bits set in v
    for (c = 0; v; c++)
    {
        v &= v - 1; // clear the least significant bit set
    }

    return (c==1);    
}



//  entry points for the resource manager
bool loadResource( const char *filename, TextureGL *texture )
{
    // Remember filename
    texture->m_texName = filename;    

    // Determine file extension
	const char *ext = strrchr( filename, '.');
	if ((!ext)||(strlen(ext)<4))
	{
		DBG::warn("Can't determine file extension for texture %s\n", filename );
        return false;        
	}    
	
	// Use the extension to determine format
	if ( strncmp( ext, ".png", 4)==0)              
    {
		PNGImage pngImage = LoadImagePNG( filename );

        // Store the texture id
		texture->m_texId = pngImage.textureId;        
		texture->m_width = pngImage.widthPow2;
		texture->m_height = pngImage.heightPow2;
		texture->m_origWidth = pngImage.width;
		texture->m_origHeight = pngImage.height;
		
        return true;        
    }

	// Unsupported format
	DBG::warn( "Unsupported texture format '%s' for texture '%s'\n", 
               ext, filename );

	return false;
}

void unloadResource( TextureGL *texture )
{
    // Release the texture from opengl
    glDeleteTextures( 1, &texture->m_texId );    
}


HTexture TextureDB::getTexture( const char *name )
{
    return BaseTextureDB::getResource( name );    
}

void TextureDB::freeTexture( HTexture hTex )
{
    return BaseTextureDB::freeResource( hTex );
}

const std::string &TextureDB::getTextureName( HTexture hTex )
{
    TextureGL *tex = m_resMgr.deref( hTex );
    AssertPtr( tex );
    
    return tex->m_texName;    
}


GLuint TextureDB::getTextureId( HTexture hTex )
{
    TextureGL *tex = m_resMgr.deref( hTex );
    AssertPtr( tex );
    
    return tex->m_texId;    
}

void TextureDB::reportTexture( const std::string &resName, unsigned int refCount, HTexture hTex )
{
	
	TextureDB &texDB = TextureDB::singleton();
	TextureGL *tex = texDB.m_resMgr.deref( hTex );

	DBG::info( "%10s|%3d|%4dx%-4d", resName.c_str(), refCount, 
				tex->m_width, tex->m_height  );
}
	
void TextureDB::reportUsage()
{
	DBG::info("=======================================\n" );
	DBG::info(" Texture Usage\n" );
	DBG::info("=======================================\n" );

	doReport( reportTexture );
}


