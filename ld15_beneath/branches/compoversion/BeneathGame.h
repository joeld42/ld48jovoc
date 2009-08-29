#ifndef BENEATH_GAME_H
#define BENEATH_GAME_H

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


class BeneathGame
{
public:
	BeneathGame();

	// fixed update, for gameplay stuff
	void updateSim( float dt );

	// "as fast as possible" update for effects and stuff
	void update( float dt );
	
	void redraw();

	void init();

protected:
	bool m_isInit;

	// debug/info font
	GLuint m_glFontTexId;
	GLuint m_fntFontId;

};

#endif