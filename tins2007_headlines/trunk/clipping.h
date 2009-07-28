#ifndef CLIPPING_H
#define CLIPPING_H

#include <string>
#include <allegro.h>

class Panel;

#define CHAR_W (12)

class Clipping {
public:
	Clipping( const std::string &text, int x=0, int y=0 );
	virtual ~Clipping();

	// does click hit clipping?
	bool hit( int x, int y, int scale );

	// location of clipping (in panel coords or screen coords for pasteboard)
	int m_x, m_y;	

	// remember where we came from in case of drag cancel		
	int m_origX, m_origY; // the clip's last known folder/bin location
	Panel *m_dragSrc;
	bool m_published;

	// data in clipping
	std::string m_text;
	BITMAP *m_sprite;	
};

#endif