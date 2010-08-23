#include <math.h>

#include <luddite/debug.h>
#include <luddite/tweakval.h>

#include "player.h"
#include "physics.h"

#include <fmod.h>

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

	m_hitPoints = MAX_LIFE;
	
	m_faceLeft = false;

	m_pulseOuch.pulse( 0.0, 1.0, 0.5, 0.0 );
	m_invincibleTimer = -1;
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

	// update invincible timer
	if (m_invincibleTimer >= 0)
	{
		m_invincibleTimer--;

		if (m_invincibleTimer <0)
		{
			m_physics->vx = 0;
		}
	}


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
				// are we carrying something?
				if (m_carry.empty())
				{
					// idle (slower rate)
					ndx = (m_animFrame/5) % 2;
				}
				else
				{
					// carry frame
					ndx = 7;
				}
			}
			else
			{
				ndx = 2+(m_animFrame % 3);
			}			

			// TODO -- hit and shoot

			// TODO -- flip horiz
		}

		//DBG::info( "player frame %d %d\n", m_animFrame, ndx );

		// override this if we're in special invincible mode
		if (m_invincibleTimer >=0 )
		{
			if (m_animFrame & 1)
			{
				ndx = 6;
			}
		}

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

			// Did we squash it?
			if ( (spr.y > e->m_sprite->y) && 
				 (m_physics->vy < 0) )
			{
				DBG::info("Squish!!" );

				// set sprite to NULL because we're going to take ownership
				Sprite *enemySpr = e->m_sprite;
				e->m_sprite = NULL;

				game->removeEntity( e );

				// add sprite to our stack
				m_carry.push_back( enemySpr );
			}			
			// if we're vulnerable
			else if (m_invincibleTimer <0)
			{							
				// Damage sfx
				FMOD::Channel *channel = 0;
				game->m_fmod->playSound(FMOD_CHANNEL_FREE, 
							game->sfx_ouch, false, &channel );

				// knock backwards
				m_physics->vx = 30 * (m_faceLeft?1:-1);
				m_physics->vy = 250;

				// take damage
				m_hitPoints -= 3;
				if (m_hitPoints <= 0)
				{
					// and die..
					game->playerDie();				
				}
				else
				{
					// short invincibility 
					m_invincibleTimer = 50;
				}
			}
		}
	}

	// Now, update stuff we're carrying
	// TODO: add some cool swaying or something
	float yval = m_owner->m_sprite->y + 8;
	for (std::list<Sprite*>::iterator spi = m_carry.begin();
		 spi != m_carry.end(); ++spi)
	{
		(*spi)->angle = 180.0;
		(*spi)->x = m_owner->m_sprite->x;
		(*spi)->y = yval;
		(*spi)->setMirrorX( m_faceLeft );

		yval += 10;

		// update sprite here because it's not owned by any entity
		(*spi)->update();
	}

}

void Player::jump( IronAndAlchemyGame *game )
{
	// add some instant velocity
	if (! m_physics->falling )
	{		
		// play jump sound
		FMOD::Channel *channel = 0;
		game->m_fmod->playSound(FMOD_CHANNEL_FREE, 
						game->sfx_jump, false, &channel );

		// add force
		m_physics->vy += _TV( 175 );
	}
}

void Player::shoot( IronAndAlchemyGame *game )
{
	float yval = m_owner->m_sprite->y + 10;
	for (std::list<Sprite*>::iterator spi = m_carry.begin();
		 spi != m_carry.end(); ++spi)
	{
		// If we're holding something that can shoot, shoot
		if ((*spi)->canShoot)
		{
			// shoot
			game->m_particles->emitBullet( Particle_BULLET,
				m_owner->m_sprite->x + (m_faceLeft?-8:8), yval, 
				m_faceLeft?180.0:0 );
		}

		yval += 10;
	}
}

void Player::collideWorld( IronAndAlchemyGame *game )
{
	// bumped into something ... kill any x velocity
	m_physics->vx = 0;	
}

void Player::drawLifeMeter()
{
	// draw the background
	glDisable( GL_TEXTURE_2D);

	glColor3f( m_hitPoints<=5?m_pulseOuch.animValue():0.0, 0.0, 0.0 );

	glBegin( GL_QUADS );
	glVertex2f( 2, 158 ); glVertex2f( 8, 158 );
	glVertex2f( 8, 157-MAX_LIFE*2 );
	glVertex2f( 2, 157-MAX_LIFE*2 ); 
	glEnd();

	// draw health bars
	glColor3f( 1.0, 1.0, 1.0f );

	glBegin( GL_LINES );
	for (int i=0; i < m_hitPoints; i++)
	{
		glVertex2f( 3, (159-MAX_LIFE*2) + i*2);
		glVertex2f( 7, (159-MAX_LIFE*2) + i*2);
	}
	glEnd();

	// Restore state
	glEnable( GL_TEXTURE_2D);
	glColor3f( 1.0, 1.0, 1.0f );
}