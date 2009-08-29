#ifndef SHAPE_H
#define SHAPE_H

#include <prmath/prmath.hpp>

class Shape
{
public:
	
	// simple case for just a single shape that
	// takes a whole texture
	static Shape *simpleShape( const char *texname );


//protected:
	Shape();

	GLuint m_texId;
	
	// corners of shape on texture
	vec2f st0, st1;
	vec2f pos;
};

#endif