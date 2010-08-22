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
}

// model the entity's movment
void Physics::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);

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
#if 0		
		// step back from oldY to Y a pixel at a time
		// until we collide
		if (oldY > y)
		{
			for(float yVal = oldY; yVal >= y; yVal -= 0.5f)
			{
				spr.y = yVal + 8;
				if (game->collideWorld( &spr ))
				{
					// back off
					y = ceil(yVal + 1.0f);
					spr.y = y + 8;
					break;
				}
			}
		}
		else
		{
			for(float yVal = oldY; yVal <= y; yVal += 0.5f)
			{
				spr.y = yVal + 8;
				if (game->collideWorld( &spr ))
				{
					// back off
					y = ceil(yVal - 1.0f);
					spr.y = y + 8;
					break;
				}
			}
		}
#endif
		y = oldY;
		spr.y = y + 8;
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