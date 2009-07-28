#include <allegro.h>
#include <alfont.h>

#include "buttonlands.h"
#include "UIbutton.h"

std::vector<BMButton*> BMButton::allUIBtns;

void BMButton::mouse_check()
{
	if ( (mouse_x > x1) && (mouse_x < x2) &&
		 (mouse_y > y1) && (mouse_y < y2)) {

		mouseIn = true;
	} else {
		mouseIn = false;
	}
}

void BMButton::draw( BITMAP * targ, ALFONT_FONT *bfont )
{
	int bcol = col;
	if (mouseIn) {
		int r,g,b;
		r = getr(col); g = getg(col); b = getb( col );
		r = min( r+20, 255); g = min(g+20, 255); b = min( b+20, 255 );
		bcol = makecol( r,g,b );
	}

	//rect( targ, x1,y1,x2,y2, makecol(255,0,0) );

	alfont_textout_centre_aa( targ, bfont, label.c_str(),
							  x1 + (x2-x1)/2, y1, bcol );
}

BMButton::BMButton( std::string _label, int _x1, int _y1, int w, int h, int _col )
{
	x1 = _x1; y1 = _y1;
	x2 = _x1 + w; y2 = _y1 + h;

	col = _col;
	label = _label;

	allUIBtns.push_back( this );
}

static std::vector<BMButton*> allUIBtns;