
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <ctype.h>

#include <GL/gl.h>
#include <GL/glu.h>

//#include <boost/foreach.hpp>
//#define foreach BOOST_FOREACH

#include <assert.h>
#include "GameState.h"

// in seconds
#define NEW_TILE_TIMEOUT (1.0)

#define MAX_ALLOWED_TILES (15)
#define INITIAL_MAX_TILES (7)

// I am too lazy to type this over and over
#define foreach_tile \
	for( int j=0; j < m_boardSizeY; ++j ) \
		for( int i=0; i < m_boardSizeX; ++i )


GameState::GameState() :
	m_boardSizeX( 15 ), m_boardSizeY( 15 ), m_level(0)
{
	// init car pos
	m_carPos.x = (float)m_boardSizeX / 2.0;
	m_carPos.y = (float)m_boardSizeY / 2.0;
	m_carDir = vec2f( 0.5, 0.5 );	

	m_newTileTimeout = NEW_TILE_TIMEOUT;
	m_maxTiles = INITIAL_MAX_TILES;

	// read the wordlist
	loadWordList( "gamedata/2of12inf.txt" );	
	
}

void GameState::loadWordList( const char *wordlist )
{
	FILE *fp = fopen( wordlist, "rt" );
	char word[100], clean_word[100];
	while(!feof(fp))
	{
		fscanf( fp, "%s", word );
		char *ch2 = clean_word;
		char lastchar = 'z';
		for (char *ch=word; *ch; ++ch)
		{
			// remove %'s (indicates plurals)..
			// remove u's following q, for gameplay purposes
			// q is "qu"
			if ((*ch!='%') && ( (*ch!='u') || (lastchar!='q') ) )
			{
				*ch2 = toupper(*ch);
				*ch2++;
			}
			lastchar = *ch;
		}
		*ch2 = '\0';
		
		// add word
		std::string strword(clean_word );
		m_wordList[strword] = true;
	}

	// hack? easter egg?
	m_wordList["LUDUM"] = true;
	m_wordList["JOVOC"] = true;

	_RPT1( _CRT_WARN, "Loaded %d words\n", m_wordList.size() );
}

void GameState::makeSymmetric()
{
	int bx = m_boardSizeX - 1;
	int by = m_boardSizeY - 1;
	
	foreach_tile
	{
		if (m_tile[i][j].blocked) m_tile[bx-i][by-j].blocked = true;
	}
}

void GameState::loadLevel( const char *levelfile )
{
	FILE *fp = fopen( levelfile, "rt" );
	fscanf( fp, "%d %d\n", &m_boardSizeX, &m_boardSizeY );

	for (int row=0; row < m_boardSizeY; ++row)
	{
		for (int col=0; col < m_boardSizeX; ++col)
		{
			m_tile[col][row] = Tile();
		}
	}

	for (int row=0; row < m_boardSizeY; ++row)
	{
		char buff[1024];
		fscanf( fp, "%s", buff );

		char *ch = buff;
		if (strlen( buff ) < m_boardSizeX *2) 
		{
			// TODO: error or warn
			continue;
		}

		for (int col=0; col < m_boardSizeX; col++) 
		{
			Tile tile;
			if (*ch=='#')
			{
				tile.blocked = true;
			}
			else if (*ch=='*')
			{
				tile.target = true;
			}
			else if (isalpha(*ch))
			{
				tile.letter = toupper(*ch);
				if (*(ch+1) == '+')
				{
					m_carPos.x = (float)col + 0.5f;
					m_carPos.y = (float)row + 0.5f;
				}
			}

			// put in map
			m_tile[col][row] = tile;
			ch += 2;
		}		
	}

	fclose( fp );
}

void GameState::nextLevel()
{
	m_pickTiles.erase( m_pickTiles.begin(), m_pickTiles.end() );
	
	m_carRoute.erase( m_carRoute.begin(), m_carRoute.end() );

	char buff[64];	
	m_level++;
	sprintf( buff, "gamedata/level_%02d.txt", m_level );
	loadLevel( buff );
}

void GameState::restartLevel()
{
	m_level--;
	nextLevel();
}

void GameState::update( float dtFixed )
{	

	// If the car is out of route points, generate a route
	updateRoute();	

	// check for win
	int currPosX, currPosY;
	currPosX = (int)m_carPos.x;
	currPosY = (int)m_carPos.y;
	if ((currPosX >= 0) && (currPosY>=0) &&
		(currPosX < m_boardSizeX) &&
		(currPosY < m_boardSizeY) &&
		(m_tile[currPosX][currPosY].target))
	{
		// win!
		nextLevel();
	}

	// Move the car along the route
	vec2f targPos, targDir;
	if (m_carRoute.size()== 0 ) updateRoute();
	
	targPos = m_carRoute.front();
	targDir = targPos - m_carPos;
	if (LengthSquared(targDir) < 0.01)
	{	
		// we've reached this route point, discard it
		m_carRoute.pop_front();
		
		// update route to make sure there is data available
		updateRoute();

		targPos = m_carRoute.front();
		targDir = targPos - m_carPos;		
	}
	targDir.Normalize();

	// Adjust car dir 
	float turnrate = 0.4f; // not entirely true, but close enough
	m_carDir = Lerp( m_carDir, targDir, turnrate );
	m_carDir.Normalize();

	// Update car pos
	float speed = 2.0;
	m_carPos += m_carDir * dtFixed * speed;

	// Now update player's tiles
	if (m_pickTiles.size() < m_maxTiles)
	{
		m_newTileTimeout -= dtFixed;
		if (m_newTileTimeout <= 0.0f)
		{
			// pick a tile at random (scrabble probabilities)
			const char *tiles = "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ";
			char letter = tiles[ (int)( ((float)rand() / (float)RAND_MAX) * strlen(tiles) ) ];
			PickTile newtile;
			newtile.letter = letter;
			newtile.xpos = m_maxTiles;
			newtile.pending = false;
			m_pickTiles.push_back( newtile );			
			m_newTileTimeout = NEW_TILE_TIMEOUT;
		}
	}

	// fade error
	foreach_tile
	{
		if (m_tile[i][j].error > 0.0)
		{
			m_tile[i][j].error -= 0.01;
		}
		else
		{
			m_tile[i][j].error = 0.0;
		}
	}
}

Tile &GameState::tile( int i, int j )
{
	assert( i < m_boardSizeX );
	assert( j < m_boardSizeY );
	assert( i >= 0 );
	assert( j >= 0 );

	return m_tile[i][j];
}

void GameState::drawDbgDisplay()
{	
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE );
	glLineWidth( 2.0 );
	
	glBegin( GL_LINES );
	
	// direction
#if 0
	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( m_carPos.x, 0.025, m_carPos.y );
	glVertex3f( m_carPos.x + m_carDir.x, 0.02, m_carPos.y +m_carDir.y  );
#endif

	// next route point
	if (!m_carRoute.empty())
	{
		glColor3f( 0.0, 0.5, 0.0 );
		glVertex3f( m_carPos.x, 0.025, m_carPos.y );
		glVertex3f( m_carRoute.front().x, 0.025, m_carRoute.front().y );	
	}

	glEnd();

	glColor3f( 0.0, 1.0, 0.0 );
	glBegin( GL_LINE_STRIP );	
	//foreach( vec2f &p, m_carRoute )
	for( std::deque<vec2f>::iterator pi = m_carRoute.begin(); 
		pi != m_carRoute.end(); ++pi)
	{
		vec2f p = *pi;
		glVertex3f( p.x, 0.025, p.y );
	}
	glEnd();		

	glEnable( GL_TEXTURE );
	glEnable( GL_LIGHTING );
}

struct CandidateTile
{
	bool operator< ( CandidateTile &other )
	{
		return value > other.value;
	}

	float value;
	int x, y;
	int dist; // for route
};

void GameState::updateRoute()
{
	if (m_carRoute.size()==0)
	{
		// TODO: something real
		//while (m_carRoute.size() < 4)
		//{
		//	vec2f pos;
		//	pos.x = ((float)rand() / (float)RAND_MAX) * m_boardSizeX;
		//	pos.y = ((float)rand() / (float)RAND_MAX) * m_boardSizeY;
		//	m_carRoute.push_back( pos );
		//}


		// Pick a destination tile
		std::vector<CandidateTile> tilebag;		
		foreach_tile
		{		
			// is it a letter? consider it for definition
			if (m_tile[i][j].letter != ' ')
			{
				CandidateTile tile;
				tile.x = i; tile.y = j;

				// add some random value to tiebreak
				tile.value = (float)rand() / (float)RAND_MAX;

				// destination tiles are always win
				if (m_tile[i][j].target)
				{
					tile.value += 10.0;
				}

				// TODO: add more value for powerups, destinations, etc
				tilebag.push_back( tile );
			}		
		}

		// now pick one 
		sort( tilebag.begin(), tilebag.end() );
		int destX = tilebag.front().x;
		int destY = tilebag.front().y;

		_RPT2( _CRT_WARN, "dest tile %d %d\n", destX, destY );

		// clear visited in preperation to route
		foreach_tile
		{
			m_tile[i][j].visited = false;
			
			// terrible, terrible hack.. make default
			// be the dest pos to avoid crashy crasy
			// and take "shortcuts" instead
			m_tile[i][j].pathx = destX;
			m_tile[i][j].pathy = destY;
		}
		
		// push the current tile
		std::deque<CandidateTile> bfs_queue;
		std::vector<CandidateTile> path;
		CandidateTile c;
		bool found = false;
		int currX, currY;
		currX = (int)m_carPos.x;
		currY = (int)m_carPos.y; // mmmm curry
		c.x = currX;
		c.y = currY;
		m_tile[c.x][c.y].visited = true;
		bfs_queue.push_back( c );

		int count = 0;
		while (bfs_queue.size())
		{
			CandidateTile c;
			c = bfs_queue.front();
			bfs_queue.pop_front();

			//_RPT2( _CRT_WARN, "visiting node %d %d\n", c.x, c.y );

			if ((c.x == destX) && (c.y == destY))
			{
				found = true;
				//_RPT0( _CRT_WARN, "found!\n" );
				break;
			}

			// put neighbors on the queue
			for (int ii=-1; ii <= 1; ++ii)
			{
				for (int jj=-1; jj <= 1; ++jj)
				{
					if ((ii!=0) && (jj!=0)) continue; // no diagonal movement
					if ((ii==0) && (jj==0)) continue; // no stay still

					int nx, ny;
					nx = c.x + ii;
					ny = c.y + jj;


					if ((nx >= 0) && (ny >=0) && 
						(nx < m_boardSizeX) &&
						(ny < m_boardSizeY ) &&
						(!m_tile[nx][ny].visited) &&
						(m_tile[nx][ny].letter != ' ') )
					{
						CandidateTile cn = c;
						cn.x = nx; cn.y = ny;
						m_tile[cn.x][cn.y].visited = true;
						bfs_queue.push_back( cn );
						m_tile[cn.x][cn.y].pathx = c.x;
						m_tile[cn.x][cn.y].pathy = c.y;
					}	
				}
			}
#if 0			
			_RPT1( _CRT_WARN, "===== iter %d ======\n", count );
			count++;			

			for (int j=0; j < m_boardSizeY; ++j)
			{
				for (int i=0; i < m_boardSizeX; ++i)
				{
					Tile &tile = m_tile[i][j];
					if ((i==currX) && (j==currY))
					{
						_RPT0(_CRT_WARN, "@@" );
					}
					else if ((i==destX) && (j==destY))
					{
						_RPT0(_CRT_WARN, "**" );
					}
					else if (tile.blocked)
					{
						_RPT0(_CRT_WARN, "##" );
					}
					else if (tile.visited)
					{
						_RPT0(_CRT_WARN, "++" );
					}
					else if (tile.letter != ' ' )
					{
						_RPT1(_CRT_WARN, "%c ", tile.letter );
					}
					else
					{
						_RPT0(_CRT_WARN, ".." );
					}
				}
				_RPT0(_CRT_WARN, "\n" );
			}
#endif
			//assert( count < 50 );
		}

		assert(found);
		
		// reconstruct path
		int px = destX;
		int py = destY;
		//_RPT0( _CRT_WARN, "reconstruct path\n" );
		while (1)
		{			
			//_RPT4( _CRT_WARN, "At pos %d %d seeking %d %d\n", px, py, currX, currY );
			m_carRoute.push_front( vec2f( (float)px + 0.5, (float)py + 0.5 ) );
			int px2, py2;
			px2 = m_tile[px][py].pathx;
			py2 = m_tile[px][py].pathy;
			px = px2; py = py2;
			if ( (px == currX) && (py==currY) )
			{
				break;
			}

			// prevent loops
			assert( m_carRoute.size() < 100 );
			if (m_carRoute.size() >= 100)
			{
				exit(1);
			}
		}	
		//_RPT0( _CRT_WARN, "done\n" );


		// TODO: find path
		//m_carRoute.push_back( vec2f( (float)destX + 0.5, (float)destY + 0.5 ) );
	}
}

void GameState::cancelWord()
{
	for (int i=0; i < m_pickTiles.size(); ++i)
	{
		m_pickTiles[i].pending = false;
	}

	foreach_tile
	{
		m_tile[i][j].ghost_letter = ' ';
	}
}

char GameState::getLetter( int i, int j )
{
	if (i>=m_boardSizeX) return ' ';
	if (j>=m_boardSizeY) return ' ';

	if (m_tile[i][j].letter != ' ')
	{
		return m_tile[i][j].letter;
	}
	else return m_tile[i][j].ghost_letter;
}
void GameState::commitWord()
{
	// check all tiles are connected
	foreach_tile
	{
		m_tile[i][j].visited = false;
	}

	int currPosX, currPosY;
	currPosX = (int)m_carPos.x;
	currPosY = (int)m_carPos.y;
	m_tile[currPosX][currPosY].visited = true;
	
	bool done = false;
	while (!done)
	{
		int changed = 0;
		foreach_tile
		{
			if (!m_tile[i][j].visited) continue;

			//visit neighbors
			for (int ii=-1; ii <= 1; ++ii)
			{
				for (int jj=-1; jj <= 1; ++jj)
				{
					if ((ii!=0) && (jj!=0)) continue; // no diagonal movement
					if ((ii==0) && (jj==0)) continue; // no stay still

					int nx, ny;
					nx = i + ii;
					ny = j + jj;


					if ((nx >= 0) && (ny >=0) && 
						(nx < m_boardSizeX) &&
						(ny < m_boardSizeY ) &&
						(!m_tile[nx][ny].visited) &&
						((m_tile[nx][ny].letter != ' ')||
						 (m_tile[nx][ny].ghost_letter != ' ')) )
					{
						// reachable
						changed++;
						m_tile[nx][ny].visited = true;
					}
				}
			}
		}

		if (!changed)
		{
			done = true;
		}
	}

	bool allReachable = true;
	foreach_tile
	{
		if ( ( (m_tile[i][j].letter!=' ') || 
			   (m_tile[i][j].ghost_letter!=' ') )
			   && (!m_tile[i][j].visited) )
		{
			// not reachable
			m_tile[i][j].error = 1.0;
			allReachable = false;
		}
	}

	if (!allReachable) return;

	// Check all words are valid
	std::vector<CheckWord> checkwords;
	// check all the words on the board
	// check rows
	bool inWord = false;
	char lastLetter = ' ';
	CheckWord ck;

	for (int j=0; j <= m_boardSizeY; ++j )
	{
		for (int i=0; i <= m_boardSizeY; ++i )
		{	
			char l = getLetter( i, j );
			if (!inWord)
			{				
				if (l!=' ')
				{
					// entered a word
					inWord = true;
					ck.word = l;
					ck.startx = i; 
					ck.starty = j; 
				} // otherwise keep looking
			}
			else
			{
				// we're in a word
				if (l==' ')
				{
					// exiting a word.. add it
					if (ck.word.size() > 1)
					{						
						ck.endx = i-1;
						ck.endy = j;
						checkwords.push_back( ck );
					}
					inWord = false;
				}
				else
				{
					// accumulate
					ck.word.push_back( l );
				}
			}
		}
	}


	// Horrible cut/paste, check columns
	for (int i=0; i <= m_boardSizeY; ++i )	
	{
		for (int j=0; j <= m_boardSizeY; ++j )
		{	
			char l = getLetter( i, j );
			if (!inWord)
			{				
				if (l!=' ')
				{
					// entered a word
					inWord = true;
					ck.word = l;
					ck.startx = i; 
					ck.starty = j; 
				} // otherwise keep looking
			}
			else
			{
				// we're in a word
				if (l==' ')
				{
					// exiting a word.. add it
					if (ck.word.size() > 1)
					{						
						ck.endx = i;
						ck.endy = j-1;
						checkwords.push_back( ck );
					}
					inWord = false;
				}
				else
				{
					// accumulate
					ck.word.push_back( l );
				}
			}
		}
	}

	bool allFound = true;
	for (int i=0; i < checkwords.size(); ++i )
	{		
		WordList::iterator wi = m_wordList.find( checkwords[i].word );
		_RPT2( _CRT_WARN, "Word: %s Found: %s\n", checkwords[i].word.c_str(), (wi!=m_wordList.end()?"TRUE":"FALSE") );

		if (wi==m_wordList.end())
		{
			allFound = false;

			// mark the error flags on the word
			for (int ii=checkwords[i].startx; ii <= checkwords[i].endx; ++ii)
			{
				for (int jj=checkwords[i].starty; jj <= checkwords[i].endy; ++jj)
				{
					m_tile[ii][jj].error = 1.0f;
				}
			}
		}
	}

	// is the board valid??
	if (!allFound) return;

	// commit the words
	std::vector<PickTile> keepTiles;
	for (int i=0; i < m_pickTiles.size(); ++i)
	{
		if (!m_pickTiles[i].pending)
		{
			keepTiles.push_back( m_pickTiles[i] );
		}
	}
	m_pickTiles = keepTiles;

	foreach_tile
	{
		if (m_tile[i][j].ghost_letter != ' ')
		{
			m_tile[i][j].letter = m_tile[i][j].ghost_letter;
			m_tile[i][j].ghost_letter = ' ';
		}
	}
}