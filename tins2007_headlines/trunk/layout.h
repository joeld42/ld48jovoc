#ifndef LAYOUT_H
#define LAYOUT_H

#include "panel.h"

#define MAX_COLS (10)
#define YCOLS (10)
#define COL_SZ (25)

class NewsLayout : public Panel {
public:
	NewsLayout( std::string name, int pcol, int x, int y, int w, int h );

	// Update internal bitmap
	virtual void draw();
	virtual void update( float dt );
		
	// set number of columns
	void setNCols( int cols );

	virtual bool dropClip( Clipping *clip, int x, int y );

	// stories
	void printStory( std::string &headline, int *pSX = NULL, int *pSY = NULL );	

	std::vector<int> m_storiesLeft;

	// display
	int m_ncols;
	int m_layout[MAX_COLS][YCOLS];

	bool m_dirty; // needs redraw?

	BITMAP *m_newsImg, *m_newsImg2;
	BITMAP *m_bodyText, *m_masthead;
};

#endif