#ifndef EDITOR_H
#define EDITOR_H

#include <Cavern.h>

class Editor
{
public:
	Editor( GLuint fntID );

	void redraw();
	void keypress( SDL_KeyboardEvent &key );

	void newLevel( vec2f size );

//protected:
	void frameView();

	Cavern *m_level;
	bool m_showHelp;
	GLuint m_fntFontId;

	// editor view
	bool m_useEditView;
	vec2f m_viewport, m_viewsize;

	// fixed game size view
	vec2f m_gameview;


};

#endif