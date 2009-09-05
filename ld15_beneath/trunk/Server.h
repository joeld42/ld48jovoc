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

class Server
{
public:
	Server( std::vector<Shape*> &shapes );
	~Server();
	
	// during game state itself	
	void game_updateSim( float dt );	
	void game_update( float dt );
	
	//void game_keypress( SDL_KeyboardEvent &key );	
	void init();

	void newGame();

	Shape *m_player;
	Cavern *m_level;	

protected:
	bool m_isInit;	
	std::vector<Shape*> &m_shapes;
	
	// todo: put this in a player class
	vec2f m_vel;

	
};


#endif