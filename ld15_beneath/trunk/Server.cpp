#include <map>

#include <Common.h>
#include <Server.h>

#include <tweakval.h>
#include <gamefontgl.h>
#include <jimgui.h>

#include <TinyXml.h>

#include <pbSpaceCave/SpaceCave.pb.h>
using namespace google;

Server::Server( std::vector<Shape*> &shapes, bool singlePlayer ) :
	m_isInit( false ),
	m_level( NULL ),	
	m_shapes( shapes ),
	m_clientHost( NULL ),
	m_singlePlayer( singlePlayer )
{
	init();
	initNetwork( );	
}

Server::~Server()
{
	if (m_clientHost)
	{
		enet_host_destroy( m_clientHost );
	}

	delete m_level;
}
void Server::initNetwork( )
{
	enet_address_set_host( &m_serverAddr, "localhost" );
	m_serverAddr.port = SPACECAVE_PORT;

	m_clientHost = enet_host_create( &m_serverAddr, 
							m_singlePlayer?1:MAX_CLIENTS, 
							0, 0 );
	if (!m_clientHost)
	{
		printf("SERVER: Failed to create host\n" );
	}
	else
	{
		printf("SERVER: Host created successfully\n" );
	}
}

void Server::game_updateSim( float dt )
{
#if 0
	vec2f thrustDir( 0.0f, 0.0f );
	float rotateAmt = 0.0;
	const float LATERAL_THRUST_AMT = _TV(1.0f);
	const float FWD_THRUST_AMT = _TV(1.0f);
	const float REV_THRUST_AMT = _TV(0.2f);
	const float THRUST_AMT = _TV(1500.0f);
	const float ROTATE_AMT = _TV(400.0f);
	const float kDRAG = _TV(0.01f);
	const float kDRAG2 = _TV(0.002f);

	// Continuous (key state) keys
	Uint8 *keyState = SDL_GetKeyState( NULL );

	// rotation
	if ( ((keyState[SDLK_LEFT]) && (!keyState[SDLK_RIGHT])) ||
		 ((keyState[SDLK_a]) && (!keyState[SDLK_d])) )
	{
		rotateAmt = -ROTATE_AMT;
	}
	else if ( ((!keyState[SDLK_LEFT]) && (keyState[SDLK_RIGHT])) ||
		     ((!keyState[SDLK_a]) && (keyState[SDLK_d])) )
	{
		rotateAmt = ROTATE_AMT;
	}

	// lateral thrust
	if ((keyState[SDLK_q]) && (!keyState[SDLK_e]))		 
	{
		thrustDir.x = -LATERAL_THRUST_AMT;
	}
	else if ((!keyState[SDLK_q]) && (keyState[SDLK_e]))		     
	{
		thrustDir.x = LATERAL_THRUST_AMT;
	}

	// forward/backward
	if ( ((keyState[SDLK_UP]) && (!keyState[SDLK_DOWN])) ||
		 ((keyState[SDLK_w]) && (!keyState[SDLK_s])) )
	{
		thrustDir.y = FWD_THRUST_AMT;
	}
	else if ( ((!keyState[SDLK_UP]) && (keyState[SDLK_DOWN])) ||
				((!keyState[SDLK_w]) && (keyState[SDLK_s])) )
	{
		thrustDir.y = -REV_THRUST_AMT;
	}

	m_player->angle += rotateAmt *dt;

	// rotate thrust dir
	thrustDir = RotateZ( thrustDir, -(float)(m_player->angle * D2R) );


	// calc drag
	float l = Length( m_vel );
	if (l > 0.1)
	{
		vec2f dragDir = -m_vel;		
		
		float kDrag = kDRAG * l + kDRAG2 * l * l;		
		dragDir.Normalize();
		dragDir *= kDrag;

		m_vel += dragDir * dt;
	}
	else
	{
		m_vel = vec2f( 0.0f, 0.0f );
	}

	// update player
	m_vel += thrustDir * THRUST_AMT * dt;

	// Check for collisions	
	vec2f newPos = m_player->pos + m_vel * dt;
	bool collision = false;
	if (m_level)
	{
		for (int i=0; i < m_level->m_collision.size(); i++)
		{
			Segment &s = m_level->m_collision[i];
			vec2f p;
			float d = distPointLine( s.a, s.b, newPos, p );						
			
			if (d < m_player->m_size.x * 0.7)
			{
				// collide
				if (s.segType == SegType_COLLIDE)
				{

					//printf("COLLISION: dist %f player %3.2f %3.2f seg %3.2f %3.2f -> %3.2f %3.2f\n", 
					//	d, m_player->pos.x, m_player->pos.y,
					//	s.a.x, s.a.y, s.b.x, s.b.y
					//	);

					collision = true;
					m_vel =- m_vel;
					break;
				}
				else if (s.segType == SegType_KILL)
				{
					// TODO: reset level properly
					m_vel = vec2f( 0.0f, 0.0f );					
					m_player->pos = m_level->m_spawnPoint;		
					collision = true;
				}
				// TODO: Dialogue
			}
		}
	}	

	if (!collision)
	{
		m_player->pos = newPos;
	}
#endif
}

// "as fast as possible" update for effects and stuff
void Server::game_update( float dt )
{
//	printf("Update %3.2f\n", dt );
}

// Do networky stuff
void Server::net_update( )
{
	ENetEvent event;
	if (enet_host_service( m_clientHost, &event, 0 ))
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				printf("SERVER: A new client connected from %x:%u.\n",
					event.peer->address.host,
					event.peer->address.port );							

				newConnection( event.peer );				

				break;

			case ENET_EVENT_TYPE_RECEIVE:
				printf("SERVER: A packet of length %u was recieved from %s on channel %u.\n",
					event.packet->dataLength,					
					event.peer->data,
					event.channelID );

				handlePacket( event.channelID, event.packet, event.peer );

				enet_packet_destroy( event.packet );
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("SERVER: %s disconnected\n", event.peer->data );
				event.peer->data = NULL;
				break;

		}
	}	
}

void Server::handlePacket( int channel, ENetPacket *packet, ENetPeer *peer )
{
	pbSpaceCave::Packet pbPacket;
	pbPacket.ParseFromArray( packet->data, packet->dataLength );

	//printf("Got Packet of type: %s, contains %s\n", 
	//	msg.GetTypeName().c_str(), 
	//	msg.DebugString().c_str() );

	switch( pbPacket.type() )
	{
		case ::pbSpaceCave::Packet_Type_CHAT:			
				printf("SERVER: got ChatPacket with message %s\n",
					pbPacket.chat().message().c_str() );			
			
				sendPacketEveryone( NET_CHANNEL_CHAT, pbPacket );
			break;

		case ::pbSpaceCave::Packet_Type_IDENTITY:
			
			// fill in the player's index
			printf("SERVER: Got request for identity for '%s'\n",
				pbPacket.identity().playername().c_str() );
			
			doIdentity( pbPacket, peer );			
			break;
	}
}

PlayerShip *Server::addPlayer()
{
	// create a new ship for the player
	PlayerShip *newShip = new PlayerShip( NULL );
	
	m_ships.push_back( newShip );

	// Now send a packet informing clients of the new ship
	// use protobuf to encode packet
	pbSpaceCave::Packet pbPacket;	
	pbPacket.set_type( ::pbSpaceCave::Packet_Type_ADD_PLAYER );
	
	sendPacketEveryone( NET_CHANNEL_GAMEUPDATE, pbPacket );

	return newShip;
}

void Server::init()
{
	// Now load game shapes
	//m_player = Shape::simpleShape( "gamedata/player.png" );
	//m_player->pos = vec2f( 300, 200 );
}

void Server::newGame()
{
	delete m_level;

	//m_level = new Cavern();
	//m_level->loadLevel( "level.xml", m_shapes );	
	//m_vel = vec2f( 0.0f, 0.0f );					
	//m_player->pos = m_level->m_spawnPoint;		
}

void Server::newConnection( ENetPeer *peer )
{
	m_clientPeers.push_back( peer );

	// Send this one all the existing ships
	pbSpaceCave::Packet pbPacket;		
	for (size_t i=0; i < m_ships.size(); i++)
	{
		pbPacket.set_type( ::pbSpaceCave::Packet_Type_ADD_PLAYER );	
		sendPacketPeer( NET_CHANNEL_GAMEUPDATE, pbPacket, peer );
	}

	// create a ship for this player
	peer->data = addPlayer();	
}

void Server::sendPacketEveryone( int channel, pbSpaceCave::Packet &pbPacket )
{
	size_t packetSize = pbPacket.ByteSize();
	void *packetData = malloc( packetSize );
	pbPacket.SerializeToArray( packetData, packetSize );

	// Now send it
	ENetPacket *packet;
	packet = enet_packet_create( packetData, packetSize, 
							ENET_PACKET_FLAG_RELIABLE );

	for (size_t i=0; i < m_clientPeers.size(); i++)
	{
		enet_peer_send( m_clientPeers[i], channel, packet );
	}

	// do this?
	//free( packetData );
}

void Server::sendPacketPeer( int channel, pbSpaceCave::Packet &pbPacket, ENetPeer *peer )
{
	size_t packetSize = pbPacket.ByteSize();
	void *packetData = malloc( packetSize );
	pbPacket.SerializeToArray( packetData, packetSize );

	// Now send it
	ENetPacket *packet;
	packet = enet_packet_create( packetData, packetSize, 
							ENET_PACKET_FLAG_RELIABLE );
	
	enet_peer_send( peer, channel, packet );
	
	// do this?
	//free( packetData );
}

void Server::doIdentity( pbSpaceCave::Packet &pbPacket, ENetPeer *peer )
{
	// TODO: uniqify name
	bool isUnique = true;
	string playerName = pbPacket.identity().playername();

	int uniqueIndex = 1;
	do
	{
		isUnique = true;
		for (size_t i=0; i < m_ships.size(); i++)
		{
			if ( (!m_ships[i]->m_playerName.empty()) &&
				  (m_ships[i]->m_playerName == playerName) )
			{
				isUnique = false;
				break;
			}	
		}

		// if we're not unique, try adding a higher suffix
		if (!isUnique)
		{
			uniqueIndex++;
			char playerNameBuff[100];
			sprintf( playerNameBuff, "%s %d", 
				     pbPacket.identity().playername().c_str(),
					 uniqueIndex );
			playerName = playerNameBuff;
		}
	}
	while (!isUnique);

	pbPacket.mutable_identity()->set_playername( playerName );

	// Figure out which ship this is
	size_t playerIndex;
	bool found = false;
	for (size_t i=0; i < m_ships.size(); i++)
	{
		if (peer->data == m_ships[i])
		{
			// set the name for our ship
			m_ships[i]->m_playerName = playerName;

			// set the index to pass back to client
			playerIndex = i;
			pbPacket.mutable_identity()->set_index( playerIndex );
			found = true;			
			break;
		}
	}

	if (found)
	{
		// Send the updated identity packet back to the client
		sendPacketPeer( NET_CHANNEL_GAMEUPDATE, pbPacket, peer );

		// Send all of the existing players settings to the new player
		pbSpaceCave::Packet pbPacketSettings;
		pbPacketSettings.set_type( ::pbSpaceCave::Packet_Type_PLAYERSETTINGS );
		for (int i=0; i < m_ships.size(); i++)
		{
			// ignore this player 'cause we're about to send it to everyone
			if (i==playerIndex) continue;
			pbPacketSettings.mutable_pset()->set_index( i );
			pbPacketSettings.mutable_pset()->set_playername( m_ships[i]->m_playerName );
			sendPacketPeer( NET_CHANNEL_GAMEUPDATE, pbPacketSettings, peer );
		}

		// Send a settings packet to everyone about the new player		
		pbPacketSettings.mutable_pset()->set_index( playerIndex );
		pbPacketSettings.mutable_pset()->set_playername( playerName );

		sendPacketEveryone( NET_CHANNEL_GAMEUPDATE, pbPacketSettings );
	}
}