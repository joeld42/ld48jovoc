#ifndef COMMON_H
#define COMMON_H

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <prmath/prmath.hpp>

// lookup textureID or load if not exist
GLuint getTexture( const std::string &filename, int *xsize=NULL, int *ysize=NULL );

// Makes a perspective camera that matches the ortho camera on the Z=0 plane
void pseudoOrtho2D( double left, double right, double bottom, double top );

float distPointLine( const vec2f &a, const vec2f &b, const vec2f &c, vec2f &p );

/// Mmmm... pie
#ifndef M_PI
# define M_PI (3.1415926535897932384626433832795)
#endif

#define D2R (M_PI / 180.0 )
#define R2D (180.0 / M_PI );

#endif
