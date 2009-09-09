#include <PlayerShip.h>

PlayerShip::PlayerShip( Shape *shape ) :
	GameObj( shape ),
	m_vel( 0.0f, 0.0f ),
	m_pos( 0.0f, 0.0f ),
	m_angle( 0.0f ),
	m_radius( 64.0 ),
	m_readyToStart( false ),
	
	m_forward( 0 ),
	m_reverse( 0 ),
	m_rotleft( 0 ),
	m_rotright( 0 ),
	m_latleft( 0 ),
	m_latright( 0 )
{
}