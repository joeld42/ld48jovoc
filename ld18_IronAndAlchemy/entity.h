#ifndef ENTITY_H
#define ENTITY_H

#include <list>

#include <luddite/atom.h>
#include <luddite/sprite.h>

#include "behavior.h"

// Container for a bunch of behaviors -- aggragate behavoirs 
// to make stuff go.
class Entity 
{
public:
	Entity( Sprite *spr );

	void addBehavior( Behavior *beh );

	// Gets the first behavior by tag
	Behavior *getBehaviorByTag( const char *tag );

	void updateSim( IronAndAlchemyGame *game, float dtFixed );

	std::list<Behavior*> m_behaviors;
	Sprite *m_sprite;
};

#endif