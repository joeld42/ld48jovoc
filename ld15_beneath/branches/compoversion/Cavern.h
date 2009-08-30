#ifndef CAVERN_H
#define CAVERN_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <prmath/prmath.hpp>

#include <Shape.h>

class Cavern
{
public:
	Cavern();

	void loadLevel( const char *levelFile );

	void addShape( Shape *s );

	void draw();

//protected:
	vec2f m_mapSize;
	std::vector<Shape*> m_shapes;	
	vec3f m_bgColor;
};

#endif