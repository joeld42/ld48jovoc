#ifndef USEFUL_H
#define USEFUL_H

#include <prmath/prmath.hpp>

// ==============================
//  Random numbers
// ==============================

// Generates a uniform random number between 0 and 1
float randUniform();

// Generates a uniform random number between minVal and maxVal
float randUniform( float minVal, float maxVal );

// Generates a random number from a normal (gaussian) distribution
// with a mean of 0 and a std deviation of 1
// (Box-Muller transform)
// NOTE: not threadsafe
float randNormal();

// Generates a random number from a normal distribution
// with the given mean and stddev
//
// NOTE: not threadsafe
float randNormal( float mean, float stddev );

// ==============================
//  general mathy stuff
// ==============================
float sgn( float n );

// ==============================
//  GL error checking
// ==============================

int checkForGLErrors( const char *s, const char * file, int line );
void checkFBO();

#define CHECKGL( msg ) checkForGLErrors( msg, __FILE__, __LINE__ );

// ==============================
//  GL camera math
// ==============================

void glhFrustumf2(matrix4x4f &matrix, 
                  float left, float right, float bottom, float top,
                  float znear, float zfar);

void glhPerspectivef2(matrix4x4f &matrix, float fovyInDegrees, float aspectRatio,
                      float znear, float zfar);



#endif

