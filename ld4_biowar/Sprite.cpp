#include "sprite.h"
#include "Map.h"

SpriteGraphic::SpriteGraphic( char *filename ) 
{
	char fn[1000];
	sprintf( fn, "Graphics/%s.bmp", filename );
	load_and_chop( fn );	
}

SpriteGraphic::~SpriteGraphic()
{
	for (std::vector<BITMAP*>::iterator fi = m_frames.begin();
		fi != m_frames.end(); fi++) 
		{
			destroy_bitmap( *fi);
		}
}

void SpriteGraphic::load_and_chop( char *filename )
{
	BITMAP *sprite, *frame;

	sprite = load_bitmap( filename, NULL );
	int nframe = sprite->w / sprite->h;
	for (int i=0; i < nframe; i++) {
		frame = create_bitmap( sprite->h, sprite->h );
		blit( sprite, frame, i*sprite->h, 0, 0,0, sprite->h, sprite->h );
		m_frames.push_back( frame );
	}

	destroy_bitmap( sprite );
}

////////////////////////////////////////////

Sprite::Sprite( SpriteGraphic *gfx ) : m_gfx(gfx )
{
	m_x = 0; m_y = 0;
	m_frame = 0;
	m_ang = 0;
}

void Sprite::draw( BITMAP *target ) 
{
	BITMAP *f = m_gfx->m_frames[m_frame];
	int y = Map::mapYtoScreenY( m_y );
	//masked_blit( f, target, 0,0,  m_x, y, f->w, f->h );
	draw_sprite( target, f, m_x, y );
}