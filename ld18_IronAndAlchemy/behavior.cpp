#include <luddite/atom.h>

#include "behavior.h"
#include "entity.h"

Behavior::Behavior( Entity *owner ) :
	m_owner( owner )
{
}

void Behavior::addTag( const char *tag )
{
	const char *atomizedTag = Atom_string( tag );
	m_tags.push_back( atomizedTag );
}

bool Behavior::hasTag( const char *tag )
{
	for (std::list<const char *>::iterator tagi = m_tags.begin();
		 tagi != m_tags.end(); ++tagi )
	{
		// note: strings are atomized so we can compare directly
		if ((*tagi)==tag) return true;
	}
	return false;
}

void Behavior::movement( IronAndAlchemyGame *game, float dtFixed )
{
	// do nothing..
}