

#include "SceneObject.h"


//--------------------------------------------------------------------

SceneObject::SceneObject() {
	sgSetVec3( pos, 0.0, 0.0, 0.0 );
	sgSetVec3( hpr, 0.0, 0.0, 0.0 );

	sgSetVec4( dbgDiffColor, 1.0, 0.5, 0.5, 1.0 );
}

void SceneObject::drawCube( float xs, float ys, float zs ) {
	
	float y0 = 0.0, y1 = ys;
	xs /= 2; zs /= 2;

	glBegin( GL_QUADS );	
	
	glNormal3f( 0.0, 0.0, -1.0 );
	glVertex3f( -xs, y0, -zs );
	glVertex3f( -xs, y1, -zs );
	glVertex3f(  xs, y1, -zs );
	glVertex3f(  xs, y0, -zs );	

	glNormal3f( 0.0, 0.0, 1.0 );
	glVertex3f(  xs, y0, zs );
	glVertex3f(  xs, y1, zs );
	glVertex3f( -xs, y1, zs );
	glVertex3f( -xs, y0, zs );	

	glNormal3f( 0.0, -1.0, 0.0 );
	glVertex3f(  xs, y0, -zs );
	glVertex3f(  xs, y0,  zs );
	glVertex3f( -xs, y0,  zs );
	glVertex3f( -xs, y0, -zs );	

	glNormal3f( 0.0,  1.0, 0.0 );
	glVertex3f( -xs, y1, -zs );
	glVertex3f( -xs, y1,  zs );
	glVertex3f(  xs, y1,  zs );
	glVertex3f(  xs, y1, -zs );

	glNormal3f( -1.0, 0.0, 0.0 );
	glVertex3f( -xs, y0,  zs );
	glVertex3f( -xs,  y1,  zs );
	glVertex3f( -xs,  y1, -zs );
	glVertex3f( -xs, y0, -zs );	

	glNormal3f( 1.0, 0.0, 0.0 );
	glVertex3f( xs, y0, -zs );
	glVertex3f( xs,  y1, -zs );
	glVertex3f( xs,  y1,  zs );
	glVertex3f( xs, y0,  zs );

	glEnd();

	

}

void SceneObject::setupMaterials() {
	glDisable( GL_TEXTURE_2D );
	//glColor3f( dbgColor[0], dbgColor[1],  dbgColor[2] );	
	glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, dbgDiffColor );
}

void SceneObject::draw() {

	setupMaterials();
	glPushMatrix();
	glTranslated( pos[0], pos[1], pos[2] );
	drawGeom();
	glPopMatrix();
}

void SceneObject::drawGeom() {	
	// base object, just draw a cube
	drawCube( 0.08f, 0.08f, 0.08f );
}
