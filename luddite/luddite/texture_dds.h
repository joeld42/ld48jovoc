#ifndef TEXTURE_DDS
#define TEXTURE_DDS

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <GL/gl.h>

// creates and returns a GL textureID
GLuint Texture_load_DDS( const char *filename );

#endif

