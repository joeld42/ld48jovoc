#ifndef TIE_H
#define TIE_H

#include <windows.h>

#include <vector>
#include <deque>

#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <plib/sg.h>
#include <plib/fnt.h> 

#include <SDL_mixer.h>

// no music because I didn't have time to make any during
// the contest
#define MUSIC 

// the font stuff
extern fntRenderer *fnt;
extern fntTexFont *fntHelv;

#define MAX_ROAD (100)

#define SQR(n) ((n)*(n))


#define TAKE_MOUSE

enum VictoryType {
	CONNECT_ALL_ROADS,
	GET_ALL_CRATES,
	KILL_ALL_SHEEP
};



#endif // TIE_H