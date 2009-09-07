#ifndef GAMECLIENT_H
#define GAMECLIENT_H

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
#include <Server.h>
#include <GameObj.h>
#include <PlayerShip.h>

struct Jgui_UIContext;

#define NAME_MAX_LEN (32)
#define NAME_MAX_ADDR (64)

class GameClient
{
public:
	GameClient();
	~GameClient();

	// fixed update, for gameplay stuff
	void updateSim( float dt );

	// "as fast as possible" update for effects and stuff
	void update( float dt );
	
	// network update
	void net_update();

	// during game state itself
	void gameview_redraw();	
	void game_keypress( SDL_KeyboardEvent &key );
	
	void loadShapes( const char *filename );

	// generic stuff
	void redraw();
	void init();
	void keypress( SDL_KeyboardEvent &key );
	void mouse( SDL_MouseButtonEvent &mouse );

	void lobbySinglePlayer();
	void lobbyMultiPlayer();

	void startGame();
	void startEditor();

	void joinGame();

	// game state
	enum GameState {
		GameState_MENU,			
		GameState_EDITOR,
		GameState_GAME
	};

	enum MenuState {
		MenuState_MAINMENU,
		MenuState_LOBBY,
		MenuState_MAPSELECT,
		MenuState_BIDDING,
	};

	bool done();
	void done( bool i_am_done );

	// network stuff
	void connectServer( const char *address );

	void chatSend( const char *message );

protected:
	

	bool m_isInit;
	bool m_playtest; // testing a level in editor?

	GameState m_gameState;
	MenuState m_menuState;
	
	// Editor stuff
	Editor *m_editor;	

	// The server (if we're hosting a local server)
	// Shouldn't really access this directly for consitancy
	// with network peers
	Server *m_localServer;
	
	// The connection to the game server
	ENetAddress m_netAddress;
	ENetHost *m_netServer;
	ENetPeer *m_serverPeer;

	void sendPacket( int channel, pbSpaceCave::Packet &pbPacket );
	void handlePacket( int channel, ENetPacket *packet, ENetPeer *peer );

	// Gui Context
	Jgui_UIContext *m_uiCtx;	

	// UI Screens
	void doMapSelectScreen();
	void doLobbyScreen();

	// lobby info
	char m_lobbyName[NAME_MAX_LEN];
	char m_remoteAddr[NAME_MAX_ADDR];
	//uint32 m_playerColor;	

	// Game data
	std::vector<Shape*> m_shapes;
	std::vector<GameObj*> m_gameObjs;
	std::vector<PlayerShip*> m_ships;
	PlayerShip *m_player;
	Cavern *m_level;

	// Controls
	int m_fwd_thrusters;
	int m_lateral_thrusters;
	int m_rotation_amt;
	::pbSpaceCave::Packet m_pbCtrlPacket;

	// debug/info font
	GLuint m_glFontTexId;
	GLuint m_fntFontId;

	// main app should quit?
	bool m_done;

};

#endif