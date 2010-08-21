
#include <luddite/debug.h>
#include <luddite/tweakval.h>

#include "player.h"
#include "physics.h"

Player::Player( Entity *owner ) :
	Behavior( owner )
{
	addTag( "movement" );
	addTag( "player" );

	// Get the physics behavior
	m_physics = dynamic_cast<Physics*>(m_owner->getBehaviorByTag( "physics" ));
	AssertPtr( m_physics );
}

void Player::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);	

	// apply player impulse force	
	m_physics->ix = ix * _TV( 200 );
	
}

void Player::jump( IronAndAlchemyGame *game )
{
	// add some instant velocity
	if (game->onGround( m_physics->x, m_physics->y ) )
	{
		m_physics->vy += _TV( 250 );
	}
}

void Player::collideWorld( IronAndAlchemyGame *game )
{
	// bumped into something ... kill any x velocity
	m_physics->vx = 0;

	DBG::info( " player collide world\n" );

}