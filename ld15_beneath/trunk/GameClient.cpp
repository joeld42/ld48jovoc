

#include <Common.h>
#include <Server.h>
#include <GameClient.h>

#include <tweakval.h>
#include <gamefontgl.h>
#include <jimgui.h>

#include <TinyXml.h>

#include <pbSpaceCave/SpaceCave.pb.h>
using namespace google;

GameClient::GameClient() :
	m_isInit( false ),
	m_editor( NULL ),
	m_gameState( GameState_MENU ),
	m_menuState( MenuState_MAINMENU ),
	m_playtest( false ),
	m_uiCtx( NULL ),
	m_done( false ),
	m_localServer( NULL ),
	m_netServer( NULL ),
	m_serverPeer( NULL ),
	m_player( NULL ),
	m_level( NULL ),
	
	m_fwd_thrusters( 0 ),
	m_lateral_thrusters( 0 ),
	m_rotation_amt( 0 )
{
	strcpy( m_lobbyName, "Player" );
	strcpy( m_remoteAddr, "127.0.0.1" );

	m_pbCtrlPacket.set_type( ::pbSpaceCave::Packet_Type_SHIPCONTROLS );
}

GameClient::~GameClient()
{
	delete m_editor;
	delete m_uiCtx;
	delete m_level;
}

bool GameClient::done()
{
	return m_done;
}

void GameClient::done( bool i_am_done )
{
	m_done = i_am_done;
}

void GameClient::update( float dt )
{
	// Update the UI context		
	if (m_uiCtx)
	{
		int mx, my;
		GLuint btns = SDL_GetMouseState( &mx, &my );		
		my = 600-my;
		m_uiCtx->m_mouseX = mx;
		m_uiCtx->m_mouseY = my;		
		m_uiCtx-> m_buttonState = btns;		
	}
	
	// Update current game state
	if (m_gameState==GameState_GAME)
	{
		if (m_localServer)
		{
			m_localServer->game_update( dt );
		}
	}
	else if (m_gameState==GameState_EDITOR)
	{
		if (m_editor) m_editor->update( dt );
	}

	// Update the server, if running
	// TODO: eventually this should be just about the 
	// only call to m_localServer
	if (m_localServer)
	{		
		m_localServer->net_update();
	}

	// Now update our networky stuff
	net_update();
}

void GameClient::net_update()
{
	// Make sure we're connected to something
	if (!m_netServer) return;

	ENetEvent event;
	while( enet_host_service( m_netServer, &event, 0 ) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			{
				printf("CLIENT: Connection to server succeeded\n" );
				
				// Now ask for an identity
				pbSpaceCave::Packet pbPacket;
				pbPacket.set_type( ::pbSpaceCave::Packet_Type_IDENTITY );		
		
				printf("CLIENT: request ident, lobby name %s\n", m_lobbyName );
				pbPacket.mutable_identity()->set_index( 999 );
				pbPacket.mutable_identity()->set_playername( std::string( m_lobbyName ) );

				sendPacket( NET_CHANNEL_GAMEUPDATE, pbPacket );
			}
			break;

		case ENET_EVENT_TYPE_RECEIVE:
				printf("CLIENT: A packet of length %u was recieved from %s on channel %u.\n",
					event.packet->dataLength,					
					event.peer->data,
					event.channelID );

				handlePacket( event.channelID, event.packet, event.peer );

				enet_packet_destroy( event.packet );
				break;
		}
	}	
}

void GameClient::updateSim( float dt )
{
	// If we're hosting the server, allow
	// it to update
	if (m_localServer)
	{		
		m_localServer->game_updateSim( dt );
	}

	// Update keyboard stuff

	// Continuous (key state) keys
	const int CONTROL_AMT = 100;
	Uint8 *keyState = SDL_GetKeyState( NULL );

	// rotation
	if ( ((keyState[SDLK_LEFT]) && (!keyState[SDLK_RIGHT])) ||
		 ((keyState[SDLK_a]) && (!keyState[SDLK_d])) )
	{
		m_rotation_amt = CONTROL_AMT;
	}
	else if ( ((!keyState[SDLK_LEFT]) && (keyState[SDLK_RIGHT])) ||
		     ((!keyState[SDLK_a]) && (keyState[SDLK_d])) )
	{
		m_rotation_amt = -CONTROL_AMT;
	}
	else m_rotation_amt = 0;

	// lateral thrust
	if ((keyState[SDLK_q]) && (!keyState[SDLK_e]))		 
	{
		m_lateral_thrusters = CONTROL_AMT;
	}
	else if ((!keyState[SDLK_q]) && (keyState[SDLK_e]))		     
	{
		m_lateral_thrusters = -CONTROL_AMT;
	}
	else m_lateral_thrusters = 0;

	// forward/backward
	if ( ((keyState[SDLK_UP]) && (!keyState[SDLK_DOWN])) ||
		 ((keyState[SDLK_w]) && (!keyState[SDLK_s])) )
	{
		m_fwd_thrusters = CONTROL_AMT;
	}
	else if ( ((!keyState[SDLK_UP]) && (keyState[SDLK_DOWN])) ||
				((!keyState[SDLK_w]) && (keyState[SDLK_s])) )
	{
		m_fwd_thrusters = -CONTROL_AMT;
	}
	else m_fwd_thrusters = 0;
		

	// If the controls have changed, tell the server	
	if ( (m_pbCtrlPacket.ctrl().fwd_thrusters() != m_fwd_thrusters) ||
		(m_pbCtrlPacket.ctrl().lateral_thrusters() != m_lateral_thrusters) ||
		(m_pbCtrlPacket.ctrl().rotation_amt() != m_rotation_amt) )
	{
		// stuff new controls into packet
		m_pbCtrlPacket.mutable_ctrl()->set_fwd_thrusters( m_fwd_thrusters );
		m_pbCtrlPacket.mutable_ctrl()->set_lateral_thrusters( m_lateral_thrusters );
		m_pbCtrlPacket.mutable_ctrl()->set_rotation_amt( m_rotation_amt );

		printf("CLIENT: Sending ctrl update\n" );
		sendPacket( NET_CHANNEL_GAMEUPDATE, m_pbCtrlPacket );
	}

}



void GameClient::init()
{
	// Load the font image
	ILuint ilFontId;
	ilGenImages( 1, &ilFontId );
	ilBindImage( ilFontId );		

	//glEnable( GL_DEPTH_TEST );
	glDisable( GL_DEPTH_TEST );
	
	if (!ilLoadImage( (ILstring)"gamedata/andelemo.png" )) {
		printf("Loading font image failed\n");
	}
	
	// Make a GL texture for it
	m_glFontTexId = ilutGLBindTexImage();
	m_fntFontId = gfCreateFont( m_glFontTexId );

	// A .finfo file contains the metrics for a font. These
	// are generated by the Fontpack utility.
	gfLoadFontMetrics( m_fntFontId, "gamedata/andelemo.finfo");

	//printf("font has %d chars\n", 
	//	gfGetFontMetric( m_fntFontId, GF_FONT_NUMCHARS ) );					


	// load all shapes
	loadShapes( "gamedata/shapes_Test.xml" );

	// Init UI context
	m_uiCtx = new Jgui_UIContext();
	Jgui_initContext( m_uiCtx );

	// TEMP: create a server object
	//m_localServer = new Server( m_shapes );
}
	
void GameClient::redraw()
{	
	// Init if needed
	if (!m_isInit)
	{
		m_isInit = true;
		init();
	}

	// set up view
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	pseudoOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	

	// Draw the game
	if (m_gameState == GameState_GAME )
	{
		gameview_redraw();
	}
	else if (m_gameState == GameState_EDITOR )
	{
		if (m_editor)
		{
			m_editor->redraw();
		}
	}
	else if (m_gameState == GameState_MENU )
	{
		glClearColor( _TV( 0.1f ), _TV(0.2f), _TV( 0.4f ), 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glColor3f( _TV(1.0f), _TV(1.0f), _TV(1.0f) );
		
		// Title text
		gfEnableFont( m_fntFontId, 32 );	
		gfBeginText();
		glTranslated( _TV(260), _TV(500), 0 );
		gfDrawString( "Spaceships in a Cave" );
		gfEndText();

		// Bottom text
		gfEnableFont( m_fntFontId, 20 );	
		gfBeginText();
		glTranslated( _TV(180), _TV(10), 0 );
		gfDrawString( "LudumDare 15 - Joel Davis (joeld42@yahoo.com)" );
		gfEndText();

		// Main Menu Buttons
		glDisable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glColor3f( _TV(0.0f), _TV(1.0f), _TV(1.0f) );

		int yval = _TV(350);
		int yspc = _TV(50);
		if (m_menuState == MenuState_MAINMENU)
		{
			// New Game			
			if (Jgui_doButton( __LINE__, m_uiCtx, "Single Player", m_fntFontId, 20, 
				_TV(365), yval, _TV( 125), _TV(30) ) )
			{
				lobbySinglePlayer();				
			}
			if (Jgui_doButton(  __LINE__, m_uiCtx, "Multiplayer", m_fntFontId, 20, 
				_TV(365), yval-=yspc,_TV( 125), _TV(30) ) )
			{
				lobbyMultiPlayer();				
			}
			if (Jgui_doButton(  __LINE__, m_uiCtx, "Editor", m_fntFontId, 20, 
				_TV(365), yval-=yspc,_TV( 125), _TV(30) ) )
			{
				startEditor();
			}
			if (Jgui_doButton(  __LINE__, m_uiCtx, "Quit", m_fntFontId, 20, 
				_TV(365), yval-=yspc, _TV( 125), _TV(30) ) )
			{
				m_done = true;
			}
		} 
		else if (m_menuState == MenuState_MAPSELECT)
		{
			doMapSelectScreen();
			
		}
		else if (m_menuState == MenuState_LOBBY)
		{
			doLobbyScreen();			
		}
	}

	// kick the UI stuff
	Jgui_frameDone( m_uiCtx );
}

void GameClient::doMapSelectScreen()
{
	// draw list of players
	gfEnableFont( m_fntFontId, 20 );	
	for ( size_t i=0; i < m_ships.size(); i++)
	{
		gfBeginText();
		glTranslated( _TV(200), _TV(450) - i*_TV(25), 0 );
		if (m_ships[i]->m_playerName.empty())
		{
			gfDrawStringFmt( "Unknown%d", i );
		}
		else
		{
			gfDrawString( m_ships[i]->m_playerName.c_str() );
		}
		gfEndText();
	}

	// start game
	const char *readyStatus = m_localServer?"Start Game":"Ready";
	if ((m_player) && (m_player->m_readyToStart))
	{
		readyStatus = "Waiting...";
	}

	if (Jgui_doButton( __LINE__, m_uiCtx, 
					readyStatus,
					m_fntFontId, 20, 
					_TV(365), _TV(200), _TV( 125), _TV(30) ) )
	{				
		// Tell the server we are ready to start
		pbSpaceCave::Packet pbPacket;
		pbPacket.set_type( ::pbSpaceCave::Packet_Type_SETREADYSTATUS );		
		pbPacket.mutable_ready()->set_readytostart( true );
		printf("CLIENT: ready to start\n" );		

		sendPacket( NET_CHANNEL_GAMEUPDATE, pbPacket );
	}
}

void GameClient::doLobbyScreen()
{
	int yval = _TV(250);
	int yspc = _TV(50);

	if (Jgui_doButton( __LINE__, m_uiCtx, "Create Server", m_fntFontId, 20, 
		_TV(365), yval, _TV( 125), _TV(30) ) )
	{
		// Create a local server that's open for connections
		m_localServer = new Server( m_shapes, false );	
		connectServer( "localhost" );

		// other players can join in the map select screen
		m_menuState = MenuState_MAPSELECT;	
	}
	if (Jgui_doButton( __LINE__, m_uiCtx, "Join Localhost", m_fntFontId, 20, 
		_TV(365), yval-=yspc, _TV( 125), _TV(30) ) )
	{
		// We are NOT running a local server
		m_localServer = NULL;
		connectServer( "localhost" );
		m_menuState = MenuState_MAPSELECT;
	}			
	if (Jgui_doButton( __LINE__, m_uiCtx, "Join Remote", m_fntFontId, 20, 
		_TV(365), yval-=yspc, _TV( 125), _TV(30) ) )
	{
		printf("TODO: Join Remote\n" );
		//m_menuState = MenuState_MAPSELECT;
	}			
}

void GameClient::gameview_redraw()
{
	glClearColor( _TV( 0.2f ), _TV(0.2f), _TV( 0.3f ), 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//Shape *player = m_localServer->m_player;
	Shape *player = m_player->m_shape;

	vec2f gameview(player->pos.x - 400, player->pos.y - 300 );
	pseudoOrtho2D( gameview.x, gameview.x + 800,
					gameview.y, gameview.y + 600 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// draw level
	m_level->draw();

	// Draw game objs
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		

	//m_localServer->m_player->drawBraindead();
	for (size_t i=0; i < m_gameObjs.size(); i++)
	{
		m_gameObjs[i]->m_shape->drawBraindead();
	}

	// debug collsion stuff
#if 0
	if (m_level)
	{
		glDisable( GL_TEXTURE_2D );
		glBegin( GL_LINES );
		for (int i=0; i < m_level->m_collision.size(); i++)
		{
			Segment &s = m_level->m_collision[i];
			glColor3f( 1.0f, 1.0f, 0.0f );
			glVertex3f( s.a.x, s.a.y, 0.0f );
			glVertex3f( s.b.x, s.b.y, 0.0f );

			// draw distance to seg
			glColor3f( 0.0, 1.0f, 1.0f );
			vec2f p;
			float d = distPointLine( s.a, s.b, m_player->pos.y, p );
			glVertex3f( m_player->pos.x, m_player->pos.y, 0.0 );
			glVertex3f( p.x, p.y, 0.0 );

		}
		glEnd();

		


		// draw player radius
		glColor3f( 1.0f, 0.0f, 1.0f );
		glBegin( GL_LINE_LOOP );
		float rad = m_player->m_size.x * 0.7;
		for (float t=0; t < 2*M_PI; t += (M_PI/10.0) )
		{
			float s = sin(t);
			float c = cos(t);
			glVertex3f( m_player->pos.x + c * rad,
						m_player->pos.y + s * rad, 0.0 );
		}
		glEnd();
	}
#endif
}

void GameClient::keypress( SDL_KeyboardEvent &key )
{
	if (m_gameState==GameState_GAME)
	{
		game_keypress( key );
	}
	else if (m_gameState==GameState_EDITOR)
	{
		if (m_editor) 
		{
			if ( (key.keysym.sym == SDLK_RETURN) && (m_editor->m_level))
			{
				// test a copy of level
				m_playtest = true;				
#if 0 
				// FIXME
				*m_level = *(m_editor->m_level);

				m_player->pos = m_level->m_spawnPoint;
				m_gameState = GameState_GAME;				
#endif
			}
			else
			{
				m_editor->keypress( key );		
			}
		}
	}
	else if (m_gameState==GameState_MENU)
	{
		switch( key.keysym.sym )
		{		
		case SDLK_F5:
			chatSend("Chat Test message 1" );
			break;

		case SDLK_F6:
			chatSend("Another chat test" );
			break;

		case SDLK_F8:			
			startEditor();			
			break;
		}
	}
}

void GameClient::mouse( SDL_MouseButtonEvent &mouse )
{
	if (m_gameState == GameState_EDITOR)
	{
		if (m_editor) m_editor->mousepress( mouse );
	}
}

void GameClient::game_keypress( SDL_KeyboardEvent &key )
{
	switch (key.keysym.sym)
	{
	//case SDLK_F5:
	//	if (m_localServer)
	//	{
	//		m_localServer->newGame();
	//	}		
	//	break;
	}

}

void GameClient::loadShapes( const char *filename )
{
	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );

	if (!xmlDoc->LoadFile() ) {
		printf("ERR! Can't load %s\n", filename );
	}

	TiXmlElement *xShapeList, *xShape;
	//TiXmlNode *xText;

	xShapeList = xmlDoc->FirstChildElement( "ShapeList" );
	assert( xShapeList );

	xShape = xShapeList->FirstChildElement( "Shape" );
	while (xShape) 
	{
		Shape *shp = new Shape();
		
		shp->name = xShape->Attribute("name");
		shp->m_collide = (!stricmp( xShape->Attribute("collide"), "true" ));
		shp->m_pattern = (!stricmp( xShape->Attribute("pattern"), "true" ));
		if (!stricmp( xShape->Attribute("blend"), "true" ))
		{
			shp->blendMode = Blend_NORMAL;
		}
		shp->m_relief = (!stricmp( xShape->Attribute("relief"), "in" ));

		vec2f st0, sz;
		sscanf( xShape->Attribute("rect"), "%f,%f,%f,%f", 
				&(st0.x), &(st0.y),
				&(sz.x), &(sz.y) );		
	
		shp->m_size = sz;
		shp->m_origSize = sz;

		// get texture and adjust sts
		int texw, texh;
		shp->mapname = std::string("gamedata/") + std::string(xShape->Attribute("map"));
		shp->m_texId = getTexture( shp->mapname, &texw, &texh );

		shp->st0 = st0 / (float)texw;
		shp->st1 = (st0 + sz) / (float)texh;

		m_shapes.push_back( shp );
		xShape = xShape->NextSiblingElement( "Shape" );
	}
		
	// done
	xmlDoc->Clear();
	delete xmlDoc;
}

void GameClient::startGame()
{	
	assert( m_player );

	// yay we are playing
	m_gameState = GameState_GAME;
	
	// Load the level
	if (m_localServer)
	{
		m_level = m_localServer->m_level;
	}
	else
	{
		m_level = new Cavern();
		m_level->loadLevel( "level.xml", m_shapes );
	}
}

void GameClient::startEditor()
{
	// switch to editor
	m_gameState = GameState_EDITOR;
	if (!m_editor)
	{
		m_editor = new Editor( m_fntFontId, m_shapes );					
	}
}

void GameClient::lobbySinglePlayer()
{
	// not really a lobby step here... just create a local
	// server and move on to stage select
	m_localServer = new Server( m_shapes, true );	
	connectServer( "localhost" );

	// and immediately go to the map select screen
	m_menuState = MenuState_MAPSELECT;	
}

void GameClient::lobbyMultiPlayer()
{
	// Go to lobby stage to create or join a server
	m_menuState = MenuState_LOBBY;
}

void GameClient::connectServer( const char *hostaddr )
{
	m_netServer = enet_host_create( NULL, 1, 0, 0 );
	if (m_netServer == NULL)
	{
		printf("CLIENT: Creating server failed..\n" );
		m_menuState = MenuState_MAINMENU;
		return;
	}
	
	printf("CLIENT: Creating server successful\n" );
	
	// connect to localhost
	enet_address_set_host( &m_netAddress, hostaddr );
	m_netAddress.port = SPACECAVE_PORT;

	m_serverPeer = enet_host_connect( m_netServer, &m_netAddress, NUM_NET_CHANNELS );
	if (!m_serverPeer)
	{
		printf("CLIENT: Connect to server fail\n" );
	}
	
}

void GameClient::chatSend( const char *message )
{
	// Do nothing if no server
	if (!m_serverPeer) return;

	// use protobuf to encode packet
	pbSpaceCave::Packet pbPacket;
	
	pbPacket.set_type( ::pbSpaceCave::Packet_Type_CHAT );
	pbPacket.mutable_chat()->set_message( message );
	//(*pbPacket.mutable_message()) = message;	
	
	sendPacket( NET_CHANNEL_CHAT, pbPacket );	
}

void GameClient::handlePacket( int channel, ENetPacket *packet, ENetPeer *peer )
{
	pbSpaceCave::Packet pbPacket;
	pbPacket.ParseFromArray( packet->data, packet->dataLength );

	//printf("Got Packet of type: %s, contains %s\n", 
	//	msg.GetTypeName().c_str(), 
	//	msg.DebugString().c_str() );

	switch( pbPacket.type() )
	{
		case ::pbSpaceCave::Packet_Type_CHAT:			
				printf("CLIENT: got ChatPacket with message %s\n",
					pbPacket.chat().message().c_str() );						
			break;

		case ::pbSpaceCave::Packet_Type_ADD_PLAYER:
			{
				printf("CLIENT: got AddPlayer\n" );
				Shape *playerShape = Shape::simpleShape( "gamedata/player.png" );
				PlayerShip *ship = new PlayerShip( playerShape );
				m_gameObjs.push_back( ship );
				m_ships.push_back( ship );
			}
			break;
		
		case ::pbSpaceCave::Packet_Type_IDENTITY:
			{
				printf("CLIENT: got Identity\n" );
				m_player = m_ships[ pbPacket.identity().index() ];
				m_player->m_playerName = pbPacket.identity().playername();
			}

		case ::pbSpaceCave::Packet_Type_PLAYERSETTINGS:
			{
				size_t ndx = pbPacket.pset().index();
				printf("CLIENT: got player settings for %d\n", ndx );
				m_ships[ ndx ]->m_playerName = pbPacket.pset().playername();
				// same for color, etc..
			}
			break;

		case ::pbSpaceCave::Packet_Type_STARTGAME:
			{				
				printf("CLIENT: got Start Game, yay\n" );
				startGame();
			}
			break;

		case ::pbSpaceCave::Packet_Type_UPDATE:
			{
				printf("CLIENT: got game update...\n" );
				for (int i=0; i < pbPacket.update().obj().size(); i++)
				{
					const ::pbSpaceCave::GameObj &gameObj = pbPacket.update().obj( i );
					size_t ndx = gameObj.shipindex();

					// get the ship info
					m_ships[ndx]->m_pos.x = gameObj.pos_x();
					m_ships[ndx]->m_pos.y = gameObj.pos_y();

					m_ships[ndx]->m_vel.x = gameObj.vel_x();
					m_ships[ndx]->m_vel.y = gameObj.vel_y();

					m_ships[ndx]->m_angle = gameObj.angle();

					// Copy into shape 
					m_ships[ndx]->m_shape->pos = m_ships[ndx]->m_pos;
					m_ships[ndx]->m_shape->angle = m_ships[ndx]->m_angle;
				}
			}
			break;
	}
}

void GameClient::sendPacket( int channel, pbSpaceCave::Packet &pbPacket )
{
	size_t packetSize = pbPacket.ByteSize();
	void *packetData = malloc( packetSize );
	pbPacket.SerializeToArray( packetData, packetSize );

	// Now send it
	ENetPacket *packet;
	packet = enet_packet_create( packetData, packetSize, 
							ENET_PACKET_FLAG_RELIABLE );

	enet_peer_send( m_serverPeer, NET_CHANNEL_CHAT, packet );

	// will get flushed next net_update
}