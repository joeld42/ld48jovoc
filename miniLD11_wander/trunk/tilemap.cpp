#include <stdio.h>
#include <stdlib.h>
#include <tilemap.h>
#include <math.h>

BITMAP *TileMap::m_waterMaskTile = NULL;
BITMAP *TileMap::m_cursorTile = NULL;
std::vector<Tile*> TileMap::m_tileset;

Tile::Tile( const char *filename, unsigned char code )
{	
	m_code = code;
	if (filename)
	{
		m_bitmap = load_bmp( filename, NULL );
		m_mask = create_bitmap( m_bitmap->w, m_bitmap->h );
		for (int i=0; i < m_bitmap->w; i++)
			for( int j=0; j < m_bitmap->h; j++)
				putpixel( m_mask, i,j, 
					(getpixel(m_bitmap, i, j ) != makecol( 0xff,00,0xff )? 
							makeacol( 0xff,0xff,0xff, 0xff ) : 
							makeacol( 0xff,0x00,0xff, 0x00 ) ) );
	}
	else
	{
		m_bitmap = NULL;
		m_mask = NULL;
	}
}

MapCell::MapCell()
{
    m_hite = 0;   
	m_tile = NULL;
	m_selected = false;
	m_gen = Gen_NONE;
}

TileMap::TileMap( int size ) : 
    m_size( size ), m_selectMap( NULL ), m_wave_offs(0)
{    
	m_map = new MapCell[ size * size ];	

	bub_bmp = NULL;
	do_water = false;
}

TileMap::~TileMap() 
{
	delete [] m_map;
}

void TileMap::init()
{    
    // Load Resources
    if (m_tileset.empty())
    {
        m_cursorTile = load_bmp( "gamedata/tilecursor.bmp", NULL );            
		m_waterMaskTile = load_bmp( "gamedata/tilewater.bmp", NULL );

		// Init tileset		
		Tile *t = new Tile( "gamedata/tiletmpl.bmp", Tile_LAND );		
		m_tileset.push_back( t );		

		t = new Tile( "gamedata/tile_palm.bmp", Tile_OBSTACLE );
		m_tileset.push_back( t );		

		t = new Tile( "gamedata/tile_tree.bmp", Tile_OBSTACLE );
		m_tileset.push_back( t );		
		
		t = new Tile( "gamedata/tile_rock.bmp", Tile_OBSTACLE );
		m_tileset.push_back( t );		

		t = new Tile( "gamedata/tile_rock2.bmp", Tile_OBSTACLE );
		m_tileset.push_back( t );		
	}
    
}

#define WAVE_NUM 30
#define WAVE_MAX 2
void TileMap::draw( BITMAP *buf, int root_x, int root_y, 
				    const std::vector<GameObj*> &objs )
{
	// for waves	
	static std::vector<int> waveTable;
	if (waveTable.empty())
	{
		for (int i=0; i < WAVE_NUM; ++i)
		{
			float t = ((float)i / (float)WAVE_NUM) * (2*M_PI);
			waveTable.push_back( ((int)(sin(t) * WAVE_MAX)) + WAVE_MAX );
		}
	}

	// clear the select map
	if (m_selectMap)
	{
		clear_bitmap( m_selectMap );
	}
    	
    // neat property of iso tilemaps -- x+y coords 
    // equals their vertical "row"
    int maxRow = (m_size-1)*2;    
    for (int row = maxRow; row >= 0; row-- )
    {
        int row_offs = abs( (row+1) - m_size) * 6;        

		// skip off-screen rows
		int testy = root_y + (maxRow-row) * 3;
		if ((testy < -20) || (testy > 260)) continue;		

        for (int j=0; j < m_size; ++j)
        {			
			int i = row - j;			

			if ((i < 0) || (i >= m_size)) continue;

			MapCell &cell = map(i,j);

            cell.sx = root_x + row_offs;
            cell.sy = root_y + (maxRow-row) * 3 - (cell.m_hite*2);
			
			// offscreen?			
            if (( cell.m_tile ) && ( cell.sx >= -10) && (cell.sy < 320) )
            {              						
				// draw tile
				BITMAP *tileBmp = cell.m_tile->m_bitmap;						
				masked_blit( tileBmp, buf, 0, 0, 
							 cell.sx, cell.sy,
                             tileBmp->w, tileBmp->h );

				// draw selection cursor
                if (cell.m_selected)
                {                            
                    masked_blit( m_cursorTile, buf, 0, 0, 
                                 cell.sx, cell.sy,
                                 m_cursorTile->w, m_cursorTile->h );
                }

				// draw water mask
				if (do_water)
				{
					if ( ((i > 0) && (map(i-1,j).m_tile == NULL) ) ||
						 ((j > 0) && (map(i,j-1).m_tile == NULL) ) )
					{
						int ndx = (m_wave_offs + row_offs) % waveTable.size();
						masked_blit( m_waterMaskTile, buf, 0, 0, 
							         cell.sx-1, 
									 cell.sy + (cell.m_hite*2) + waveTable[ndx] -1,
									 m_waterMaskTile->w, m_waterMaskTile->h );
					}
				}

				// draw into the select map
				if (m_selectMap)
				{
					int col = encodeTilePos( i, j );
					set_trans_blender( getr( col ), getg(col), getb(col), 0x00 );
				
					BITMAP *maskBmp = cell.m_tile->m_mask;
				
					//draw_sprite( m_selectMap, maskBmp, m_map[i][j].sx, m_map[i][j].sy );
					draw_lit_sprite( m_selectMap, maskBmp,
									cell.sx, cell.sy, 0xff );			
				}

				// draw any objects on this tile
				for (int ondx = 0; ondx < objs.size(); ++ondx )
				{
					GameObj *o = objs[ondx];
					if (o->m_bmp && (o->m_x == i) && (o->m_y == j))
					{
						draw_sprite( buf, o->m_bmp, cell.sx+1, cell.sy );
					}
				}
            }					
			row_offs += 12;
        }        
    }	          
}

MapCell &TileMap::map( int i, int j )
{
	return m_map[ j*m_size + i ];
}

void TileMap::reset()
{
    for (int i=0; i < m_size; ++i)
    {        
        for (int j=0; j < m_size; ++j)
        {			
			MapCell &cell = map(i, j );

			cell.m_tile = NULL;

            // DBG 
			//if ( ((i&8)!=0) == ((j&8)!=0)) cell.m_tile = m_tileset[0];
			
			cell.m_hite = 0;
        }
    }

#if 0    
    for (int i=0; i < 500; i++)
    {
        int x = 1 + (rand()  % (m_size-2));
        int y = 1 + (rand()  % (m_size-2));

        // check neighbors
        if ( (!map(x-1,y).m_tile ) ||
             (!map(x+1, y).m_tile ) ||
             (!map(x, y-1).m_tile ) ||
             (!map(x,y+1).m_tile ) )
        {
            continue;            
        }
        if (map(x,y).m_hite < 5)
        {
            map(x,y).m_hite++;            
        }        
    }
#endif

#if 0
    for (int j=0; j < m_size; ++j)    
    {
        
        for (int i=0; i < m_size; ++i)
        {
            if (m_map[i][j].m_code == Tile_LAND)
            {
                printf("#");                
            }
            else
            {
                printf(".");                
            }            
        }
        printf("\n");            
    }
#endif    

}

void TileMap::enableSelect( BITMAP *buf )
{
    m_selectMap = create_bitmap( buf->w, buf->h );    
    clear_bitmap( m_selectMap );        
}


bool TileMap::screenToMap( int sx, int sy, int &mx, int &my )
{
    if (m_selectMap != NULL)
    {        
		decodeTilePos( getpixel( m_selectMap, sx, sy ), mx, my );
		if ((mx > 0) && (my > 0 ))
		{
			mx--; my--;
			return true; // on a tile
		}
		else
		{
			return false; // not on a tile
		}
    }
	return false;
}

void TileMap::mapToScreen( int mx, int my, int &sx, int &sy )
{
	MapCell &cell = map(mx,my);
    sx = cell.sx;
    sy = cell.sy;
}

    
int TileMap::encodeTilePos( int x, int y )
{
	int r,g,b;
	x += 1; y += 1;

	r = x & 0xff;
	g = y & 0xff;
	b = ( ((x & 0xf00) >> 8) | ((y & 0xf00) >> 4));
	return makecol( r,g,b );
}

void TileMap::decodeTilePos( int col, int &x, int &y )
{
	int r,g,b;
	r = getr( col );
	g = getg( col );
	b = getb( col );

	x = ((b & 0x0f) << 8) | r;
	y = ((b & 0xf0) << 4) | g;
}

void TileMap::clearSelected()
{
	for (int i=0; i < m_size*m_size; ++i)
		m_map[i].m_selected = false;
}


void TileMap::paste( TileMap *other, int x0, int y0 )
{
	for (int i=0; i < other->m_size; i++)
	{
		for (int j=0; j < other->m_size; j++)
		{
			if (other->map(i,j).m_tile)
			{
				//printf("%d", other->map(i,j).m_hite );
				map(x0+i, y0+j) = other->map(i,j);
			}
			//else
			//{
			//	printf("." );
			//}			
		}
		//printf("\n");
	}
}