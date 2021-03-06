#ifndef CAVERN_H
#define CAVERN_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <prmath/prmath.hpp>

#include <Shape.h>

#ifndef WIN32
#define stricmp strcasecmp
#endif

// line segment for collision
enum {
	SegType_COLLIDE,
	SegType_DIALOGUE,
	SegType_KILL,

	SegType_LAST
};

extern const char *SegTypeNames[];

struct Segment
{
	vec2f a, b;
	int segType;
};

class Cavern
{
public:
	Cavern();

	void loadLevel( const char *levelFile, std::vector<Shape*> &shapeList );
	void saveLevel( const char *levelFile );

	void addShape( Shape *s );
	void sortShapes();

	void draw();

	void addSegment( vec2f a, vec2f b, int type );

//protected:
	vec2f m_mapSize;
	std::vector<Shape*> m_shapes;	
	std::vector<Segment> m_collision;
	vec3f m_bgColor;

	vec2f m_spawnPoint;
};

#endif
