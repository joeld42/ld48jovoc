#ifndef PHYSICS_H
#define PHYSICS_H

#include "game.h"
#include "behavior.h"

class Physics : public Behavior
{
public:
	Physics( Entity *owner );

	// Override to change the entity's movment
	virtual void movement( IronAndAlchemyGame *game, float dtFixed );

	// check collide with map, return true if hit something
	bool checkCollideMap( IronAndAlchemyGame *game );

	// physics properties

	// y is ground-y not center y like for sprites
	float x,y, // position
		vx, vy, // velocity
		fx, fy, // forces
		ix, iy; // impules (instant forces)
};

#endif