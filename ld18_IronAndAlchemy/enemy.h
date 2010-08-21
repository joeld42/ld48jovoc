#ifndef ENEMY_H
#define ENEMY_H

#include "behavior.h"

enum EnemyType
{
	Enemy_NIL = 0,

	Enemy_REDBUG,
};

class IronAndAlchemyGame;
class Entity;
class Physics;

class Enemy : public Behavior 
{
public:
	Enemy( Entity *owner );

	// enemy movement
	virtual void movement( IronAndAlchemyGame *game, float dtFixed );

	Physics *m_physics;
};

#endif