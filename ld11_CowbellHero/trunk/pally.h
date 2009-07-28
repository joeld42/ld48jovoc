#ifndef FOCUS_PALLY_H
#define FOCUS_PALLY_H

// simplistic color class
struct Color
{
	Color();
	Color( float r, float g, float b );
	Color( int r, int g, int b );

	float r,g,b;
};

// not a pallete in the 8-bit sense, but in the graphic design sense of it.
class Pally
{
public:
	Pally();
	Color base, design1, design2, accent;	

	// Generates random (but aesthetic) color scheme
	void generate();
};




#endif