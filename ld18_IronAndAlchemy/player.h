#ifndef PLAYER_H
#define PLAYER_H

#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"

class Physics;
class Player : public Behavior
{
public:
	Player( Entity *m_owner );
	
	virtual void movement( IronAndAlchemyGame *game, float dtFixed );
	virtual void collideWorld( IronAndAlchemyGame *game );

	void jump( IronAndAlchemyGame *game );

	// movment impulse
	float ix, iy; // not sure if iy is useful
	Physics *m_physics; // to interfere with the physics	

	// buttons
	bool m_jumpPressed;
	bool m_firePressed;

	// frame counter
	int m_frameTimer;
	int m_animFrame;
	
	bool m_faceLeft;
};

#endif