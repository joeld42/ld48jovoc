#ifndef PARTICLE_H
#define PARTICLE_H
/*
 *  particle.h
 *  stillpond
 *
 *  Created by joeld on 7/20/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */

#include "quadbuff.h"

#define PARTI_D2R (3.1415926535897932384626433832795f/180.0f)

// TODO: particle types are pond-specific, need to generalize
enum ParticleType 
{
	Particle_DOT,  // black dot
	Particle_DOT2, // white dot
	Particle_RIPPLE,
	Particle_SMOKE,
};

struct PartiVert
{
	GLfloat pos[2];
	GLfloat st[2];
	GLfloat color[4];
};

struct Particle
{
	ParticleType ptype;
	float s0, t0, s1, t1;
	float x, y;
	float dx, dy;
	float sx, sy;
	float angle;
	float age, maxAge;
};

#define MAX_PARTS (1000)

class ParticleBuff : public QuadBuff<PartiVert>
{
public:
	ParticleBuff( GLuint texId );
	
	void emitRadial( ParticleType p, float x, float y, float radius );
	void emitDirectional( ParticleType p, float x, float y, 
						 float angDir, float angSpread );
	
	void updateParts( float dt );
	void renderAll();
	
	void clearParticles();
	
	// info
	size_t numParts();
	
private:
	GLuint m_texId;
	
	void buildVerts();
	Particle *emitPart( ParticleType ptype);
	
	size_t m_numParts;
	Particle m_parts[ MAX_PARTS ];
};

#endif
