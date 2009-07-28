#ifndef TIE_SCENE_OBJ
#define TIE_SCENE_OBJ

#include <plib/sg.h>

class SceneObject {
public:

	SceneObject();

	// does the materials and xform, and calls drawGeom
	virtual void draw();

	// just the geom
	virtual void drawGeom();

	// interal useful drawing stuff
	void setupMaterials();
	void drawCube( float xs, float ys, float zs );
	
	sgVec3 pos, hpr;
	sgVec4 dbgDiffColor;
	
	float fallRate;
};

#endif