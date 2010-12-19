#ifndef BONSAI_H
#define BONSAI_H

#include "glsw.h"
#include "quadbuff.h"

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
	TreeLand( float *heightData );

	void build();
	void renderAll();

	void _makeQuad( int i, int j );

	float hite( float x, float y );

protected:
	Luddite::HTexture m_htexTerrainColor;

	float *m_hiteData; // doesn't own, borrowed from Bonsai
};

class Bonsai
{
public:
	Bonsai();
	~Bonsai();

	void init();

	void buildAll();
	void renderAll();

	void synthesize( size_t ndx, float ii, float jj );

	void setCamera( PVRTMat4 &camMVP );

protected:
	TreeLand *m_treeLand;

	GLuint m_progTreeland;	

	GLint m_paramTreeland_PMV;
	GLint m_paramTreeland_samplerDif0;

	// highfield	
	float *m_hiteData;

};


#endif


