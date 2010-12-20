#include <luddite/GLee.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <luddite/debug.h>
#include <luddite/resource.h>
#include <luddite/texture.h>
#include <luddite/random.h>

#include <SDL.h>

#include "bonsai.h"
#include "noise.h"
#include "pally.h"

#include "png.h"
#include "zlib.h"

#define TESS_AMOUNT (200)

#define HITE_RES (1024)

#define CACHE_DIR "terrainCache"

// debug flags for terrain
#define NO_CACHE (0)   // turn off cache (for debugging)
#define SYNTH_STEP (1)  // speed up synth (fucks up normals tho)

bool loadShader( const char *shaderKey, GLuint &program );

float fstep( float a, float b, float t );

TreeLand::TreeLand( float *heightData, 
				   Luddite::HTexture htexColor,
				   Luddite::HTexture htexNorm ) : 
	m_hiteData( heightData ),
	m_htexTerrainColor( htexColor ),
	m_htexTerrainNorm( htexNorm )
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
		// fixme: make these unhard-coded
		newVert[i].pos[0] *= 500.0;
		newVert[i].pos[1] *= 500.0;
		newVert[i].pos[2] *= 100.0;
	}
}

void TreeLand::renderAll()
{
	//glBindTexture( GL_TEXTURE_2D, m_texId );		
	

	//glEnable( GL_TEXTURE );
	//glBindTexture( GL_TEXTURE_2D, texId );		
	

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

	glActiveTexture( GL_TEXTURE1 );
	glEnable( GL_TEXTURE_2D );

	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );	

	

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
		m_paramTreeland_samplerNrm0 = glGetUniformLocation( m_progTreeland, "sampler_nrm0" );

		m_paramTreeland_lightDir = glGetUniformLocation( m_progTreeland, "lightDir0" );
		m_paramTreeland_eyeDir = glGetUniformLocation( m_progTreeland, "eyeDir" );
	}
}

void progress( const char *task, int row, int totalrows )
{
	static char lastTask[100] = "";	
	static int lastPct = 0;

	// new section?
	if (strcmp( lastTask, task))
	{
		lastPct = -1;
		strcpy( lastTask, task );
	}

	// update display?	
	int pct = row * 100 / totalrows;
	if (pct != lastPct )
	{
		char buff[200];
		sprintf( buff, " [%s %d%%] LD19 Jovoc - Discovery", task, pct );
		SDL_WM_SetCaption( buff , NULL );

		//DBG::info("%s: %d\n", task, pct );
		
		lastPct = pct;
	}
}

void Bonsai::buildAll()
{
	// generate a unique random seed for the whole thing from the name
	DBG::info("Generate planet '%s'\n", m_name );
	int master_seed = 1;
	for (const char *ch=m_name; *ch; ch++)
	{
		master_seed *= (*ch);
	}

	// generate random seeds for each step .. this keeps them
	// somewhat resistant to changes in the algoritms 
	int pally_seed, param_seed, terrain_seed;
	srand( master_seed );
	pally_seed = rand();
	terrain_seed = rand();
	param_seed = rand();
		
	// Make the palette
	m_pally.generate( pally_seed );

	// synthesize the height data
	m_hiteData = new float [ HITE_RES * HITE_RES ];
	m_terrainColor = new GLubyte [ HITE_RES * HITE_RES * 3 ];
	m_terrainNorm = new GLubyte [ HITE_RES * HITE_RES * 3 ];

	// Set up params
	srand( param_seed );
	m_params.offs = randUniform( 0.0, 1000.0 );

	m_params.base = (int)randUniform( 0, NUM_BASE );	
	m_params.base_distAmt = randUniform( 0.0, 0.3 );
	m_params.base_scale = randUniform( 0.3, 1.0 );	

	m_params.baseVeg = (int)randUniform( 0, NUM_BASEVEG );	

	m_params.baseVeg_hite_repeats = (int)randUniform( 1, 5 );		
	m_params.baseVeg_hite_repeats *= m_params.baseVeg_hite_repeats;	

	m_params.patchVeg = (int)randUniform( 0, NUM_PATCHVEG );	
	m_params.patchVeg_scale = randUniform( 1.0, 20.0 );
	m_params.patchVeg_nscale = randUniform( 0.01, 8.0 );
	m_params.patchVeg_nscale *= m_params.patchVeg_nscale;
	m_params.patchMixMineral = (randUniform() > 0.5)?1:0;
	m_params.patchVeg_thresh = randUniform( 0.55, 0.85 );	

	// Decoration
	m_params.deco = (int)randUniform( 0, NUM_DECORATION );	
	m_params.decoLavaScale = randUniform( 1.0, 10.0 );
	m_params.decoLavaWidth = randUniform( 0.01, 0.1 );

	// =============================================

	// build textures
	Luddite::HTexture htexColor;
	Luddite::HTexture htexNorms;	
	

	// check if there is cached image data available		
	htexColor = _checkCachedColorImage( "DIF0", m_terrainColor, HITE_RES);
	if ( (!htexColor.isNull() ) && (_checkCachedHeight( "HITE", m_hiteData, HITE_RES ) ))
	{
		DBG::info( "Land '%s' read cached color and height.\n", m_name );
	}
	else
	{
		// Synthesize
		srand( terrain_seed );

		// step for tuning/debugging quicker
		// step=1 for final quality
		int step = SYNTH_STEP;
		for (int j=0; j < HITE_RES; j += step)
		{
			// status
			progress( "Generating", j, HITE_RES );			

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

		// Calculate (fake) ambient occlusion
		for (int j=0; j < HITE_RES; j += 1)
		{
			progress( "AmbOccl", j, HITE_RES );

			for (int i=0; i < HITE_RES; i += 1 )
			{
				float ambOccl = 0.0f;
				int c = 0;
				size_t ndx = (j*HITE_RES)+i;

				// TODO: gaussian weight
				for (int ii=-8; ii < 8; ii += 2)
				{
					for (int jj=-8; jj < 8; jj += 2)
					{
						int i2 = i+ii;
						int j2 = j+jj;
						c += 1;
						
						float occl = 0.0;
						if ((i2 >=0) && (i2 < HITE_RES)  &&
							(j2 >=0) && (j2 < HITE_RES) )
						{
							size_t ndx2 = (j2*HITE_RES)+i2;
							occl = m_hiteData[ndx2] - m_hiteData[ndx];							
							occl = fstep( 0.0, 0.015, occl );														
							
							ambOccl += occl;
						}						
					}
				}

				// avg
				ambOccl /= c;
				ambOccl = clamp( 1.0-ambOccl, 0.0, 1.0 );

				// darken color image (dbg replace)
#if 0
				m_terrainColor[ndx * 3 + 0] = (int)(ambOccl * 255);
				m_terrainColor[ndx * 3 + 1] = (int)(ambOccl * 255);
				m_terrainColor[ndx * 3 + 2] = (int)(ambOccl * 255);
#else
				m_terrainColor[ndx * 3 + 0] *= ambOccl;
				m_terrainColor[ndx * 3 + 1] *= ambOccl;
				m_terrainColor[ndx * 3 + 2] *= ambOccl;
#endif

			}
		}
				

		// Cache the height & color data
		SDL_WM_SetCaption( "[Cache Height/Color] LD19 Jovoc - Discovery", NULL );

		_cacheColorImage( "DIF0", m_terrainColor, HITE_RES );
		_cacheHeight( "HITE", m_hiteData, HITE_RES );
	}

	// check if there is cached normal data available
	htexNorms = _checkCachedColorImage( "NRM", m_terrainNorm, HITE_RES);
	if ( !htexNorms.isNull() )
	{
		DBG::info( "Land '%s' read cached normals.\n", m_name );
	}
	else
	{
		// Shitty calc normals -- ideally this should use the
		// height samples to not soften it but for now do it this
		// way cause it's easier/faster
		for (int j=0; j < HITE_RES-1; j++)
		{
			progress( "Calc Normals", j, HITE_RES );

			for (int i=0; i < HITE_RES-1; i++ )
			{
				PVRTVec3 nrm;
				size_t ndx = (j*HITE_RES)+i;
				float a = m_hiteData[ndx];
				float b = m_hiteData[ (j*HITE_RES)+(i+1) ];
				float c = m_hiteData[ ((j+1)*HITE_RES)+i ];

				//float scl = 5.4;
				float scl = 50.0f;
				float dy1 = (b - a) * scl;
				float dy2 = (c - a) * scl;				

				PVRTVec3 n( dy1, dy2, 1.0 - sqrt( dy1*dy1 + dy2*dy2 ) );
				n.normalize();

				m_terrainNorm[ndx * 3 + 0] = (int)((n.x * 128.0) + 128.0);
				m_terrainNorm[ndx * 3 + 1] = (int)((n.y * 128.0) + 128.0);
				m_terrainNorm[ndx * 3 + 2] = (int)((n.z * 128.0) + 128.0);
			}
		}

		// Cache the normal map
		SDL_WM_SetCaption( "[Cache Normals] LD19 Jovoc - Discovery", NULL );
		_cacheColorImage( "NRM", m_terrainNorm, HITE_RES );
	}

	SDL_WM_SetCaption( "LD19 Jovoc - Discovery [Press TAB to ungrab mouse]", NULL );

	// Build texture
	static int treeId = 0;
	
	Luddite::TextureDB *texDB = Luddite::TextureDB::singletonPtr();
	if (htexColor.isNull())
	{
		char s[20];
		sprintf( s, "treeland%d", treeId++);
		htexColor = texDB->buildTextureFromData( s, m_terrainColor, 1024, 1024 );
	}

	if (htexNorms.isNull())
	{
		char s[20];
		sprintf( s, "treenorms%d", treeId++);
		htexNorms = texDB->buildTextureFromData( s, m_terrainNorm, 1024, 1024 );
	}

	// Make a land part
	m_treeLand = new TreeLand( m_hiteData, htexColor, htexNorms );
	m_treeLand->build();
}

void Bonsai::renderAll()
{
	Luddite::TextureDB &texDB = Luddite::TextureDB::singleton();
	GLuint texId = texDB.getTextureId( m_treeLand->m_htexTerrainColor );
	GLuint texIdNorm = texDB.getTextureId( m_treeLand->m_htexTerrainNorm );
			
	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texId );

	glActiveTexture( GL_TEXTURE1 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texIdNorm );

	glUseProgram( m_progTreeland );
	glUniform1i( m_paramTreeland_samplerDif0, 0 );
	glUniform1i( m_paramTreeland_samplerNrm0, 1 );	

	PVRTVec3 lightDir( 1, 0, 0 );
	lightDir.normalize();

	glUniform3f( m_paramTreeland_lightDir, 1.0, 0.0, 0.0 );
	//glUniform3f( m_paramTreeland_lightDir, lightDir.x, lightDir.y, lightDir.z );

	m_treeLand->renderAll();

	glUseProgram( 0 );
}
	
void Bonsai::setCamera( PVRTMat4 &camMVP )
{
	glUseProgram( m_progTreeland );
	glUniformMatrix4fv( m_paramTreeland_PMV, 1, GL_FALSE, camMVP.f );	

	// get eye dir from cam
	glUniform3f( m_paramTreeland_eyeDir, -camMVP[2][0], -camMVP[2][1], -camMVP[2][2] );
}

PVRTVec3 mix( PVRTVec3 a, PVRTVec3 b, float t )
{
	return (a * t) + (b * (1.0-t));
}

float fstep( float a, float b, float t )
{
	if (t < a) return 0.0;
	else if (t > b) return (1.0);
	return (t - a) / (b - a);
}

void Bonsai::synthesize( size_t ndx, float ii, float jj )
{	 
	// Results
	PVRTVec3 C(0.0, 0.0, 0.0); // result color
	float H=0.0; // result height

	// intermediate heights
	float Hs = 0.0; // structural height
	float Hf = 0.0; // fine height (detail)

	// Radius val .. make taller in middle
	//float rval = (1.0 - (sqrt( ii*ii + jj*jj ) / 2.0));
	float rval = ((sqrt( ii*ii + jj*jj ) / 2.0));

	// ==========================================
	// Layer -- Base Height
	// ==========================================	
	if (m_params.base == BASE_ROLLING_HILLS)
	{
		// "Rolling hills" -- base height

		// domain distortion
		float distDir = pnoise( ii * m_params.base_scale, 
			10.0 + m_params.offs, 
			jj *  
			m_params.base_scale );

		float ii2 = ii + distDir * m_params.base_distAmt;
		float jj2 = jj + distDir * m_params.base_distAmt;

		// structural height
		Hs = pnoise( ii2 * 2, 0.0, jj2 * 2 );				
		Hs = rval + (Hs * 0.3);	
	}
	else if (m_params.base == BASE_JAGGY)
	{
		// "Jagged mountians" -- base height		

		// structural height
		Hs = 5.0 * fabs(pturb( ii * m_params.base_scale, 
				m_params.offs - 5, 
				jj * m_params.base_scale, 6, true ));
		
		Hs = (rval + Hs) / 2.0;	
	}

	// ==========================================
	// Layer -- Base Veg
	// ==========================================	
	// Base veg
	if (m_params.baseVeg == BASEVEG_TURBY)
	{
		// color with turbulence
		float val = pturb( ii * 4, m_params.offs, jj * 4, 8, false );

		// add some height detail
		Hf = val * 0.05;
	
		// set color
		float v2 = clamp(val);
		C = mix( m_pally.m_colorOrganic1, m_pally.m_colorOrganic2, v2 );
	}
	else if (m_params.baseVeg == BASEVEG_HITE)
	{
		// Use height to drive color
		float t = Hs * m_params.baseVeg_hite_repeats;
		t = t - floor( t );

		C = mix( m_pally.m_colorOrganic2, m_pally.m_colorOrganic1, t );
	}

	// ==========================================
	// Layer -- Patch Veg
	// ==========================================	
	if (m_params.patchVeg == PATCHVEG_PATCHY)
	{
		float val = pturb( ii * m_params.patchVeg_scale, 
							m_params.offs, 
							jj * m_params.patchVeg_scale, 3, false );

		if (val > m_params.patchVeg_thresh)
		{			
			float v = pnoise( ii * m_params.patchVeg_nscale, m_params.offs, 
							jj * m_params.patchVeg_nscale );
			v = fabs(v);

			// FIXME: using accent color for this because there's no
			// accents yet
			if (m_params.patchMixMineral)
			{
				C = mix( m_pally.m_colorMineral1, m_pally.m_colorAccent, v );
			}
			else
			{
				C = mix( m_pally.m_colorOrganic2, m_pally.m_colorAccent, v );
			}

			// Add a little bit of height
			Hf += fstep( m_params.patchVeg_thresh, m_params.patchVeg_thresh + 0.05, val ) * 0.01;
		}
	}

	// ==========================================
	// Layer -- Decoration
	// ==========================================	
	if (m_params.deco == DECORATION_LAVATUBES)
	{
		float v = pnoise( ii * m_params.decoLavaScale, 0.0, 
							jj * m_params.decoLavaScale );
		float v2 = 1.0 - fstep( 0.0, m_params.decoLavaWidth, fabs(v) );

		//C = PVRTVec3( clamp( -v, 0, 1),v2, clamp( v, 0, 1));
		if (v2 > 0.01)
		{
			// replace color with tube
			C = mix( m_pally.m_colorMineral2, m_pally.m_colorMineral1, v2 );
			
			// Replace Hf
			Hf = v2 * m_params.decoLavaWidth * 0.5;
		}

	}


	// ====================================================

	// final height	
	H = Hs + Hf;

	// final color
	m_terrainColor[ndx * 3 + 0] = C.x * 255.0;
	m_terrainColor[ndx * 3 + 1] = C.y * 255.0;
	m_terrainColor[ndx * 3 + 2] = C.z * 255.0;


	// apply height
	m_hiteData[ndx] = H;
		
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
		// flip 
		row_pointers[i] = &(data[(1023-i) * sz * 3]);
	}
	png_write_image( png_ptr, row_pointers );

	png_write_end( png_ptr, info_ptr );
	png_destroy_write_struct( &png_ptr, &info_ptr );

	fclose( fp );

	DBG::info( "Wrote cache image %s\n", filename );
}

Luddite::HTexture Bonsai::_checkCachedColorImage( const char *suffix, GLubyte *data, int sz )
{
#if NO_CACHE
	return Luddite::HTexture();
#endif

	char filename[256];
	sprintf( filename, "./%s/%s_%s.png", CACHE_DIR, m_name, suffix );

	FILE *fp = fopen( filename, "rb" );
	if (!fp)
	{
		// no cached image exists
		return Luddite::HTexture();
	}

	Luddite::TextureDB &texDB = Luddite::TextureDB::singleton();
	Luddite::HTexture htex = texDB.getTexture( filename );

	return htex;

#if 0
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
#endif

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
#if NO_CACHE
	return false;
#endif

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

float Bonsai::getHeight( PVRTVec3 pos )
{	
	pos.x /= 500.0;
	pos.z /= 500.0;	

	return m_treeLand->hite( pos.x, pos.z ) * 100;
}

PVRTVec3 Bonsai::getSkyColor()
{
	return m_pally.m_colorSky;
}