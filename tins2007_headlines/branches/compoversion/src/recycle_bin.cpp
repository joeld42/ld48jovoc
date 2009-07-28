#include <allegro.h>

#include <assert.h>

#include "panel.h"
#include "recycle_bin.h"
#include "wire_news.h"

extern WireNews *p_wirenews;

RecycleBin::RecycleBin( std::string name, HeadlinePool *hpool,
					    int pcol, int x, int y, int w, int h ) :
	Panel( name, pcol, x, y, w, h ), m_hpool( hpool )
{
	m_nextClipTime = 0.0f;
}

	
void RecycleBin::draw()
{
	Panel::draw();

	// draw my clippings
	for (std::vector<Clipping*>::iterator ci = m_clips.begin();
		 ci != m_clips.end(); ci++)
	{
		Clipping *c = (*ci);
		draw_sprite( m_surf, c->m_sprite, c->m_x, c->m_y );
	}
}

void RecycleBin::update( float dt)
{
	if (m_nextClipTime > dt)
	{
		m_nextClipTime -= dt;
	}
	else
	{
		// time for a new clipping		
		std::string wkey;

		// get word from the bucket or from the wire 
		// headlines?
		if ( (p_wirenews->m_heads.size() < 2) || (rand() % 100 < 25 ) )
		{
			int ndx = rand() % m_hpool->m_words.size();
			wkey = m_hpool->m_words[ ndx ];
		}
		else
		{
			int num = 0;
			for (int i=0; i < p_wirenews->m_heads.size(); i++ )
			{
				num+= p_wirenews->m_heads[i]->m_words.size();
			}

			int ndx = rand() % num;
			num = 0;
			for (int i=0; i < p_wirenews->m_heads.size(); i++ )
			{
				if ( ndx < p_wirenews->m_heads[i]->m_words.size())
				{
					wkey = p_wirenews->m_heads[i]->m_words[ndx];
					break;
				}
				else
				{
					ndx -= p_wirenews->m_heads[i]->m_words.size();
				}
			}
		}
		
		HeadlinePool::WordInfoMap::iterator wi = m_hpool->m_wordmap.find( wkey );
		assert( wi != m_hpool->m_wordmap.end() );

		WordInfo &w = (*wi).second;
		Clipping *clip = new Clipping( w.m_word, 20 + rand() % 150, 20 );
		m_clips.push_back( clip );

		// reset timer
		m_nextClipTime = 0.3f + ((float)rand() / (float)RAND_MAX)* 0.2f;
	}

	// update clipping pos
	// TODO: cool sawtool floaty motion
	for (std::vector<Clipping*>::iterator ci = m_clips.begin();
		 ci != m_clips.end(); ci++)
	{
		Clipping *c = (*ci);
		c->m_y += 50 * dt;

		// off the bottom? should destroy?
		if (c->m_y > 208)
		{
			ci = m_clips.erase( ci );
			delete c;
		}
	}

}

bool RecycleBin::dropClip( Clipping *clip, int x, int y )
{
	m_clips.push_back( clip );
	clip->m_x = x - (clip->m_sprite->w / 2);
	clip->m_y = y;

	return true;
}