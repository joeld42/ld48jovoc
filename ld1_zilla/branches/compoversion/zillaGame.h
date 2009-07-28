
#ifndef ZILLA_H
#define ZILLA_H

#include <vector>
#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <windows.h>
#include <crtdbg.h>

#include <math.h>
#include <GL/glut.h>

#include <plib/sg.h>
#include <plib/fnt.h>

#include <png.h>


typedef int Tri[3];

#define MAX_TEXTURES (50)
#define ID_UNASSIGNED (0xFFFF)

// A meshpart is a part of the mesh that uses 
// the same texture state
struct MeshPart {
	char *name;
	Tri *tri;
	int nTri;
};

struct Texture {
	char *name;
	unsigned long w, h;
	unsigned char *data;
	bool hasAlpha;

	Texture( const char *filename, bool hasAlpha = false );

	void loadPngRGB( const char *filename );
	void loadPngRGBA( const char *filename );

	void bindTexture();
	static void resetBound();

	// texture id's
	GLuint id;

	static GLuint texIdPool[MAX_TEXTURES];
	static int currTexId;

	static Texture *currentBound;
};

class Mesh {
public:
	Mesh( const char *filename );

	void draw();

protected:
	sgVec3 *vtx;
	sgVec3 *nrm;

	sgVec2 *st;
	Texture *tex;

	int nVtx;

	// the mesh is split into parts
	std::vector<MeshPart*> part;

	// file reading internals
	void loadFile( const char *filename );
	void skaReadVertexSection( FILE *fp, int num );
	void skaReadNormalSection( FILE *fp, int num );
	void skaReadSurfaceSection( FILE *fp, int num );
	void skaReadUVmapSection( FILE *fp, int num );
	void skaSkipUnknownSection( FILE *fp );
	
	static int nextDisplayList;
	int dlistId;
};


struct Tile {
	Tile( char *_name, char _symbol, bool _passable, bool _destructible );

	char *name;
	char symbol;
	bool passable;
	bool destructible;

	Mesh *mesh;
};

#define MAX_LEVEL_DIM (50)
struct Level {

	Level( char *name, char *desc, int w, int h );

	void setTile( int x, int y, Tile *tile );

	void draw();

	void worldToMap( float worldX, float worldZ,
					 int &mapX, int &mapY );

	void mapToWorld( int mapX, int mapY,
					 float &worldX, float &worldZ );
					 

	Tile *map[MAX_LEVEL_DIM][MAX_LEVEL_DIM];
	char *name;
	char *desc;
	int w, h;
};

class Actor {
public:
	Actor( Mesh *mesh, sgVec2 pos );

	enum State {
		Idle,
		Moving,
		Attacking
	};

	void draw();
	virtual void update( Level *lvl, float t );

	bool moveTowardTarget( float t );

	Mesh *mesh;

	// info
	sgVec2 pos;
	float heading;
	State state;
	

	// abilities
	float speed;

	// moving
	sgVec2 target;
	float targetRad;

	std::list<sgVec2*> waypoints;
};

class Godzilla : public Actor {
public:
	Godzilla( Mesh *mesh, sgVec2 pos ) : Actor( mesh, pos ) { start=true; }

	virtual void update( Level *lvl, float t );

	bool start;
};

class ZillaGame {
public:
	ZillaGame( );

	enum GameState {
		TitleScreen,
		InGame
	};

	void loadGameData();

	void Display();

	void Display3D();
	void Display2D();

	void DisplayTitle3D();
	void DisplayTitle2D();

	void Update( float timeDelta );	
	void UpdateTitle( float timeDelta );	

	void Key( unsigned char key, int x, int y );
	void Special( int key, int x, int y );

	void Mouse( int button, int state, int x, int y );
	
protected:
	void loadTileset( const char *filename );
	void loadLevels( const char *filename );

	sgVec3 camPos, camLookat;
	std::list<Tile*> tileset;
	std::vector<Level*> levels;

	Level *level;
	int levelNdx;

	float currAngle, currHeight, currZoom;
	bool dataLoaded;

	// camera matrix for later use
	GLdouble modelview[16], proj[16];
	GLint viewport[4];
	sgVec3 dbgClickPnt;

	// fontstuff
	fntRenderer *fnt;
	fntTexFont *fntHand, *fntHelv, *fntHelvBold;

	// skybox
	Mesh *skybox;

	// game state
	GameState state;

	// actors
	std::vector<Actor*> actors;

	// hero actors
	Godzilla *godzilla;
	bool follow;

	// title screen
	Texture *titleScreen;

	// fps counter
	static clock_t lastFpsTick;
	static int fpsFrameCount, c_fpsUpdateNum;
	static float fps;

public:
	static ZillaGame *theGame;
	
	static int c_Width, c_Height;

	static void cbDisplay();
	static void cbIdle();
	static void cbSpecial( int key, int x, int y);
	static void cbKey( unsigned char key, int x, int y  );
	static void cbMouse( int button, int state, int x, int y );

protected:
	static clock_t lastTick;

};

#endif