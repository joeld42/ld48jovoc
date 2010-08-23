#ifndef PLAYER_H
#define PLAYER_H

#include <luddite/sprite.h>
#include <luddite/avar.h>
#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"


#define MAX_LIFE (30)

class Physics;
class Player : public Behavior
{
public:
	Player( Entity *m_owner );
	
	virtual void movement( IronAndAlchemyGame *game, float dtFixed );
	virtual void collideWorld( IronAndAlchemyGame *game );

	void jump( IronAndAlchemyGame *game );

	void shoot( IronAndAlchemyGame *game );

	// stuff
	void drawLifeMeter();

	// enemies I'm carrying
	std::list<Sprite*> m_carry;

	// movment impulse
	float ix, iy; // not sure if iy is useful
	Physics *m_physics; // to interfere with the physics	

	// life
	int m_hitPoints;
	int m_invincibleTimer;
	AnimFloat m_pulseOuch;

	// buttons
	bool m_jumpPressed;
	bool m_firePressed;

	// frame counter
	int m_frameTimer;
	int m_animFrame;
	
	bool m_faceLeft;
};

#endif