#ifndef PLAYER_H
#define PLAYER_H

#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"

class Player : public Behavior
{
public:
	Player( Entity *m_owner );
	
	virtual void movement( IronAndAlchemyGame *game, float dtFixed );
};

#endif