#ifndef LUDDITE_H
#define LUDDITE_H

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <GL/gl.h>
#include "resource.h"
#include "texture.h"

// common utilities and stuff
// TODO

// resource shortcuts
GLuint Texture_get( const char *filename );

#endif