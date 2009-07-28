#ifndef FOLDER_H
#define FOLDER_H

#include <allegro.h>
#include "panel.h"
#include "headline.h"
#include "clipping.h"

class ClipFolder : public Panel 
{
public:
	ClipFolder( std::string name, int pcol, int x, int y, int w, int h );

	// Update internal bitmap
	virtual void draw();
	virtual void update( float dt );

	virtual bool dropClip( Clipping *clip, int x, int y );

	std::vector<Clipping*> m_clips;
};

#endif