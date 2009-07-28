#ifndef SHADER_H
#define SHADER_H

// shader style functions for map generation
struct Color {	
	float r,g,b;				

	Color();
	Color(float _r, float _g, float _b );
};

Color rgb2hsv( const Color &rgb );
Color hsv2rgb( const Color &hsv );


#endif