#ifndef SPRITE_H
#define SPRITE_H

#include <allegro.h>
#include <vector>

class SpriteGraphic {
public:
	SpriteGraphic( char *filename );
	~SpriteGraphic();	

	void load_and_chop( char *filename );

	std::vector<BITMAP*> m_frames;

};

class Sprite {
public:
	SpriteGraphic *m_gfx;
	int m_x, m_y, m_ang, m_frame;

	Sprite( SpriteGraphic *gfx );

	void draw( BITMAP *target );

};

#endif