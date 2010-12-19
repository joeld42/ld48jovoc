#ifndef PALLY_H
#define PALLY_H

#include "PVRT/PVRTVector.h"
#include "PVRT/PVRTMatrix.h"

// pallete... really more of a color scheme genrator than a usual palette
struct Pally
{
	Pally();
	void generate( int seed );

	PVRTVec3 m_colorSky;

	PVRTVec3 m_colorOrganic1;
	PVRTVec3 m_colorOrganic2;

	PVRTVec3 m_colorMineral1;
	PVRTVec3 m_colorMineral2;

	PVRTVec3 m_colorAccent;

};


#endif