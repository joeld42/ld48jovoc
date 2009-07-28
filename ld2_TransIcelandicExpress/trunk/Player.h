#ifndef TIE_PLAYER_H
#define TIE_PLAYER_H

#include "SceneObject.h"

class Player : public SceneObject {

public:
	Player();

	virtual void drawGeom();

	sgVec3 vel;
};

#endif