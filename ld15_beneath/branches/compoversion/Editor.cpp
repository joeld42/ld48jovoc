#include <Editor.h>
#include <tweakval.h>

Editor::Editor()
{
	m_level = NULL;
}

void Editor::redraw()
{
	glClearColor( _TV( 0.2f ), _TV(0.0f), _TV( 0.0f ), 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Editor::keypress( SDL_KeyboardEvent &key )
{
}