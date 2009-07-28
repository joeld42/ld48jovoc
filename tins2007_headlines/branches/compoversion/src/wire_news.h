#ifndef WIRE_NEWS_H
#define WIRE_NEWS_H

#include "panel.h"
#include "headline.h"

#define HEADLINE_TIME (10.0f)

class WireNews : public Panel {
public:
	WireNews( std::string name, HeadlinePool *hpool,
		  int pcol, int x, int y, int w, int h );

	// Update internal bitmap
	virtual void draw();

	virtual void update( float dt );

	void click( int x, int y );

	bool checkHeadline( std::vector<Clipping*> &clips, std::string &hltext );
	
	std::vector<Headline*> m_heads;
	HeadlinePool *m_hpool;	
	
	// 
	float m_nextHeadlineTime;

	// for display
	BITMAP *m_hblank;
};

#endif