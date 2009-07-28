
#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <prmath/prmath.hpp>

#include "gamefontgl.h"

#include "Gamepad.h"

// the button states
vec2f gpButtonPos[GP_MAX_BUTTONS];

// The game pad diagram
ILuint ilGamepadDiag, ilGamepadIndicators;
GLuint texGamepadDiag, texGamepadIndicators;

void initGamepadDiagram() 
{
	static int init = 0;

	if (init) return;
	init = 1;


	// Gamepad button positions for diagram
	gpButtonPos[0] = vec2f( 177, 52 );
	gpButtonPos[1] = vec2f( 189, 42 );
	gpButtonPos[2] = vec2f( 203, 40 );
	gpButtonPos[3] = vec2f( 164, 41 );
	gpButtonPos[4] = vec2f( 176, 32 );
	gpButtonPos[5] = vec2f( 190, 28 );
	gpButtonPos[6] = vec2f( 72, -15); // left trigger
	gpButtonPos[7] = vec2f( 170, -15 ); // right trigger
	gpButtonPos[8] = vec2f( 102, 40 ); // start

	// Load the images for the gamepad diagram
	ilGenImages( 1, &ilGamepadDiag );
	if (!ilLoadImage( "gamepad.png" )) 
		printf("Loading gamepad.png image failed\n");
		
	texGamepadDiag = ilutGLBindTexImage();

	ilGenImages( 1, &ilGamepadIndicators );
	if (!ilLoadImage( "gamepad-indicators.png" )) 
		printf("Loading gamepad-indicators.png image failed\n");
		
	texGamepadIndicators = ilutGLBindTexImage();
}

//////////////////////////////////////////////////////////////////////////
// Draw Gamepad Diagram
//////////////////////////////////////////////////////////////////////////

void drawGamepadDiagram( GamePadState &gamepad, GLuint fntFontId )  {
	int i;

	glBindTexture( GL_TEXTURE_2D, texGamepadDiag );
	glPushMatrix();	
	glTranslated( 800-256, 0, 0 );

	// main background image
	glColor3f( 1.0, 1.0, 1.0 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2d( 0.0, 1.0 ); glVertex3f( 0.0,  128.0, 0.0 );
	glTexCoord2d( 1.0, 1.0 ); glVertex3f(  256.0,  128.0, 0.0 );
	glTexCoord2d( 1.0, 0.0 ); glVertex3f(  256.0, 0.0, 0.0 );
	glEnd();

	// buttons
	float x, y, s1, s2, t;
	glBindTexture( GL_TEXTURE_2D, texGamepadIndicators );
	for (i=0; i < 9; i++) {
		glBegin( GL_QUADS );
		if (gamepad.button[i]) {
			x = gpButtonPos[i][0] - 16;
			y = (128-gpButtonPos[i][1]) - 16;
			if ((i==6)||(i==7)) {
				// arrow indicators for trigger buttons
				s1 = 0.25; s2 = 0.5;
			} else {
				// regular glowy indicator for regular buttons
				s1 = 0.0; s2 = 0.25;

			}
			glTexCoord2d( s1, 0.0 );   glVertex3f( x,y, 0.0 );
			glTexCoord2d( s1, 1.0 );   glVertex3f( x, y+32, 0.0 );
			glTexCoord2d( s2, 1.0 );  glVertex3f( x+32,y+32, 0.0 );
			glTexCoord2d( s2, 0.0 );  glVertex3f( x+32, y, 0.0 );
		}
		glEnd();
	}

	// control sticks
	x = 89 + (gamepad.stick[0] * 20); x = x - 8;
	y = (128-64) + (gamepad.stick[1] * -20); y = y - 8;
	glBegin (GL_QUADS);
	glTexCoord2d( 0.5, 0.0 );   glVertex3f( x,y, 0.0 );
	glTexCoord2d( 0.5, 1.0 );   glVertex3f( x, y+16, 0.0 );
	glTexCoord2d( 0.75, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
	glTexCoord2d( 0.75, 0.0 );  glVertex3f( x+16, y, 0.0 );
	glEnd();

	x = 150 + (gamepad.stick2[0] * 20); x = x - 8;
	y = (128-64) + (gamepad.stick2[1] * -20); y = y - 8;
	glBegin (GL_QUADS);
	glTexCoord2d( 0.5, 0.0 );   glVertex3f( x,y, 0.0 );
	glTexCoord2d( 0.5, 1.0 );   glVertex3f( x, y+16, 0.0 );
	glTexCoord2d( 0.75, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
	glTexCoord2d( 0.75, 0.0 );  glVertex3f( x+16, y, 0.0 );
	glEnd();

	t = ((-0.5f*gamepad.throttle)+0.5f); // make t 0...1
	x = 144 + (t*40); x = x - 8;
	y = (128-25) + (t*9); y = y - 8;
	glBegin (GL_QUADS);
	glTexCoord2d( 0.5, 0.0 );   glVertex3f( x,y, 0.0 );
	glTexCoord2d( 0.5, 1.0 );   glVertex3f( x, y+16, 0.0 );
	glTexCoord2d( 0.75, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
	glTexCoord2d( 0.75, 0.0 );  glVertex3f( x+16, y, 0.0 );
	glEnd();
		
	

	// hat
	if ( gamepad.hat & SDL_HAT_UP ) {		
		x = 63 - 8; y = (128-24) - 8;		
		glBegin (GL_QUADS);
		glTexCoord2d( 0.0, 0.0 );   glVertex3f( x,y, 0.0 );
		glTexCoord2d( 0.0, 1.0 );   glVertex3f( x, y+16, 0.0 );
		glTexCoord2d( 0.25, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
		glTexCoord2d( 0.25, 0.0 );  glVertex3f( x+16, y, 0.0 );
		glEnd();
	}

	if ( gamepad.hat & SDL_HAT_DOWN ) {		
		x = 61 - 8; y = (128-51) - 8;		
		glBegin (GL_QUADS);
		glTexCoord2d( 0.0, 0.0 );   glVertex3f( x,y, 0.0 );
		glTexCoord2d( 0.0, 1.0 );   glVertex3f( x, y+16, 0.0 );
		glTexCoord2d( 0.25, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
		glTexCoord2d( 0.25, 0.0 );  glVertex3f( x+16, y, 0.0 );
		glEnd();
	}

	if ( gamepad.hat & SDL_HAT_LEFT ) {		
		x = 47 - 8; y = (128-38) - 8;		
		glBegin (GL_QUADS);
		glTexCoord2d( 0.0, 0.0 );   glVertex3f( x,y, 0.0 );
		glTexCoord2d( 0.0, 1.0 );   glVertex3f( x, y+16, 0.0 );
		glTexCoord2d( 0.25, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
		glTexCoord2d( 0.25, 0.0 );  glVertex3f( x+16, y, 0.0 );
		glEnd();
	}

	if ( gamepad.hat & SDL_HAT_RIGHT ) {		
		x = 76 - 8; y = (128-38) - 8;		
		glBegin (GL_QUADS);
		glTexCoord2d( 0.0, 0.0 );   glVertex3f( x,y, 0.0 );
		glTexCoord2d( 0.0, 1.0 );   glVertex3f( x, y+16, 0.0 );
		glTexCoord2d( 0.25, 1.0 );  glVertex3f( x+16,y+16, 0.0 );
		glTexCoord2d( 0.25, 0.0 );  glVertex3f( x+16, y, 0.0 );
		glEnd();
	}

	glPopMatrix();	

	// diagram label
	static int gpNameWidth = -1;
	gfEnableFont( fntFontId, 15 );
	if (gpNameWidth==-1) {
		gpNameWidth = gfGetStringWidth( gamepad.name );

		// BUG: for some reason the y value for stick2 gets set to -1.0 
		// at startup and just stays there.. set it to 0.0 at least so it
		// doesn't look funny. I wouldn't advise using throttle or stick2
		gamepad.stick2[1] = 0.0;
	}
	gfBeginText();
	glColor3f( 1.0f, 1.0f, 1.0f );
	glTranslated( 790-gpNameWidth, 20, 0 );
	gfDrawString( gamepad.name );
	gfEndText();

}
