#ifndef SHAPE_H
#define SHAPE_H

#include <prmath/prmath.hpp>

enum {
	Blend_OFF,
	Blend_NORMAL,
	Blend_ADD
};

class Shape
{
public:
	
	// simple case for just a single shape that
	// takes a whole texture
	static Shape *simpleShape( const std::string &texname );

	// simple drawing, for cards and stuff.. 
	void drawBraindead();	
	void drawBraindeadQuad();	

//protected:
	Shape();

	GLuint m_texId;
	int blendMode;
	
	// corners of shape on texture
	vec2f st0, st1;
	vec2f pos;

	// Some of this is redundant, should have shape
	// and shapeInstances, but keep it simple for the compoversion
	std::string name;
	bool m_collide;
	bool m_pattern;
	vec2f m_size;
};

#endif