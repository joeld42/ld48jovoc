#ifndef HERO_H
#define HERO_H

#include <allegro.h>

#include <string>
#include <vector>

enum {
	SIDES_NONE  =0,
	SIDES_2     =2,
	SIDES_4     =4,
	SIDES_6     =6,
	SIDES_8     =8,
	SIDES_10    =10,
	SIDES_12    =12,
	SIDES_20    =20,
	

	// specials
	SIDES_X     = 101,	
};

struct BMDie
{
	// stats
	int nrmsides, altsides;
	bool shadow;

	// curr state
	int sides;
	int val;
	bool dead;

	int selected;
	int targetable;

	void roll();

	// UI stuff
	int xpos, ypos;
};

struct BMHero 
{
	std::string name;
	BMDie die[5];
	std::string bio;
	BITMAP *photo;

	void rollAll();
};

void load_buttons( const char *filename, std::vector<BMHero*> &heros );

#endif