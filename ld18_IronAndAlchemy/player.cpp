#include <luddite/debug.h>

#include "player.h"

Player::Player( Entity *owner ) :
	Behavior( owner )
{
	addTag( "movement" );
	addTag( "player" );
}

void Player::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);	

	// tmp
	// fall if we're above ground
	if (spr.y > 8)
	{
		spr.y -= 40.0 * dtFixed;
	}
}