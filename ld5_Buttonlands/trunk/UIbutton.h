#ifndef UI_BUTTON
#define UI_BUTTON

#include <allegro.h>
#include <string>
#include <vector>

#include "buttonlands.h"

// The user-interface kind, not the beating-people-up kind

struct BMButton {
	int x1, y1, x2, y2, col;
	std::string label;
	
	void mouse_check();
	void draw( BITMAP * targ, ALFONT_FONT *bfont );

	bool mouseIn;

	BMButton( std::string _label,
			  int _x1, int _y1, int _x2, int _y2, int _col );

	static std::vector<BMButton*> allUIBtns;

};

#endif