#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include <crtdbg.h>
#include <assert.h>
#include <allegro.h>

#define ONE_OVER_SQRT2 (0.707106781186547)

#define LERP(a,b,t) ( ((b)*t) + ((a)*(1.0-t)) );

char *buildHints[] = {
	"Place land tiles on the map",
	"Land must touch existing land",
	"Roads must line up",
	NULL
};
int buildHintNdx = 0;

char *turnMsg[] = {
	"Grasslanders",
	"Seafarers",
	"Badlanders"
};

/* Global graphics stuff */
RGB pally[256];
BITMAP *tiles;
BITMAP *tile_template;
BITMAP *background;
BITMAP *cursor;
BITMAP *redgreentile;

/* game stuff */
enum { 
	EMPTY,
		
	PLAINS,
	BADLANDS,
	WATER
};

enum {
	ROAD_N = 0x08,
	ROAD_S = 0x04,
	ROAD_E = 0x02,
	ROAD_W = 0x01,

	ROAD_CITY = 0x10
};

// drawstyle
enum {
	DS_PLAIN,
	DS_RED,
	DS_GREEN
};


enum {
	ACT_IDLE,
	ACT_PLACE_TILE,
	ACT_PLACE_BUILDING
};

struct Player {	
	Player() {
		land = EMPTY;
		buildingsLeft = 5;
		score = 0;
		color = 11;

		for (int i=0; i < 5; i++) {
			regionCount[i] = 0;
			regionConn[i] = 0;			
		}
	}
	int land;
	int buildingsLeft;

	int color;
	int score;

	int regionCount[5], regionConn[5];
};
Player player[3];
int currPlayerNdx;

struct Tile {
	int terrain;
	int roads;
};

struct TileBlock {
	TileBlock (int x, int y, int xo, int yo ) {
		m_xsz = x;
		m_ysz = y;
		m_xo = xo;
		m_yo = yo;
		m_drawstyle = DS_PLAIN;		

		for (int j=0; j < 20; j++) {
			for (int i=0; i < 20; i++) {

				m_tile[i][j].terrain = EMPTY;
#if 0
				if ( sqrt((i-10)*(i-10) + (j-10)*(j-10) ) < 5 ) {
					m_tile[i][j].terrain = 1 + (rand() % 3);
					m_tile[i][j].roads = (rand() & 0x40)?rand() % 16:0;
				}
#endif

			}
		}
	}

	TileBlock ( const TileBlock &other ) {
		*this = other;
	}

	const TileBlock &operator=( const TileBlock &other ) {
		m_xsz = other.m_xsz;
		m_ysz = other.m_ysz;
		m_xo = other.m_xo;
		m_yo = other.m_yo;
		m_srcXo = other.m_srcXo;
		m_srcYo = other.m_srcYo;
		m_destXo = other.m_destXo;
		m_destYo = other.m_destYo;		

		for (int j=0; j < 20; j++) {
			for (int i=0; i < 20; i++) {				
					m_tile[i][j].terrain = other.m_tile[i][j].terrain;
					m_tile[i][j].roads = other.m_tile[i][j].roads;
			}
		}

		return *this;
	}

	int getTerrain( int x, int y ) {
		if ( (x>=0) && ( x < m_xsz) &&
			(y>=0) && ( y < m_ysz) ) {
			return m_tile[x][y].terrain;
		}
		else return EMPTY;
	}

	void drawIsoMap( BITMAP *dest );
	void drawSquare( BITMAP *dest, int m, int n );

	void mapToScreen( int mx, int my, int &sx, int &sy );
	void screenToMap( int sx, int sy, int &mx, int &my );

	void rotate90();

	int m_drawstyle;
	
	int m_xsz, m_ysz, // size
		m_xo, m_yo;	  // screen origin

	int m_destXo, m_destYo; // for when the tiles are sliding
	int m_srcXo, m_srcYo;
	int m_destNdx;	

	Tile m_tile[20][20];	
};

TileBlock *worldMap;
std::vector<TileBlock*> gameTiles, roadTiles, pickTiles;

TileBlock *chosenTile = NULL;

TileBlock validMove( 3,2, 0,0);
int showValidMove = 0;

int chosenOrigX, chosenOrigY;

bool doubleTile = false;
bool didDoubleTile = false;
bool firstMove = true;
int currAct;
bool placementValid = false;

int build_sx, build_sy;

char visited[20][20];
void clearVisited();

bool gameOver = false;

// X positions of tiles on the screen
int tile_x_pos[] = {
	70, 198, 328, 464, 592, 722
};

bool do_slide = false;
float slide_t = 0.0;

char statusMsg[1000];
int statusMsgColor = 0;
float statusMsgColorF = 0.0;

void ShowMessage( const char *msg ) {
	strcpy (statusMsg, msg );
	statusMsgColor = 176;
	statusMsgColorF = 176.0;
}

void TileBlock::drawSquare( BITMAP *dest, int m, int n )
{
	int sx, sy, tilex, tiley;

	//_RPT2( _CRT_WARN, "Drawsquare %d %d\n", m, n );
	
	switch (m_tile[m][n].terrain) {
	case EMPTY:
		return;
	case PLAINS:
		tiley = 0;
		break;
	case WATER:
		tiley = 30;
		break;
	case BADLANDS:
		tiley = 60;
		break;
	}

	if (m_tile[m][n].roads & ROAD_CITY) {
		tilex = 16*40;
	} else {
		tilex = m_tile[m][n].roads * 40;
	}

	mapToScreen( m, n, sx, sy );

	masked_blit( tiles, dest, tilex,tiley, sx-20, sy-30, 40, 30 );
	if (m_drawstyle == DS_RED ) {
		masked_blit( redgreentile, dest, 0,0, sx-20, sy-20, 40, 20 );
	} else if (m_drawstyle == DS_GREEN ) {
		masked_blit( redgreentile, dest, 40,0, sx-20, sy-20, 40, 20 );
	}
}

void TileBlock::mapToScreen( int mx, int my, int &sx, int &sy )
{
	float x, y, x1, y1;
	x = (float)mx; y = (float)my;
	x1 = (ONE_OVER_SQRT2 * x) - (ONE_OVER_SQRT2 * y);
	y1 = (ONE_OVER_SQRT2 * x) + (ONE_OVER_SQRT2 * y);

	y1 *= (20.0 * ONE_OVER_SQRT2);
	x1 *= (40.0 * ONE_OVER_SQRT2);

	sx = (int)x1 + m_xo;
	sy = (m_yo - (int)y1);
}

void TileBlock::screenToMap( int sx, int sy, int &mx, int &my )
{
	float x, y, x1, y1;
	x = (float)(sx - m_xo);
	y = (float)(m_yo - sy);

	y /= (20.0 * ONE_OVER_SQRT2);
	x /= (40.0 * ONE_OVER_SQRT2);

	x1 = (ONE_OVER_SQRT2 * x) + (ONE_OVER_SQRT2 * y);
	y1 = -(ONE_OVER_SQRT2 * x) + (ONE_OVER_SQRT2 * y);

	mx = (int)(x1);
	my = (int)(y1);
}

void TileBlock::drawIsoMap( BITMAP *dest )
{
	int startN, startM, m, n;
	

	// do it by hand
	if ((m_xsz==2) &&(m_ysz==3)) {
		drawSquare( dest, 1, 2 );
		drawSquare( dest, 0, 2 );
		drawSquare( dest, 1, 1 );		
		drawSquare( dest, 0, 1 );
		drawSquare( dest, 1, 0 );
		drawSquare( dest, 0, 0 );
	}

	startN = m_ysz-1;	
	while (startN>=0) {
		m = m_xsz-1; 		
		n = startN;	
		
		while ((m>=0)&&(n<m_ysz)) {						
			drawSquare( dest, m, n );

			n++;
			m--;
			
			if (n==m_xsz) break;
		}	
	startN--;
	}
	
	startM = m_xsz-1;
	while (startM>=0) {
		m = startM;
		n = 0;
		while ((m>=0) && ( n<m_ysz)) {				
				drawSquare( dest, m, n );
			n++;
			m--;
			
			if (n==m_ysz) break;
		}	
	startM--;
	}
}

void TileBlock::rotate90() {
	TileBlock rot( *this );
	int i,j, oldRoad, road;

	rot.m_xsz = m_ysz;
	rot.m_ysz = m_xsz;	

	for (i=0; i < m_xsz; i++) {
		for (j=0; j < m_ysz; j++) {					
			rot.m_tile[j][i].terrain = m_tile[m_xsz-(i+1)][j].terrain;
			oldRoad = m_tile[m_xsz-(i+1)][j].roads;
			road = 0;
			if (oldRoad & ROAD_N) road |= ROAD_E;
			if (oldRoad & ROAD_E) road |= ROAD_S;
			if (oldRoad & ROAD_S) road |= ROAD_W;
			if (oldRoad & ROAD_W) road |= ROAD_N;			
			rot.m_tile[j][i].roads = road;
		}
	}

	*this = rot;
}

/////////////////////////////////////
void buildGameTiles() 
{
	TileBlock *gt;
	int mtlA, mtlB, i, j;

	for ( int combo = 0; combo < 3; combo++) {
		switch( combo ) {
		case 0:
			mtlA = WATER;
			mtlB = PLAINS;
			break;
		case 1:
			mtlA = WATER;
			mtlB = BADLANDS;
			break;
		case 2:
			mtlA = PLAINS;
			mtlB = BADLANDS;
			break;		
		}

		for (int t = 0; t <= 63; t++ ) {
			gt = new TileBlock( 3, 2, 0, 0 );
		
			gt->m_tile[0][0].terrain = (t&0x01)?mtlA:mtlB;
			gt->m_tile[0][1].terrain = (t&0x02)?mtlA:mtlB;
			gt->m_tile[1][0].terrain = (t&0x04)?mtlA:mtlB;
			gt->m_tile[1][1].terrain = (t&0x08)?mtlA:mtlB;
			gt->m_tile[2][0].terrain = (t&0x10)?mtlA:mtlB;
			gt->m_tile[2][1].terrain = (t&0x20)?mtlA:mtlB;							

			// clear any roads
			for ( i=0; i < 3; i++) {
				for ( j=0; j < 3; j++) {
					gt->m_tile[i][j].roads = 0x0;
				}
			}

			// check that this block has no isolated terrain
			bool bad = false;
			int terr;
			for ( i=0; i < 3; i++) {
				for ( j=0; j < 3; j++) {					
					terr = gt->m_tile[i][j].terrain;
					if (	(gt->getTerrain( i-1, j ) != terr ) &&
							(gt->getTerrain( i+1, j ) != terr ) &&
							(gt->getTerrain( i, j-1 ) != terr ) &&
							(gt->getTerrain( i, j+1 ) != terr ) ) {
						bad = true;
						break;
					}
				}
				if (bad) break;
			}

			if (!bad) {
				gameTiles.push_back( gt );
			} else {
				delete gt;
			}

		}
	}	
}

// really, just single digit hex number,
int charToRoad( char ch ) {
	if ((ch>='0') && (ch <= '9')) {
		return ch - '0';
	} else {
		assert( (ch>='A') && (ch <='F'));
		return (ch - 'A') + 10;
	}
}

void buildRoadTiles()
{
	FILE *fp = fopen("roads.dat", "rt");	
	char line[1000], cmd[100], arg[100];
	TileBlock *rt = NULL;
	int i,j, count;
	bool mirror;
	
	while (fgets( line, 1000, fp )) {
		if (line[0]=='#') continue;
		if (line[0]=='\n') continue;

		sscanf( line, "%s", cmd );
		if (!stricmp( cmd, "tile")) {
			// new tile
			rt = new TileBlock( 3, 2, 0, 0);
			for (i=0; i < 3; i++) 
				for (j=0; j < 2; j++)
					rt->m_tile[i][j].terrain = EMPTY;	

			count = 1;
			mirror = false;

		} else if (!stricmp( cmd, "count")) {
			sscanf( line, "%*s %d", &count );
			_RPT1( _CRT_WARN, "COUNT IS %d\n", count );
		} else if (!stricmp( cmd, "mirror")) {
			sscanf( line, "%*s %s", arg );
			if (!stricmp(arg,"yes")) {
				mirror = true;
			} else {
				mirror = false;
			}
		} else {
			_RPT1( _CRT_WARN, "Line is %s, must be a tile\n", line );
			assert( rt );
			rt->m_tile[0][1].roads = charToRoad( line[0] );
			rt->m_tile[1][1].roads = charToRoad( line[1] );
			rt->m_tile[2][1].roads = charToRoad( line[2] );
			
			fgets( line, 1000, fp );
			rt->m_tile[0][0].roads = charToRoad( line[0] );
			rt->m_tile[1][0].roads = charToRoad( line[1] );
			rt->m_tile[2][0].roads = charToRoad( line[2] );

			for (i=0; i< count; i++) {
				_RPT2( _CRT_WARN, "Adding road tile %p %d\n", rt, roadTiles.size() );
				roadTiles.push_back( rt );				
			}
			rt = NULL;
		}
	}

	fclose(fp);
}

TileBlock *pickFromBag( ) {
	TileBlock	*rt, // road tile
				*gt, // game tile
				*pt; // pick tile
	int rtNdx, gtNdx;

	// pick a game tile and a road tile at random
	gtNdx = (int)( ((float)rand() / (float)RAND_MAX) * (gameTiles.size()-1) );
	rtNdx = (int)( ((float)rand() / (float)RAND_MAX) * (roadTiles.size()) );
	gt = gameTiles[ gtNdx ];
	rt = roadTiles[ rtNdx ];

	// combine them
	pt = new TileBlock( *gt );

	// add roads
	for (int i=0; i < 3; i++) {
		for (int j=0; j < 3; j++) {
			pt->m_tile[i][j].roads = rt->m_tile[i][j].roads;
		}
	}

	return pt;
}

int oppositeRoad( int road ) {
	switch (road) {
	case ROAD_N:
		return ROAD_S;
		break;
	case ROAD_S:
		return ROAD_N;
		break;
	case ROAD_W:
		return ROAD_E;
		break;
	case ROAD_E:
		return ROAD_W;
		break;
	}
	return 0;
}

// no longer assumes tile falls within map bounds
bool checkPosValid( TileBlock *tile, int mx, int my ) {
	int i,j;

	if (mx + tile->m_xsz > worldMap->m_xsz) return false;
	if (my + tile->m_ysz > worldMap->m_ysz) return false;

	// check that it doesn't overlap existing tiles
	for ( i=0; i < tile->m_xsz; i++) {
		for (j=0; j < tile->m_ysz; j++) {
			if (worldMap->m_tile[i+mx][j+my].terrain != EMPTY) return false;
		}
	}

	// check that it borders at least one tile
	if (!firstMove) {
		bool touchLand = false;
		for ( i=0; i < tile->m_xsz; i++) {
			if (worldMap->getTerrain( mx+i, my-1 )!=EMPTY) { touchLand = true; break; }
			if (worldMap->getTerrain( mx+i, my+tile->m_ysz )!=EMPTY) { touchLand = true; break; }
		}
		for ( j=0; j < tile->m_ysz; j++) {
			if (touchLand) break;
			if (worldMap->getTerrain( mx-1, my+j )!=EMPTY) { touchLand = true; break; }
			if (worldMap->getTerrain( mx+tile->m_xsz, my+j )!=EMPTY) { touchLand = true; break; }
		}

		if (!touchLand) {
			return false;
		}
	}

	// check connections	
	int mapR, myR;
	for ( i=0; i < tile->m_xsz; i++) {

		// Check Eastward connections
		if ((worldMap->getTerrain( mx+i, my-1 )!=EMPTY) ) { 
			myR  = tile->m_tile[i][0].roads;
			mapR = worldMap->m_tile[mx+i][my-1].roads;
			
			// anything can connect a city
			if (!(mapR & ROAD_CITY) && !(myR & ROAD_CITY)) {
				if ( ( (myR & ROAD_E) && (!(mapR & ROAD_W) ) ) ||
					( !(myR & ROAD_E) && ((mapR & ROAD_W) ) ) ) {
					// they don't link				
					return false;
				}
			}
		}

		// Check Westward connections
		if ((worldMap->getTerrain( mx+i, my+tile->m_ysz )!=EMPTY) ) { 
			myR  = tile->m_tile[i][tile->m_ysz-1].roads;
			mapR = worldMap->m_tile[mx+i][my+tile->m_ysz].roads;
			
			// anything can connect a city
			if (!(mapR & ROAD_CITY) && !(myR & ROAD_CITY)) {
				if ( ( (myR & ROAD_W) && (!(mapR & ROAD_E) ) ) ||
					( !(myR & ROAD_W) && ((mapR & ROAD_E) ) ) ) {
					// they don't link				
					return false;
				}
			}
		}
	}

	for ( j=0; j < tile->m_ysz; j++) {

		// Check Southward connections
		if ((worldMap->getTerrain( mx-1, my+j )!=EMPTY) ) { 
			myR  = tile->m_tile[0][j].roads;
			mapR = worldMap->m_tile[mx-1][my+j].roads;
			
			// anything can connect a city
			if (!(mapR & ROAD_CITY) && !(myR & ROAD_CITY)) {
				if ( ( (myR & ROAD_S) && (!(mapR & ROAD_N) ) ) ||
					( !(myR & ROAD_S) && ((mapR & ROAD_N) ) ) ) {
					// they don't link				
					return false;
				}
			}
		}

		// Check Northward connections
		if ((worldMap->getTerrain( mx+tile->m_xsz, my+j )!=EMPTY) ) { 
			myR  = tile->m_tile[tile->m_xsz-1][j].roads;
			mapR = worldMap->m_tile[mx+tile->m_xsz][my+j].roads;
			
			// anything can connect a city
			if (!(mapR & ROAD_CITY) && !(myR & ROAD_CITY)) {
				if ( ( (myR & ROAD_N) && (!(mapR & ROAD_S) ) ) ||
					( !(myR & ROAD_N) && ((mapR & ROAD_S) ) ) ) {
					// they don't link				
					return false;
				}
			}
		}			
	}

	return true;
}

bool movesLeft( TileBlock *tile ) {
	TileBlock tile2( *tile );

	for (int i = 0; i < 4; i++ ) {

		//_RPT1( _CRT_WARN, "checking rotation %d\n", i );

		for (int x = 0; x < 20; x++) {
			for (int y = 0; y < 20; y++) {
				
				//_RPT2( _CRT_WARN, "checking pos %d %d\n", x, y );

				if( checkPosValid( &tile2, x, y )) {

					//_RPT0( _CRT_WARN, "Good\n" );

					validMove = tile2;
					validMove.m_drawstyle = DS_GREEN;
					worldMap->mapToScreen ( x, y, validMove.m_xo, validMove.m_yo );					

					//char buff[100];
					//sprintf( buff, "A tile can be placed at %d %d", x, y );
					//ShowMessage( buff );
					return true;
				}

				//_RPT0( _CRT_WARN, "No good\n" );
			}
		}

		tile2.rotate90();

	}
	return false;
}

void _doCountRegion( TileBlock *map, int terrain, int x, int y, int &count ) {
	
	// reasons not to continue searching
	if (visited[x][y]) return;
	if ( (x>=20) || (x <0) || (y>=20) || (y <0)) return;
	if (map->getTerrain(x,y) != terrain) return;

	visited[x][y] = 1;
	count++;

	_doCountRegion( map, terrain, x, y-1, count );
	_doCountRegion( map, terrain, x, y+1, count );
	_doCountRegion( map, terrain, x-1, y, count );
	_doCountRegion( map, terrain, x+1, y, count );
}

int countRegion( TileBlock *map, int x, int y )
{	
	clearVisited();	

	int count = 0;
	_doCountRegion( map, map->getTerrain(x,y), x, y, count );

	return count;
}

int cityX[100], cityY[100];
int nCity = 0;

void reachCities( int x, int y ) {

	if (visited[x][y]) {
		_RPT2( _CRT_WARN, "reachCities already visited %d %d\n", x, y );	
		return;
	}

	_RPT2( _CRT_WARN, "reachCities looking at %d %d\n", x, y );

	if (worldMap->getTerrain( x, y ) == EMPTY ) {
		_RPT2( _CRT_WARN, "reachCities %d %d is empty\n", x, y );
	}

	if (worldMap->getTerrain( x, y ) != EMPTY ) {
		visited[x][y] = 1;
		
		// if a city is found, count it
		if (worldMap->m_tile[x][y].roads & ROAD_CITY) {			
			cityX[nCity] = x;
			cityY[nCity] = y;
			nCity++;
		}

		// move north
		if ( (worldMap->m_tile[x][y].roads & ROAD_N) &&
			 (worldMap->getTerrain( x+1, y ) != EMPTY) &&
			 (worldMap->m_tile[x+1][y].roads & ROAD_S) ) {
			reachCities( x+1, y );
		}

		// move south
		if ( (worldMap->m_tile[x][y].roads & ROAD_S) &&
			 (worldMap->getTerrain( x-1, y ) != EMPTY) &&
			 (worldMap->m_tile[x-1][y].roads & ROAD_N) ) {
			reachCities( x-1, y );
		} else {
			_RPT2( _CRT_WARN, "reachCities at %d %d can't go south \n", x, y );
		}

		// move west
		if ( (worldMap->m_tile[x][y].roads & ROAD_W) &&
			 (worldMap->getTerrain( x, y+1 ) != EMPTY) &&
			 (worldMap->m_tile[x][y+1].roads & ROAD_E) ) {
			reachCities( x, y+1 );
		} else {
			_RPT2( _CRT_WARN, "reachCities at %d %d can't go west \n", x, y );
		}

		// move east
		if ( (worldMap->m_tile[x][y].roads & ROAD_E) &&
			 (worldMap->getTerrain( x, y-1 ) != EMPTY) &&
			 (worldMap->m_tile[x][y-1].roads & ROAD_W) ) {
			reachCities( x, y-1 );
		} else {
			_RPT2( _CRT_WARN, "reachCities at %d %d can't go east \n", x, y );
		}

	}
}

void clearVisited()
{
	int i,j;
	for ( i=0; i < 20; i++) {
		for ( j=0; j < 20; j++) {
			visited[i][j] = 0;
		}
	}
}

void updateScore( int playerNdx ) 
{
	int count, score = 0, i,j, ii,jj;
	char regionVisited[20][20];

	// build a list of regions
	int regionX[10], regionY[10];
	int nRegion = 0;

	for ( i=0; i < 5; i++) {
		player[playerNdx].regionCount[i] = 0;
		player[playerNdx].regionConn[i] = 1;
	}

	for ( i=0; i < 20; i++) {
		for ( j=0; j < 20; j++) {
			regionVisited[i][j] = 0;
		}
	}

	for ( i=0; i < 20; i++) {
		for ( j=0; j < 20; j++) {

			if ((worldMap->getTerrain( i,j ) == player[playerNdx].land) &&
				(worldMap->m_tile[i][j].roads & ROAD_CITY ) &&
				(regionVisited[i][j]==1) ) {
				_RPT3( _CRT_WARN, "player %s Region at %d %d has a city but it's joined\n", 
						turnMsg[playerNdx], i, j );				
			}

			if ((worldMap->getTerrain( i,j ) == player[playerNdx].land) &&
				(worldMap->m_tile[i][j].roads & ROAD_CITY ) &&
				(regionVisited[i][j]==0) ) {

				count = countRegion( worldMap, i, j );				

				_RPT4( _CRT_WARN, "player %s Region at %d %d count %d\n", 
						turnMsg[playerNdx], i, j, count );						

				// copy visited into regionvisited
				for ( ii=0; ii < 20; ii++) {					
					for ( jj=0; jj < 20; jj++) {
						if (visited[ii][jj]) {
							regionVisited[ii][jj] = 1;							
						}						
					}					
				}				
				player[playerNdx].regionCount[nRegion] = count;

				regionX[nRegion] = i;
				regionY[nRegion] = j;

				nRegion++;								
			}
		}
	}

	// now figure out connections for each region
	for (int r = 0; r < nRegion; r++) {

		player[playerNdx].regionConn[r] = 0;

		// clear region visited
		for ( i=0; i < 20; i++) {
			for ( j=0; j < 20; j++) {
				regionVisited[i][j] = 0;
			}
		}
		
		// use countregion to define the extents of this region
		countRegion( worldMap, regionX[r], regionY[r] );
		for ( i=0; i < 20; i++) {
			for ( j=0; j < 20; j++) {
				regionVisited[i][j] = visited[i][j];
			}
		}

		// list all cities in or reachable from this region
		// duplicates are okay
		nCity = 0;
		for ( i=0; i < 20; i++) {
			for ( j=0; j < 20; j++) {
				if (regionVisited[i][j] && 
					worldMap->m_tile[i][j].roads & ROAD_CITY ) {

					_RPT2( _CRT_WARN, "Looking from city at %d %d\n", i, j );
					clearVisited();
					reachCities( i, j );
				}
			}
		}

		_RPT2( _CRT_WARN, "DBG>>> Region %d reaches %d cities\n",
							r, nCity );

		// Go through the list of cities and count them if we haven't
		// visited their region yet		
		for ( i=0; i < 20; i++) {
			for ( j=0; j < 20; j++) {
				regionVisited[i][j] = 0;				
			}
		}

		for ( int c=0; c < nCity; c++) {
			if (!regionVisited[ cityX[c] ][ cityY[c] ]) {
				player[playerNdx].regionConn[r]++;
				clearVisited();
				countRegion( worldMap, cityX[c], cityY[c] );

				// mark the region the city is in as visited
				for ( i=0; i < 20; i++) {
					for ( j=0; j < 20; j++) {
						if (visited[i][j]) regionVisited[i][j] = 1;
					}
				}
			}
		}

		// thats it.
	}

	// total up score	
	player[playerNdx].score = 0;
	for (i = 0; i < nRegion; i++) {
		player[playerNdx].score += (player[playerNdx].regionCount[i] *
									player[playerNdx].regionConn[i] );
	}
	
}

void endOfTurn()
{
	updateScore(0);
	updateScore(1);
	updateScore(2);
	
	didDoubleTile = false; // no lingering ddt

	currPlayerNdx++;
	if (currPlayerNdx==3) currPlayerNdx = 0;
}

bool checkPosValidBuilding( int terrain, int mx, int my ) {

	// have to build on matching terrain
	if (worldMap->getTerrain( mx, my ) != terrain) {
		return false;	
	}

	// check that there is at least 9 connected spaces
	int count = countRegion( worldMap, mx, my );
	if (count < 9) return false;	

	return true;
}


void updatePickTilesStart()
{
	// figure out "dest" locations
	TileBlock *newTile = NULL;
	int slide = 0;
	for (int i = 5; i >=0; i-- ) {
		if ((i<5) && (pickTiles[i] && !pickTiles[i+1])) {
			slide++;
			do_slide = true;
			slide_t = 0.0;
			_RPT1( _CRT_WARN, "Slide is now %d\n", slide );
		}

		if (pickTiles[i]) {			
			// slide down
			pickTiles[i]->m_destNdx = i+slide;
			pickTiles[i]->m_srcXo = pickTiles[i]->m_xo;
			pickTiles[i]->m_srcYo = pickTiles[i]->m_yo;
			pickTiles[i]->m_destXo = tile_x_pos[i+slide];
			pickTiles[i]->m_destYo = pickTiles[i]->m_yo;
		}		
	}

	if (slide==0) {
		do_slide = true;
		slide_t = 0.99;
	}
}

void updatePickTilesDone()
{
	int i;

	do_slide = false;
	std::vector<TileBlock*> newPicks;
	for (i=0; i < 6; i++) {
		newPicks.push_back( NULL );
	}

	for (i=0; i < 6; i++) {
		if (pickTiles[i]) {
			newPicks[pickTiles[i]->m_destNdx ] = pickTiles[i];
		}
	}

	for (i=0; i < 6; i++) {
		_RPT2( _CRT_WARN, "Newpick %d is %p\n", i, newPicks[i] );

		if (newPicks[i]) {
			break;
		} else {
			_RPT1( _CRT_WARN, "creating new tile %d\n", i );
			newPicks[i] = pickFromBag();
			newPicks[i]->m_xo = tile_x_pos[i];
			newPicks[i]->m_yo = 558;
		}
	}

	for (i=0; i < 6; i++) {
		pickTiles[i] = newPicks[i];
		_RPT2( _CRT_WARN, "pickTiles %d is %p\n", i, pickTiles[i] );
	}
}

void textout_fancy( BITMAP *dest, FONT *font, const char *msg, int x, int y, int color, int brite, int dark )
{
	textout( dest, font, msg, x, y-1, dark );
	textout( dest, font, msg, x, y+1, brite );
	textout( dest, font, msg, x, y, color );
}

void redraw() 
{

	static BITMAP *osb = NULL; // off screen buffer

	if (!osb) {
		osb = create_bitmap( 800, 600 );

	}


   acquire_bitmap( osb );

   /* set transparent text */
   text_mode(-1);


   blit( background, osb, 0, 0, 0,0, 800, 600 );

   worldMap->drawIsoMap( osb );

   //_RPT0(_CRT_WARN, " SMALL TILES ------\n");
   
   

#if 0
   char buff[20];
   for (int j=0; j < 21; j++) {
	   for (int i=0; i < 21; i++) {
		   int sx = -1, sy = -1;
		   worldMap->mapToScreen( i, j, sx, sy );
		   //_RPT4 ( _CRT_WARN, "ij %d %d sx sy %d %d\n", i, j, sx, sy );
		   sprintf( buff, "(%d, %d)", i,j );
		   if ( ((i%5)==0) && ((j%5)==0) ) {
			   textout_centre(osb, font, buff, sx+1, sy+1, 9);
			   textout_centre(osb, font, buff, sx-1, sy+1, 9);
			   textout_centre(osb, font, buff, sx+1, sy-1, 9);
			   textout_centre(osb, font, buff, sx-1, sy-1, 9);
			   textout_centre(osb, font, buff, sx, sy, 14);
		   }
		   putpixel( osb, sx, sy, 14 );
	   }
   }
#endif 

   // draw stats & scores   
   char buff[100];
   int i, j;
   for ( i=0; i < 3; i++) {
	   sprintf( buff, "%d", player[i].score );
	   textout( osb, font, buff, 80, 39 + (32*i), 31 );
	   textout( osb, font, buff, 80, 41 + (32*i), 20 );
	   textout( osb, font, buff, 80, 40 + (32*i), player[i].color );
   }

   for ( i=0; i < 3; i++) {
	   for (j=0; j < 10; j++) {
		   if (j < player[i].buildingsLeft) {
			   buff[j]='o';
		   } else {
			   buff[j]=0;
		   }
	   }
	   textout( osb, font, buff, 718, 19 + (45*i), 79 );
	   textout( osb, font, buff, 718, 21 + (45*i), 64 );
	   textout( osb, font, buff, 718, 20 + (45*i), player[i].color);
   }

   // draw current turn stuff      
   textout_fancy( osb, font, turnMsg[currPlayerNdx], 9, 305, player[currPlayerNdx].color, 20, 31 );
   textout_fancy( osb, font, "Turn", 9, 317, player[currPlayerNdx].color, 20, 31 );

   textout_fancy( osb, font, "Regions:", 9, 365, player[currPlayerNdx].color, 20, 31 );
   textout_fancy( osb, font, "  Size  Conn  Total", 9, 380, player[currPlayerNdx].color, 20, 31 );

   for (i = 0; i < 5; i++) {
	   if (player[currPlayerNdx].regionCount[i] == 0) break;
		
	   sprintf( buff, "%d: %2d x %2d    = %d",
				i+1, player[currPlayerNdx].regionCount[i],
				player[currPlayerNdx].regionConn[i],
				player[currPlayerNdx].regionCount[i] *
				player[currPlayerNdx].regionConn[i] );

		textout_fancy( osb, font, buff, 9, 395 + 12*i, player[currPlayerNdx].color, 20, 31 );	
   }

   sprintf( buff, "Total Score: %d", player[currPlayerNdx].score );
   textout_fancy( osb, font, buff, 180, 395 + 35, player[currPlayerNdx].color, 20, 31 );  

   // draw status message
   if (statusMsgColor) {
		textout( osb, font, statusMsg, 30, 585, statusMsgColor );
   }

   // draw valid move 
   if (showValidMove) {
	   sprintf( buff, "show valid mvoe is %d\n", showValidMove );
	   ShowMessage( buff );
		validMove.drawIsoMap( osb );		
   }

   // draw floating tiles
   for (int ti=0; ti < 6; ti++) {					
		if (pickTiles[ti]) {			
			pickTiles[ti]->drawIsoMap( osb );		
		}
	}

   // do building
   if (currAct==ACT_PLACE_BUILDING) {
	   if (placementValid) {
		   masked_blit( redgreentile, osb, 40, 0, build_sx-20, build_sy+13-30, 40, 30 );
	   } else {
		   masked_blit( redgreentile, osb, 0, 0, build_sx-20, build_sy+13-30, 40, 30 );
	   }
	   masked_blit( tiles, osb, 16*40, currPlayerNdx*30, build_sx-20, build_sy-30, 40, 30 );
   }

   // draw game over message
   if (gameOver) {
	   int winnerColor = 2;
	   if ( (player[0].score > player[1].score) && 
		    (player[0].score > player[2].score) ) {
		   strcpy( buff, "Game Over! Grasslanders win!" );
		   winnerColor = player[0].color;		   
	   } else if ( (player[1].score > player[0].score) && 
		    (player[1].score > player[2].score) ) {
		   strcpy( buff, "Game Over! Seafarers win!" );
		   winnerColor = player[1].color; 
	   } else if ( (player[2].score > player[0].score) && 
		    (player[2].score > player[1].score) ) {
		   strcpy( buff, "Game Over! Badlanders win!" );
		   winnerColor = player[2].color; 
	   } else if ( (player[0].score == player[1].score) && 
		    (player[0].score > player[2].score) ) {
		   strcpy( buff, "Game Over! Grasslander and Seafarers tie!" );
		   winnerColor = 15; 
	   } else if ( (player[1].score == player[2].score) && 
		    (player[1].score > player[0].score) ) {
		   strcpy( buff, "Game Over! Badlanders and Seafarers tie!" );
		   winnerColor = 56; 
	   } else if ( (player[0].score == player[2].score) && 
		    (player[0].score > player[1].score) ) {
		   strcpy( buff, "Game Over! Badlanders and Grasslanders tie!" );
		   winnerColor = 85; 
	   } else if ( (player[0].score == player[1].score) &&
		     (player[1].score == player[2].score) ) {
		   strcpy( buff, "Game Over! Three way tie!" );
		   winnerColor = 0xff; 
	   }
					   
	   rectfill( osb, 150, 530, 800-150, 565, 190 );
	   rect    ( osb, 150, 530, 800-150, 565, 180 );
	   textout_centre( osb, font, buff, 400, 545, winnerColor );
	   	   	   
   }
   
   // draw cursor
   masked_blit( cursor, osb, 0, 0, mouse_x, mouse_y, 32, 32 );

   release_bitmap( osb );

   /* swapbuffers */
   acquire_screen();
   blit( osb, screen, 0, 0, 0, 0, 800, 600 );
   release_screen();


}

/////////////////////////////////////
int main( int argc, char *argv[] )
{
	int i,j;

	// srand
	srand( time(0) );

   /* you should always do this at the start of Allegro programs */
   allegro_init();

   /* set up the keyboard handler */
   install_keyboard(); 
	install_mouse();

   /* set a graphics mode sized 800x600 */
   //set_color_depth(16);
   if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0) != 0) {
	//if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 800, 600, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set any graphic mode\n%s\n", allegro_error);
      return 1;
   }
   
   /* load resources */
   tiles = load_bitmap( "gfx/alltiles.bmp", pally ); 
   tile_template = load_bitmap( "gfx/template.bmp", NULL ); 
   background = load_bitmap( "gfx/background.bmp", NULL );
   cursor = load_bitmap( "gfx/cursor.bmp", NULL );
   redgreentile = load_bitmap("gfx/redgreentile.bmp", NULL );   

   /* Init game */
   worldMap = new TileBlock( 20, 20, 400, 450 );

   
   /* set up the game tiles */
   buildGameTiles();
	buildRoadTiles();

	/* set up the players */
	player[0].land = PLAINS;
	player[1].land = WATER;
	player[2].land = BADLANDS;

	player[0].color = 11;
	player[1].color = 15;
	player[2].color = 207;

	currAct = ACT_IDLE;

   TileBlock *gt;
   for ( i=0; i < 4; i++) {
	   gt = pickFromBag();
	   gt->m_xo = tile_x_pos[i];
	   gt->m_yo = 558;
	   //gt = new TileBlock( 3, 2, tile_x_pos[i],  558 );
	  
		pickTiles.push_back( gt );
   }
   pickTiles.push_back( NULL );
   pickTiles.push_back( NULL );

   set_window_title("Land Game");
	set_palette(pally);

   /* clear the screen to white */
   clear_to_color(screen, 8);     

   ShowMessage( "Welcome to Outlands - see README.txt for instructions" );

   // main loop
   int last_retrace_count=retrace_count;
   int old_mouse_b = mouse_b;
   int mx, my, ptNdx;   
   
   while (1) {

	   if (mouse_needs_poll()) {
		   poll_mouse();
	   }	   

	   // check the show valid move button
	   if ((!gameOver) && (!do_slide) && ( (old_mouse_b&1) && !(mouse_b&1) )) {
		   if ( (mouse_x > 725) && (mouse_x < 789) &&
			    (mouse_y > 403) && (mouse_y < 442) ) {			   
				showValidMove = 70;
				_RPT0( _CRT_WARN, " Setting showvalidMove\n" );
		   }
	   }

	   // place tiles and buildings
	   if ((!gameOver) && (!do_slide) && ( (old_mouse_b&1) && !(mouse_b&1) )) {
		   

		   // button 1 released		   
		  if (currAct==ACT_IDLE) {
			   ptNdx = -1;			   
			   if ((!chosenTile) && (mouse_y > 500) && (mouse_y < 570)) {

				currAct = ACT_PLACE_TILE;				

				if (buildHints[ buildHintNdx ]) { 
					ShowMessage( buildHints[ buildHintNdx ] );
					buildHintNdx++;
				}

				for (int i = 0; i < 6; i++) {
					if ( mouse_x < tile_x_pos[i] + 70 ) {
						ptNdx = i;
						break;
					}
				}

				if ((ptNdx >= 0) && (pickTiles[ptNdx])) {
					chosenTile = pickTiles[ptNdx];
					chosenTile->m_drawstyle = DS_RED;

					chosenOrigX = chosenTile->m_xo;
					chosenOrigY = chosenTile->m_yo;

					if ((ptNdx==4) || (ptNdx==5)) {
						doubleTile = true;
					}
				}			
			   } else if (	(mouse_y > 6+currPlayerNdx * 45) && 
							(mouse_y < 46 + currPlayerNdx*45) &&
							(mouse_x > 665) && (mouse_x < 763) ) {
				   
				   currAct = ACT_PLACE_BUILDING;
				   player[currPlayerNdx].buildingsLeft--;

				   ShowMessage( "Build need at least 9 squares of land. Right click to cancel." );

			   }
		   } else if ( (currAct==ACT_PLACE_TILE) && (chosenTile) &&
					   (mouse_y > 500) && (mouse_y < 570)) {

			   // drop tile
			   currAct = ACT_IDLE;
			   chosenTile->m_xo = chosenOrigX;
			   chosenTile->m_yo = chosenOrigY;
			   chosenTile->m_drawstyle = DS_PLAIN;			   

			   if (chosenTile->m_xsz == 2) {
				   chosenTile->rotate90();
			   }
			   
				chosenTile = NULL;
			   doubleTile = false;

			   // if they placed the first of a double tile already
			   // their turn is over.
			   if (didDoubleTile) {
				   didDoubleTile = false;
					updatePickTilesStart();
					endOfTurn();
					currAct = ACT_IDLE;
			   }

		   } else if (placementValid) {

			  if (currAct==ACT_PLACE_TILE) {

			   firstMove = false;
			   for (i=0; i < chosenTile->m_xsz; i++) {
				   for (j=0; j < chosenTile->m_ysz; j++) {
					   worldMap->m_tile[mx+i][my+j] = chosenTile->m_tile[i][j];
				   }
			   }
			   

			   for (i=0; i < 6; i++) {
				   if (pickTiles[i]==chosenTile) {
					   pickTiles[i] = NULL;
				   }
			   }
			   delete chosenTile;
			   chosenTile = NULL;

			   if (doubleTile) {
				   doubleTile = false;
					ptNdx = 4;
					if (!pickTiles[ptNdx]) ptNdx=5;
					if (!pickTiles[ptNdx]) {
						// one one tile in doubletile bin, nevermind
						updatePickTilesStart();
						endOfTurn();
						currAct = ACT_IDLE;
					} else {
						// TODO: PLAY SFX "Place again"
						ShowMessage( "Double Tile! Place again!" );
						chosenTile = pickTiles[ptNdx];

						chosenOrigX = chosenTile->m_xo;
						chosenOrigY = chosenTile->m_yo;

						didDoubleTile = true;

						// update the score from the first tile
						updateScore(0);
						updateScore(1);
						updateScore(2);
					}
			   } else {
					updatePickTilesStart();
					endOfTurn();
					currAct = ACT_IDLE;
				}
			  } else if (currAct==ACT_PLACE_BUILDING) {				  
					ShowMessage( "Building Constructed" );
					worldMap->m_tile[mx][my].terrain = player[currPlayerNdx].land;
					worldMap->m_tile[mx][my].roads = ROAD_CITY | ROAD_N | ROAD_S | ROAD_E | ROAD_W;
					currAct = ACT_IDLE;
					endOfTurn();
			  }
		   }
	   }

	   if ( !(old_mouse_b&2) && (mouse_b&2) ) {
		   // button 2 press
		   if (currAct==ACT_PLACE_TILE) {
			if (chosenTile) {
				   chosenTile->rotate90();
			}
		   }

		   // drop building
		   if (currAct==ACT_PLACE_BUILDING) {
			   currAct = ACT_IDLE;
			   player[currPlayerNdx].buildingsLeft++;			   
		   }
	   }

	   old_mouse_b = mouse_b;

	   // chosen tile "cursor"
	   if (chosenTile) {		   

		   worldMap->screenToMap( mouse_x-20, mouse_y+20, mx, my );		   
		   if (	(mx >= worldMap->m_xsz - chosenTile->m_xsz +1 ) ||
				(mx <0 ) ||
				(my >= worldMap->m_ysz - chosenTile->m_ysz +1 ) ||
				(my <0 ) ) {

				chosenTile->m_xo = mouse_x-20;
				chosenTile->m_yo = mouse_y+20;
				placementValid = false;				

		   } else {
				worldMap->mapToScreen( mx, my, chosenTile->m_xo, chosenTile->m_yo );
				placementValid = checkPosValid( chosenTile, mx, my );
		   }
	   
			// drawstyle
		   if (placementValid) {
			   chosenTile->m_drawstyle = DS_GREEN;
		   } else {
			   chosenTile->m_drawstyle = DS_RED;
		   }
	   }	    

	   // place building
	   if (currAct == ACT_PLACE_BUILDING) {
			worldMap->screenToMap( mouse_x, mouse_y, mx, my );		   
		   if (	(mx >= worldMap->m_xsz) || (mx <0 ) ||
				(my >= worldMap->m_ysz) || (my <0 ) ) {

				build_sx = mouse_x+10;
				build_sy = mouse_y;
				placementValid = false;

		   } else {
				worldMap->mapToScreen( mx, my, build_sx, build_sy );
				placementValid = checkPosValidBuilding( player[currPlayerNdx].land, mx, my );
		   }	  
	   }

		// actual redraw
		if (retrace_count != last_retrace_count) {

			// update animation
			if (do_slide) {
				slide_t += (1.0/70.0) * (retrace_count-last_retrace_count);
				if (slide_t > 1.0) {
					for (std::vector<TileBlock*>::iterator ti = pickTiles.begin();
						ti != pickTiles.end(); ti++) {			

						if (*ti) {
							(*ti)->m_xo = (*ti)->m_destXo;
							(*ti)->m_yo = (*ti)->m_destYo;
						}
					}
					updatePickTilesDone();					
				} else {
					// smooth out the motion
					float tt = (3*slide_t - 2*slide_t*slide_t) *slide_t;

					for (std::vector<TileBlock*>::iterator ti = pickTiles.begin();
						ti != pickTiles.end(); ti++) {			

						if (*ti) {							
							(*ti)->m_xo = LERP( (*ti)->m_srcXo, (*ti)->m_destXo, tt );
							(*ti)->m_yo = LERP( (*ti)->m_srcYo, (*ti)->m_destYo, tt );
						}
					}
				}
			}

			// update status
			if (statusMsgColor) {				
				float inc = (1.0/14.0) * (float)(retrace_count-last_retrace_count);				
				statusMsgColorF = statusMsgColorF + inc;
				statusMsgColor = statusMsgColorF;
				if (statusMsgColor >= 192) statusMsgColor = 0;								
			}

			// update hint
			if (showValidMove) {
				showValidMove -= (retrace_count - last_retrace_count);
				if (showValidMove < 0) showValidMove = 0;
			}

			// redraw
			last_retrace_count = retrace_count;
			redraw();
		}   

		// are there any moves left?
		if (!do_slide) {
			bool good = false;
			for (i=0; i < 6; i++) {				
				if (pickTiles[i]) {
					if (movesLeft( pickTiles[i])) { 
						good = true;
						break;
					}
				}
			}

			if (!good) {
				gameOver = true;
				// TODO: Play SFX
			}
		}

		/* bail */
	   if (key[KEY_ESC]) {
		   exit(0);
		   break;
	   }

		yield_timeslice();		
   }
   
   return 0;
}

END_OF_MAIN();
