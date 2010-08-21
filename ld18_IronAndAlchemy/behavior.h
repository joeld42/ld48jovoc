#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <list>

#include <luddite/atom.h>

class IronAndAlchemyGame;
class Entity;

class Behavior 
{
public:
	Behavior( Entity *owner );

	void addTag( const char *tag );
	bool hasTag( const char *tag );

	// Override to change the entity's movment
	virtual void movement( IronAndAlchemyGame *game, float dtFixed );

	//protected:

	// NOTE: tags must be atomized
	std::list<const char *> m_tags;

	Entity *m_owner;
};

#endif