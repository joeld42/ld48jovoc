#ifndef FONT_H
#define FONT_H
/*
 *  font.h
 *  stillpond
 *
 *  Created by joeld on 7/11/10.
 *  Copyright 2010 Tapnik. All rights reserved.
 *
 */

#include "quadbuff.h"

struct FontVert
{
	GLfloat pos[2];
	GLfloat st[2];
	GLfloat color[4];
};

class Font : public QuadBuff<FontVert>
{
public:
	Font( GLuint texId, int fontSz );
	
	// Draw a text string
    // TODO: merge drawStringFmt from glgamefont
	void drawString( float x, float y, const char *s );
	void drawStringCentered( float x, float y, const char *s );
	
	// draw at 0,0; i.e. if you are using gl transforms to place text
	void drawString( const char *s ); 
	
	
	float calcWidth( const char *s );  
	
	// Draw all the text in this buff
	void renderAll();
	
	// add glyph data
	void addGlyph( char ch,
				   int baseline,
				   int w, int h,
				  
				   // Placement on texture
				   float s0, float t0,
				   float s1, float t1 );
	
	void setColor( float r, float g, float b, float a );
	
	
private:
		
	struct Glyph
	{
		int baseline;
		int w, h;
		float s0,t0,s1,t1;
	};
	
	void _drawGlyph( const Glyph &g, int x, int y );
	
	Glyph  m_glyphs[256];
	GLuint m_texId;
	int    m_fontSz;
	
	// current color
	float colorR, colorG, colorB, colorA;
};


// The generated font data
// Font *makeFont_nesfont_8( GLuint fontTex );

#endif
