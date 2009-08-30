#include <tinyxml.h>
#include <Common.h>
#include <Cavern.h>

const char *BlendModeNames[] =
{
	"off", "normal", "add"
};

Cavern::Cavern():
	m_mapSize( 2400, 4800 ),
	m_bgColor( 0.2, 0.2, 0.2 ),
	m_spawnPoint( 0.0f, 0.0f )
{	
}

bool cmp_Shapes( Shape *a, Shape *b)
{
	

	// first sort by sortnum
	if (a->sortNum != b->sortNum)
	{
		return a->sortNum > b->sortNum;
	}
	else if (a->blendMode != b->blendMode)
	{
		return a->blendMode < b->blendMode;
	}
	// then by texture
	else 
	{
		return a->m_texId < b->m_texId;
	}
	
}

void Cavern::addShape( Shape *s )
{
	m_shapes.push_back( s );
	sortShapes();
}

void Cavern::sortShapes()
{
	// here ... sort shapes
	std::sort( m_shapes.begin(), m_shapes.end(), cmp_Shapes );
}

void Cavern::draw()
{
	GLuint curTexId;
	GLuint curBlend;

	//printf("--- draw\n" );
	
	glEnable( GL_TEXTURE_2D );
	
	for (int i=0; i < m_shapes.size(); ++i)
	{
		Shape *s = m_shapes[i];

		//printf("draw shape %s\n", s->name.c_str() );
		
		// Change blend modes?
		if ((i==0) || (s->blendMode != curBlend))
		{
			if (s->blendMode == Blend_OFF)
			{
				//printf("BLEND OFF\n" );
				glDisable( GL_BLEND );
				glEnable( GL_ALPHA_TEST );
				glAlphaFunc( GL_GREATER, 0.5 );
			}
			else
			{
				//printf("BLEND ON\n" );
				glEnable( GL_BLEND );
				glDisable( GL_ALPHA_TEST );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
			}		

			curBlend = s->blendMode;
		}

		// Change textures?
		if ((i==0)||(s->m_texId != curTexId))
		{
			//printf("texture %s\n", s->mapname.c_str() );
			glBindTexture( GL_TEXTURE_2D, s->m_texId );
			curTexId = s->m_texId;
		}

		// FIXME: use DrawElements or something
		s->drawBraindeadQuad();

	}	

	//printf("Done\n" );
}

void Cavern::saveLevel( const char *levelFile )
{
	TiXmlDocument *xmlDoc = new TiXmlDocument();
	//TiXmlDeclarion *decl = new TiXmlDeclarion( "1.0", "", "" );

	TiXmlElement *xCavern = new TiXmlElement( "Cavern" );

	char bgcolorstr[1024];
	sprintf( bgcolorstr, "%f,%f,%f", 
			 m_bgColor.r, m_bgColor.g, m_bgColor.b );

	xCavern->SetAttribute( "bgcolor", bgcolorstr );
	xCavern->SetAttribute( "width", m_mapSize.x );
	xCavern->SetAttribute( "height", m_mapSize.y );

	char buff[1024];

	sprintf(buff, "%f,%f", m_spawnPoint.x,  m_spawnPoint.y ); 
	xCavern->SetAttribute( "spawnPoint", buff );

	for (int i=0; i < m_shapes.size(); i++)
	{
		Shape *shp = m_shapes[i];
		
		TiXmlElement *xShape = new TiXmlElement( "Shape" );
		xShape->SetAttribute( "name", shp->name.c_str() );
		xShape->SetAttribute( "mapname", shp->mapname.c_str() );
		xShape->SetAttribute( "collide", shp->m_collide?"True":"False" );
		xShape->SetAttribute( "pattern", shp->m_pattern?"True":"False" );
		xShape->SetAttribute( "blendMode", BlendModeNames[ shp->blendMode ] );
		xShape->SetAttribute( "angle", shp->angle );
		xShape->SetAttribute( "sortNum", shp->sortNum );
		
		//sprintf( buff, "%f,%f,%f,%f", shp->st0.x, shp->st0.y, shp->st1.x, shp->st1.y );
		//xShape->SetAttribute( "sts", buff );

		sprintf( buff, "%f,%f", shp->pos.x, shp->pos.y );
		xShape->SetAttribute( "pos", buff );

		sprintf( buff, "%f,%f", shp->m_size.x, shp->m_size.y );
		xShape->SetAttribute( "size", buff );

		//sprintf( buff, "%f,%f", shp->m_origSize.x, shp->m_origSize.y );
		//xShape->SetAttribute( "origSize", buff );


		xCavern->LinkEndChild( xShape );
	}

	for (int i=0; i < m_collision.size(); i++)
	{
		Segment &s = m_collision[i];
		
		TiXmlElement *xSegment = new TiXmlElement( "Segment" );
		sprintf(buff, "%f,%f", s.a.x, s.a.y );
		xSegment->SetAttribute( "start", buff );
		sprintf(buff, "%f,%f", s.b.x, s.b.y );
		xSegment->SetAttribute( "end", buff );

		xCavern->LinkEndChild( xSegment );
	}

	//xmlDoc->LinkEndChild( decl );
	xmlDoc->LinkEndChild( xCavern );
	xmlDoc->SaveFile( levelFile );

	printf("Level %s saved...\n", levelFile );
}

// slightly different format than the 
void Cavern::loadLevel( const char *levelFile, std::vector<Shape*> &shapeList )
{
	TiXmlDocument *xmlDoc = new TiXmlDocument( levelFile );

	if (!xmlDoc->LoadFile() ) {
		printf("ERR! Can't load %s\n", levelFile );
	}

	TiXmlElement *xCavern, *xShape;
	//TiXmlNode *xText;

	xCavern = xmlDoc->FirstChildElement( "Cavern" );
	assert( xCavern );

	m_mapSize = vec2f( atof( xCavern->Attribute("width") ),
					   atof( xCavern->Attribute("height") ) );

	sscanf( xCavern->Attribute("spawnPoint"), "%f,%f",
				&(m_spawnPoint.x), &(m_spawnPoint.y) );

	xShape = xCavern->FirstChildElement( "Shape" );
	while (xShape) 
	{
		Shape *shp = new Shape();
		
		shp->name = xShape->Attribute("name");
		shp->mapname = xShape->Attribute("mapname");
		shp->m_collide = (!stricmp( xShape->Attribute("collide"), "true" ));
		shp->m_pattern = (!stricmp( xShape->Attribute("pattern"), "true" ));

		const char *blend = xShape->Attribute("blendMode");
		for (int i=0; i< 3; i++)
		{
			if (!stricmp( blend, BlendModeNames[i]) ) shp->blendMode = i;
		}
		
		shp->angle = atof( xShape->Attribute("angle") );
		shp->sortNum = atoi( xShape->Attribute("sortNum") );
		
		//sscanf( xShape->Attribute("sts"), "%f,%f,%f,%f",
		//		&(shp->st0.x), &(shp->st0.y),
		//		&(shp->st1.x), &(shp->st1.y) );

		sscanf( xShape->Attribute("pos"), "%f,%f",
				&(shp->pos.x), &(shp->pos.y) );

		sscanf( xShape->Attribute("size"), "%f,%f",
				&(shp->m_size.x), &(shp->m_size.y) );

		//sscanf( xShape->Attribute("origSize"), "%f,%f",
		//		&(shp->m_origSize.x), &(shp->m_origSize.y) );

		// Get sts and origSize from the shape List in case these
		// changed
		bool found = false;
		for (int i=0; i < shapeList.size(); ++i)
		{
			if (shp->name == shapeList[i]->name)
			{
				shp->m_origSize = shapeList[i]->m_origSize;
				shp->st0 = shapeList[i]->st0;
				shp->st1 = shapeList[i]->st1;
				found = true;
				break;
			}
		}
		if (!found)
		{
			printf("WARNING: Couldn't find shape %s\n", shp->name.c_str() );
			delete shp;
		}
		else
		{
			// Load the map
			shp->m_texId = getTexture( shp->mapname, NULL, NULL );
			m_shapes.push_back( shp );
		}
		xShape = xShape->NextSiblingElement( "Shape" );
	}

	sortShapes();

	// done
	xmlDoc->Clear();
	delete xmlDoc;
}

void Cavern::addSegment( vec2f a, vec2f b )
{
	Segment s;
	s.a = a;
	s.b = b;
	m_collision.push_back( s );
}