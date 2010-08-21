/*
 *  font.cpp
 *  stillpond
 *
 *  Created by joeld on 7/11/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */

#include <assert.h>
#include <string.h>

#include "font.h"

using namespace Luddite;

Luddite::Font::Font( GLuint texId, int fontSz ) :
	QuadBuff<FontVert>( 0, true, GL_DYNAMIC_DRAW ),
	m_texId( texId ),
	m_fontSz( fontSz ),
	colorR(0), colorG(0), colorB(0), colorA(1)
{
	memset( m_glyphs, 0, sizeof(m_glyphs));
}
void Luddite::Font::addGlyph( char ch,
			  int baseline,
			  int w, int h,
			  
			  // Placement on texture
			  float s0, float t0,
			  float s1, float t1 )
{
	Glyph &g = m_glyphs[ch];
	
	// Size in pixels
	g.baseline = baseline;
	g.w = w; g.h = h;
	
	// Placement in texture space
	g.s0 = s0; g.t0 = t0;
	g.s1 = s1; g.t1 = t1;
}

void Luddite::Font::setColor( float r, float g, float b, float a )
{
	colorR = r;
	colorG = g;
	colorB = b;
	colorA = a;
}

void Luddite::Font::drawString( const char *s )
{
	drawString( 0.0, 0.0, s );
}

void Luddite::Font::drawString( float orig_x, float curr_y, const char *s )
{
	float curr_x = orig_x;
	const char *ch;	
	float start_x, bl;	
	
	for ( ch=s; *ch; ch++) {
		
		if (*ch=='\n') {
			curr_x = orig_x;
			//curr_y -= _activeCharset->leading;
			curr_y -= m_fontSz;
			
		} else if (*ch==' ') {
			curr_x += (m_fontSz*0.5f); /* todo: fix this */
		} else {
			Glyph &g = m_glyphs[*ch];
			if (g.h) {
				
				//get baseline
				bl = curr_y - (g.h - g.baseline);
				
				_drawGlyph( g, (int)curr_x, (int)bl );							
				
				// Advance cursor (FIXME: hacky spacing)
				curr_x += g.w + (m_fontSz > 14?1:0);
			}
		}		
	}
}

float Luddite::Font::calcWidth( const char *s )
{
	float curr_w = 0;
	float max_w = 0;
	const char *ch;	
	
	for ( ch=s; *ch; ch++) {
		
		if (*ch=='\n') {
			curr_w = 0;
		} else if (*ch==' ') {
			curr_w += (m_fontSz*0.5f); /* todo: fix this */
		} else {
			Glyph &g = m_glyphs[*ch];
			if (g.h) {
				// Advance cursor (FIXME: hacky spacing)
				curr_w += g.w + (m_fontSz > 14?1:0);
			}
		}
		
		if ((curr_w) > max_w)
		{
			max_w = curr_w;
		}
	}
	return max_w;
}

void Luddite::Font::drawStringCentered( float x, float y, const char *s )
{
	float w = calcWidth( s );
	drawString( x - (w/2), y, s );
}

void Luddite::Font::_drawGlyph( const Glyph &g, int x, int y )
{
	// Get next vert
	FontVert *newVert;
	newVert = this->addQuad();
	
	// Upper triangle
	newVert[0].st[0] = g.s0;  newVert[0].st[1] = g.t0;
	newVert[0].pos[0] = x;    newVert[0].pos[1] = y + g.h;
	
	newVert[1].st[0] = g.s0; newVert[1].st[1] = g.t1;
	newVert[1].pos[0] = x;   newVert[1].pos[1] = y;
	
	newVert[2].st[0] = g.s1;     newVert[2].st[1] = g.t0;
	newVert[2].pos[0] = x+g.w; newVert[2].pos[1] = y + g.h;
	
	
	// Lower triangle
	newVert[3].st[0] = g.s1;     newVert[3].st[1] = g.t0;
	newVert[3].pos[0] = x + g.w; newVert[3].pos[1] = y + g.h;
	
	newVert[4].st[0] = g.s0; newVert[4].st[1] = g.t1;
	newVert[4].pos[0] = x;   newVert[4].pos[1] = y;
	
	newVert[5].st[0] = g.s1; newVert[5].st[1] = g.t1;
	newVert[5].pos[0] = x+g.w;   newVert[5].pos[1] = y;
	
	for (int i=0; i < 6; i++)
	{
		// ABGR?
		newVert[i].color[0] = colorR;
		newVert[i].color[1] = colorG;
		newVert[i].color[2] = colorB;
		newVert[i].color[3] = colorA;
	}
	
}


// Draw all the sprites in this buff
void Luddite::Font::renderAll()
{
	glBindTexture( GL_TEXTURE_2D, m_texId );
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo() );
	updateBuffer();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, sizeof(FontVert), 0 );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );	
	glTexCoordPointer( 2, GL_FLOAT, sizeof(FontVert), (void*)(2*sizeof(GLfloat)) );
	
	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4, GL_FLOAT, sizeof(FontVert), (void*)(4*sizeof(GLfloat)) );
	
	glDrawArrays( GL_TRIANGLES, 0, this->size() );
	
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
}





