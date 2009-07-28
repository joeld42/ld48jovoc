#include <windows.h>
#include <vector>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <prmath/prmath.hpp>

// Kudzu simulation triangle
struct KSimTri {
	vec3f a, b, c; // verts
	vec2f ka, kb, kc;   // coords in kudzu space

	vec3f nrm; // triangle norm

	// links to other triangles across edges
	// -1 means edge of mesh
	int ablink, bclink, calink;

	// barycentric stuff
	vec3f uv2bary( vec2f uv );	
	vec3f pos2bary( vec3f pos );
	vec3f bary2pos( vec3f bary );
	vec2f bary2uv( vec3f bary );
};

// Kudzu node
struct KNode {
	vec2f pos; // in kudzu space	
	vec3f pos3; // 3d pos cached

	// only used during firing
	float dir;
	float v;
	float bdist; // how far it has travelled since the last bunch
	//bool unlinked; // unlinked nodes (just used for "kudzu explosion" )
	bool erase_flag;
	bool explode_flag;

	//std::vector<int> links;	
};

// Kudzu bunch -- a bunch of leafy leaves, these
// recharge you and count for coverage
struct KBunch {
	vec2f pos; // in kudzu space	
	vec3f pos3; // 3d pos cached
	float age;
};

struct KLevel {
	void loadSimMesh( const char *filename );

	// useful stuff

	// kudzu space (st) to 3d position
	vec3f k2pos( vec2f kpos );

	// lookup normal at pos
	vec3f k2nrm( vec2f kpos );	

	// lookup sim info 
	bool getSimInfo( vec2f kpos, float heading, 
					 vec3f &pos, vec3f &nrm, vec3f &dir,
					 KSimTri &tri, vec3f &bary);

	std::vector<KSimTri> m_tri;
	std::vector<KNode> m_nodes;
	
	std::vector<KBunch> m_bunches;
	
};

bool pointInTri( const vec2f &p, KSimTri &tri );