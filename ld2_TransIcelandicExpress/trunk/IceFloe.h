#ifndef TIE_ICE_FLOE_H
#define TIE_ICE_FLOE_H

#include "SceneObject.h"

#include <vector>

enum FloeType {
	FLOE_ICE,
	FLOE_LAND,
	FLOE_BLOCKER,
	FLOE_CUSTOM
};

class TessTri {
public:
	sgVec3 a, b, c;
};

class IceFloe : public SceneObject { 

public:
	IceFloe();

	virtual void drawGeom();

	void build( int nsrcpnt, sgVec2 *srcpnt );

	// 2d, x and z only
	bool contains( sgVec3 &p );

	sgVec3 *pnts;
	int npnts;
	float health, height;
	int breakable, anchored;
	FloeType type;
	float sinkDelay, tscale;
	bool sinking;



	// the tesselated top
	std::vector<TessTri*> tess;

	sgVec4 landColor;
};

#endif