#ifndef FOCUS_BACKGROUND_H
#define FOCUS_BACKGROUND_H

#include <windows.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "pally.h"

struct Background
{
	virtual void redraw();
	virtual void update( float bpm );
};

// "mondrian"-like cells
struct BGMondrian : public Background
{
	BGMondrian();

	virtual void redraw();
	virtual void update( float bpm );

	void doRedraw( float left, float right, float bottom, float top, int axis, int level );

	float angle;	
	Pally pally;
};



#endif