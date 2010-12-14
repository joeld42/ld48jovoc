#include <string.h>

#include "texture.h"
#include "debug.h"

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

#if 0
	// DDS textures
	if (strncmp( ext, ".dds", 4)==0)
	{        
        texture->m_texId = Texture_load_DDS( filename );        
        return true;        
	}
#endif

    // textures supported by stb_image
    // TODO: support hdr better
	else if ( (strncmp( ext, ".png", 4)==0) ||
              (strncmp( ext, ".jpg", 4)==0) ||
              (strncmp( ext, ".tga", 4)==0) ||
              (strncmp( ext, ".bmp", 4)==0) ||
              (strncmp( ext, ".psd", 4)==0) ||
              (strncmp( ext, ".gif", 4)==0) ||
              (strncmp( ext, ".pic", 4)==0) ||
              (strncmp( ext, ".hdr", 4)==0) )
    {
        unsigned char *data = NULL;
        int w, h, n;
        
        // Load the pixel data
        //data = stbi_load( filename, &w, &h, &n, 0 );
        if (!data)
        {
            DBG::error( "Failed to load texture '%s'\n", filename );
            return false;            
        }
        

        // warn if doesn't fit 
        // TODO: maybe resize or pad?
        if ((!isPow2(w)) || (!isPow2(h)))
        {
            DBG::warn( "Non-power-of-two texture '%s' (%dx%d)\n" ,
                       filename, w, h );            
        }

        // Get a texture id
        glGenTextures( 1, &(texture->m_texId) );

        // Bind the texture
        glBindTexture( GL_TEXTURE_2D, texture->m_texId );

        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        //glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

        GLint fmt;
        switch(n)
        {
        case 1:
            fmt = GL_LUMINANCE;
            break;
        case 2:
            fmt = GL_LUMINANCE_ALPHA;
            break;
        case 3:
            fmt = GL_RGB;
            break;
        case 4:
            fmt = GL_RGBA;            
            break;
        default:
            DBG::error( "Unexpected # components (%d) in texture '%s'",
                        n, filename );            
            break;            
        }
        
        glTexImage2D( GL_TEXTURE_2D, 0, n, 
                      w, h, 0,
                      fmt, GL_UNSIGNED_BYTE, data );        
        
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


