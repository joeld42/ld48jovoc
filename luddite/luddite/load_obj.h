#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

// Crappy obj loader for prototyping. 
// DO NOT USE IN REAL STUFF
#include <windows.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

GLuint load_obj( const char *filename );

#endif