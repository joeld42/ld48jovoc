#ifndef PALLY_H
#define PALLY_H

#include <prmath/prmath.hpp>

// pallete... really more of a color scheme genrator than a usual palette
struct Pally
{
	Pally();
	void generate( int seed );

	vec3f m_colorSky;

	vec3f m_colorOrganic1;
	vec3f m_colorOrganic2;

	vec3f m_colorMineral1;
	vec3f m_colorMineral2;

	vec3f m_colorAccent;

};


#endif