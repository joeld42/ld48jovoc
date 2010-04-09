#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <GL/gl.h>

// Compatible with resource.h
GLuint Texture_load( const char *filename );

#endif

