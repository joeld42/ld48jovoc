#include "TIE.h"
#include "Sheep.h"


//--------------------------------------------------------------------
Sheep::Sheep() {
	annoyed = 0;
}

void Sheep::drawGeom() {	

	// body
	glPushMatrix();
	glTranslated( 0.0, 0.03, 0.0 );
	drawCube( 0.05f, 0.03f, 0.03f );
	glPopMatrix();

	// head
	glPushMatrix();
	glTranslated( -0.04, 0.04, 0.0 );
	drawCube( 0.025f, 0.025f, 0.025f );
	glPopMatrix();

	// leg
	glPushMatrix();
	glTranslated( -0.025, 0.0, -0.015 );
	drawCube( 0.01f, 0.035f, 0.01f );
	glPopMatrix();

	// leg
	glPushMatrix();
	glTranslated(  0.025, 0.0, -0.015 );
	drawCube( 0.01f, 0.035f, 0.01f );
	glPopMatrix();

	// leg
	glPushMatrix();
	glTranslated( -0.025, 0.0, 0.015 );
	drawCube( 0.01f, 0.035f, 0.01f );
	glPopMatrix();

	// leg
	glPushMatrix();
	glTranslated( 0.025, 0.0, 0.015 );
	drawCube( 0.01f, 0.035f, 0.01f );
	glPopMatrix();
}


