#include <math.h>
#include <luddite/tweakval.h>

#include "physics.h"


Physics::Physics( Entity *owner ) :
	Behavior( owner )
{
	addTag("movement" );
	addTag("physics" );

	Sprite &spr = (*m_owner->m_sprite);
	x = spr.x;
	y = spr.y + 8;

	vx = 0; vy = 0;
	fx = 0; fy = 0;
	ix = 0; iy = 0;

	falling = false;
}

// model the entity's movment
void Physics::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);
	falling = true;

	// Apply gravity		
	const float MAX_FALL = -500;
	vy += _TV(-700.0f) * dtFixed;		
	if (vy < MAX_FALL)
	{
		vy = MAX_FALL;
	}
	
	//fy += _TV(-100) * dtFixed;	

	// remember old pos
	float oldX = x;
	float oldY = y;
	float oldVX = vx;
	float oldVY = vy;

	// update x first
	x += (vx + ix) * dtFixed;
	spr.x = x;

	if (checkCollideMap(game) )
	{
		// roll back x
		x = oldX;
		vx = 0;		
		spr.x = x;
	}	

	// integrate x
	vx += fx * dtFixed;	

	// Now update y
	y += (vy + iy) * dtFixed;	
	//DBG::info("oldY %3.2f newY %3.2f\n", oldY, y );

	// try new pos, check if we hit the world			
	spr.y = y + 8;

	// Check for collisions with the world
	if (game->collideWorld( &spr ))
	{		
		// roll back y and kill velocity				
		vy = 0;
		fy = 0;

		y = oldY;
		spr.y = y + 8;

		// mark that we're standing
		falling = false;
	}	

	// integrate y	
	//vy += fy * dtFixed;	

	// apply update to sprite
	spr.x = x;
	spr.y = y + 8;	

	//DBG::info("spr y %3.2f vy %3.2f fy %3.2f\n", spr.y, vy, fy );
	
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