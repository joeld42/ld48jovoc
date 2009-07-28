// No, not that kind of clipping. A cutting from a newspaper.
#include <iostream>
#include <string>

#include <allegro.h>
#include <alfont.h>

#include "clipping.h"

extern ALFONT_FONT *g_fontNews;

Clipping::Clipping( const std::string &text, int x, int y ) :
	m_text( text ),
	m_x( x ), m_y( y ),
	m_published( false )
{		
	// Make a sprite representation of the text

	// TMP: replace with alfont 
	int w = (int)m_text.size() * CHAR_W + 10;
	if (w < 20) w = 20;

	m_sprite = create_bitmap( w, 20 );
	rectfill( m_sprite, 0, 0, w, 20, makecol32( 235,223,195 ) );

	//textout_ex( m_sprite, font, m_text.c_str(), 5, 10, makecol(0,0,0), -1 );	
	alfont_set_font_size( g_fontNews, 18 );

	// pseudo - fixed width
	for (int i=0; i < m_text.size(); i++)
	{
		char buff[5];
		sprintf( buff, "%c", m_text[i] );
		alfont_textout_centre( m_sprite, g_fontNews, buff, 9 + CHAR_W *i, 2, makecol(0,0,0) );	
	}

	// ragged edge 
	for (int i=0; i < m_sprite->w; i++)
	{
		if (rand() % 2) putpixel( m_sprite, i, 0, makecol32( 0xff, 0, 0xff ) );		
		if (rand() % 2) putpixel( m_sprite, i, m_sprite->h-1, makecol32( 0xff, 0, 0xff ) );	
	}

	for (int i=0; i < m_sprite->h; i++)
	{
		if (rand() % 2) putpixel( m_sprite, 0, i, makecol32( 0xff, 0, 0xff ) );		
		if (rand() % 2) putpixel( m_sprite, m_sprite->w-1, i, makecol32( 0xff, 0, 0xff ) );	
	}

}

 Clipping::~Clipping()
{
	destroy_bitmap( m_sprite );
}

 bool Clipping::hit( int x, int y, int scale )
 {
	 //std::cout << "hit test: " << x << " " << y << " " 
		//<< m_text << " " << m_sprite->w << " " << m_sprite->h << std::endl;

	 if ( (x >= m_x) && (y >= m_y) &&
		  (x <= m_x+m_sprite->w*scale) && (y <= m_y+m_sprite->h*scale) ) 
	 {
		 std::cout << "HIT" << std::endl;
		 return true;
	 }
	 else return false;
 }