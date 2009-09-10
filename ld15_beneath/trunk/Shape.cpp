#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

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
	m_relief = false;

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

#if 1
	float sv = sin( angle * D2R ) * m_size.x;
	float cv = cos( angle * D2R ) * m_size.y;		

	glColor3f( 1.0, 1.0, 1.0 );	
	if (!m_relief)
	{	
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
	else
	{
		glBegin( GL_TRIANGLE_FAN );	
		doVert( vec2f( st0.x + st1.x/2.0f, st0.y + st1.y/2.0f ),
				vec3f( pos.x, pos.y, zval - 0.5) );

		doVert( vec2f( st0.x, st0.y ),
				vec3f( pos.x - (cv-sv), pos.y + (sv+cv), zval) );
		doVert( vec2f( st0.x, st1.y  ),
				vec3f( pos.x - (cv+sv), pos.y + (sv-cv), zval) );
		doVert( vec2f( st1.x, st1.y ),
				vec3f( pos.x - (-cv+sv), pos.y + (-sv-cv), zval) );
		doVert( vec2f( st1.x, st0.y  ),
				vec3f( pos.x - (-cv-sv), pos.y + (-sv+cv), zval) );	
		doVert( vec2f( st0.x, st0.y ),
				vec3f( pos.x - (cv-sv), pos.y + (sv+cv), zval) );
		glEnd();
	}
#else

	glColor3f( 1.0, 1.0, 1.0 );	
	glBegin( GL_QUADS );
	
	vec2f p =  RotateZ( vec2f( -m_size.x, -m_size.y), (float)D2R*angle );
	doVert( vec2f( st0.x, st0.y ), vec3f( pos.x+p.x, pos.y+p.y, zval) );

	p = RotateZ( vec2f( -m_size.x, -m_size.y), (float)D2R*angle );
	doVert( vec2f( st0.x, st1.y ), vec3f( pos.x+p.x, pos.y+p.y, zval) );
	
	p = RotateZ( vec2f( -m_size.x, -m_size.y), (float)D2R*angle );
	doVert( vec2f( st1.x, st1.y ), vec3f( pos.x+p.x, pos.y+p.y, zval) );
	
	p = RotateZ( vec2f( -m_size.x, -m_size.y), (float)D2R*angle );
	doVert( vec2f( st1.x, st0.y ), vec3f( pos.x+p.x, pos.y+p.y, zval) );

	glEnd();
#endif

}

void Shape::doVert( const vec2f &st, const vec3f &pos )
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
