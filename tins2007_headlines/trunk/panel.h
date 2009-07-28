#ifndef PANEL_H
#define PANEL_H

#include <string>
#include <allegro.h>

#include "clipping.h"

// A "widget" representing some gameplay element
// base class
class Panel {
public:
	Panel( std::string name, int pcol, int x, int y, int w, int h );

	// Update internal bitmap
	virtual void draw();

	virtual void update( float dt );

	// panels should return true if they accept the drop,
	// they assume ownership of the clip
	virtual bool dropClip( Clipping *clip, int x, int y );

	// checks if mouse is over panel 
	virtual bool hit( int x, int y );

	// stuff
	BITMAP *m_surf, *m_background;
	int m_x, m_y, m_w, m_h;
	std::string m_name;
	int m_pcol;
};

#endif