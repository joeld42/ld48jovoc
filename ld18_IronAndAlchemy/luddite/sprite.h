#ifndef SPRITE_H
#define SPRITE_H
/*
 *  sprite.h
 *
 *  Created by joeld on 7/10/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */
#include <list>

//#import <OpenGLES/ES1/gl.h>
//#import <OpenGLES/ES1/glext.h>
#include "luddite/GLee.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "quadbuff.h"

#define SPRITE_D2R (3.1415926535897932384626433832795f/180.0f)

struct SpriteVert
{
	GLfloat pos[2];
	GLfloat st[2];
};

class Sprite;

class SpriteBuff : public QuadBuff<SpriteVert>
{
public:
	SpriteBuff( GLuint texId );
	
	Sprite *makeSprite( float s0=0.0, float t0=0.0,
					    float s1=1.0, float t1=1.0 );
	
	void removeSprite( Sprite *spr );
	void removeAll();
	
	// Draw all the sprites in this buff
	void renderAll();
		
private:
	GLuint m_texId;
	std::list<Sprite*> m_sprites;
};

class Sprite
{
public:
	Sprite( SpriteBuff *owner, SpriteVert *vertData );
	~Sprite();
	
	// Center position, rotation, size
	float x, y;
	float angle;
	float sx, sy;
	
	// Updates vert data if pos/angle/sz change
	void update();
	
	// Test if a point hits us
	bool testHit( float x, float y );
	
	// set st coords
	void setTexCoords( float s0, float t0,
				  float s1, float t1 );
	
	// Flips texture coords horizontally
	void flipHoriz();
	
	// Buffer owning all sprites that share this texture
	SpriteBuff *m_myBuff;
	
	// Pointer into Spritebuff
	SpriteVert *m_vertData;

	// HACKY flip and keep track
	bool mirrorX;
	void setMirrorX( bool doFlip );

	// HACKY 
	bool canShoot;
};

#endif