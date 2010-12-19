#include <luddite/GLee.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <luddite/debug.h>
#include <luddite/resource.h>
#include <luddite/texture.h>

#include "bonsai.h"
#include "noise.h"

#define TESS_AMOUNT (100)

#define HITE_RES (1024)

bool loadShader( const char *shaderKey, GLuint &program );

TreeLand::TreeLand( float *heightData ) : m_hiteData( heightData )
{
}

void TreeLand::build()
{
	// TODO: generate height field
	Luddite::TextureDB *texDB = Luddite::TextureDB::singletonPtr();
	m_htexTerrainColor = texDB->getTexture("gamedata/ash_uvgrid03-1.png" );	

	// land surface
	for (int j=0; j < TESS_AMOUNT; j++)
	{
		for (int i=0; i < TESS_AMOUNT; i++)
		{
			// make a quad
			_makeQuad( i, j );
		}
	}
}

float TreeLand::hite( float x, float y )
{
	// TMP
	//return sin( sqrt( x*x + y*y ) * 20 ) * 0.15;

	int ii = (int)( ((x + 1.0) / 2.0) * HITE_RES );
	int jj = (int)( ((y + 1.0) / 2.0) * HITE_RES );

	return m_hiteData[ jj* HITE_RES + ii];
}

void TreeLand::_makeQuad( int i, int j )
{
	// Get next vert
	TreeVert *newVert;
	newVert = this->addQuad();

	// TODO: un-hardcode this
	const float stSize = 1.0/TESS_AMOUNT;	
	const float tileSize = 2.0/TESS_AMOUNT;

	float s0 = i * stSize;
	float t0 = 1.0 - (j * stSize);
	float s1 = s0 + stSize;
	float t1 = t0 + stSize;

	
	// Upper triangle
	newVert[0].st[0] = s0;  newVert[0].st[1] = t0;	
	newVert[0].pos[0] = (i*tileSize) - 1.0; 
	newVert[0].pos[1] = (j+1)*tileSize  - 1.0;
	newVert[0].pos[2] = hite( newVert[0].pos[0], newVert[0].pos[1] );
		
	newVert[1].st[0] = s0; newVert[1].st[1] = t1;	
	newVert[1].pos[0] = i*tileSize  - 1.0;   newVert[1].pos[1] = j*tileSize  - 1.0;
	newVert[1].pos[2] = hite( newVert[1].pos[0], newVert[1].pos[1] );
	
	newVert[2].st[0] = s1;     newVert[2].st[1] = t0;
	newVert[2].pos[0] = (i+1)*tileSize  - 1.0; newVert[2].pos[1] = (j+1)*tileSize  - 1.0;
	newVert[2].pos[2] = hite( newVert[2].pos[0], newVert[2].pos[1] );
	
	// Lower triangle
	newVert[3].st[0] = s1;     newVert[3].st[1] = t0;	
	newVert[3].pos[0] = (i+1)*tileSize  - 1.0; newVert[3].pos[1] = (j+1)*tileSize  - 1.0;
	newVert[3].pos[2] = hite( newVert[3].pos[0], newVert[3].pos[1] );
	
	newVert[4].st[0] = s0; newVert[4].st[1] = t1;	
	newVert[4].pos[0] = i*tileSize  - 1.0;   newVert[4].pos[1] = j*tileSize  - 1.0;
	newVert[4].pos[2] = hite( newVert[4].pos[0], newVert[4].pos[1] );
	
	newVert[5].st[0] = s1; newVert[5].st[1] = t1;	
	newVert[5].pos[0] = (i+1)*tileSize  - 1.0;   newVert[5].pos[1] = j*tileSize  - 1.0;
	newVert[5].pos[2] = hite( newVert[5].pos[0], newVert[5].pos[1] );
}

void TreeLand::renderAll()
{
	//glBindTexture( GL_TEXTURE_2D, m_texId );		
	Luddite::TextureDB &texDB = Luddite::TextureDB::singleton();
	GLuint texId = texDB.getTextureId( m_htexTerrainColor );

	glEnable( GL_TEXTURE );
	glBindTexture( GL_TEXTURE_2D, texId );		

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo() );
	updateBuffer();
	
	//glEnableClientState( GL_VERTEX_ARRAY );
	//glVertexPointer( 2, GL_FLOAT, sizeof(TreeVert), 0 );
	
	//glEnableClientState( GL_TEXTURE_COORD_ARRAY );	
	//glTexCoordPointer( 2, GL_FLOAT, sizeof(TreeVert), (void*)(2*sizeof(GLfloat)) );
	
	//glEnableClientState( GL_COLOR_ARRAY );
	//glColorPointer( 4, GL_FLOAT, sizeof(TilemapVert), (void*)(4*sizeof(GLfloat)) );

	glEnableVertexAttribArray( Attrib_POSITION );    
	glVertexAttribPointer( Attrib_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(TreeVert), 0 );

    glEnableVertexAttribArray( Attrib_TEXCOORD );  
	glVertexAttribPointer( Attrib_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(TreeVert), 
						  (void*)(3*sizeof(GLfloat)) );
	
	glDrawArrays( GL_TRIANGLES, 0, this->size() );
	
	glDisableVertexAttribArray( Attrib_POSITION );
    //glDisableVertexAttribArray( Attrib_NORMAL );
    glDisableVertexAttribArray( Attrib_TEXCOORD );

	//glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	//glDisableClientState( GL_COLOR_ARRAY );
	//glDisableClientState( GL_VERTEX_ARRAY );
}


Bonsai::Bonsai() :
	m_treeLand( NULL )
{
}

Bonsai::~Bonsai()
{
	delete [] m_hiteData;
}

void Bonsai::init()
{
	if (loadShader("shaders.Treeland", m_progTreeland ))
	{
		glUseProgram( m_progTreeland );
	
		m_paramTreeland_PMV = glGetUniformLocation( m_progTreeland, "matrixPMV" );
		m_paramTreeland_samplerDif0 = glGetUniformLocation( m_progTreeland, "sampler_dif0" );
	}
}

void Bonsai::buildAll()
{
	// synthesize the height data
	m_hiteData = new float [ HITE_RES * HITE_RES ];

	// HERE -- set up params
	
	// Synthesize
	for (int j=0; j < HITE_RES; j++)
	{
		for (int i=0; i < HITE_RES; i++)
		{
			float ii = (((float)i / (float)HITE_RES) * 2.0) - 1.0;
			float jj = (((float)j / (float)HITE_RES) * 2.0) - 1.0;
			size_t ndx = (j*HITE_RES)+i;

			synthesize( ndx, ii, jj );
		}
	}

	// Make a land part
	m_treeLand = new TreeLand( m_hiteData );
	m_treeLand->build();
}

void Bonsai::renderAll()
{
	glUseProgram( m_progTreeland );
	
	glUniform1i( m_paramTreeland_samplerDif0, 0 );

	m_treeLand->renderAll();

	glUseProgram( 0 );
}
	
void Bonsai::setCamera( PVRTMat4 &camMVP )
{
	glUseProgram( m_progTreeland );
	glUniformMatrix4fv( m_paramTreeland_PMV, 1, GL_FALSE, camMVP.f );	
}

void Bonsai::synthesize( size_t ndx, float ii, float jj )
{	 
	float rval = (1.0 - (sqrt( ii*ii + jj*jj ) / 2.0)) * 0.3;

	//m_hiteData[ndx] = sin( sqrt( ii*ii + jj*jj ) * 20 ) * 0.15;
	m_hiteData[ndx] = pnoise( ii * 4, 0.0, jj * 4 ) * 0.15 + pow( (float)rval, 3.0f) ;
}