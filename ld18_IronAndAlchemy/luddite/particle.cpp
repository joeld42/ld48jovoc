/*
 *  particle.cpp
 *
 *  Created by joeld on 7/20/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */
#include "math.h"

#include "random.h"

#include "particle.h"

ParticleBuff::ParticleBuff( GLuint texId ):
			QuadBuff<PartiVert>( MAX_PARTS, false, GL_DYNAMIC_DRAW ),
			m_texId( texId ),
			m_numParts(0)
{
}

size_t ParticleBuff::numParts()
{
	return m_numParts;
}

void ParticleBuff::clearParticles()
{
	m_numParts = 0;
}

void ParticleBuff::emitRadial( ParticleType ptype, float x, float y, float radius )
{
	Particle *p = emitPart( ptype );
	if (!p) return; // Buff full
	
	// set up general stuff
	const float initialVel = 50.0;
	p->x = x + randNormal() * radius;
	p->y = y + randNormal() * radius;
	p->maxAge = 3.0 + randNormal();

	//p->r = 1.0; p->g = 1.0; p->b = 1.0;
	float t = randUniform();
	float rA = 166/255.0; float gA = 202/255.0; float bA = 222/255.0;
	float rB = 31/255.0; float gB = 45/255.0; float bB = 227/255.0;
	p->r = (rA*t) + (rB*(1.0-t));
	p->g = (gA*t) + (gB*(1.0-t));
	p->b = (bA*t) + (bB*(1.0-t));
	
	if ((ptype==Particle_RING) || (ptype==Particle_SMOKE))
	{
		p->angle = randUniform( 0.0, 360.0);		
		p->dx = 0.0;
		p->dy = 0.0;
	}
	else if (ptype == Particle_DOT)
	{
		p->dx = randNormal() * initialVel;
		p->dy = (1.0+randNormal()) * initialVel;
	}
	else
	{
		p->angle = randUniform() * 360.0;
		p->dx = randNormal() * initialVel;
		p->dy = randNormal() * initialVel;
	}
	p->age = 0;
}

void ParticleBuff::emitBullet( ParticleType ptype, float x, float y, float angle )
{
	Particle *p = emitPart( ptype );
	if (!p) return; // Buff full
	
	// set up general stuff
	const float initialVel = 150.0;
	p->x = x;p->y = y;
	p->age = 0;
	p->maxAge = 5.0;
	p->sx = 4; p->sy = 4;
	p->r = 1.0; p->g = 1.0; p->b = 1.0;

	p->dx = cos( angle * PARTI_D2R ) * initialVel;
	p->dy = sin( angle * PARTI_D2R ) * initialVel;
}

void ParticleBuff::emitDirectional( ParticleType p, float x, float y, 
					  float angDir, float angSpread )
{
	// TODO
}

// Emits but does not initialize a part
Particle *ParticleBuff::emitPart( ParticleType ptype )
{
	// Is buff full?
	if (m_numParts == MAX_PARTS)
	{
		return NULL;
	}
	
	// Get tex coords
	float s0, t0, s1, t1;	
	
	// get next part
	Particle *p = &(m_parts[m_numParts++]);
	
	switch (ptype)
	{
		case Particle_DOT:
		case Particle_BULLET:
			s0 = 0.0; t0 = 0.5;
			s1 = 0.5; t1 = 1.0;
			break;
			
		case Particle_DOT2:
			s0 = 0.5; t0 = 0.5;
			s1 = 1.0; t1 = 1.0;
			break;
			
		case Particle_RING:
			s0 = 0.5; t0 = 0.0;
			s1 = 1.0; t1 = 0.5;
			break;
			
		case Particle_SMOKE:
			s0 = 0.0; t0 = 0.0;
			s1 = 0.5; t1 = 0.5;
			break;
	}
	
	p->ptype = ptype;
	p->s0 = s0; p->t0 = t0;
	p->s1 = s1; p->t1 = t1;	
	p->sx = 10.0; p->sy = 10.0;
	
	// leave it to caller to set up motion
	return p;
}

// fill up vertex buffer from part list
void ParticleBuff::updateParts( float dt )
{
	for (size_t i=0; i < m_numParts; ++i)
	{
		Particle &p = m_parts[i];
		
		float lasty = p.y;
	
		p.x = p.x + p.dx * dt;
		p.y = p.y + p.dy * dt;			
		
		// update age
		p.age += dt;
	
		float life = p.age / p.maxAge;
		
		switch (p.ptype)
		{
			case Particle_DOT:
			case Particle_DOT2:
				// Apply gravity
				p.dy = p.dy + (-100*dt);
				
				// update size
				p.sx = p.sy = 10.0 * (1.0 - life);
				break;
			
			case Particle_BULLET:
				break;

			case Particle_SMOKE:
				// grow and fade
				p.sx = 30.0 + 30 * life;
				p.sy = 30.0 + 30 * life;
				break;
				
			case Particle_RING:
				// grow and fade
				p.sx = 10.0 + 50 * life;
				p.sy = 10.0 + 50 * life;
				break;
		}
		

	}
	
	// kill off old particles
	for (int i = m_numParts-1; i >=0; i--)
	{
		// did this particle age out?
		if (m_parts[i].age > m_parts[i].maxAge)
		{
			// If this is not the last particle,
			// swap the last one into its place
			if (i<m_numParts-1)
			{
				m_parts[i] = m_parts[m_numParts-1];
			}
			m_numParts--;
		}
	}
}

	// Fill up vertex buffer from parts
void ParticleBuff::buildVerts()
{
	PartiVert *vert = m_bufferData;
	for (size_t i=0; i < m_numParts; ++i)
	{
		const Particle &p = m_parts[i];
		
		// Texture coords
		
		// Upper tri
		vert[0].st[0] =  p.s0; vert[0].st[1]  =  p.t1;
		vert[1].st[0] =  p.s1; vert[1].st[1]  =  p.t0;
		vert[2].st[0] =  p.s0; vert[2].st[1]  =  p.t0;
		
		// Lower Tri
		vert[3].st[0]  =  p.s0; vert[3].st[1]  =  p.t1;
		vert[4].st[0]  =  p.s1; vert[4].st[1]  =  p.t0;
		vert[5].st[0]  =  p.s1; vert[5].st[1]  =  p.t1;
		
		// Update position
		float ca = cos( p.angle * PARTI_D2R );
		float sa = sin( p.angle * PARTI_D2R );
		float sx2 = p.sx * 0.5f;
		float sy2 = p.sy * 0.5f;

		// Upper Tri
		vert[0].pos[0] = p.x + ((-sx2)*ca - (-sy2)*sa);
		vert[0].pos[1] = p.y + ((-sy2)*ca + (-sx2)*sa);
		
		vert[1].pos[0] = p.x + (( sx2)*ca - ( sy2)*sa);
		vert[1].pos[1] = p.y + (( sy2)*ca + ( sx2)*sa);
		
		vert[2].pos[0] = p.x + ((-sx2)*ca - ( sy2)*sa);
		vert[2].pos[1] = p.y + (( sy2)*ca + (-sx2)*sa);
		
		// Lower Tri
		vert[3].pos[0] = p.x + ((-sx2)*ca - (-sy2)*sa);
		vert[3].pos[1] = p.y + ((-sy2)*ca + (-sx2)*sa);
		
		vert[4].pos[0] = p.x + (( sx2)*ca - ( sy2)*sa);
		vert[4].pos[1] = p.y + (( sy2)*ca + ( sx2)*sa);
		
		vert[5].pos[0] = p.x + (( sx2)*ca - (-sy2)*sa);
		vert[5].pos[1] = p.y + ((-sy2)*ca + ( sx2)*sa);		
		
		// color
		float a = 1.0 - (p.age / p.maxAge);
		for (int j=0; j < 6; ++j)
		{
			vert[j].color[0] = p.r * a;
			vert[j].color[1] = p.g * a;
			vert[j].color[2] = p.b * a;
			vert[j].color[3] = a;
		}
		
		// Next quad
		vert += 6;
	}
	
	// set the buff size
	m_buffSize = m_numParts * 6;
}

// Draw all the sprites in this buff
void ParticleBuff::renderAll()
{
	// ready the very buffer
	buildVerts();
	
	// Draw the parts
	glBindTexture( GL_TEXTURE_2D, m_texId );
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo() );
	updateBuffer();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, sizeof(PartiVert), 0 );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );	
	glTexCoordPointer( 2, GL_FLOAT, sizeof(PartiVert), (void*)(2*sizeof(GLfloat)) );
	
	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4, GL_FLOAT, sizeof(PartiVert), (void*)(4*sizeof(GLfloat)) );
	
	glDrawArrays( GL_TRIANGLES, 0, this->size() );
	
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
}
