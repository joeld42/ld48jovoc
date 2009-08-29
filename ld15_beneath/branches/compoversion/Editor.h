#ifndef EDITOR_H
#define EDITOR_H

#include <Cavern.h>

class Editor
{
public:
	Editor();

	void redraw();
	void keypress( SDL_KeyboardEvent &key );

	Cavern *m_level;
};

#endif