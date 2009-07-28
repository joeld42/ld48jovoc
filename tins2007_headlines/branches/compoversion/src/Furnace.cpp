#include <iostream>
#include <allegro.h>
#include <string>
#include <math.h>

#include "furnace.h"

#define FIRE_SZ 32
#define NOISE_SZ 256

using namespace std;

Furnace::Furnace( std::string name, int pcol, int x, int y, int w, int h ) :
	Panel( name, pcol, x, y, w, h ), offs_t( 0.0f )
{
	// Make fire bitmap2
	m_fire_fx = create_bitmap_ex( 8, FIRE_SZ, 2*FIRE_SZ );
	m_fire_fx2 = create_bitmap_ex( 8, FIRE_SZ, 2*FIRE_SZ );

	// final truecolor bitmap
	m_fire_final = create_bitmap( FIRE_SZ, 2*FIRE_SZ );

	int old_cc = get_color_conversion();
	set_color_conversion( COLORCONV_NONE );

	// load the noise pattern
	m_fire_noise = load_bitmap( "gamedata/fire_noise.bmp", NULL );	

	// load the palette for the fire bitmap
	m_fire_pal_bmp = load_bitmap( "gamedata/fire_pal.bmp", m_fire_pal );	

	set_color_conversion( old_cc );
}

void Furnace::draw()
{	
	stretch_blit( m_fire_final, m_surf, 0, 0, FIRE_SZ, 2*FIRE_SZ,
					0, 0, m_w, m_h );

	//blit (m_fire_noise, m_surf, 0, 0, 0, 0, 256, 256 );
}

void Furnace::update( float dt )
{
	// update fire
	acquire_bitmap( m_fire_fx );
	static bool init = true;

	// update the bottom line of the fire
	if (init)
	{
		for (int i = 0; i < FIRE_SZ; i++)
		{		
			_putpixel( m_fire_fx, i, (2*FIRE_SZ)-1, rand() % 256 );
		}
		init = false;
	}
	else
	{
		const int amt = 80; // Must be even
		for (int i = 0; i < FIRE_SZ; i++)
		{		
			int pxl = getpixel( m_fire_fx, i, (2*FIRE_SZ)-1 );
			pxl += ( (rand() % amt) - (amt/2) );
			if (pxl < 0 ) pxl = 0;
			if (pxl > 255 ) pxl = 255;
			_putpixel( m_fire_fx, i, (2*FIRE_SZ)-1, pxl );
		}
	}

	// blurify the rest of the bitmap	
	for (int j=FIRE_SZ*2-2; j >=0; j--)
	{
		for (int i = 0; i < FIRE_SZ; i++)
		{
			int pix = _getpixel( m_fire_fx, i, j ) * 2;
			pix += _getpixel( m_fire_fx, i, j+1 );
			if (i > 0) pix += _getpixel( m_fire_fx, i-1, j+1 );
			if (i < FIRE_SZ-1) pix += _getpixel( m_fire_fx, i+1, j+1 );
			pix /= 5;			
			
			const int amt = 5; // add noise to fire
			const float coolant = 0.75f; // higher numbers cool faster
			pix += ( (rand() % amt) - (int)((float)(amt)*coolant) );
			if (pix < 0 ) pix = 0;
			if (pix > 255 ) pix = 255;

			_putpixel( m_fire_fx, i, j,  pix );		
		}
	}
	
	release_bitmap( m_fire_fx );

	// Update noise offsets
	offs_t += dt;
	offs1x = 1024 + (int)( ((sin( offs_t * 0.08f ) * 512.0f)  + sin(offs_t * 0.1) * 32.0f) );
	offs1y = 2048 + (int)( ((cos( offs_t*  0.08f ) * 1024.0f) + cos(offs_t * 0.1) * 32.0f) );

	offs2x = 512 + (int)(sin( offs_t * 0.1f ) * 512.0f);
	offs2y = 512 - (int)(cos( offs_t*  0.09f ) * 512.0f);

	// Apply noise pattern
	acquire_bitmap( m_fire_fx2 );	
	for (int j=0; j < FIRE_SZ*2; j++)
	{
		for (int i=0; i < FIRE_SZ; i++)
		{
			int ii, jj;
			ii = i + offs1x;
			jj = j + offs1y;
			int n1 = _getpixel( m_fire_noise, ii % NOISE_SZ, jj % NOISE_SZ );			

			ii = i + offs2x;
			jj = j + offs2y;
			int n2 = _getpixel( m_fire_noise, ii % NOISE_SZ, jj % NOISE_SZ );			
			
			unsigned int p0 = _getpixel( m_fire_fx, i,j ); // firey fire

			unsigned int p = (n1 * n2) >> 8;
			p =( ((p0 * p) >> 5) + (p0 >> 1));
			if (p > 0xff) p = 0xff;
			
			putpixel( m_fire_fx2, i,j, p );
		}
	}
	release_bitmap( m_fire_fx2 );	

	// Convert to truecolor
	set_palette( m_fire_pal );
	blit( m_fire_fx2, m_fire_final, 0,0,0,0, FIRE_SZ, FIRE_SZ*2 );
}

bool Furnace::dropClip( Clipping *clip, int x, int y )
{
	// we'll take the clip, so we can kill it
	delete( clip );

	int fx, fy;
	fx = ((float)x / (float)m_w) * FIRE_SZ;
	fy = ((float)y / (float)m_h) * FIRE_SZ*2;
	cout << "BURNINATE CLIP " << fx << " " << fy << endl;	

	// make some fire 
	acquire_bitmap( m_fire_fx );
	circlefill( m_fire_fx, fx, fy, 5, 255 );
	release_bitmap( m_fire_fx );

	// we'll take it
	return true;
}