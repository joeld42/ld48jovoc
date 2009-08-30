#ifndef COMMON_H
#define COMMON_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

// lookup textureID or load if not exist
GLuint getTexture( const std::string &filename, int *xsize=NULL, int *ysize=NULL );

// Makes a perspective camera that matches the ortho camera on the Z=0 plane
void pseudoOrtho2D( double left, double right, double bottom, double top );

/// Mmmm... pie
#define M_PI (3.1415926535897932384626433832795)
#define D2R (M_PI / 180.0 )
#define R2D (180.0 / M_PI );

#endif