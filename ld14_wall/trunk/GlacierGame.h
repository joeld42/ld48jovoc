#ifndef GLACIER_GAME_H
#define GLACIER_GAME_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <prmath/prmath.hpp>

#include <vector>

#include "boost/program_options.hpp"

#define MAP_ROWS (10)
#define MAP_COLS (25)

// Save me some typing

#define foreach_map \
	for (int mi = 0; mi < MAP_ROWS; ++mi ) \
		for (int mj = 0; mj < MAP_COLS; ++mj )

#define MAPITER( _map ) \
	_map[mi][mj]

struct Tetromino
{
	Tetromino();
	Tetromino( int b00, int b01,
			   int b10, int b11,
			   int b20, int b21,
			   int b30, int b31 );


	GLuint m_mesh;	
	int m_blocks[4][2];
};

struct MapSquare
{
	MapSquare();
	int m_hite;
	float m_path;	
	
	// used in dropTet -- can we scurry here?
	bool m_scurry;
};

struct Critter
{
	Critter()
	{
		px = 0; py = 0;
		recharge = 0;
		dead = 0;
	}
	int px, py;
	int imageNdx;
	int recharge; // how many simticks before it can move again?
	bool dead;
};

enum 
{
	CRITTER_MAMMOTH,
	CRITTER_LAST	
	//etc...
};

struct LandBlock;

class GlacierGame
{
public:
	GlacierGame();

	void update( float dtFixed );

	void applyOptions( boost::program_options::variables_map &arg_map );

//protected

	// The glacier's progress southward
	float m_latitude;
	float m_travelSpeed;
	int m_mapStartLat;

	void gameConfig();

	void clearMap();
	void scrollMap();
	void updateHite();

	void loadLevel( const char *levelName );
	void saveLevel( const char *levelName );

	void updatePathDiffusion();
	void updateCritters();

	void calcHeights();

	void clearBlocks();
	
	bool dropTet( bool checkOnly, int &tileHite );

	Tetromino *randTet();

	std::vector<Tetromino*> m_tets;

	MapSquare m_map[MAP_ROWS][MAP_COLS];
	int m_selX, m_selY;
	int m_tetRot;
	Tetromino *m_currTet, *m_nextTet;
	
	std::vector<Critter> m_critters;

	bool m_editMode;
	std::vector<LandBlock*> m_landBlocks;

	// critter properties
	static const char *m_critNames[16];
	int m_critSpeed[16];

	char m_levelHite[ MAP_ROWS ][1000];

	// gameplay opts
	boost::program_options::options_description m_gameplayOpts;
};

#endif