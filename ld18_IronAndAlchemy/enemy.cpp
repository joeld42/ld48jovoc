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
	
	// turn around if we're about to walk off a cliff
	if (!game->_collideWorldPnt( spr.x + (m_walkDir*2), spr.y-9))
	{
		m_walkDir = -m_walkDir;
	}

	// apply enemy impulse force
	m_physics->ix = m_walkDir * _TV( 3500 ) * dtFixed;		
	
}

void Enemy::collideWorld( IronAndAlchemyGame *game )
{
	// bumped into something ... walk the other way	
	m_walkDir = -m_walkDir;
}