#include <sys/stat.h> 

#include <iostream>
#include <fstream>
#include <string>

#include "GlacierGame.h"
#include "GameView.h"

const char *GlacierGame::m_critNames[16] = {
	"mammoth",
	"unknown",
	"unknown",
	"unknown",

	"unknown",
	"unknown",
	"unknown",
	"unknown",

	"unknown",
	"unknown",
	"unknown",
	"unknown",

	"unknown",
	"unknown",
	"unknown",
	"unknown" };


using namespace std;

Tetromino::Tetromino()
{
	m_mesh = 0;
	m_blocks[0][0] = 0;
	m_blocks[0][1] = 0;

	m_blocks[1][0] = 0;
	m_blocks[1][1] = 0;

	m_blocks[2][0] = 0;
	m_blocks[2][1] = 0;

	m_blocks[3][0] = 0;
	m_blocks[3][1] = 0;
}

Tetromino::Tetromino( int b00, int b01,
				  	  int b10, int b11,
			          int b20, int b21,
			          int b30, int b31 )
{
	m_mesh = 0;
	m_blocks[0][0] = b00;
	m_blocks[0][1] = b01;

	m_blocks[1][0] = b10;
	m_blocks[1][1] = b11;

	m_blocks[2][0] = b20;
	m_blocks[2][1] = b21;

	m_blocks[3][0] = b30;
	m_blocks[3][1] = b31;
}

MapSquare::MapSquare() :		
		m_hite( 0 ),
		m_path( 0.0 ),
		m_scurry( false )
{
}

GlacierGame::GlacierGame()
{
	m_latitude = 0.0; // the north pole.. in this crazy world
	m_mapStartLat = 0; // the start of the map
	m_travelSpeed = 1.0;
	
	m_tetRot = 0;
	m_editMode = false;

	clearMap();

	// Initialize the tetrominos

	// LINE
	m_tets.push_back( new Tetromino( 1, 0,
								 1, 0,
								 1, 0,
								 1, 0 ) );

	// ZEE1
	m_tets.push_back( new Tetromino( 0, 0,
								 1, 0,
								 1, 1,
								 0, 1 ) );

	// ZEE2
	m_tets.push_back( new Tetromino( 0, 0,
								 0, 1,
								 1, 1,
								 1, 0 ) );

	// SQUARE
	m_tets.push_back( new Tetromino( 0, 0,
								 0, 0,
								 1, 1,
								 1, 1 ) );

	// ELL1
	m_tets.push_back( new Tetromino( 0, 0,
								 0, 1,
								 0, 1,
								 1, 1 ) );

	// ELL2
	m_tets.push_back( new Tetromino( 0, 0,
								 1, 0,
								 1, 0,
								 1, 1 ) );

	// TEE1
	m_tets.push_back( new Tetromino( 0, 0,
								 1, 0,
								 1, 1,
								 1, 0 ) );

	// TEE2
	m_tets.push_back( new Tetromino( 0, 0,
								 0, 1,
								 1, 1,
								 0, 1 ) );

	// init tets
	m_nextTet = randTet();
	m_currTet = randTet();

	// Default critter data	
	m_critSpeed[0] = 1;

	//m_critNames[1] = "smilodon";


	
	updateHite();
}

void GlacierGame::saveLevel( const char *levelName )
{
	FILE *fp = fopen( levelName, "wt" );
	for (size_t ndx=0; ndx < m_landBlocks.size(); ++ndx)
	{
		LandBlock *block = m_landBlocks[ndx];
		fprintf( fp, "LAND %d %d\n", block->m_hite, block->m_pnts.size() );
		for (size_t pndx = 0; pndx < block->m_pnts.size(); ++pndx )
		{
			fprintf( fp, "  %f %f\n", block->m_pnts[pndx].x, block->m_pnts[pndx].y );
		}
	}
	fclose( fp );
}
void GlacierGame::clearBlocks()
{
	// nuke existing blocks
	for (size_t ndx=0; ndx < m_landBlocks.size(); ++ndx )
	{
		delete m_landBlocks[ndx];
	}
	m_landBlocks.erase( m_landBlocks.begin(), m_landBlocks.end() );
}

void GlacierGame::loadLevel( const char *levelName )
{	
	clearBlocks();
	clearMap();

	foreach_map
	{
		MapSquare &t = MAPITER( m_map );
		t.m_hite = 0;
	}

	FILE *fp = fopen( levelName, "rt" );
	if (!fp) return;

	char token[1000];
	while (!feof(fp))
	{
		fscanf( fp, "%s", token );
		if (!stricmp( token, "LAND" ))
		{
			int hite, verts;
			fscanf( fp, "%d %d\n", &hite, &verts );
			LandBlock *blk = new LandBlock();
			blk->m_hite = hite;			
			_RPT2( _CRT_WARN, "Loading block h %d verts %d\n", hite, verts );

			for (int i=0; i < verts; ++i)
			{
				float x, y;
				fscanf( fp, "%f %f\n", &x, &y );
				blk->m_pnts.push_back( vec2f( x, y ) );
			}
			
			m_landBlocks.push_back( blk );
		}
	}
	fclose( fp );

	// recalculate heights
	calcHeights();	
	updateHite();

	// add some critters to the first row
	m_critters.erase( m_critters.begin(), m_critters.end() );

	for (int i=0; i < MAP_ROWS; ++i)
	{		
		if (m_levelHite[i][1] > 0)
		{
		Critter c;
			c.imageNdx = CRITTER_MAMMOTH;			
			c.px = i;		
			c.py = 1;
			c.recharge = m_critSpeed[ c.imageNdx ];
			m_critters.push_back( c );	
		}
	}	
}
void GlacierGame::calcHeights()
{
	// Update level heights
	for (int i=0; i < MAP_ROWS; i++)
	{
		for (int j=0; j < 1000; j++)
		{						
			// Check to see if this point is contained in any blocks
			vec2f p( (i*10) + 5, (j*10) + 5 );
			int maxHite = 0;
			for (size_t ndx=0 ; ndx < m_landBlocks.size(); ++ndx )
			{
				LandBlock *blk = m_landBlocks[ndx];
				int lastSgn = 0;
				bool contains = true;
				for (size_t a=0; a < blk->m_pnts.size(); a++)
				{
					size_t b = a+1;
					if (b == blk->m_pnts.size()) b = 0;
					vec2f edge = blk->m_pnts[b] - blk->m_pnts[a];
					vec2f pb = p - blk->m_pnts[a];

					float d = edge.x*pb.y - edge.y*pb.x;
				
					int sgn = 1;
					if (d < 0) sgn = -1;

					if (!lastSgn)
					{
						lastSgn = sgn;
					}
					else
					{
						if (lastSgn != sgn)
						{
							// not in
							contains = false;
							break;
						}
					}
				}

				if (contains)
				{
					maxHite = max( blk->m_hite, maxHite );
				}
			}

			m_levelHite[i][j] = maxHite;			
		}
	}

#if 0
	// DBG: fill the map with some random stuff
	for (int count=0; count < 5000; ++count)
	{
		int i = rand() % MAP_ROWS;
		int j = rand() % 1000;
		if (!m_levelHite[i][j]<3 )
		{
				m_levelHite[i][j] += 1;			
		}
	}
#endif
}

Tetromino *GlacierGame::randTet()
{
	int ndx = (rand() / (float)RAND_MAX) * m_tets.size();
	return m_tets[ndx];
}

void GlacierGame::clearMap()
{
	foreach_map
	{
		MapSquare &t = MAPITER( m_map );
		t.m_hite = 0;
		t.m_path = 0.0;
	}
}

void GlacierGame::updateHite()
{
	foreach_map
	{
		MapSquare &t = MAPITER( m_map );
		
		// Set hite from level
		int levelY = mj + m_mapStartLat / 10;
		if (levelY > 999) levelY = 999;
		t.m_hite = max( t.m_hite, (int)(m_levelHite[mi][levelY]) );
		//t.m_hite = (int)(m_levelHite[mi][levelY]);

		//_RPT2( _CRT_WARN, "tile %d %d z %d\n",  mi, mj, t.m_hite );
	}
}

void GlacierGame::scrollMap()
{
	// scroll start of map
	m_mapStartLat += 10;

	// shift tiles
	foreach_map
	{
		MapSquare &t = MAPITER( m_map );

		// Last column?
		if (mj == MAP_COLS-1)
		{
			t = MapSquare();
			t.m_hite = 0;
		}
		else
		{		
			// replace with next column
			t = m_map[mi][mj+1];			
		}

		// clear the path value so it re-calcs
		//t.m_path = 0.0;
	}

	// fix height
	updateHite();

	// scroll existing critters
	for (int i=0; i < m_critters.size(); ++i)
	{
		m_critters[i].py -= 1;
		if (m_critters[i].py < 0)
		{
			// he's already escaped.. 
			m_critters[i].py = 0;
		}
	}

#if 0
	// add some critters to the last row
	int p = (rand() % MAP_ROWS);
	Critter c;
	c.imageNdx = CRITTER_MAMMOTH;
	c.py = MAP_COLS -1;
	c.px = p;
	c.recharge = m_critSpeed[ c.imageNdx ];
	m_critters.push_back( c );	
#endif
}

// returns the height highest tile overlapping 
// the tet. If the retval is negated (in checkOnly), 
// then you can't legally put a tile there
bool GlacierGame::dropTet( bool checkOnly, int &tileHite )
{
	tileHite = -99;
	bool isLegal = true;
	vector<int> clobberedCritters;
	
	if (!checkOnly)
	{
		foreach_map
		{
			MapSquare &t = MAPITER( m_map );
			t.m_scurry = false;
		}
	}


	for (int i=0; i < 4; i++)
	{
		for (int j=0; j < 2; j++)
		{
			if (m_currTet->m_blocks[i][j])
			{
				int mx, my;
				switch (m_tetRot)
				{
					case 0:
						mx = j + m_selX;
						my = i + m_selY - 4;
						break;

					case 1:					
						mx = i + m_selX - 4;
						my = (1-j) + m_selY-2;
						break;

					case 2:
						mx = (1-j) + m_selX - 2;
						my = (3-i) + m_selY;
						break;

					case 3:
						mx = (3-i) + m_selX;
						my = j + m_selY;
						break;
				}
				
				if ((mx >= 0) && (mx < MAP_ROWS) &&
					(my >= 4) && (my < MAP_COLS))
				{
					if (m_map[mx][my].m_hite != tileHite)
					{
						if (tileHite != -99) isLegal = false;
						tileHite = std::max( tileHite, m_map[mx][my].m_hite);
					}

					if (!checkOnly)
					{
						m_map[mx][my].m_hite += 1;

						// check if we clobbered any critters
						for (int ci=0; ci < m_critters.size(); ci++)
						{
							Critter &c = m_critters[ci];
							if ( (c.px == mx) && (c.py == my) )
							{
								// clobbered!
								clobberedCritters.push_back( ci );
							}
						}

						// Mark nearby squares as candidates for scurry
						if (mx > 0) m_map[mx-1][my].m_scurry = true;
						if (mx < MAP_ROWS) m_map[mx+1][my].m_scurry = true;
						if (my > 0) m_map[mx][my-1].m_scurry = true;
						if (my < MAP_COLS) m_map[mx][my+1].m_scurry = true;
					}
				}
				else
				{
					// tile is out of bounds
					isLegal = false;
				}
			}
		}
	}

	if (!checkOnly)
	{
		m_currTet = m_nextTet;
		m_nextTet = randTet();					

		// Scurry any clobbered critters
		for (int ci = 0; ci < clobberedCritters.size(); ++ci )
		{
			Critter &c = m_critters[clobberedCritters[ci]];
			
			// Try and scurry
			float bestScurry = -999;
			int scX = -1, scY =-1;
			foreach_map
			{
				MapSquare &t = MAPITER( m_map );
				if ((t.m_scurry) && (t.m_hite == tileHite))
				{
					// It's an appropriate scurry candidate
					if (t.m_path > bestScurry)
					{
						bestScurry = t.m_path;
						scX = mi;
						scY = mj;
					}
				}
			}

			if (scX == -1)
			{
				// didn't find a scurry :(
				_RPT2( _CRT_WARN, "No scurry, squished at %d %d\n", c.px, c.py );
				c.dead = true;
			}
			else
			{
				c.px = scX; c.py = scY;

				// the square is no longer available for scurry
				m_map[c.px][c.py].m_scurry = false;
			}
		}
	}

	// could make this limit a gameplay param
	if (tileHite > 3) isLegal = false;
	
	if (tileHite==-99) 
	{
		isLegal = false;
		tileHite = 0;
	}

	return isLegal;
}
void GlacierGame::update( float dtFixed )
{
	// Make sure the game config is up to date
	gameConfig();

	// Don't update if we're in edit mode
	if (m_editMode) return;

	// Update	
	m_latitude += m_travelSpeed * dtFixed;

	// scroll map if needed
	int currLat = (int)floor(m_latitude/10.0);
	if (currLat*10 > m_mapStartLat)
	{
		scrollMap();
	}

	// update critters
	updateCritters();	

	// update path diffusion
	updatePathDiffusion();
}

void GlacierGame::gameConfig()
{
	// hey cool this works on windows
	struct _stat statInfo;
	_stat( "settings.txt", &statInfo );
	static time_t lastModTime = 0;
	if (statInfo.st_mtime != lastModTime)
	{
		// config has changed, reread it
		lastModTime = statInfo.st_mtime;
		boost::program_options::variables_map arg_map;
		bool goodOpts = true;

		try
		{
			ifstream ifs_config("settings.txt");
			boost::program_options::store(					
				boost::program_options::parse_config_file( ifs_config, m_gameplayOpts ),
				arg_map );

			boost::program_options::notify( arg_map );
		}
		catch (boost::program_options::error err_opt) 
		{		
			_RPT1( _CRT_WARN, "Error reading settings.text: %s\n", err_opt.what() );

			// don't freak out on errors, could be in the
			// middle of editing the file, just ignore them
			// until next save
			goodOpts = false;
		}	

		if (goodOpts)
		{
			applyOptions( arg_map );
		}
	}

}

void GlacierGame::applyOptions( boost::program_options::variables_map &arg_map )
{
	if (arg_map.count("gameplay.travel_speed"))
	{
		m_travelSpeed = arg_map["gameplay.travel_speed"].as<float>();
	}

	// Apply critter options
	for (int i=0; i < CRITTER_LAST; i++)
	{
		string argCritSpeed = string(m_critNames[i]) + ".delay";
		if (arg_map.count(argCritSpeed))
		{
			m_critSpeed[i] = arg_map[argCritSpeed].as<int>();

		}
	}

	//DBG
	_RPT1( _CRT_WARN, "mammoth delay is %d\n", m_critSpeed[CRITTER_MAMMOTH] );
}

void GlacierGame::updatePathDiffusion()
{
	// diffuse the map
	float newPathVals[MAP_ROWS][MAP_COLS];

	foreach_map
	{
		MapSquare &m = MAPITER( m_map );
		int count = 0;
		float sum = 0;
		for (int i=-1; i <= 1; ++i)
		{
			for (int j=-1; j <= 1; ++j )
			{
				if  ((i!=0) && (j!=0)) continue;
				if ((i==0) && (j==0)) continue;
				int mx = mi + i;
				int my = mj + j;
				if ( (mx >=0) && (mx < MAP_ROWS) &&
					 (my >=0) && (my < MAP_COLS) )
				{
					MapSquare &m2 = m_map[mx][my];
					
					if (m.m_hite == m2.m_hite)
					{	
						// reachable, full strength
						sum += m2.m_path;
					}
					else
					{
						sum += m2.m_path * 0.2;
						//sum += m2.m_path;
					}
					count++;
				}
			}
		}

		newPathVals[mi][mj] = (count > 0) ? sum / count : 0;
	}	

	// always the "freedom row" to 1
	// and the "glacier row" to 0
	for (int i=0; i < MAP_ROWS; i++)
	{
		newPathVals[i][0] = 0.0;
		newPathVals[i][MAP_COLS-1] = 100.0;
	}

	// set the critters to 0 so they path around
	// each other
	for (int i=0; i < m_critters.size(); ++i )
	{
		Critter &c = m_critters[i];
		newPathVals[c.px][c.py] = 0;
	}

	// copy in the new map weights
	foreach_map
	{
		MapSquare &m = MAPITER( m_map );
		m.m_path = newPathVals[mi][mj];
	}
}

void GlacierGame::updateCritters()
{
	std::vector<Critter> newCrits;

	for (int i=0; i < m_critters.size(); ++i )
	{
		Critter &c = m_critters[i];

		// Check if the critter has escaped
		if (c.py == 0)
		{
			// TODO: Lose! or something...
			continue;
		}

		// check if it's dead
		if (c.dead)
		{
			continue;
		}

		// Recharge our move
		if (c.recharge == 0)
		{
			int newX = c.px, newY = c.py;
			float newVal = m_map[c.py][c.py].m_path;
			for (int i=-1; i <= 1; ++i)
			{
				for (int j=-1; j <= 1; ++j )
				{
					if  ((i!=0) && (j!=0)) continue;
					if ((i==0) && (j==0)) continue;
					int mx = c.px + i;
					int my = c.py + j;
					if ( (mx >=0) && (mx < MAP_ROWS) &&
						 (my >=0) && (my < MAP_COLS) )
					{
						// same elevation?
						if (m_map[mx][my].m_hite != m_map[c.px][c.py].m_hite)
						{
							continue;
						}

						// walk in the most appealing direction
						 if (m_map[mx][my].m_path > newVal)
						 {
							 newVal = m_map[mx][my].m_path;
							 newX = mx; newY = my;
						 }
					}
				}
			}
			c.px = newX;
			c.py = newY;

			c.recharge = m_critSpeed[ c.imageNdx ];
		}
		else
		{
			c.recharge--;
		}

		// keep this crit
		newCrits.push_back( c );
	}

	m_critters = newCrits;
}