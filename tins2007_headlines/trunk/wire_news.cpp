#include <allegro.h>
#include <alfont.h>

#include <assert.h>

#include <algorithm>
#include <list>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "panel.h"
#include "wire_news.h"

extern ALFONT_FONT *g_fontWireTiny;
extern ALFONT_FONT *g_fontWireHeadline;

WireNews::WireNews( std::string name, HeadlinePool *hpool,
					int pcol, int x, int y, int w, int h ) :

	Panel( name, pcol, x, y, w, h ), m_hpool( hpool )
{
	m_nextHeadlineTime = 2.0f;

	m_hblank = load_bitmap( "gamedata/headline_clip.bmp", NULL );
}

void WireNews::draw()
{
	Panel::draw();

	for (int i=0; i < m_heads.size(); i++)
	{
		//draw_sprite( m_surf, m_heads[i]->m_wirePic, 0, 200-(i*50) );
		draw_sprite( m_surf, m_heads[i]->m_wirePic, 0, m_heads[i]->m_currY );
	}
}

void WireNews::update( float dt )
{
	Panel::update( dt );
	
	if (m_nextHeadlineTime > dt)
	{
		m_nextHeadlineTime -= dt;
	}
	else
	{
		// time for a new headline (if we're not already full
		if (m_heads.size() < 5 )
		{
			Headline *h = new Headline();

			// copy the headline info over
			*h = m_hpool->m_headlines[ rand() % m_hpool->m_headlines.size() ];

			// make a headling image
			h->m_wirePic = create_bitmap( 250, 50 );
			//rectfill( h->m_wirePic, 0, 0, 250, 50, makecol32( 0, 0, 200 ) );
			blit( m_hblank, h->m_wirePic, 0, 0, 0, 0, 250, 50 );
			h->m_currY =-50;

			std::string hs = h->m_text;
			int txty = 5;			

			alfont_set_font_size( g_fontWireHeadline, 12 );
			std::vector<std::string> sline;
			boost::algorithm::split( sline, h->m_text, boost::algorithm::is_any_of(" \t"), 
									 boost::algorithm::token_compress_on );
			
			for (int i = 0; i < sline.size(); i++ )
			{
				std::cout << i << " -- " << sline[i] << std::endl;
			}
			
			int sndx = 0;
			bool firstline = true;
			while( sndx < sline.size() )
			{
				std::string ln;
				
				std::string sWord = sline[ sndx ];
				while ( alfont_text_length( g_fontWireHeadline, 
					(ln + " " + sWord).c_str() ) < ( 240 - (firstline?15:0) ) ) 
				{					
					ln += " ";
					ln += sWord;					
					sndx++;

					if (sndx < sline.size() )
					{
						sWord = sline[ sndx ];
					}
				}
				firstline = false;
				alfont_textout( h->m_wirePic, g_fontWireHeadline,
								 ln.c_str(), 7, txty +2, makecol( 44, 122, 176 ) );

				alfont_textout( h->m_wirePic, g_fontWireHeadline,
								 ln.c_str(), 5, txty, makecol( 255, 255, 255 ) );
				
				txty += 12;				
			}

			alfont_set_font_size( g_fontWireTiny, 10 );
			alfont_textout( h->m_wirePic, g_fontWireTiny,
							h->m_src.c_str(), 8, 35, makecol32( 158, 210, 244 ) );

			alfont_textout_right( h->m_wirePic, g_fontWireTiny,
							h->m_mediaSrc.c_str(), 250-8, 35, makecol32( 158, 210, 244 ) );

			//rect( h->m_wirePic, 0, 0, 249, 49, makecol32( 0, 255, 255 ) );

			// add to list
			m_heads.push_back( h );	

			m_nextHeadlineTime = 10.0f;
		}
	}

	// update yvals
	for (int i=0; i < m_heads.size(); i++)
	{
		int targY = 200-(i*50);
		if (m_heads[i]->m_currY > targY) m_heads[i]->m_currY--;
		if (m_heads[i]->m_currY < targY) m_heads[i]->m_currY++;
	}
}

bool _cmpX( Clipping *a, Clipping *b )
{
	return a->m_x < b->m_x;
}

bool WireNews::checkHeadline( std::vector<Clipping*> &clips, std::string &hltext )
{
	if (!clips.size()) return false;
	
	std::vector<Clipping*> clipsleft = clips;
	std::string testheadline;

	std::sort( clipsleft.begin(), clipsleft.end(), _cmpX );

	while (clipsleft.size())
	{
		printf ("%d clips left\n", clipsleft.size() );

		//find the upper-left clip
		Clipping *cleft = NULL;
		for (int i=0; i < clipsleft.size(); i++)
		{

			if ( (cleft==NULL) ||
			    ( clipsleft[i]->m_y + clipsleft[i]->m_x < cleft->m_y + cleft->m_x) )
			{
				cleft = clipsleft[i];
			}
		}

		// now assemble all of the clips on the same "line" as us
		int hy = cleft->m_y;		
		std::vector<Clipping*> clips2;
		for (int i=0; i < clipsleft.size(); i++ )
		{
			if ( abs(clipsleft[i]->m_y - hy) < 24 )
			{
				testheadline += clipsleft[i]->m_text;				
			}
			else
			{
				// try it next time
				clips2.push_back( clipsleft[i] );
			}
		}

		// and keep going with remaining clips
		clipsleft = clips2;
	}

	// lowercasify
	std::transform( testheadline.begin(), testheadline.end(), 
		             testheadline.begin(), tolower );

	// DBG:
	std::cout << testheadline << std::endl;

	// Now check if the headlines match
	for (std::vector<Headline*>::iterator hi = m_heads.begin();
		 hi != m_heads.end(); hi++)
	{
		Headline *hl = (*hi);
		std::string hlB;
		for (int i=0; i < hl->m_text.size(); i++)
		{
			char ch = hl->m_text[i];
			if (isalpha(ch) || isdigit(ch))
			{
				hlB += tolower( ch );
			}
		}
		std::cout << "TEST: " << hlB << std::endl;

		// They matched it!
		if (hlB == testheadline) 
		{
			m_heads.erase( hi ); // remove from wire news
			hltext = hl->m_text;
			delete( hl );

			return true;
		}		
	}

	return false;
}

void WireNews::click( int x, int y )
{
	// Check for the mini-button
	if ( (x >= 230) && ( x <= 245) )
	{
		for (std::vector<Headline*>::iterator hi = m_heads.begin();
			 hi != m_heads.end(); hi++)
		{
			if ( ( y >= (*hi)->m_currY + 5) &&
				 ( y <= (*hi)->m_currY + 18) )
			{
				delete( *hi );
				m_heads.erase( hi ); // remove from wire news				
				
				break;
			}
		}
	}
}