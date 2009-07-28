#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <SDL.h>
#include <SDL_endian.h>

// The gamepad
//
// This is set up for my gamepad, a Logitech wingman, 
// but as long as you stick with the basics it should
// be pretty universal... 
#define GP_MAX_BUTTONS (10)
struct GamePadState {
	SDL_Joystick *sdlinfo;
	bool button[GP_MAX_BUTTONS];
	unsigned int hat;
	vec2f stick, stick2;
	float throttle;
	char *name;
};


// The helpful gamepad diagram
void initGamepadDiagram();
void drawGamepadDiagram( GamePadState &gamepad, GLuint fntFontId );

#endif