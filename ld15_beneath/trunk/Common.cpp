#include <map>

#include <Common.h>
#include <Server.h>

#include <tweakval.h>
#include <gamefontgl.h>
#include <jimgui.h>

#include <TinyXml.h>

// don't call this every frame or nothin'..
typedef std::map<std::string,GLuint> TextureDB;
TextureDB g_textureDB;
GLuint getTexture( const std::string &filename, int *xsize, int *ysize )
{
	GLuint texId;

	// See if the texture is already loaded
	TextureDB::iterator ti;
	ti = g_textureDB.find( filename );
	if (ti == g_textureDB.end() )
	{
		// Load the font image
		ILuint ilImgId;
		ilGenImages( 1, &ilImgId );
		ilBindImage( ilImgId );		
	
		if (!ilLoadImage( (ILstring)filename.c_str() )) {
			printf("Loading font image failed\n");
		}
//		printf("Loaded Texture %s\n", filename.c_str() );
	
		// Make a GL texture for it
		texId = ilutGLBindTexImage();		

		// and remember it
		g_textureDB[ filename ] = texId;
	}
	else
	{
		// found the texture
		texId = (*ti).second;
	}

	// now get the size if they asked for it
	if ((xsize) && (ysize))
	{
		glBindTexture( GL_TEXTURE_2D, texId );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, xsize );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, ysize );
	}

	return texId;
}

void pseudoOrtho2D( double left, double right, double bottom, double top )
{
#if 0
	// for now, real ortho
	gluOrtho2D( left, right, bottom, top );
#else	
	float w2, h2;
	w2 = (right - left) /2;
	h2 = (top - bottom) /2;
	float nv = _TV( 1.0f );
	glFrustum( -w2, w2, -h2, h2, nv, _TV(5.0f) );
	gluLookAt( left + w2, bottom + h2, nv,
			   left + w2, bottom + h2, 0.0,
			   0.0, 1.0, 0.0 );
#endif
}

float distPointLine( const vec2f &a, const vec2f &b, const vec2f &c, vec2f &p )
{
	vec2f ab = b-a;
	float lmag2 = LengthSquared(ab);
	float r = ((c.x - a.x)*(b.x-a.x) + (c.y-a.y)*(b.y-a.y)) / lmag2;

	if (r < 0.0f )
	{
		p = a;
		return Length(a-c);
	}
	else if (r > 1.0f)
	{
		p = b;
		return Length(b-c);
	}
	else
	{
		p=a + r * ab;
		return Length( c-p );
	}
}