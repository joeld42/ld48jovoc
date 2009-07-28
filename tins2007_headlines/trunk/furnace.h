#ifndef FURNACE_H
#define FURNACE_H

#include <allegro.h>

#include "panel.h"

class Furnace : public Panel 
{
public:
	Furnace( std::string name, int pcol, int x, int y, int w, int h );

	// Update internal bitmap
	virtual void draw();
	virtual void update( float dt );

	virtual bool dropClip( Clipping *clip, int x, int y );

	// fire pal bitmap is used only for the palette
	BITMAP *m_fire_pal_bmp;
	PALETTE m_fire_pal;

	// fire effect
	BITMAP *m_fire_fx;     // flame effect
	BITMAP *m_fire_fx2;    // flame effect + noise pattern
	BITMAP *m_fire_final;  // converted to truecolor

	// Add a noise pattern to make flame more flamey
	BITMAP *m_fire_noise;
	int offs1x, offs1y, offs2x, offs2y;
	float offs_t;
};

#endif