#include <tilemap.h>
#include <mkterrain.h>

Tile *makeTileVariant( Tile *orig, TerrainPally &pal )
{
    Tile *vt = new Tile( *orig );

    // copy tile and re-color
    for (int j=0; j < vt->m_bitmap->h; j++)
    {
        for (int i=0; i < vt->m_bitmap->w; i++)
        {
            int col = getpixel( orig->m_bitmap, i,j );
            int newcol = col;

            // if not trans color, recolor it
            if (col!=makecol( 0xff, 0, 0xff ) )
            {

                // DBG: swap BGR for testing
                newcol = makecol( getb( col ), 0, getg(col) );
                
            }

            putpixel( vt->m_bitmap, i, j, newcol );            
        }        
    }
    
    return vt;
    
}


