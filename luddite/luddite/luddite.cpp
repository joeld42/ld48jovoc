#include "luddite.h"
#include "texture.h"
#include "resource.h"

GLuint Texture_get( const char *filename )
{
	return Resource_get( filename, Texture_load );
}