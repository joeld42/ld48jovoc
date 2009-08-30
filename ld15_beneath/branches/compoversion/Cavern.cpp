#include <Cavern.h>

Cavern::Cavern():
	m_mapSize( 2400, 4800 ),
	m_bgColor( 0.2, 0.2, 0.2 )
{
	
}

void Cavern::loadLevel( const char *levelFile )
{
	// TODO
}

bool cmp_Shapes( Shape *a, Shape *b)
{
	// first sort by blend Mode
	if (a->blendMode != b->blendMode)
	{
		return a->blendMode > b->blendMode;
	}
	// then by texture
	else if (a->m_texId != b->m_texId)
	{
		return a->m_texId > b->m_texId;
	}
	else
	{
		return a->sortNum < b->sortNum;
	}
}

void Cavern::addShape( Shape *s )
{
	m_shapes.push_back( s );

	// here ... sort shapes
	std::sort( m_shapes.begin(), m_shapes.end(), cmp_Shapes );
}

void Cavern::draw()
{
	GLuint curTexId;
	GLuint curBlend;
	
	glEnable( GL_TEXTURE_2D );
	
	for (int i=0; i < m_shapes.size(); ++i)
	{
		Shape *s = m_shapes[i];
		
		// Change blend modes?
		if ((i==0) || (s->blendMode != curBlend))
		{
			if (s->blendMode == Blend_OFF)
			{
				glDisable( GL_BLEND );
				glEnable( GL_ALPHA_TEST );
				glAlphaFunc( GL_GREATER, 0.5 );
			}
			else
			{
				glEnable( GL_BLEND );
				glDisable( GL_ALPHA_TEST );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
			}		

			curBlend = s->blendMode;
		}

		// Change textures?
		if ((i==0)||(s->m_texId != curTexId))
		{
			glBindTexture( GL_TEXTURE_2D, s->m_texId );
			curTexId = s->m_texId;
		}

		// FIXME: use DrawElements or something
		s->drawBraindeadQuad();

	}
	glEnd();
}