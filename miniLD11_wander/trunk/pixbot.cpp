#include <allegro.h>
#include <math.h>
#include <stdlib.h>

float lerp( float t, float a, float b )
{
	return (a*t) + (b*(1.0-t));
}

// Uses the "pixelbot" algorithm from David Bollinger
// with slight changes to permit different template shapes
// http://www.davebollinger.com/works/pixelrobots/
BITMAP *make_pixbot( BITMAP *codeMask )
{
	BITMAP *bot = create_bitmap( codeMask->w, codeMask->h );
	unsigned int rndbits = 0;
	for (int j=0; j < codeMask->h; j++)
	{
		for (int i=0; i < codeMask->w; i++)
		{
			if (getpixel( codeMask, i,j ) == makecol( 0xff, 0xff, 0xff ) )
			{
				// off, make mask color
				putpixel( bot, i, j, makecol( 0xff, 0x00, 0xff ) );
			}
			else
			{
				// do we needs more bits? 
				if (!rndbits) rndbits = rand();				
				bool bit = rndbits & 0x01;
				rndbits >>= 1;

				int col;
				// on, choose randomly (if left side) or mirror			
				if (i <= codeMask->w /2 )
				{
					col = bit ? makecol( 0xff, 0xff, 0xff ) : makecol( 0xff, 0x00, 0xff );
				} else {
					col = getpixel( bot, (codeMask->w-1) - i, j );
				}
				putpixel( bot, i,j, col );
			}
		}
	}

	// Now, outline the bot
	for (int j=0; j < bot->h; j++)
	{
		for (int i=0; i < bot->w; i++)
		{
			// if this pixel is not set, and borders any
			// interior pixel, then set it to border
			if (getpixel(bot, i, j) == makecol( 0xff,0x00,0xff))
			{				
				if ( ((i>0) && (getpixel( bot, i-1,j) == makecol( 0xff, 0xff, 0xff )) ) ||
					 ((i<(bot->w-1)) && (getpixel( bot, i+1,j) == makecol( 0xff, 0xff, 0xff )) ) ||
					 ((j>0) && (getpixel( bot, i,j-1) == makecol( 0xff, 0xff, 0xff )) ) ||
					 ((j<(bot->h-1)) && (getpixel( bot, i,j+1) == makecol( 0xff, 0xff, 0xff )) ) )
				{
					// border pix!
					putpixel( bot, i,j, makecol( 0,0,0 ) );
				}
			}
		}
	}


	// Colorize the bot
	for (int ndx=0; ndx < 3; ++ndx)
	{
		// get this target color
		int targ = makecol( ndx==0?0xff:0x00, ndx==1?0xff:0x00, ndx==2?0xff:0x00 );
		
		// Get the y-extents of this target color
		bool found = false;
		int miny, maxy;
		for (int j=0; j < bot->h; j++)
		{
			for (int i=0; i < bot->w; i++)
			{
				if (getpixel( codeMask, i,j ) == targ )
				{
					if (!found) 
					{
						miny = j;
						maxy = j;
						found = true;
					} else {
						if (j < miny) miny = j;
						if (j > maxy) maxy = j;
					}
				}
			}
		}

		// skip this color code if not present
		if (!found) continue;

		// colorize the robot
		// make a random color according to the robot rules		
		float hue   = ((float)rand() / (float)RAND_MAX) * 360.0;

		for (int j=0; j < bot->h; j++)
		{
			for (int i=0; i < bot->w; i++)
			{
				if ( (getpixel( codeMask, i,j ) == targ ) && 
					 (getpixel( bot, i,j ) == makecol( 0xff, 0xff, 0xff ) ) )
				{
					float sat   = lerp( fabs( (float) i - (bot->w/2) ) / (float)(bot->w/2), 1.0, 0.25 );
					float brite = lerp( (float)( j - miny ) / (float)(maxy - miny), 0.4, 0.8 );
		
					int r,g,b;
					hsv_to_rgb( hue, sat, brite, &r, &g, &b );

					putpixel( bot, i,j, makecol( r,g,b) );
				}
			}
		}		
	}

	return bot;
}