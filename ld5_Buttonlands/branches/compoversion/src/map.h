#ifndef MAP_H
#define MAP_H

#include <string>

#include <allegro.h>

#include "buttonlands.h"

#define MAX_LOC (30)

// background 
enum {
	BG_FOREST,
	BG_RUINS,
	BG_TOWN,
};

struct BMLoc {
	std::string name;
	int bgType;
	int xpos, ypos;
	std::string desc;
	int index;

	int heroVisited;
};

// dual edge, just used to draw the blocking stuff
struct DualEdge {
	float x1, y1, x2, y2;
};

struct BMAdjInfo {
	char pass;
	char terrain;
	DualEdge *de;

	BMAdjInfo() {
		pass = 0;
		de = NULL;
	}
};

enum {	

	// passable terrains
	TERR_ROAD,

	// impassable terrains
	TERR_MOUNTIAN,
};


// used for road making
struct Triangle {
	float ax, ay, bx, by, cx, cy;	
};

struct BMMap {

	BITMAP *bmpMap;       // Map image		   

	// adjacency (roads)
	BMAdjInfo adj[MAX_LOC][MAX_LOC]; 	

	BMLoc loc[MAX_LOC];

	int nloc;

	BMMap() {
		bmpMap = NULL;
	}

	// makes the map, creates bitmaps, etc
	void make_map( int num_loc, float spacing );
	void make_edges_delauney( );

	bool is_connected();

	DualEdge *find_dual_edge( std::vector<Triangle> &tris, int cndx, int andx );
};

#endif