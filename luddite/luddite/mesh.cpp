/*
 *  mesh.cpp
 *  pmines
 *
 *  Created by joeld on 11/26/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */

#include "mesh.h"

Mesh::Mesh() :
	m_numTris( 0 ),
	m_vertData( NULL )
{
}

Mesh::Mesh( size_t numTris, MeshVert *data ) :
	m_numTris( numTris ),
	m_vertData( data )
{
}

void Mesh::build()
{
	IGraphics *gfx = IGraphics::singletonPtr();
	
	m_vbo = gfx->genBuffer();
	gfx->bindArrayBuffer( m_vbo );
	
	size_t uiSize = sizeof( MeshVert ) * m_numTris * 3;
	
	// Create the buffer
	//glBufferData( GL_ARRAY_BUFFER, uiSize, m_bufferData, m_usage );
	gfx->bufferData( uiSize, m_vertData, GL_STATIC_DRAW );
	
}

// Hack for my planet texture ..
// Scale STs 2x in S direction so planet texture is not
// stretched
void Mesh::scaleSTs()
{
	for (int i=0; i < m_numTris*3; i++)
	{
		m_vertData[i].m_st[0] *= 2; 
	}
}

void Mesh::drawPaper( Luddite::HTexture hTexPaper, Luddite::HTexture hTexPlanetMap )
{
	IGraphics *gfx = IGraphics::singletonPtr();
	
	gfx->bindPaperTexture( hTexPaper, hTexPlanetMap );
	gfx->bindArrayBuffer( m_vbo );

	gfx->bindPaperMaterial();
	
	gfx->drawArraysTriangles( 0, m_numTris * 3 );	
}


void Mesh::drawSkybox()
{
	IGraphics *gfx = IGraphics::singletonPtr();
	
	gfx->bindArrayBuffer( m_vbo );
	gfx->bindSkyboxMaterial();
	
	gfx->drawArraysTriangles( 0, m_numTris * 3 );	
}