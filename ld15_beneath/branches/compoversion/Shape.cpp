#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <tweakval.h>
#include <Common.h>
#include <Shape.h>


using namespace prmath;

Shape::Shape()
{	
	st0 = vec2f( 0.0, 0.0 );
	st1 = vec2f( 1.0, 1.0 );
	pos = vec2f( 0, 0 );
	blendMode = Blend_OFF;
	m_pattern = false;

	angle = 0;
	sortNum = 1000;
}

Shape *Shape::simpleShape(const std::string &texname )
{
	Shape *shp = new Shape();
	
#if 0
	ILuint ilImgId;
	ilGenImages( 1, &ilImgId );
	ilBindImage( ilImgId );		
	
	if (!ilLoadImage( (ILstring)texname )) {
		printf("Loading %s image failed\n", texname);
	}
	
	// Make a GL texture for it
	shp->m_texId = ilutGLBindTexImage();
#endif
	
	int w, h;
	shp->m_texId = getTexture( texname, &w, &h );
	shp->m_size = vec2f( w, h );
	printf("LOADED %s, %d x %d\n", texname.c_str(), w, h );

	return shp;
}
	
void Shape::drawBraindead()
{
	glBindTexture( GL_TEXTURE_2D, m_texId );	
	drawBraindeadQuad();
}

void Shape::drawBraindeadQuad()
{
	float zval = -sortNum / 10000.0f;

	float sv = sin( angle * D2R ) * m_size.x;
	float cv = cos( angle * D2R ) * m_size.y;	

	glColor3f( 1.0, 1.0, 1.0 );
	
	glBegin( GL_QUADS );
	
	doVert( vec2f( st0.x, st0.y ),
			vec3f( pos.x - (cv-sv), pos.y + (sv+cv), zval) );

	doVert( vec2f( st0.x, st1.y  ),
			vec3f( pos.x - (cv+sv), pos.y + (sv-cv), zval) );

	doVert( vec2f( st1.x, st1.y ),
			vec3f( pos.x - (-cv+sv), pos.y + (-sv-cv), zval) );

	doVert( vec2f( st1.x, st0.y  ),
			vec3f( pos.x - (-cv-sv), pos.y + (-sv+cv), zval) );
	
	glEnd();
}

void Shape::doVert( vec2f &st, vec3f &pos )
{
	if (m_pattern)
	{
		// patterns use pos as ST (repeating)
		glTexCoord2d( pos.x / m_origSize.x, -pos.y / m_origSize.y  ); 	
		glVertex3f( pos.x, pos.y, pos.z );
	}
	else
	{
		glTexCoord2d( st.x, st.y  ); 	
		glVertex3f( pos.x, pos.y, pos.z );
	}
}