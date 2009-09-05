#include <map>

#include <tweakval.h>
#include <gamefontgl.h>
#include <jimgui.h>

#include <Common.h>
#include <Server.h>

#include <TinyXml.h>


Server::Server( std::vector<Shape*> &shapes ) :
	m_isInit( false ),
	m_level( NULL ),
	m_vel( 0.0f, 0.0f ),
	m_shapes( shapes )
{
	init();
}

Server::~Server()
{
	delete m_level;
}

void Server::game_updateSim( float dt )
{
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
}

// "as fast as possible" update for effects and stuff
void Server::game_update( float dt )
{
//	printf("Update %3.2f\n", dt );
}

void Server::init()
{
	// Now load game shapes
	m_player = Shape::simpleShape( "gamedata/player.png" );
	m_player->pos = vec2f( 300, 200 );
}

void Server::newGame()
{
	delete m_level;

	m_level = new Cavern();
	m_level->loadLevel( "level.xml", m_shapes );	
	m_vel = vec2f( 0.0f, 0.0f );					
	m_player->pos = m_level->m_spawnPoint;		
}


