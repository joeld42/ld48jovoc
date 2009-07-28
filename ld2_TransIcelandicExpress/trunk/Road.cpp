#include "TIE.h"
#include "Road.h"
#include "Game.h"

sgVec4 Road::pylonColor = { 0.8f, 0.4f, 0.4f, 1.0f };
sgVec4 Road::baseColor  = { 0.3f, 0.3f, 0.3f, 1.0f };
sgVec4 Road::roadColor  = { 0.3f, 0.3f, 0.3f, 1.0f };


//--------------------------------------------------------------------
Road::Road(): SceneObject(){
	road = 0;
	pylon = 0;
	building_pylon = 0;
	building_road = 0;
	base_height = 0.0;
}

void Road::drawGeom() {
	if ((!pylon) && (!road)) return;

	if (building_pylon) {
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE );
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
		glColor3f( 1.0, 0.5, 1.0 );
	}

	if (pylon) {
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, baseColor );
		drawCube( 0.25, base_height, 0.25 );

		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pylonColor );
		drawCube( 0.125, TransIcelandicExpress::c_RoadLevel+0.05, 0.125 );
	}

	if (building_pylon) {
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );		
	}

	if (building_road) {
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE );
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
		glColor3f( 0.6f, 0.3f, 0.6f );
	}

	if (road) {
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, roadColor );
		glTranslated( 0.0, TransIcelandicExpress::c_RoadLevel-0.01, 0.0 );
		drawCube( 1.0f, 0.01f, 1.0f );
	}

	if (building_road) {
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );		
	}
	
}
