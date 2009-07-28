#ifndef TIE_SHEEP_H
#define TIE_SHEEP_H

#include "TIE.h"
#include "SceneObject.h"

class Sheep : public SceneObject {
public:
	Sheep( );
	virtual void drawGeom();

	int annoyed;
	float annoyed_time;
};

#endif