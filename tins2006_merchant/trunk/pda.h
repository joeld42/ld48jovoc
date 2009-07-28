#ifndef PDA_H
#define PDA_H

#include <allegro.h>

// PDA mode
enum {
	PDA_TITLESCREEN, 
		PDA_STARTGAME,  // submodes of titlescreen
		PDA_GENERATE,	
};
extern int g_pdaMode;

extern int g_genPercent;

extern int g_currTabItem;

void pda_draw( BITMAP *targ );
void pda_keypress( int k );
void pda_update();

#endif