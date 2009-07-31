
#ifndef ZILLA_H
#define ZILLA_H

#include <vector>
#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifdef WIN32
# include <windows.h>
# include <crtdbg.h>
#endif

#include <math.h>
#include <GL/glut.h>

#include <prmath/vector2.hpp>
#include <prmath/vector3.hpp>

#include <png.h>


typedef int Tri[3];

#define MAX_TEXTURES (50)
#define ID_UNASSIGNED (0xFFFF)

#define ENABLE_FONT (0)

struct MapPos {
	MapPos() {}
	MapPos( int _x, int _y ) { x = _x; y = _y; }
	int x, y;
};

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
	vec3f *vtx;
	vec3f *nrm;

	vec2f *st;
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
	Tile( char *_name, char _symbol, bool _passable,
			bool _destructible, bool _tankpassable );

	char *name;
	char symbol;
	bool passable;
	bool destructible;
	bool tankpassable;

	Mesh *mesh;
};

struct TileInfo {
	bool pass, visited;
	float cost, pathcost;
};

#define MAX_LEVEL_DIM (100)
struct Level {

	enum PathMode {
		ModeZilla,
		ModeTank
	};

	Level( char *name, char *desc, int w, int h );

	void setTile( int x, int y, Tile *tile );

	void draw();

	void worldToMap( float worldX, float worldZ,
					 int &mapX, int &mapY );

	void mapToWorld( int mapX, int mapY,
					 float &worldX, float &worldZ );

	bool pickRandomMatch( const char *syms, int &resultx, int &resulty );

	// path is returned by filling 'waypoints'
	bool pathTo( int startx, int starty, 
				 int targx,  int targy,
				 PathMode mode, 
				 std::list<vec2f*> &waypoints );
					 

	Tile *map[MAX_LEVEL_DIM][MAX_LEVEL_DIM];
	char *name;
	char *desc;
	int w, h;

protected:
	void buildPath( int startx, int starty, 
					int targx,  int targy,				 
					std::list<vec2f*> &waypoints );

	// pathfinding map
	TileInfo pathMap[MAX_LEVEL_DIM][MAX_LEVEL_DIM];
};

class Actor {
public:
	Actor( Mesh *mesh, vec2f pos );

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
	vec2f pos;
	float heading;
	State state;
	

	// abilities
	float speed;

	// moving
	vec2f target;
	float targetRad;

	std::list<vec2f*> waypoints;
};

class Godzilla : public Actor {
public:
	Godzilla( Mesh *mesh, vec2f pos ) : Actor( mesh, pos ) { start=true; }

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
	void Motion( int x, int y );
	
protected:
	void loadTileset( const char *filename );
	void loadLevels( const char *filename );

	// dir is in screen space (kind of)
	void moveCamera( vec2f dir );

	vec3f camPos, camLookat;
	std::list<Tile*> tileset;
	std::vector<Level*> levels;

	Level *level;
	int levelNdx;

	float currAngle, currHeight, currZoom;
	bool dataLoaded;

	// camera matrix for later use
	GLdouble modelview[16], proj[16];
	GLint viewport[4];
	
	// mouse stuff
	vec3f cursor3d;
	vec2f cursor2d;
	int mapcursorX, mapcursorY;
	bool onMap;

#if ENABLE_FONT
	// fontstuff
	fntRenderer *fnt;
	fntTexFont *fntHand, *fntHelv, *fntHelvBold;
#endif

	// skybox
	Mesh *skybox;

	// game state
	GameState state;

	// actors
	std::vector<Actor*> actors;

	// hero actors
	Mesh *tankMesh;
	Godzilla *godzilla;
	bool follow;

	// dbg stuff
	bool drawpath;

	// title screen
	Texture *titleScreen;

	// UI stuff
	Texture *cursor, *btntank;	

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
	static void cbMotion( int x, int y );

protected:
	static clock_t lastTick;

};

#endif
