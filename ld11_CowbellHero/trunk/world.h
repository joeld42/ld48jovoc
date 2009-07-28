#include <stdlib.h>

#ifndef FOCUS_WORLD_H
#define FOCUS_WORLD_H

#include <windows.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <fmod.h>

#include "beat.h"
#include "background.h"

enum 
{
	GEM_REST,

	GEM_NOTE, // unplayed
	GEM_NOTE_HIT, 
	GEM_NOTE_MISSED,
	GEM_NOTE_STARPOWER, 
};

// The main "game" class
struct World
{
	World();
	~World();

	Background *bg;

	void redraw( Beatronome &nome);
	void update( Beatronome &nome );
	void init();
	
	// The music
	FSOUND_STREAM *mp3file;

	void setMusic( const char *musicfile );
	void playMusic();
	void stopMusic();

	void makeGemTrack();

	std::vector<unsigned char> gemtrack;

	// gameplay gfx
	ILuint ilFretboard,ilGem, ilTarget, ilBackground;
	GLuint texFretboard, texGem, texTarget, texBackground;

	bool initialized;
};

#endif