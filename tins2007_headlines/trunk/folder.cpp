#include <allegro.h>

#include <assert.h>

#include "panel.h"
#include "folder.h"

ClipFolder::ClipFolder( std::string name, int pcol, int x, int y, int w, int h ) :
	Panel( name, pcol, x, y, w, h )
{
}

void ClipFolder::draw()
{
	Panel::draw();

	// draw my clippings
	for (std::vector<Clipping*>::iterator ci = m_clips.begin();
		 ci != m_clips.end(); ci++)
	{
		Clipping *c = (*ci);
		draw_sprite( m_surf, c->m_sprite, c->m_x, c->m_y );
	}

	
}

void ClipFolder::update( float dt )
{
	Panel::update( dt );
}

bool ClipFolder::dropClip( Clipping *clip, int x, int y )
{
	m_clips.push_back( clip );
	clip->m_x = x - (clip->m_sprite->w / 2);
	clip->m_y = y;

	return true;
}