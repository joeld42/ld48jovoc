#ifndef MESH_H
#define MESH_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <prmath/prmath.hpp>

// loads an obj file into a display list
GLuint LoadObjFile( const char *obj, vec3f scale = vec3f(1.0, 1.0, 1.0) );

#endif