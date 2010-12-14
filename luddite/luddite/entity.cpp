#include <luddite/debug.h>
#include <luddite/sprite.h>

#include "entity.h"
#include "behavior.h"

Entity::Entity( Sprite *spr ) :
	m_sprite( spr )
{
}

Entity::~Entity()
{
	// delete our sprite
	delete m_sprite;

	// delete our behaviors
	for (std::list<Behavior*>::iterator bi = m_behaviors.begin();
			bi != m_behaviors.end(); ++bi)
	{
		delete (*bi);
	}
}

void Entity::addBehavior( Behavior *beh )
{
	m_behaviors.push_back( beh );
}

Behavior *Entity::getBehaviorByTag( const char *tag )
{
	const char *atomizedTag = Atom_string( tag );

	for (std::list<Behavior*>::iterator bi = m_behaviors.begin();
			bi != m_behaviors.end(); ++bi)
	{
		if ((*bi)->hasTag( atomizedTag ))
		{
			return (*bi);
		}
	}
	
	return NULL;
}

void Entity::updateSim( IronAndAlchemyGame *game, float dtFixed )
{	
	if (!m_sprite) return;

	for (std::list<Behavior*>::iterator bi = m_behaviors.begin();
			bi != m_behaviors.end(); ++bi)
	{
		// Apply movment to all valid behaviors		
		(*bi)->movement( game, dtFixed );	
	}

	// now commit changes to our sprite	
	m_sprite->update();
	
}

