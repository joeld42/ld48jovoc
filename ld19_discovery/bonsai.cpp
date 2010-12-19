#include <luddite/GLee.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <luddite/debug.h>
#include <luddite/resource.h>
#include <luddite/texture.h>

#include "bonsai.h"
#include "noise.h"

#include "png.h"
#include "zlib.h"

#define TESS_AMOUNT (200)

#define HITE_RES (1024)

#define CACHE_DIR "terrainCache"

bool loadShader( const char *shaderKey, GLuint &program );

TreeLand::TreeLand( float *heightData, 
				   Luddite::HTexture htexColor ) : 
	m_hiteData( heightData ),
	m_htexTerrainColor( htexColor )
{
}

void TreeLand::build()
{
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
	const float stSize = 1.0/(TESS_AMOUNT+1);	
	const float tileSize = 2.0/(TESS_AMOUNT+1);

	float s0 = i * stSize;
	float t0 = j * stSize;
	float s1 = s0 + stSize;
	float t1 = t0 - stSize;

	
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

	for (int i=0; i < 6; i++)
	{
		newVert[i].pos[0] *= 500.0;
		newVert[i].pos[1] *= 500.0;
		newVert[i].pos[2] *= 100.0;
	}
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


Bonsai::Bonsai( const char *name ) :
	m_treeLand( NULL )
{
	m_name = strdup( name );
}

Bonsai::~Bonsai()
{
	delete [] m_hiteData;

	delete [] m_terrainColor;
	delete [] m_terrainNorm;

	free( (void *)m_name );
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
	m_terrainColor = new GLubyte [ HITE_RES * HITE_RES * 3 ];
	m_terrainNorm = new GLubyte [ HITE_RES * HITE_RES * 3 ];

	// HERE -- set up params
	
	// check if there is cached image data available		
	if ((_checkCachedColorImage( "DIF0", m_terrainColor, HITE_RES)) &&
		(_checkCachedHeight( "HITE", m_hiteData, HITE_RES ) ))
	{
		DBG::info( "Land '%s' read cached color and height.\n", m_name );
	}
	else
	{
		// Synthesize

		// step for tuning/debugging quicker
		// step=1 for final quality
		int step = 1;
		for (int j=0; j < HITE_RES; j += step)
		{
			for (int i=0; i < HITE_RES; i += step )
			{
				float ii = (((float)i / (float)HITE_RES) * 2.0) - 1.0;
				float jj = (((float)j / (float)HITE_RES) * 2.0) - 1.0;

				// synthesize a texel
				size_t ndx = (j*HITE_RES)+i;
				synthesize( ndx, ii, jj );

				for (int sj = 0; sj < step; sj++)
				{
					for(int si=0; si < step; si++)
					{			
						if (si || sj)
						{
							size_t ndx2 = ((j+sj)*HITE_RES)+(i+si);

							m_terrainColor[ndx2 * 3 + 0] = m_terrainColor[ndx * 3 + 0];
							m_terrainColor[ndx2 * 3 + 1] = m_terrainColor[ndx * 3 + 1];
							m_terrainColor[ndx2 * 3 + 2] = m_terrainColor[ndx * 3 + 2];

							m_hiteData[ndx2] = m_hiteData[ndx];

						}
					}
				}
			}
		}

		// Cache the height data
		_cacheColorImage( "DIF0", m_terrainColor, HITE_RES );
		_cacheHeight( "HITE", m_hiteData, HITE_RES );
	}

	// check if there is cached normal data available
	// TODO
	if (false)
	{
	}
	else
	{
		// Shitty calc normals -- ideally this should use the
		// height samples to not soften it but for now do it this
		// way cause it's easier/faster
		for (int j=0; j < HITE_RES-1; j++)
		{
			for (int i=0; i < HITE_RES-1; i++ )
			{
				PVRTVec3 nrm;
				size_t ndx = (j*HITE_RES)+i;
				float a = m_hiteData[ndx];
				float b = m_hiteData[ (j*HITE_RES)+(i+1) ];
				float c = m_hiteData[ ((j+1)*HITE_RES)+i ];

				float scl = 100.0;
				float nx = (b - a) * scl;
				float ny = (c - a) * scl;
				float nz = 1.0 - sqrt( nx*nx + ny*ny );

				m_terrainNorm[ndx * 3 + 0] = (int)((nx * 128.0) + 128.0);
				m_terrainNorm[ndx * 3 + 1] = (int)((ny * 128.0) + 128.0);
				m_terrainNorm[ndx * 3 + 2] = (int)((nz * 128.0) + 128.0);
			}
		}

		// Cache the normal map
		_cacheColorImage( "NRM", m_terrainNorm, HITE_RES );
	}

	// Build texture
	static int treeId = 0;
	char s[20];
	sprintf( s, "treeland%d", treeId++);

	Luddite::TextureDB *texDB = Luddite::TextureDB::singletonPtr();
	Luddite::HTexture htex = texDB->buildTextureFromData( s, m_terrainColor, 1024, 1024 );

	Luddite::HTexture htexNrm = texDB->buildTextureFromData( s, m_terrainNorm, 1024, 1024 );

	// Make a land part
	m_treeLand = new TreeLand( m_hiteData, htexNrm );
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
	//float rval = (1.0 - (sqrt( ii*ii + jj*jj ) / 2.0));
	float rval = ((sqrt( ii*ii + jj*jj ) / 2.0));

	// domain distortion
	float distDir = pnoise( ii * 2, 10.0, jj * 2 );
	float ii2 = ii + distDir * 0.4;
	float jj2 = jj + distDir * 0.4;

	float hval = pnoise( ii2 * 2, 0.0, jj2 * 2 );

	// color with turbulence
	float val = pturb( ii * 4, 0.0, jj * 4, 8, false );
	
	// set color
	float v2 = clamp(val);
	m_terrainColor[ndx * 3 + 0] = 0.0;
	m_terrainColor[ndx * 3 + 1] = fabs(v2) * 255.0;
	m_terrainColor[ndx * 3 + 2] = (1.0 - fabs(v2)) * 255.0;

	// todo: set normal
	
	// set height	
	m_hiteData[ndx] = rval + (hval * 0.3) + (val*0.05);
		
}

void Bonsai::_cacheColorImage( const char *suffix, GLubyte *data, int sz )
{
	char filename[256];
	sprintf( filename, "./%s/%s_%s.png", CACHE_DIR, m_name, suffix );

	FILE *fp = fopen( filename, "wb" );
	if (!fp)
	{
		DBG::warn( "Error writing cache file %s\n", filename );
		return;
	}

	png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING,
					NULL, NULL, NULL );

	if (!png_ptr)
	{
		DBG::warn( "error creating png write struct\n" );
	}

	png_infop info_ptr = png_create_info_struct( png_ptr );
	if (!info_ptr)
	{
		png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
		DBG::warn( "error creating png info struct\n" );
	}

	// init IO and compression
	png_init_io( png_ptr, fp );
	png_set_compression_level( png_ptr, Z_BEST_COMPRESSION );	

	// set content header
	png_set_IHDR( png_ptr, info_ptr, sz, sz,
					8, PNG_COLOR_TYPE_RGB,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT );

	// write header info
	png_write_info(png_ptr, info_ptr);	
	
	// write the thing
	// FIXME -- should use sz
	png_byte *row_pointers[1024];
	for (int i=0; i < sz; i++)
	{
		row_pointers[i] = &(data[i * sz * 3]);
	}
	png_write_image( png_ptr, row_pointers );

	png_write_end( png_ptr, info_ptr );
	png_destroy_write_struct( &png_ptr, &info_ptr );

	fclose( fp );

	DBG::info( "Wrote cache image %s\n", filename );
}

bool Bonsai::_checkCachedColorImage( const char *suffix, GLubyte *data, int sz )
{
	char filename[256];
	sprintf( filename, "./%s/%s_%s.png", CACHE_DIR, m_name, suffix );

	FILE *fp = fopen( filename, "rb" );
	if (!fp)
	{
		// no cached image exists
		return false;
	}

	// cached image exists, load it
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
   if (!png_ptr)
   {
      fclose(fp);
	  DBG::warn("error png_create_read_struct on cached image %s\n", filename );		
      return false;
   }
   
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
   {
      fclose(fp);
	  DBG::warn("error png_create_info_struct on cached image %s\n", filename );		
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return (ERROR);
   }

   png_init_io(png_ptr, fp);

   // read the thing
   //png_read_png(png_ptr, info_ptr, png_transforms, png_voidp_NULL);
	png_byte *row_pointers[1024];
	for (int i=0; i < sz; i++)
	{
		row_pointers[i] = &(data[i * sz * 3]);
	}
	png_read_image( png_ptr, row_pointers );

	//png_read_end(png_ptr, info_ptr);	

   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

   return true;
}

// FIXME: use a readable image format (like float tiff) so we
// can use these elsewhere
void Bonsai::_cacheHeight( const char *suffix, float *data, int sz )
{
	char filename[256];
	sprintf( filename, "./%s/%s_%s.dat.gz", CACHE_DIR, m_name, suffix );

	gzFile fp = gzopen( filename, "wb" );
	if (!fp)
	{
		DBG::warn( "Error writing cache file %s\n", filename );
		return;
	}

	// write as raw floats
	//fwrite( data, sizeof(float)*sz, sz, fp );
	gzwrite( fp, data, sizeof(float)*sz*sz );
	gzclose( fp );

	DBG::info( "Wrote cache height data %s\n", filename );
}

bool Bonsai::_checkCachedHeight( const char *suffix, float *data, int sz )
{
	char filename[256];
	sprintf( filename, "./%s/%s_%s.dat.gz", CACHE_DIR, m_name, suffix );

	// just open it to see if it exists
	FILE *fp1 = fopen( filename, "rb" );
	if (!fp1)
	{
		// No cached height data exists
		return false;
	}
	fclose( fp1 );

	// exists, reopen it with zlib
	gzFile fp = gzopen( filename, "rb" );
	if (!fp)
	{		
		return false;
	}
	
	gzread( fp, data, sizeof(float)*sz*sz );
	gzclose( fp );

	return true;
}