#ifndef TIE_GAME_H
#define TIE_GAME_H

#include "TIE.h"
#include "Level.h"
#include "SceneObject.h"
#include "Player.h"
#include "IceFloe.h"
#include "Road.h"
#include "Sheep.h"
#include "Crate.h"
#include "Sound.h"

class TransIcelandicExpress {

public:
	TransIcelandicExpress();

	void initialize();
	void eventLoop( void );

	void loadLevel( const char *filename );
	LevelInfo preloadLevel( char *filename );

	IceFloe *getFloe( sgVec3 &pos );
	float getHeight( sgVec3 &pos );

	static float c_RoadLevel;
	static float c_PlayerHeight;

protected:
	void do_quit();

	void mouseMotion( int xrel, int yrel );

	void computeTimeDelta();

	// simulation
	void simulate();

	// sound
	void play_sound( int sfx );

	// building
	void updateBlueprint();
	void buildSomething();

	void checkStability();
	void fallRoad( int i, int j );

	// drawing routines
	void redraw();
	void draw3d();
	void draw2d();
	void setupLights();

	void drawOcean();

	std::deque<float> dbgVel;

	// The player
	Player *player;
	sgVec3 playerStartPos;
	float cf_moveForward, 
		  cf_moveSideways,
		  cf_jump;
	bool falling, jumping, onroad, elevating;
	int num_crates;

	bool deathSnd;

	// levels
	std::vector<LevelInfo> levelInfo;
	int levelListNum;

	// The roadways
	Road *road[MAX_ROAD][MAX_ROAD];
	int roadX, roadY;
	Road *blueprint;
	bool blueprint_on;
	bool allconnected;
	bool checkStable;
	
	VictoryType victory;


	// The ice floes
	std::vector<IceFloe*> floes;

	std::vector<Sheep*> sheep;
	std::vector<Crate*> crates;

	std::vector<SceneObject*> brokenThings;

	// scene lighting
	sgVec4 light_pos;
	sgVec4 light_diff, light_amb, light_spec;

	sgVec4 ocean_diff;

	// the camera
	sgVec3 cameraPos; // WRT player
	bool showHelp, dbgPhysics, showLevels;

	// the music
	// (move to sound.h)
	Mix_Music *music;
	Mix_Chunk *sfx[ NUM_SFX ];
	int sfx_chan[ NUM_SFX ];
	bool musicOn;

	
	
	// some dots to do something with libSG
	#define NUM_DOTS (1000)
	sgVec3 dots[NUM_DOTS];
	bool dot_init;

	float angle;
	float deltaT;
};


#endif