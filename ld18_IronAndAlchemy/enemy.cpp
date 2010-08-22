#include "enemy.h"
#include "entity.h"
#include "physics.h"

#include <luddite/debug.h>
#include <luddite/tweakval.h>

Enemy::Enemy( Entity *owner ) : Behavior( owner ),
	m_walkDir( -1 )
{
	addTag( "movement" );
	addTag( "enemy" );

	// Get the physics behavior
	m_physics = dynamic_cast<Physics*>(m_owner->getBehaviorByTag( "physics" ));
	AssertPtr( m_physics );
}

// enemy movement
void Enemy::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);

	// walk if we are on the ground
	//if (game->onGround( m_physics->x, m_physics->y ))
	//{
		// apply enemy impulse force
		m_physics->ix = m_walkDir * _TV( 50 );		
	//}	
}

void Enemy::collideWorld( IronAndAlchemyGame *game )
{
	// bumped into something ... walk the other way
	DBG::info("Bump!" );
	m_walkDir = -m_walkDir;
}