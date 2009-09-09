#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
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

#include <enet/enet.h>
#include <Shape.h>

class GameObj
{
public:
	GameObj( Shape *shape );

	Shape *m_shape;
};


#endif