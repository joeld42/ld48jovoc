#ifndef TIE_ROAD_H
#define TIE_ROAD_H

#include "SceneObject.h"

class Road : public SceneObject{
public:
	Road();

	virtual void drawGeom();

	int road;
	int pylon;
	int building_pylon, building_road;

	float base_height;

	static sgVec4 pylonColor, baseColor, roadColor;
};


#endif