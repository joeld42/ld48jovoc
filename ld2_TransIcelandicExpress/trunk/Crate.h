#ifndef TIE_CRATE_H
#define TIE_CRATE_H

#include "SceneObject.h"

class Crate : public SceneObject{
public:
	Crate();

	virtual void drawGeom();

	int onLand;
};


#endif