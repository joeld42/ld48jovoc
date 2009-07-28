#include <allegro.h>
#include <vector>
#include <assert.h>

#include "panel.h"
#include "layout.h"

// masthead
#define MAST_HITE (30)

NewsLayout::NewsLayout( std::string name, int pcol, int x, int y, int w, int h ) :
	Panel( name, pcol, x,y, w, h )
{
	m_newsImg= NULL;
	m_newsImg2= NULL;
	m_dirty = true;

	m_bodyText = load_bitmap( "gamedata/bodytext.bmp", NULL );
	m_masthead = load_bitmap( "gamedata/masthead.bmp", NULL );
}

// Update internal bitmap
void NewsLayout::draw()
{	
	// newslayout doesn't need to redraw each frame
	if (m_dirty)
	{
		m_dirty = false;

		Panel::draw();

		if (m_newsImg)
		{
			blit( m_newsImg, m_surf, 0, 0, 
				  (m_w - m_newsImg->w)/2, 0, m_newsImg->w, m_newsImg->h );
		}
	}
}

void NewsLayout::update( float dt )
{
	Panel::update( dt );
}

// resets cols and updates news Image and
// fills data
void NewsLayout::setNCols( int cols )
{
	m_ncols = cols;
	m_dirty = true;
	
	if (m_newsImg)
	{
		destroy_bitmap( m_newsImg );		
	}

	if (m_newsImg2)
	{
		destroy_bitmap( m_newsImg2 );
	}

	// create blank page
	m_newsImg = create_bitmap( m_ncols * COL_SZ, m_h );
	m_newsImg2 = create_bitmap( m_ncols * COL_SZ, m_h );
	rectfill( m_newsImg, 0, 0, m_newsImg->w, m_newsImg->h, makecol32( 235,223,195 ) );

	// draw masthead and bottom margin
	stretch_blit( m_masthead, m_newsImg, 0, 0,250, 30, 0, 0, m_ncols*COL_SZ, 30 );
	rectfill( m_newsImg, 0, 238, m_ncols*COL_SZ, 250, makecol32( 250, 250, 255 ) );

	// draw column dividers
	for (int i = 1; i < m_ncols; i++)
	{
		vline( m_newsImg, i*COL_SZ, MAST_HITE, m_h - 3, makecol32( 205,193,165 ) );
	}

	// Init layout
	for (int i=0; i < m_ncols; i++)
		for (int j=0; j < YCOLS; j++)
			m_layout[i][j] = 0;

	int avgStoriesPerCol = 1;
	int numStories = avgStoriesPerCol * m_ncols;
	int snum = 1;
	for (int i=0; i < numStories; i++)
	{
		int si, sj;
		si = ((float)rand() / (float)RAND_MAX) * m_ncols;
		sj = ((float)rand() / (float)RAND_MAX) * YCOLS;
		m_layout[si][sj] = snum++;
	}

	// spacial case, always put a story at 0,0
	m_layout[0][0] = snum++;

	
	// only let each story grow once per iteration	
	bool storyGrown[200]; // hack

	// Now grow the stories to the right and down
	bool somethingChanged = false;
	do {
		// init
		somethingChanged = false;
		for (int i=0; i < snum; i++)
		{
			storyGrown[ i ] = false;
		}

		// growth
		for (int i=0; i < m_ncols; i++)
		{
			for (int j=0; j < YCOLS; j++)
			{
				// no story yet?
				if (m_layout[i][j] == 0)
				{
					// try to grow from the left first
					if ((i>0)&&(m_layout[i-1][j] != 0)&&
						(!storyGrown[m_layout[i-1][j]]) )
					{
						m_layout[i][j] = m_layout[i-1][j];
						storyGrown[m_layout[i-1][j]] = true;
						somethingChanged = true;
					}

					// otherwise try to grow from above
					if ((j>0)&&(m_layout[i][j-1] != 0) &&
						(!storyGrown[m_layout[i][j-1]])) 
					{
						m_layout[i][j] = m_layout[i][j-1];
						storyGrown[m_layout[i][j-1]] = true;
						somethingChanged = true;
					}
				}
			}
		}
	} while(somethingChanged);

	// Count the number of unique stories
	m_storiesLeft.clear();
	
	// reuse storygrown as "story seen" :)
	for (int i=0; i < snum; i++)		
		storyGrown[ i ] = false;	

	// check for unique stories
	for (int i=0; i < m_ncols; i++)		
		for (int j=0; j < YCOLS; j++)
			if (!storyGrown[ m_layout[i][j] ])
			{
				m_storiesLeft.push_back( m_layout[i][j] );
				storyGrown[ m_layout[i][j] ] = true;
			}

	// scramble the storiesLeft so they show up in
	// random order
	for (int i=0; i < m_storiesLeft.size(); i++)
	{
		int tmp = m_storiesLeft[i];
		int ndx = rand() % m_storiesLeft.size();
		m_storiesLeft[i] = m_storiesLeft[ndx];
		m_storiesLeft[ndx] = tmp;
	}
	
	int yChunkSz = ( (m_newsImg->h - MAST_HITE) - 3 ) / YCOLS;
#if 0
	// DBG: draw layout IDs
	static int colors[] = {
		makecol32( 0,0,0 ),
		makecol32( 0,0,128 ),
		makecol32( 0,128, 0 ),
		makecol32( 0,128, 128 ),
		makecol32( 128, 0,0 ),
		makecol32( 128, 0,128 ),
		makecol32( 128, 128, 0 ),
		makecol32( 128, 128, 128 ),
	};
	for (int i=0; i < m_ncols; i++)
	{
		for (int j=0; j < YCOLS; j++)
		{
			rect( m_newsImg, 
				i*COL_SZ, MAST_HITE + j*yChunkSz,
				(i+1)*COL_SZ - 3, (MAST_HITE + (j+1)*yChunkSz) - 3,
				colors[ m_layout[i][j] % 8 ] );

			char buff[10];
			sprintf( buff, "%c", m_layout[i][j] + 'A' );
			textout_ex( m_newsImg, font, buff,
				i*COL_SZ + 5, MAST_HITE + j*yChunkSz + 5,
				colors[ m_layout[i][j] % 8 ], -1 );
		}
	}
#endif

}

// Woot! Print a story!
void NewsLayout::printStory( std::string &headline, int *pSX, int *pSY )
{
	// check if we're out of stories
	if (!m_storiesLeft.size()) return;

	// Get the headline width
	int storyId = m_storiesLeft.back();
	m_storiesLeft.pop_back();

	// find the upper left corner of the story
	int sx = -1, sy = -1;
	for (int i=0; i < m_ncols; i++)
	{
		for (int j=0; j < YCOLS; j++)
		{
			if (m_layout[i][j] == storyId) 
			{
				sx = i; sy = j;
				break;
			}
		}
		if (sx>=0) break;
	}
	printf("Found story at %d %d\n", sx, sy );

	int yChunkSz = ( (m_newsImg->h - MAST_HITE) - 3 ) / YCOLS;

	// return story location to app
	if ((pSX)&&(pSY))
	{
		*pSX = sx*COL_SZ + 12;
		*pSY = MAST_HITE + sy*yChunkSz + 12;
	}
	
	// Fill the story with pseudo-text
	int colw = 0;
	for (int c=sx; (c < m_ncols) && (m_layout[c][sy] == storyId); c++)
	{
		// find height of column
		int colh=0;
		for (int r=sy; (r < YCOLS) && (m_layout[c][r] == storyId); r++) colh++;		

		// Draw story text in column
		int hite = sy*yChunkSz;
		int offs = ((float)rand() / (float)RAND_MAX) * (m_bodyText->h - (colh*yChunkSz));
		if (offs < 0) offs = 0;
		blit( m_bodyText, m_newsImg, 
			  0, offs,
			  c*COL_SZ, MAST_HITE + sy*yChunkSz,
			  m_bodyText->w, colh * yChunkSz );		

		colw++;
	}
	
	// TODO: draw real headline
	for (int i=0; i < 5; i++ )
	{
		hline( m_newsImg, 
				sx*COL_SZ+3, MAST_HITE + sy*yChunkSz + i, (sx+colw)*COL_SZ-3, 
				makecol32( 0,0,0 ) );
	}

	
	// set dirty
	// TOTAL HACK -- don't set dirty, but always set
	// dirty in dropClip so new story doesn't show up 
	// until clips reach us
	//m_dirty = true;
}

bool NewsLayout::dropClip( Clipping *clip, int x, int y )
{
	// Only accept published clips
	if (!clip->m_published) return false;

	// HACK: see printstory
	m_dirty = true;

	// yay
	delete( clip );
	return true;
}