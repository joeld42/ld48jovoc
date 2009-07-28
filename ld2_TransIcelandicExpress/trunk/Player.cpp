#include "Player.h"

//--------------------------------------------------------------------
Player::Player() : SceneObject() {
	sgSetVec3( vel, 0.0, 0.0, 0.0 );
}

void Player::drawGeom() {
	// base object, just draw a cube
	drawCube( 0.015f, 0.03f, 0.015f );
}
