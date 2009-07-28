#ifndef COMBAT_H
#define COMBAT_H

#include <allegro.h>

#include "buttonlands.h"
#include "hero.h"

void set_hero( BMHero *hero );
void set_enemy( BMHero *enemy );

void combat_draw( BITMAP *targ );
int combat_update();

enum {
	STILL_FIGHTING,
	HERO_WON,
	ENEMY_WON,
	DRAW
};

void start_combat();

void draw_portrait( BITMAP *targ, int x, int y, BMHero *hero, bool drawdice, bool equipMode=false );
void init_combat();
#endif