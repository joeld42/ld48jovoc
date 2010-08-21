#include <luddite/tweakval.h>

#include "physics.h"


Physics::Physics( Entity *owner ) :
	Behavior( owner )
{
	addTag("movement" );
	addTag("physics" );

	Sprite &spr = (*m_owner->m_sprite);
	x = spr.x;
	y = spr.y + 16;

	vx = 0; vy = 0;
	fx = 0; fy = 0;
	ix = 0; iy = 0;
}

// model the entity's movment
void Physics::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);

	// check onground
	if (!game->onGround( spr.x, spr.y ))
	{
		fy += _TV(-8000.0f) * dtFixed;
	}	

	// integrate
	vx += fx * dtFixed;
	vy += fy * dtFixed;

	x += (vx + ix) * dtFixed;
	y += (vy + iy) * dtFixed;

	// Check if we hit the ground
	if (game->onGround(x, y))
	{
		fy = 0.0;
		vy = 0.0;
		y = 8.0;
	}

	// apply sim pos to sprite pos
	spr.x = x; spr.y = y + 16;
}