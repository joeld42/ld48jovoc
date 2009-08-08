#ifndef MAKE_TERRAIN_H
#define MAKE_TERRAIN_H

#include <tilemap.h>

struct TerrainPally
{
    const char *desc;    
    int color[5];    
};

Tile *makeTileVariant( Tile *orig, TerrainPally &pal );


#endif




