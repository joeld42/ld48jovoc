#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <string>
#include <vector>
#include <deque>
#include <map>

#include <prmath/prmath.hpp>

//#define SCREEN_RES_X (1280)
//#define SCREEN_RES_Y (1024)
#define SCREEN_RES_X (800)
#define SCREEN_RES_Y (600)

#define MAX_BOARD_SIZE 30

struct Tile
{	
	Tile():blocked(false), 
		letter(' '), 
		ghost_letter(' '), 
		error( 0.0 ), target(false) {}
	bool blocked;
	char letter; // space means no letter
	char ghost_letter;

	// for routing
	bool visited;
	bool target;
	
	float error; // to indicate errors;
	int pathx, pathy;
};

// tiles a player can choose from
struct PickTile
{
	PickTile() : pending( false), dragging(false) {}

	char letter;
	float xpos;
	bool dragging;
	bool pending;
};

struct CheckWord
{
	int startx, starty, endx, endy;
	std::string word;
};

class GameState
{
public:
	GameState();
	
	void update( float dtFixed );

	int boardWidth() const { return m_boardSizeX; }
	int boardHeight() const { return m_boardSizeY; }

	vec2f getCarPos() const { return m_carPos; }
	vec2f getCarDir() const { return m_carDir; }

	Tile &tile( int i, int j );
	char getLetter( int i, int j );

	void makeSymmetric();

	void drawDbgDisplay();

	void loadLevel( const char *levelfile );

	void loadWordList( const char *wordlist );

	// actions
	void commitWord();
	void cancelWord();

	void nextLevel();
	void restartLevel();

	std::vector<PickTile> &pickTiles() { return m_pickTiles; }
	int getMaxTiles() { return m_maxTiles; }

protected:
	
	void updateRoute();

	vec2f m_carPos, m_carDir;

	std::deque<vec2f> m_carRoute;

	// meant to use a trie but im running out of 
	// time and hey, it's only memory, right?
	typedef std::map<std::string,bool> WordList;
	WordList m_wordList;

	// tiles the player can pick from
	std::vector<PickTile> m_pickTiles;
	float m_newTileTimeout;
	int m_maxTiles;
	int m_level;

	vec2f m_dragTilePos;

	int m_boardSizeX, m_boardSizeY;
	Tile m_tile[MAX_BOARD_SIZE][MAX_BOARD_SIZE];

};


#endif