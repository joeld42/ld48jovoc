
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

void Player::jump()
{
	// add some instant velocity
	// TODO: check if they are on the ground
	m_physics->vy += _TV( 150 );
}