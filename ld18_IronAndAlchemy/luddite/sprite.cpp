/*
 *  sprite.cpp
 *  stillpond
 *
 *  Created by joeld on 7/10/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */
#include <math.h>
#include <stdlib.h>

#include "sprite.h"


// ========================================================
#pragma mark SpriteBuff
SpriteBuff::SpriteBuff( GLuint texId ) :
	QuadBuff<SpriteVert>( 0, false, GL_DYNAMIC_DRAW ),
	m_texId( texId )
{
}

Sprite *SpriteBuff::makeSprite( float s0, float t0,
							    float s1, float t1 )
{
	SpriteVert *newVert;
	newVert = this->addQuad();
	
	Sprite *sprite = new Sprite( this, newVert );
	sprite->setTexCoords( s0, t0, s1, t1 );

	// Call update to initialize the pos data
	sprite->update();
	
	// Remember the sprite
	m_sprites.push_back( sprite );
	
	return sprite;
}



// Draw all the sprites in this buff
void SpriteBuff::renderAll()
{
	glBindTexture( GL_TEXTURE_2D, m_texId );
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo() );
	updateBuffer();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, sizeof(SpriteVert), 0 );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );	
	glTexCoordPointer( 2, GL_FLOAT, sizeof(SpriteVert), (void*)(2*sizeof(GLfloat)) );
	
	//glEnableClientState( GL_NORMAL_ARRAY );
	//glNormalPointer( GL_FLOAT, sizeof(MapVert), (void*)(6*sizeof(GLfloat)) );
	
	glDrawArrays( GL_TRIANGLES, 0, this->size() );
	
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	//glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
}

void SpriteBuff::removeSprite( Sprite *spr )
{
	printf("removeSprite, sprites Size is %d\n", m_sprites.size() );
	
	// Swap out the vert data with the last sprite in our list
	
	// Find the sprite with the highest vert data index, i.e.
	// the one on the end of the list
	// FIXME: make this better aaagrrggghh
	Sprite *lastSprite = m_sprites.back();
	for (std::list<Sprite*>::iterator spi = m_sprites.begin();
		 spi != m_sprites.end(); spi++)
	{
		if ((*spi)->m_vertData > lastSprite->m_vertData)
		{
			lastSprite = (*spi);
		}
	}
	
	// relocate the last sprite to take this one's place
	memcpy( spr->m_vertData, lastSprite->m_vertData, sizeof(SpriteVert)*6 ); 
	lastSprite->m_vertData = spr->m_vertData;
	
	// Remove the sprite
	m_sprites.remove( spr );
	
	// And account for this quad's verts
	m_buffSize -= 6;
	printf("removeSprite -- Shrink m_buffSize to %d\n", m_buffSize );
}
					
void SpriteBuff::removeAll()
{
	std::list<Sprite*> spritesToRemove = m_sprites;
	
	// delete sprites
	for (std::list<Sprite*>::iterator spi = spritesToRemove.begin();
		 spi != spritesToRemove.end(); spi++)
	{
		Sprite *spr = (*spi);
		printf("about to delete sprite %p\n", spr );
		delete spr;
	}
	
	// clear sprite list (should be empty anyways)
	m_sprites.clear();
	
	// reset buffer
	clear();
}

// ========================================================
#pragma mark Sprite
Sprite::Sprite( SpriteBuff *owner, SpriteVert *vertData ) :
	m_myBuff( owner ),
	m_vertData( vertData ),
	x(0.0f), y(0.0f),
	angle( 0.0f ), 
	sx( 1.0 ), sy(1.0)
{
}

Sprite::~Sprite()
{
	m_myBuff->removeSprite( this );
}	

// set st pos
void Sprite::setTexCoords( float s0, float t0,
						  float s1, float t1 )
{
	// Upper tri
	m_vertData[0].st[0]  =  s0; m_vertData[0].st[1]  =  t1;
	m_vertData[1].st[0]  =  s1; m_vertData[1].st[1]  =  t0;
	m_vertData[2].st[0]  =  s0; m_vertData[2].st[1]  =  t0;
	
	// Lower Tri
	m_vertData[3].st[0]  =  s0; m_vertData[3].st[1]  =  t1;
	m_vertData[4].st[0]  =  s1; m_vertData[4].st[1]  =  t0;
	m_vertData[5].st[0]  =  s1; m_vertData[5].st[1]  =  t1;
	
}


void Sprite::flipHoriz()
{
	m_vertData[0].st[0] = m_vertData[5].st[0];
	m_vertData[4].st[0] = m_vertData[2].st[0];
	
	std::swap( m_vertData[2].st[0], m_vertData[1].st[0] );
	std::swap( m_vertData[3].st[0], m_vertData[5].st[0] );
	
}

void Sprite::update()
{
	float ca = cos( angle * SPRITE_D2R );
	float sa = sin( angle * SPRITE_D2R );
	float sx2 = sx * 0.5f;
	float sy2 = sy * 0.5f;
	
	// Upper Tri
	m_vertData[0].pos[0] = x + ((-sx2)*ca - (-sy2)*sa);
	m_vertData[0].pos[1] = y + ((-sy2)*ca + (-sx2)*sa);

	m_vertData[1].pos[0] = x + (( sx2)*ca - ( sy2)*sa);
	m_vertData[1].pos[1] = y + (( sy2)*ca + ( sx2)*sa);
	
	m_vertData[2].pos[0] = x + ((-sx2)*ca - ( sy2)*sa);
	m_vertData[2].pos[1] = y + (( sy2)*ca + (-sx2)*sa);

	// Lower Tri
	m_vertData[3].pos[0] = x + ((-sx2)*ca - (-sy2)*sa);
	m_vertData[3].pos[1] = y + ((-sy2)*ca + (-sx2)*sa);
	
	m_vertData[4].pos[0] = x + (( sx2)*ca - ( sy2)*sa);
	m_vertData[4].pos[1] = y + (( sy2)*ca + ( sx2)*sa);
	
	m_vertData[5].pos[0] = x + (( sx2)*ca - (-sy2)*sa);
	m_vertData[5].pos[1] = y + ((-sy2)*ca + ( sx2)*sa);
}

// Tests if a point hits us
bool Sprite::testHit( float tx, float ty )
{
	float px, py;
	
	// Unrotate the point to align with the bbox
	tx = tx - x;
	ty = ty - y;
	
	float ca = cos( -angle * SPRITE_D2R );
	float sa = sin( -angle * SPRITE_D2R );
	
	px = (tx*ca) - (ty*sa);
	py = (ty*ca) + (tx*sa);
	
	// Test against sprite
	float sx2 = sx / 2;
	float sy2 = sy / 2;
	if ( (px >= -sx2) && 
		 (px <= sx2) &&
		 (py >= -sy2) &&
		 (py <= sy2 ) )
	{
		// we hit the sprite
		return true;
	}
	
	return false;
}
