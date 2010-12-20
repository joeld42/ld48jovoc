#ifndef BONSAI_H
#define BONSAI_H

#include "glsw.h"
#include "quadbuff.h"
#include "pally.h"

#include "PVRT/PVRTVector.h"
#include "PVRT/PVRTMatrix.h"

enum 
{
	Attrib_POSITION,
	Attrib_NORMAL,
	Attrib_TEXCOORD,
	Attrib_COLOR,
        
	Attrib_NUM_ATTRIB        
};

struct TreeVert
{
	GLfloat pos[3];
	GLfloat st[2];
};



// The land the tree sits on
class TreeLand : public QuadBuff<TreeVert>
{
public:
	TreeLand( float *heightData, Luddite::HTexture htexColor, Luddite::HTexture htexNorm );

	void build();
	void renderAll();

	void _makeQuad( int i, int j );

	float hite( float x, float y );

//protected:
	Luddite::HTexture m_htexTerrainColor;
	Luddite::HTexture m_htexTerrainNorm;

	float *m_hiteData; // doesn't own, borrowed from Bonsai	
};

// ===========================================
//  texture synth params
// ===========================================

// Base height
enum {
	BASE_ROLLING_HILLS,
	BASE_JAGGY,

	NUM_BASE
};

// Base Veg
enum {
	BASEVEG_HITE,
	BASEVEG_TURBY,	

	NUM_BASEVEG
};

// Patch Veg
enum
{
	PATCHVEG_NONE,
	PATCHVEG_PATCHY,

	NUM_PATCHVEG
};

// Decoration (minerals)
enum
{
	DECORATION_NONE,
	DECORATION_LAVATUBES,

	NUM_DECORATION
};

struct SynthParams
{

	// global noise offset
	float offs;

	// Base layer
	int base;  // base type
	float base_distAmt; // distortion amount
	float base_scale;

	// Base veg
	int baseVeg;	
	int baseVeg_hite_repeats;

	// Patch veg
	int patchVeg;
	int patchMixMineral;
	float patchVeg_scale;
	float patchVeg_nscale;
	float patchVeg_thresh;

	// Decoration
	int deco;
	float decoLavaWidth;
	float decoLavaScale;


};

// This is named bonsai for no good reason, it was a different idea
// that morphed into this one.
class Bonsai
{
public:
	Bonsai( const char *name );
	~Bonsai();

	void init();

	void buildAll();
	void renderAll();

	float getHeight( PVRTVec3 pos );

	void synthesize( size_t ndx, float ii, float jj );

	void setCamera( PVRTMat4 &camMVP );	

	PVRTVec3 getSkyColor();

//protected:

	void _synthLand();
	void _calcNormals();

	void _cacheColorImage( const char *suffix, GLubyte *data, int sz );
	void _cacheHeight( const char *suffix, float *data, int sz );	

	Luddite::HTexture _checkCachedColorImage( const char *suffix, GLubyte *data, int sz );
	bool _checkCachedHeight( const char *suffix, float *data, int sz );

	const char *m_name;
	TreeLand *m_treeLand;

	GLuint m_progTreeland;	

	GLint m_paramTreeland_PMV;
	GLint m_paramTreeland_lightDir;	
	GLint m_paramTreeland_eyeDir;

	GLint m_paramTreeland_samplerDif0;
	GLint m_paramTreeland_samplerNrm0;

	// src palette
	Pally m_pally;

	// Synth Params 
	SynthParams m_params;

	// highfield	
	float *m_hiteData;

	// color data
	GLubyte *m_terrainColor;
	GLubyte *m_terrainNorm;

};


#endif


