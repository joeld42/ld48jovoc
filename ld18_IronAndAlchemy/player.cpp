#include <math.h>

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

	m_frameTimer = 0;
	m_animFrame =0;
	
	m_faceLeft = false;
}

void Player::movement( IronAndAlchemyGame *game, float dtFixed )
{
	Sprite &spr = (*m_owner->m_sprite);	

	// apply player impulse force	
	m_physics->ix = ix * _TV( 60 );

	// update facing flags
	if (ix < -0.01) 
	{
		m_faceLeft = true;
	}
	else if (ix > 0.01)
	{
		m_faceLeft = false;
	}
	// otherwise leave it as is


	// update animation frame
	m_frameTimer--;
	if (m_frameTimer <= 0)
	{
		// update anim frame
		m_animFrame++;
		if (m_animFrame > 10000) m_animFrame = 0;

		// reset frame timer
		m_frameTimer = 8;

		// now apply that to sprite
		int ndx = 0;
		if ( m_physics->falling )
		{
			// 'jumping' frame
			ndx = 5;
		}
		else
		{
			
			if (fabs(ix) < 0.01)
			{
				// idle (slower rate)
				ndx = (m_animFrame/5) % 2;
			}
			else
			{
				ndx = 2+(m_animFrame % 3);
			}			

			// TODO -- hit and shoot

			// TODO -- flip horiz
		}

		//DBG::info( "player frame %d %d\n", m_animFrame, ndx );

		float s0 = (ndx%4) * 0.25;
		float t0 = (ndx/4) * 0.5;
		spr.setTexCoords( s0, t0, s0+ 0.25, t0+0.5);

		if (m_faceLeft)
		{
			spr.flipHoriz();
		}
	}


	// Check if we collided with an enemy
	for (std::list<Entity*>::iterator ei = game->m_entities.begin();
		ei != game->m_entities.end(); ++ei )
	{
		// skip the player
		if ((*ei) == m_owner) continue;
		Entity *e = (*ei);

		// did we hit them?
		if (m_owner->m_sprite->testHit( e->m_sprite->x,
							   e->m_sprite->y ) )
		{
			// yep...
			DBG::info("Hit enemy!!!\n" );

			// take damage
			// TODO

			// and die..
			game->playerDie();				
		}
	}

}

void Player::jump( IronAndAlchemyGame *game )
{
	// add some instant velocity
	if (! m_physics->falling )
	{		
		m_physics->vy += _TV( 175 );
	}
}

void Player::collideWorld( IronAndAlchemyGame *game )
{
	// bumped into something ... kill any x velocity
	m_physics->vx = 0;	
}