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

	DBG::info("---- in Physics::movement, x is %3.2f\n", x );
	Assert( x >= 8.0, "collide check" );	

	// check onground
	if (!game->onGround( spr.x, spr.y ))
	{
		fy += _TV(-8000.0f) * dtFixed;
	}	

	// remember old pos
	float oldX = x;
	float oldY = y;

	// integrate 
	vx += fx * dtFixed;
	vy += fy * dtFixed;

	// Check for collisions with the world
	DBG::info("Before X update:, x is %3.2f\n", x );

	// update x first
	x += (vx + ix) * dtFixed;
	spr.x = x;

	if (checkCollideMap(game) )
	{	
		DBG::info("ROLLBACK: player hit map, x is %3.2f oldX is %3.2f\n", x, oldX );

		// roll back x
		x = oldX;
		spr.x = x;
	}
	else
	{
		DBG::info("no collision, x is %3.2f\n", x );
	}
	DBG::info("After:, x is %3.2f\n", x );
	Assert( x >= 8.0, "collide check" );	

	// Now update y
	y += (vy + iy) * dtFixed;

	DBG::info("Before Y update:, x is %3.2f\n", x );
	// Check if we hit the ground
	if (game->onGround(x, y))
	{
		fy = 0.0;
		vy = 0.0;
		y = 8.0;
	}

	// try new pos, check if we hit the world			
	spr.y = y + 16;

	// Check for collisions with the world
	if (checkCollideMap(game) )
	{		
		// roll back
		y = oldY;
		spr.y = y + 16;	
	}	

	DBG::info("After Y update, x is %3.2f\n", x );
	Assert( x >= 8.0, "collide check" );

	// apply update to sprite
	spr.x = x;
	spr.y = y + 16;	
	
}

bool Physics::checkCollideMap( IronAndAlchemyGame *game )
{
	if (game->collideWorld( m_owner->m_sprite ))
	{
		// notify about collision
		for (std::list<Behavior*>::iterator bi = m_owner->m_behaviors.begin();
			 bi != m_owner->m_behaviors.end(); ++bi )
		{
			(*bi)->collideWorld( game );
		}

		return true;
	}

	return false;
}