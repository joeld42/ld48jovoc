#ifndef SHAPE_H
#define SHAPE_H

#include <prmath/prmath.hpp>

class Shape
{
public:
	
	// simple case for just a single shape that
	// takes a whole texture
	static Shape *simpleShape( const std::string &texname );


//protected:
	Shape();

	GLuint m_texId;
	
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