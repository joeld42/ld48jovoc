#include <windows.h>

#include <vector>
#include <deque>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <prmath/prmath.hpp>
using namespace prmath;

#include "gamefontgl.h"
#include "game_shell.h"

#include "iprof/prof.h"
#include "debug.h"


using namespace std;
using namespace October;
using namespace DBG;

#define MAX_CONSOLE_LINES (120)
#define MAX_FLOATING_LINES (500)

#define MAX_LINE (1024)

struct DbgTextLine {	
	vec2f screenPos;	
	char line[MAX_LINE];
};

DbgTextLine g_dbgText_Console[MAX_CONSOLE_LINES];
Uint32 g_nConsoleLines;

DbgTextLine g_dbgText_Floating[MAX_FLOATING_LINES];
Uint32 g_nFloatingLines;

// To project text to 2d
matrix4x4d g_text3D_Modelview;
matrix4x4d g_text3D_Projection;
int g_text3D_Viewport[4];

// "console style" debug text on screen
void DBG::Printf( const char *fmt, ... )
{
	Prof( DBG_Printf_con );

	// console is full for this frame
	if (g_nConsoleLines >= MAX_CONSOLE_LINES) return;

	va_list args; 
	va_start(args,fmt); 	
	vsprintf( g_dbgText_Console[g_nConsoleLines++].line, fmt, args);
	va_end(args); 
}

// 3D debug text
void DBG::Printf( float x, float y, float z,  const char *fmt, ... )
{
	Prof( DBG_Printf_xyz );

	// floating text is full for this frame
	if (g_nFloatingLines >= MAX_FLOATING_LINES) return;	
	
	GLdouble xx, yy, zz;
	gluProject( x, y, z, 
				g_text3D_Modelview.m16, 
				g_text3D_Projection.m16, 
				g_text3D_Viewport,
				&xx, &yy, &zz );	

	// check near/far clip
	if ((zz >= 0.0f) && (zz <= 1.0f))
	{
		va_list args; 
		va_start(args,fmt); 	

		g_dbgText_Floating[g_nFloatingLines].screenPos = vec2f( (float)xx, (float)yy );
		vsprintf( g_dbgText_Floating[g_nFloatingLines++].line, fmt, args);

		va_end(args); 
	}	
}

// 2D debug text, x,y are in screen space
void DBG::Printf( int x, int y,  const char *fmt, ... )
{
	Prof( DBG_Printf_xy );

	// floating text is full for this frame
	if (g_nFloatingLines >= MAX_FLOATING_LINES) return;

	va_list args; 
	va_start(args,fmt); 	
	g_dbgText_Floating[g_nFloatingLines].screenPos = vec2f( (float)x, (float)y );
	vsprintf( g_dbgText_Floating[g_nFloatingLines++].line, fmt, args);
	va_end(args); 
}

// Used to implement this text, call from gameshell
void DBG::_dbgtext_NewFrame()
{
	Prof( DBG_dbgtext_NewFrame );

	g_nConsoleLines = 0;
	g_nFloatingLines = 0;
}

void DBG::_dbgtext_FinishFrame2D()
{
	Prof( DBG_dbgtext_FinishFrame2D );

	gfEnableFont( October::GameShell::Game().ConsoleFont(), 12 );
	glColor3f ( 1.0f, 1.0f, 1.0f ) ;
	
	// start text
	gfBeginText(); 

	// Draw "Console" text	
	for (Uint32 i=0; i < g_nConsoleLines; i++) 
	{
		glPushMatrix();
		glTranslated ( 20, October::GameShell::Game().GetScreensizeY() - ((i*12)+20), 0 ) ;
		gfDrawString( g_dbgText_Console[i].line );
		glPopMatrix();
	}

	// Draw 2d and 3d text
	for (Uint32 i=0; i < g_nFloatingLines; i++) 
	{
		glPushMatrix();
		glTranslated ( g_dbgText_Floating[i].screenPos.x, g_dbgText_Floating[i].screenPos.y, 0 ) ;
		gfDrawString( g_dbgText_Floating[i].line );
		glPopMatrix();
	}

	// text done
	gfEndText();	
}

void DBG::_dbgtext_GetProjection()
{
	Prof( DBG_dbgtext_GetProjection );

	// Don't draw anything here, just snarf the current modelview matrix
	glGetDoublev( GL_MODELVIEW_MATRIX,  g_text3D_Modelview.m16 );
	glGetDoublev( GL_PROJECTION_MATRIX, g_text3D_Projection.m16 );
	glGetIntegerv( GL_VIEWPORT, g_text3D_Viewport );	
}
