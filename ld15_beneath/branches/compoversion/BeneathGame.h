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

#include <Shape.h>
#include <Cavern.h>
#include <Editor.h>

class BeneathGame
{
public:
	BeneathGame();

	// fixed update, for gameplay stuff
	void updateSim( float dt );

	// "as fast as possible" update for effects and stuff
	void update( float dt );
	
	// during game state itself
	void game_redraw();
	void game_updateSim( float dt );	
	void game_update( float dt );
	void game_keypress( SDL_KeyboardEvent &key );
	
	// generic stuff
	void redraw();
	void init();
	void keypress( SDL_KeyboardEvent &key );

	// game state
	enum GameState {
		GameState_MENU,
		GameState_EDITOR,
		GameState_GAME
	};

protected:
	bool m_isInit;

	GameState m_gameState;
	Editor *m_editor;

	Shape *m_player;

	// debug/info font
	GLuint m_glFontTexId;
	GLuint m_fntFontId;

};

#endif