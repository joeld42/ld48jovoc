#include "focus.h"

#include "background.h"

void Background::redraw()
{	
}

void Background::update( float bpm )
{

}

void BGMondrian::redraw()
{
	//glPushMatrix();
	srand( 600 );

	//glBegin( GL_QUADS );
	doRedraw( 0, 800, 0, 600, 0, 0 );
	//glEnd();

	//glPopMatrix();
}

void BGMondrian::doRedraw( float left, float right, float bottom, float top, int axis, int level )
{
	if (level < 4)
	{
		float amt = frand( 0.0f, 1.0f );

		// low level, split
		float l_left, l_right, l_top, l_bottom;
		float r_left, r_right, r_top, r_bottom;

		if (axis < 2)
		{
			l_top = top;
			r_top = top;
			l_bottom = bottom;
			r_bottom = bottom;

			l_left = left;
			l_right = (float)lerp( left, right, amt ); 
			r_left = l_right;
			r_right = right;
		}
		else
		{
			l_left = left;
			r_left = left;
			l_right = right;
			r_right = right;

			l_top = top;
			l_bottom = (float)lerp( top, bottom, amt );
			r_top = l_bottom;
			r_bottom = bottom;
		}

		doRedraw( l_left, l_right, l_bottom, l_top, (axis+1) % 3, level+1 );
		doRedraw( r_left, r_right, r_bottom, r_top, (axis+1) % 3, level+1 );
	}
	else
	{
		// higher level
		int colorNdx = irand( 0, 4);
		if (colorNdx == 0)
		{
			glColor3f( pally.base.r, pally.base.g, pally.base.b );
		}
		else if (colorNdx==1)
		{
			glColor3f( pally.design1.r, pally.design1.g, pally.design1.b );
		}
		else if (colorNdx==2)
		{
			glColor3f( pally.design2.r, pally.design2.g, pally.design2.b );
		}
		else
		{
			glColor3f( pally.accent.r, pally.accent.g, pally.accent.b );
		}

		// draw the quad
		printf("%f %f %f %f\n", left, right, bottom, top );

		glBegin( GL_POLYGON );
		glEnd();

		glBegin( GL_POLYGON );
		glVertex3f( left, bottom, 0 );
		glVertex3f( right, bottom, 0 );
		glVertex3f( right, top, 0 );
		glVertex3f( left, top, 0 );

		//glVertex3f( 0, 0, 0 );
		//glVertex3f( 800, 0, 0 );
		//glVertex3f( 800, 600, 0 );
		//glVertex3f( 0, 600, 0 );
		glEnd();

		//glBegin( GL_POLYGON );
		//glVertex3f( 0, 0, 0 );
		//glVertex3f( 800, 0, 0 );
		//glVertex3f( 800, 600, 0 );
		//glVertex3f( 0, 600, 0 );
		//glEnd();
	}
}

void BGMondrian::update( float bpm )
{
}

BGMondrian::BGMondrian() :
	angle( 0.0f )
{
	// todo: get palette
}

#if 0

BGMondrianCell::BGMondrianCell( float _left, float _right, float _top, float _bottom ) :
	left( _left ), right(_right), top( _top ), bottom (_bottom ), leftChild( NULL ), rightChild(NULL)
{
}

void BGMondrianCell::split( int axis, float amt )
{
	BGMondrianCell *cellLeft, *cellRight;

	float l_left, l_right, l_top, l_bottom;
	float r_left, r_right, r_top, r_bottom;

	if (axis == 0)
	{
		l_top = top;
		r_top = top;
		l_bottom = bottom;
		r_bottom = bottom;

		l_left = left;
		l_right = (float)lerp( left, right, amt ); 
		r_left = l_right;
		r_right = right;
	}
	else
	{
		l_left = left;
		r_left = left;
		l_right = right;
		r_right = right;

		l_top = top;
		l_bottom = (float)lerp( top, bottom, amt );
		r_top = l_bottom;
		r_bottom = bottom;
	}

	cellLeft = new BGMondrianCell ( l_left, l_right, l_top, l_bottom );
	cellRight = new BGMondrianCell( r_left, r_right, r_top, r_bottom );
}
#endif