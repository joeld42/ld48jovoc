#include <string>
#include <allegro.h>
#include "panel.h"

Panel::Panel( std::string name, int pcol, int x, int y, int w, int h ) :
	m_name( name ), m_pcol(pcol), m_x( x ), m_y( y ), m_w( w ), m_h( h )
{
	// Create a bitmap for our panel
	m_surf = create_bitmap( m_w, m_h );

	m_background = NULL;
}

// Update internal bitmap
void Panel::draw()
{	
	// Default, base class placholder
	if (!m_background)
	{

#if 0
		rectfill( m_surf, 0, 0, m_w, m_h, 0x550055 );

		rect( m_surf, 0, 0, m_w-1, m_h-1, m_pcol );
		line( m_surf, 0, 0, m_w-1, m_h-1, m_pcol );
		line( m_surf, 0, m_h-1, m_w-1, 0, m_pcol );	

		textout_centre_ex(m_surf, font, m_name.c_str(), 
			m_w/2, m_h/2, m_pcol, -1);
#endif 

	}
	else
	{
		blit( m_background, m_surf, 0, 0, 0, 0, m_background->w, m_background->h );
	}
}

void Panel::update( float dt)
{
	// do nothing
}

bool Panel::dropClip( Clipping *clip, int x, int y )
{
	// Default: don't accept
	return false;
}

bool Panel::hit( int x, int y )
{
	// inside bbox?
	if ( (x < m_x) || (y < m_y) ||
		 (x >= m_x + m_w) || ( y >= m_y + m_h ) ) return false;
	
	// check the colorkey for the image
	int p = getpixel( m_surf, x - m_x, y - m_y );
	if (p==makecol32(0xff, 0, 0xff)) return false;
	

	// otherwise
	return true;
}