#ifndef BENEATH_GAME_H
#define BENEATH_GAME_H

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
#include <Cavern.h>
#include <Editor.h>

// protobufs for packets
#include <pbSpaceCave/SpaceCave.pb.h>

// no point in having more since only 8 players max
#define MAX_CLIENTS (8)

// just some port above 49152
#define SPACECAVE_PORT (62511)

// Channels
enum {
	NET_CHANNEL_CHAT,	
	NET_CHANNEL_GAMEUPDATE,

	NUM_NET_CHANNELS
};

// ==================================================
enum {
	PacketType_CHAT_MESSAGE = 100,	

	PacketType_LAST
};

// ==================================================
class Server
{
public:
	Server( std::vector<Shape*> &shapes, bool singlePlayer );
	~Server();
	
	// during game state itself	
	void game_updateSim( float dt );	
	void game_update( float dt );

	void net_update( );
	
	//void game_keypress( SDL_KeyboardEvent &key );	
	void init();
	void initNetwork( );

	void newGame();

	bool m_singlePlayer;
	Shape *m_player;
	Cavern *m_level;	

protected:

	// Networking stuff
	ENetAddress m_serverAddr;
	ENetHost *m_clientHost;

	// Game stuff
	bool m_isInit;	
	std::vector<Shape*> &m_shapes;
	
	// todo: put this in a player class
	vec2f m_vel;	
};

#endif