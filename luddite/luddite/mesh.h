#ifndef MESH_H
#define MESH_H
/*
 *  mesh.h
 *  pmines
 *
 *  Created by joeld on 11/26/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */

#include <IGraphics.h>

#include <PVRTVector.h>
#include <PVRTMatrix.h>

struct MeshVert
{
    float m_pos[3];
    float m_st[2];
};

class Mesh
{
public:
	Mesh();
	
	Mesh( size_t numTris, MeshVert *data );

	void build();
	void drawPaper( Luddite::HTexture hTexPaper, Luddite::HTexture hTexPlanetMap ); 

	void drawSkybox();
	
	// HACK : fix sts, scale 2x in S
	void scaleSTs();
	
protected:	
	size_t m_numTris;
	MeshVert *m_vertData;

	GLuint m_vbo;
};


#define MESH_PLANET2_NUM_TRI (960)
extern MeshVert planet2_meshData[2880];

#define MESH_GRID_NUM_TRI (162)
extern MeshVert grid_meshData[486];

#define MESH_CUBE_NUM_TRI (12)
extern MeshVert cube_meshData[36];

#endif

