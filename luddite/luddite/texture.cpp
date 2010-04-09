#include <string.h>

#include "texture.h"
#include "debug.h"

// Loader implementations
#include "texture_dds.h"


static GLuint errorTextureId()
{
	// TODO: make an actual missing/error texture
	return 1;
}

GLuint Texture_load( const char *filename )
{
	const char *ext = strrchr( filename, '.');
	if ((!ext)||(strlen(ext)<4))
	{
		DBG::warn("Can't determine file extension for texture %s\n", filename );
		return errorTextureId();
	}

	// DDS textures
	if (strncmp( ext, ".dds", 4)==0)
	{
	}

	// Unsupported format
	DBG::warn( "Unsupported texture format '%s' for texture '%s'\n", ext, filename );
	return errorTextureId;
}