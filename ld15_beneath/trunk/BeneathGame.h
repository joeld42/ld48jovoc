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

struct Jgui_UIContext;

class BeneathGame
{
public:
	BeneathGame();
	~BeneathGame();

	// fixed update, for gameplay stuff
	void updateSim( float dt );

	// "as fast as possible" update for effects and stuff
	void update( float dt );
	
	// during game state itself
	void game_redraw();
	void game_updateSim( float dt );	
	void game_update( float dt );
	void game_keypress( SDL_KeyboardEvent &key );
	
	void loadShapes( const char *filename );

	// generic stuff
	void redraw();
	void init();
	void keypress( SDL_KeyboardEvent &key );
	void mouse( SDL_MouseButtonEvent &mouse );

	void newGame();
	void startEditor();

	// game state
	enum GameState {
		GameState_MENU,
		GameState_EDITOR,
		GameState_GAME
	};

	bool done();
	void done( bool i_am_done );

protected:
	bool m_isInit;
	bool m_playtest; // testing a level in editor?

	GameState m_gameState;
	Editor *m_editor;

	// Gui Context
	Jgui_UIContext *m_uiCtx;

	Shape *m_player;
	// todo: put this in a player class
	vec2f m_vel;

	Cavern *m_level;
	std::vector<Shape*> m_shapes;

	// debug/info font
	GLuint m_glFontTexId;
	GLuint m_fntFontId;

	// main app should quit?
	bool m_done;

};


#endif