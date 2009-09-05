#ifndef JIMGUI_H
#define JIMGUI_H
// =======================================
// Joel's IM Gui -- joeld42@yahoo.com
// =======================================
#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <GL/gl.h>

#include <gamefontgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Widget ID's are application specific, 0
// is reserved but other than that they just
// need to be unique

struct Jgui_UIContext
{
	// NOTE: it is the responsibility of the app to fill
	// these in every frame. See comments in src for 
	// an example
	GLuint m_mouseX, m_mouseY;
	GLuint m_buttonState;

	// Internal stuff
	GLuint m_widgetHot;
	GLuint m_widgetActive;
	GLuint m_lastButtonState;	
};

// Initializes a context, alloc/release by the caller
void Jgui_initContext( Jgui_UIContext *ctx );

// call this every frame after doing UI controls
void Jgui_frameDone( Jgui_UIContext *ctx );

bool Jgui_doButton( GLuint self,
				    Jgui_UIContext *ctx,
				    const char *label,
				    GLuint fontId, 
					GLuint fontSize,
				    GLuint x, GLuint y,
				    GLuint w, GLuint h );


#ifdef __cplusplus
}
#endif


#endif