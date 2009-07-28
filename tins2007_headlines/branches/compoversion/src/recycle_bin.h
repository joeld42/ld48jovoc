#ifndef RECYCLE_BIN_H
#define RECYCLE_BIN_H

#include <allegro.h>
#include "panel.h"
#include "headline.h"
#include "clipping.h"

class RecycleBin : public Panel 
{
public:
	RecycleBin( std::string name, HeadlinePool *hpool,
				int pcol, int x, int y, int w, int h );

	// Update internal bitmap
	virtual void draw();
	virtual void update( float dt );
	
	virtual bool dropClip( Clipping *clip, int x, int y );

	HeadlinePool *m_hpool;
	std::vector<Clipping*> m_clips;

	float m_nextClipTime;
};

#endif