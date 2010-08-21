#include "enemy.h"
#include "entity.h"
#include "physics.h"

Enemy::Enemy( Entity *owner ) : Behavior( owner )
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
	// todo
}