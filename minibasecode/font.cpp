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

#include "useful.h"
#include "font.h"



const float SCL = 1.0;

Font::Font( GLuint texId, int fontSz ) :
	QuadBuff<FontVert>( 0, true, GL_DYNAMIC_DRAW ),
	m_texId( texId ),
	m_fontSz( fontSz ),
	colorR(0), colorG(0), colorB(0), colorA(1)
{
	memset( m_glyphs, 0, sizeof(m_glyphs));
}

void Font::addGlyph( char ch,
			  int baseline,
			  int w, int h,
			  
			  // Placement on texture
			  float s0, float t0,
			  float s1, float t1 )
{
	Glyph &g = m_glyphs[(int)ch];
	
	// Size in pixels
	g.baseline = baseline;
	g.w = w; g.h = h;
	
    // hack : work around a bug in gfxpack
    const float FUDGE = 106.0/128.0;
//        const float FUDGE = 1.0;
    
	// Placement in texture space
//	g.s0 = s0 * FUDGE; g.t0 = (t0*FUDGE);
//	g.s1 = s1 * FUDGE; g.t1 = (t1*FUDGE);

    // FLIP Y
    g.s0 = s0 * FUDGE; g.t0 = 1.0-(t0*FUDGE);
    g.s1 = s1 * FUDGE; g.t1 = 1.0-(t1*FUDGE);
}

void Font::setColor( float r, float g, float b, float a )
{
	colorR = r;
	colorG = g;
	colorB = b;
	colorA = a;
}

void Font::drawString( const char *s )
{
	drawString( 0.0, 0.0, s );
}

void Font::drawString( float orig_x, float curr_y, const char *s )
{
	float curr_x = orig_x;
	const char *ch;	
	float bl;	
	
	for ( ch=s; *ch; ch++) {
		
		if (*ch=='\n') {
			curr_x = orig_x;
			//curr_y -= _activeCharset->leading;
			curr_y -= m_fontSz;
			
		} else if (*ch==' ') {
			curr_x += (m_fontSz*0.5f) * SCL; /* todo: fix this */
		} else {
			Glyph &g = m_glyphs[(int)*ch];
			if (g.h) {
				
				//get baseline
				bl = curr_y - (g.h - g.baseline) * SCL;
				
				_drawGlyph( g, (int)curr_x, (int)bl );
				
				//glTexCoord2f( g.xpos, g.ypos + g.height );
				//glVertex2f( curr_x, bl );
				//glTexCoord2f( g->xpos + g->w, g->ypos + g->h );
				//glVertex2f( curr_x + g->w, bl  );
				//glTexCoord2f( g->xpos + g->w, g->ypos  );
				//glVertex2f( curr_x + g->w, bl + g->h );
				//glTexCoord2f( g->xpos, g->ypos );
				//glVertex2f( curr_x, bl + g->h );
				
				// Advance cursor (FIXME: hacky spacing)
				curr_x += ( g.w + (m_fontSz > 14?1:0) ) * SCL;
			}
		}		
	}
}

float Font::calcWidth( const char *s )
{
	float curr_w = 0;
	float max_w = 0;
	const char *ch;	
	
	for ( ch=s; *ch; ch++) {
		
		if (*ch=='\n') {
			curr_w = 0;
		} else if (*ch==' ') {
			curr_w += (m_fontSz*0.5f)*SCL; /* todo: fix this */
		} else {
			Glyph &g = m_glyphs[(int)*ch];
			if (g.h) {
				// Advance cursor (FIXME: hacky spacing)
				curr_w += (g.w*SCL) + (m_fontSz > 14?1:0);
			}
		}
		
		if ((curr_w) > max_w)
		{
			max_w = curr_w;
		}
	}
	return max_w;
}

void Font::drawStringCentered( float x, float y, const char *s )
{
	float w = calcWidth( s );
	drawString( x - (w/2), y, s );
}

void Font::_drawGlyph( const Glyph &g, int x, int y )
{
	// Get next vert
	FontVert *newVert;
	newVert = this->addQuad();
    
    //printf( "texcoords %f %f -> %f %f\n",
    //       g.s0, g.t0, g.s1, g.t1 );
	
	// Upper triangle
	newVert[0].st[0] = g.s0;  newVert[0].st[1] = g.t0;
	newVert[0].pos[0] = x;    newVert[0].pos[1] = y + g.h*SCL;
	
	newVert[1].st[0] = g.s0; newVert[1].st[1] = g.t1;
	newVert[1].pos[0] = x;   newVert[1].pos[1] = y;
	
	newVert[2].st[0] = g.s1;     newVert[2].st[1] = g.t0;
	newVert[2].pos[0] = x+g.w*SCL; newVert[2].pos[1] = y + g.h*SCL;
	
	
	// Lower triangle
	newVert[3].st[0] = g.s1;     newVert[3].st[1] = g.t0;
	newVert[3].pos[0] = x + g.w*SCL; newVert[3].pos[1] = y + g.h*SCL;
	
	newVert[4].st[0] = g.s0; newVert[4].st[1] = g.t1;
	newVert[4].pos[0] = x;   newVert[4].pos[1] = y;
	
	newVert[5].st[0] = g.s1; newVert[5].st[1] = g.t1;
	newVert[5].pos[0] = x+g.w*SCL;   newVert[5].pos[1] = y;
	
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
void Font::renderAll()
{
    CHECKGL( "font::renderAll enter" );
    
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
    
    // Unbind any buffer
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    CHECKGL( "font::renderAll leave" );
}





