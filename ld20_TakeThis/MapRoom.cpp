//
//  MapRoom.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/30/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#ifndef WIN32
#  include <dirent.h>
#else
#  include <windows.h>
#endif


#include <prmath/prmath.hpp>

#include "MapRoom.h"

std::map<std::string,VoxChunk*> MapRoom::m_tileset;
VoxChunk *MapRoom::m_octorok;

// note: zelda map size is 16x11
MapRoom::MapRoom( int x, int y, int z) :
    m_xSize(x),
    m_ySize(y),
    m_zSize(z)
{
    m_map.resize( x*y*z );
    
    buildMap( MAP_START_ZONE );
    //buildMap( MAP_CAVE );
    
}

void MapRoom::drawSlab( int x0, int y0, int z0,
                       int x1, int y1, int z1,
                       VoxChunk *mid,
                       VoxChunk *side,
                       VoxChunk *corner )
{
    
    
    for (int i=x0; i < x1; i++)
    {
        for (int j=z0; j < z1; j++)
        {
            MapTile &t = m_map[index(i,y0,j)];
            t.chunk = mid;
            t.rot = (rand() % 4 ) * 90;
        }
    }
    
    for (int i=x0; i < x1; i++)
    {
        MapTile &t = m_map[index(i,y0,z0)];
        t.chunk = side;
        t.rot=90;
        
        MapTile &t2 = m_map[index(i,y0,z1-1)];
        t2.chunk = side;
        t2.rot=270;
    }
    
    for (int i=z0; i < z1; i++)
    {
        MapTile &t = m_map[index(x0,y0,i)];
        t.chunk = side;
        t.rot=180;
        
        MapTile &t2 = m_map[index(x1-1,y0,i)];
        t2.chunk = side;
        t2.rot=0;
    }
    
    

    m_map[index(x0,y0,z0)].chunk = corner;
    m_map[index(x0,y0,z0)].rot = 90;
    
    m_map[index(x1-1,y0,z0)].chunk = corner;
    m_map[index(x1-1,y0,z0)].rot = 0;
    
    m_map[index(x0,y0,z1-1)].chunk = corner;
    m_map[index(x0,y0,z1-1)].rot = 180;
    
    m_map[index(x1-1,y0,z1-1)].chunk = corner;
    m_map[index(x1-1,y0,z1-1)].rot = 270;

    
}

size_t MapRoom::index( int x, int y, int z ) const
{
    return (z*m_xSize*m_ySize)+(y*m_xSize) + x;
}

// load the tileset
void MapRoom::initTiles()
{

#ifndef WIN32
    DIR *dp;
    struct dirent *dirp;


    if((dp  = opendir("gamedata/voxtiles")) == NULL) {
        printf("Can't open tiles dir gamedata/voxtiles\n" );
        exit(1);
    }
#else
	WIN32_FIND_DATAA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFileA( "gamedata/voxtiles/*", &ffd );

#endif

#ifndef WIN32
	while ((dirp = readdir(dp)) != NULL) {
        char *tilename = dirp->d_name;
#else    
	do
	{
		char *tilename = ffd.cFileName;
#endif
        
        char buff[1024];
        sprintf( buff, "gamedata/voxtiles/%s", tilename );
        
        std::string tileId = tilename;
        std::string tileExt;
        
        size_t extPos = tileId.rfind( '.' );
        if (extPos != std::string::npos)
        {
            tileExt = tileId.substr( extPos );
            tileId = tileId.substr( 0, extPos );
              
			if (tileExt != ".csv") continue;
        
			printf("Load tile %s for tile id %s\n", 
				buff, tileId.c_str() );
			VoxChunk *tile = VoxChunk::loadCSVFile( buff );
			tile->m_chunkName = tileId;
        
			// HACK the floor height
			if ( (tileId.find( "overland" ) != std::string::npos) ||
				 (tileId.find( "cave" ) != std::string::npos) )
			{
				tile->m_floorHeight = 1.0;
			}
        
			if (tileId.find( "stairs" ) != std::string::npos)
			{
				tile->m_floorHeight = 0.5;
			}        

			m_tileset[ tileId ] = tile;
		}
#ifndef WIN32
    }
    closedir(dp);
#else
	} 
	while (FindNextFileA( hFind, &ffd) != 0 );

	FindClose( hFind );
#endif

    
    // Now load enemies
    m_octorok = VoxChunk::loadCSVFile( "gamedata/octaroc.csv" );
    if (!m_octorok)
    {
        printf("FAIL to load octaroc\n" );
    }
    
    
}

size_t MapRoom::instMapGeo( VoxVert *dest, size_t maxNumVert )
{
    VoxVert *curr = dest;
    size_t currSize = 0;
    
    printf( "instMapGeo, size is %d %d %d\n", m_xSize, m_ySize, m_zSize );
    
    matrix4x4f center, centerI;
    center.Translate( -0.5, -0.5, -0.5 );
    centerI.Translate( 0.5, 0.5, 0.5);
    
    for (int i=0; i < m_xSize; i++)
    {
        for (int j=0; j < m_ySize; j++)
        {
            for (int k=0; k < m_zSize; k++)
            {
                size_t tileSz;
                VoxVert *vtile;
                
                size_t ndx = index(i,j,k);
                
                
                // check if is surrounded
                bool isSurround = false;
                if ((i>0) && (i < m_xSize-1) &&
                             (j < m_ySize-1) &&
                    (k>0) && (k < m_zSize-1))
                {
                    isSurround = true;
                    for (int ii=-1; ii<= 1; ii++)
                    {
                        for (int jj=0; jj <= 1; jj++)
                        {
                            for (int kk=-1; kk <= 1; kk++)
                            {
                                size_t ndx2 = index(i+ii,j+jj, k+kk);
                                if (!m_map[ndx2].chunk)
                                {
                                    isSurround = false;
                                    break;
                                }
                            }
                            if (!isSurround) break;
                        }
                        if (!isSurround) break;
                    }
                }
                if (isSurround) continue;
                    
                
                //printf("Ndx %zu size %zu\n", ndx, m_map.size() );
                MapTile &t = m_map[ndx];
                if (!t.chunk) continue;
                
                VoxChunk *chunk = t.chunk;
                vtile = chunk->genTris( tileSz );
                
                if (tileSz + currSize >= maxNumVert)
                {
                    // oops buffer is full
                    printf("WARNING: map buffer full (%zu verts)\n",
                           currSize );
                    return currSize;
                }
                //printf("Adding tile %d,%d,%d size %zu\n", i,j,k, tileSz );
                
                // copy tile int dest
                memcpy( curr, vtile, sizeof(VoxVert)*tileSz );
                
                // now go translate the tile to it's final pos
                matrix4x4f  xlate, rot, m;
                xlate.Translate( i, j, k );
                rot.RotateY( t.rot * M_PI/180.0 );
                
                m = center * rot * xlate * centerI;
                
                for (int vi=0; vi != tileSz; vi++)
                {
                    //curr[vi].m_pos += vec3f( i,j,k );
                    curr[vi].m_pos *= m;
                }
                
                // incr. the end of buff
                curr += tileSz;
                currSize += tileSz;
            }
        }
    }
    
    return currSize;
}

bool MapRoom::isVacant( float x, float y, float z ) const
{
    // world bounds??
    if ( (x < 0) || (x > m_xSize) ||
         (y < 0) || (y > m_ySize) ||
         (z < 0) || (z > m_zSize) )
    {
        //out of bounds
        return false;
    }
    
    // get map index
    int xx, yy, zz;
    xx = floor( x );
    yy = floor( y );
    zz = floor( z );
        
    //printf("index %d %d %d\n", xx, yy, zz );
    const MapTile &t = m_map[index(xx,yy,zz)];
    if (t.chunk)
    {
        if (y - floor(y) < t.chunk->m_floorHeight)
        {
            return false;
        }
    }
    
    // it's open
    return true;
}

float MapRoom::groundHeight( float x, float z ) const
{
    // world bounds??
    if ( (x < 0) || (x > m_xSize) ||
         (z < 0) || (z > m_zSize) )
    {
        //out of bounds
        return 0.0;
    }   
    
    // find the top tile at this location
    int yy = m_ySize-1;
    int xx, zz;
    xx = floor( x );
    zz = floor( z );
    
    while ((yy > 0) &&  (!m_map[index(xx,yy,zz)].chunk))
    {
        yy -= 1;
    }
    
    // Did we hit the bottom of the map?
    if ((yy==0)&& (!m_map[index(xx,yy,zz)].chunk))
    {
        return 0.0;
    }
    const MapTile &t = m_map[index(xx,yy,zz)];
    
    // height within tile
    float tileHeight = 1.0;
    
#if 1
    // HACK
    tileHeight = t.chunk->m_floorHeight;
    
#else
    int ii = (x - floor(x)) * WORLD_TILE_SIZE;
    int kk = (z - floor(z)) * WORLD_TILE_SIZE;
    
    //printf("tile %s pos %d %d\n", 
    //       t.chunk->m_chunkName.c_str(), ii, kk );
    
    // todo: handle rotate
    int jj = t.chunk->m_ySize;
    
    while (jj > 0)
    {
        jj -= 1;
        GLuint col = t.chunk->getVoxel( ii, jj, kk );
        
        if (!t.chunk->isClearCol(col)) break;
    }
    tileHeight = float(jj) / float(WORLD_TILE_SIZE);
#endif
    
    return float(yy) + tileHeight;
}

void MapRoom::clearMap()
{
    for (int i=0; i < m_xSize; i++)
    {
        for (int j=0; j < m_ySize; j++)
        {
            for( int k=0; k < m_zSize; k++)
            {
                m_map[index(i,j,k)] = MapTile();
            }
        }
    }
    
    m_enemies.clear();
    m_items.clear();
    
    m_message1 = "";
    m_message2 = "";
}

void MapRoom::buildMap( int map )
{
    clearMap();
    m_mapCode = map;
    
    switch (map)
    {
        case MAP_START_ZONE:
            buildMap_StartRoom( false );
            break;
            
        case MAP_START_OPEN:
            buildMap_StartRoom( true );
            break;
            
            
        case MAP_CAVE_COLD:
        case MAP_CAVE_DANCE:
        case MAP_CAVE_SWORD:
            buildMap_Cave( map);
            break;
            
        default:
            break;
    }
}

void MapRoom::buildMap_StartRoom( bool unlocked )
{
    // DBG: fill in some tiles
    VoxChunk *ground = m_tileset["overland_ground_open"];
    VoxChunk *ground2 = m_tileset["overland_ground_side"];
    VoxChunk *ground3 = m_tileset["overland_ground_corner"];
    VoxChunk *bush = m_tileset["overland_tree"];
    VoxChunk *cave = m_tileset["overland_cave"];
    VoxChunk *stairs = m_tileset["overland_stairs"];
    VoxChunk *rock = m_tileset["overland_rock"];
    //VoxChunk *col1 = m_tileset["column"];
    //VoxChunk *col2 = m_tileset["column_vines"];
    //VoxChunk *col3 = m_tileset["column_ruins"];
    //VoxChunk *tree = m_tileset["juniper"];
    
    m_bgColor = vec3f( 0.0, 0.0, 0.5 );
   
    
    printf("Ground chunk is %p name %s\n", ground, ground->m_chunkName.c_str() );
    printf("Bush chunk is %p name %s\n", bush, bush->m_chunkName.c_str() );
    
    m_map[index(m_xSize-1,1, m_zSize-1)].chunk = bush;
    
    drawSlab( 0, 0, 0,m_xSize, 1, m_zSize, ground, ground2, ground3 );
    drawSlab( 2, 1, 0, m_xSize-2, 2, 6, ground, ground2, ground3 );
    drawSlab( 2, 2, 0, 8, 2, 4, ground, ground2, ground3 );
    drawSlab( 2, 3, 0, 4, 2, 4, ground, ground2, ground3 );
    drawSlab( 2, 1, 7, 4, 2, 10, ground, ground2, ground3 );
        
    srand( 8 );
    int treePos[10][2] = { {8, 7},
        {3, 5},
        {0, 6},
        {15, 1},
        {6, 7},
        {12, 3},
        {3, 10},
        {13, 4},
        {6, 2},
        {5, 4} };
    
    for (int i=0; i < 10; i++)
    {
        int x,y,z;
        //x = rand() % m_xSize;
        //z = rand() % m_zSize;
        x = treePos[i][0];
        z = treePos[i][1];
        y=0;
        
        //printf( "{%d, %d},\n", x, z );
        
        while ( m_map[index(x,y,z)].chunk )
        {
            y++;
            if (y >= m_ySize) break;
        }
        MapTile &t = m_map[index(x,y,z)];
        t.chunk = bush;
        t.rot = (rand() % 4 ) * 90;
    }
    
    // barriers or passages to other place
    for (int z=0; z < m_zSize; z++)
    {
        if (!m_map[index(0,1,z)].chunk)
        {
            m_map[index(0,1,z)].chunk = rock;
        }
        
        if (!m_map[index(m_xSize-1,1,z)].chunk)
        {
            m_map[index(m_xSize-1,1,z)].chunk = rock;
        }

    }
    
    
    // lower cave
    m_map[index(5,1,5)].chunk = cave;
    m_map[index(5,1,5)].rot = 270;
    m_map[index(5,1,6)].teleport = true;
    m_map[index(5,1,6)].teleWhere = MAP_CAVE_X;
    m_map[index(5,1,6)].telePos = vec3f( m_xSize/2, 1, m_zSize - 2 );
    
    m_map[index(7,1,6)].chunk = stairs;
    m_map[index(7,1,6)].rot = 0;
    
    // middle cave
    m_map[index(6,2,3)].chunk = cave;
    m_map[index(6,2,3)].rot = 270;
    m_map[index(6,2,4)].teleport = true;
    m_map[index(6,2,4)].teleWhere = MAP_CAVE_X;
    m_map[index(6,2,4)].telePos = vec3f( m_xSize/2, 1, m_zSize - 2 );

    
    
    // upper cave
    m_map[index(3,3,1)].chunk = cave;
    m_map[index(3,3,1)].rot = 0;
    m_map[index(4,3,1)].teleport = true;
    m_map[index(4,3,1)].teleWhere = MAP_CAVE_X;
    m_map[index(4,3,1)].telePos = vec3f( m_xSize/2, 1, m_zSize - 2 );
    
    m_map[index(8,2,2)].chunk = stairs;
    m_map[index(8,2,2)].rot = 90;
    
    // Add some enemies
    m_enemies.push_back( VoxSprite( m_octorok ) );
    m_enemies[0].m_pos = vec3f( 9, 1, 8 );
    
    m_enemies.push_back( VoxSprite( m_octorok ) );
    m_enemies[1].m_pos = vec3f( 4, 2, 4 );
    
    m_playerStartPos = vec3f( m_xSize/2, 1, m_zSize - 2 );
    
}

void MapRoom::buildMap_Cave( int code )
{
    VoxChunk *ground = m_tileset["cave_ground_open"];
    VoxChunk *ground2 = m_tileset["cave_wall_side"];
    VoxChunk *ground3 = m_tileset["cave_wall_corner"];
    VoxChunk *bush = m_tileset["overland_tree"];
    VoxChunk *cave = m_tileset["overland_cave"];
    VoxChunk *stairs = m_tileset["overland_stairs"];
    VoxChunk *rock = m_tileset["overland_rock"];
    VoxChunk *fire = m_tileset["cave_fire"];
    VoxChunk *oldguy = m_tileset["cave_oldguy"];

    m_bgColor = vec3f( 0.0, 0.0, 0.0 );
    
    drawSlab( 0, 0, 0, m_xSize, 1, m_zSize, ground, ground2, ground3 );
    
    for (int x=0; x < m_xSize; x++)
    {
        for (int y=0; y < 3; y++)
        {
            MapTile &t = m_map[index(x,1+y,0 )];
            t.chunk = ground2;
            t.rot = 270;
        }
        
        if ((x<7)||(x>9))
        {
            MapTile &t = m_map[index(x,1, m_zSize-1 )];
            t.chunk = ground2;
            t.rot = 270;
        }
    }
    m_map[index(6,1,m_zSize-1)].chunk = ground3;
    
    m_map[index(10,1,m_zSize-1)].chunk = ground3;
    m_map[index(10,1,m_zSize-1)].rot = 180;
    
    for (int z=0; z < m_zSize; z++)
    {
        MapTile &t = m_map[index(0,1,z )];
        t.chunk = ground2;
        t.rot = 0;
        
        MapTile &t2 = m_map[index(m_xSize-1,1,z )];
        t2.chunk = ground2;
        t2.rot = 180;
    }
    
    drawSlab( 0, 1, 0, 3, 2, 4, ground, ground2, ground3 );
    drawSlab( 0, 2, 0, 3, 2, 4, ground, ground2, ground3 );
    
    m_playerStartPos = vec3f( m_xSize/2, 1, m_zSize - 2 );
    
       
    VoxChunk *c = NULL;
    if (code==MAP_CAVE_COLD)
    {
        c = rock;
        m_message1 = "IT'S DANG COLD IN";
        m_message2 = "HERE. BRRRRRRRRRR.";
    }
    else if (code==MAP_CAVE_DANCE)
    {
        c = bush;
        m_message1 = "IT'S DANCING TIME!";
        m_message2 = "";
    }
    else if (code==MAP_CAVE_SWORD)
    {
        c = fire;
        
        m_message1 = "IT'S DANGEROUS TO GO";
        m_message2 = "ALONE! TAKE THIS.";
    }
    
    m_map[index(4,1,5)].chunk = c;
    m_map[index(12,1,5)].chunk = c;
    
    m_map[index(8,1,4)].chunk = oldguy;
    m_map[index(8,1,4)].rot = 270;
    
    // the way back
    for (int x=7; x <=9; x++)
    {
        m_map[index(x,1, m_zSize-1 )].teleport = true;
        m_map[index(x,1, m_zSize-1 )].teleWhere = MAP_START_ZONE;
        m_map[index(x,1, m_zSize-1 )].telePos = vec3f( 4.5, 1.0, 6.5);
    }

    
}