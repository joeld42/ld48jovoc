#include <windows.h>

#include <vector>
#include <deque>

#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <plib/sg.h>
#include <plib/fnt.h>

#include <SDL_mixer.h>

// the font stuff
fntRenderer *fnt;
fntTexFont *fntHelv;

#define MAX_ROAD (100)

class TessTri {
public:
	sgVec3 a, b, c;
};

#define TAKE_MOUSE

class SceneObject {
public:

	SceneObject();

	// does the materials and xform, and calls drawGeom
	virtual void draw();

	// just the geom
	virtual void drawGeom();

	// interal useful drawing stuff
	void setupMaterials();
	void drawCube( float xs, float ys, float zs );
	
	sgVec3 pos, hpr;
	sgVec4 dbgDiffColor;	
};

class Player : public SceneObject {

public:
	Player();

	virtual void drawGeom();

	sgVec3 vel;
};

class IceFloe : public SceneObject { 

public:
	IceFloe();

	virtual void drawGeom();

	void build( int nsrcpnt, sgVec2 *srcpnt );

	// 2d, x anz only
	bool contains( sgVec3 &p );

	sgVec3 *pnts;
	int npnts;
	float health, height;
	int breakable, anchored;

	std::vector<TessTri*> tess;

	// for the tesselator
	static IceFloe *floeToTess;
	static GLUtriangulatorObj *gluTess;
	static GLenum tessMode;
	static sgVec3 *fanCenter, *stripA, *stripB;

	static void tessBegin( GLenum type );
	static void tessEnd(void );
	static void tessVertex( void *data );
	static void tessError( GLenum error );
};
IceFloe *IceFloe::floeToTess = NULL;
GLUtriangulatorObj *IceFloe::gluTess = NULL;
GLenum IceFloe::tessMode = GL_TRIANGLES;
sgVec3 *IceFloe::fanCenter = NULL;
sgVec3 *IceFloe::stripA = NULL;
sgVec3 *IceFloe::stripB = NULL;


class Road : public SceneObject{
public:
	Road();

	virtual void drawGeom();

	int road;
	int pylon;
	float base_height;

	static sgVec4 pylonColor, baseColor, roadColor;
};

sgVec4 Road::pylonColor = { 0.8f, 0.4f, 0.4f, 1.0f };
sgVec4 Road::baseColor  = { 0.3f, 0.3f, 0.3f, 1.0f };
sgVec4 Road::roadColor  = { 0.3f, 0.3f, 0.3f, 1.0f };


class TransIcelandicExpress {

public:
	TransIcelandicExpress();

	void initialize();
	void eventLoop( void );

	void loadLevel( const char *filename );

	float getHeight( sgVec3 &pos );

	static float c_RoadLevel;
	static float c_PlayerHeight;

protected:
	void do_quit();

	void mouseMotion( int xrel, int yrel );

	void computeTimeDelta();

	// simulation
	void simulate();

	// drawing routines
	void redraw();
	void draw3d();
	void draw2d();
	void setupLights();

	void drawOcean();

	std::deque<float> dbgVel;

	// The player
	Player *player;
	float cf_moveForward, 
		  cf_moveSideways;	

	// The roadways
	Road *road[MAX_ROAD][MAX_ROAD];
	int roadX, roadY;

	// The ice floes
	std::vector<IceFloe*> floes;

	std::vector<SceneObject*> sheep;
	std::vector<SceneObject*> crates;

	// scene lighting
	sgVec4 light_pos;
	sgVec4 light_diff, light_amb, light_spec;

	sgVec4 ocean_diff;

	// the camera
	sgVec3 cameraPos; // WRT player
	bool showHelp;

	// the music
	Mix_Music *music;
	Mix_Chunk *sfx[3];
	int sfx_chan[3];
	bool musicOn;

	
	
	// some dots to do something with libSG
	#define NUM_DOTS (1000)
	sgVec3 dots[NUM_DOTS];
	bool dot_init;

	float angle;
	float deltaT;
};

float TransIcelandicExpress::c_RoadLevel = 0.3f;
float TransIcelandicExpress::c_PlayerHeight = 0.03f;

//--------------------------------------------------------------------
Road::Road(): SceneObject(){
	road = 0;
	pylon = 0;
	base_height = 0.0;
}

void Road::drawGeom() {
	if (pylon) {
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, baseColor );
		drawCube( 0.25, base_height, 0.25 );

		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pylonColor );
		drawCube( 0.125, TransIcelandicExpress::c_RoadLevel, 0.125 );
	}

	if (road) {
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, roadColor );
		glTranslated( 0.0, TransIcelandicExpress::c_RoadLevel-0.001, 0.0 );
		drawCube( 1.0f, 0.001f, 1.0f );
	}
}


//--------------------------------------------------------------------

SceneObject::SceneObject() {
	sgSetVec3( pos, 0.0, 0.0, 0.0 );
	sgSetVec3( hpr, 0.0, 0.0, 0.0 );

	sgSetVec4( dbgDiffColor, 1.0, 0.5, 0.5, 1.0 );
}

void SceneObject::drawCube( float xs, float ys, float zs ) {
	
	float y0 = 0.0, y1 = ys;
	xs /= 2; zs /= 2;

	glBegin( GL_QUADS );	
	
	glNormal3f( 0.0, 0.0, -1.0 );
	glVertex3f( -xs, y0, -zs );
	glVertex3f( -xs, y1, -zs );
	glVertex3f(  xs, y1, -zs );
	glVertex3f(  xs, y0, -zs );	

	glNormal3f( 0.0, 0.0, 1.0 );
	glVertex3f(  xs, y0, zs );
	glVertex3f(  xs, y1, zs );
	glVertex3f( -xs, y1, zs );
	glVertex3f( -xs, y0, zs );	

	glNormal3f( 0.0, -1.0, 0.0 );
	glVertex3f(  xs, y0, -zs );
	glVertex3f(  xs, y0,  zs );
	glVertex3f( -xs, y0,  zs );
	glVertex3f( -xs, y0, -zs );	

	glNormal3f( 0.0,  1.0, 0.0 );
	glVertex3f( -xs, y1, -zs );
	glVertex3f( -xs, y1,  zs );
	glVertex3f(  xs, y1,  zs );
	glVertex3f(  xs, y1, -zs );

	glNormal3f( -1.0, 0.0, 0.0 );
	glVertex3f( -xs, y0,  zs );
	glVertex3f( -xs,  y1,  zs );
	glVertex3f( -xs,  y1, -zs );
	glVertex3f( -xs, y0, -zs );	

	glNormal3f( 1.0, 0.0, 0.0 );
	glVertex3f( xs, y0, -zs );
	glVertex3f( xs,  y1, -zs );
	glVertex3f( xs,  y1,  zs );
	glVertex3f( xs, y0,  zs );

	glEnd();

	

}

void SceneObject::setupMaterials() {
	glDisable( GL_TEXTURE_2D );
	//glColor3f( dbgColor[0], dbgColor[1],  dbgColor[2] );	
	glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, dbgDiffColor );
}

void SceneObject::draw() {

	setupMaterials();
	glPushMatrix();
	glTranslated( pos[0], pos[1], pos[2] );
	drawGeom();
	glPopMatrix();
}

void SceneObject::drawGeom() {	
	// base object, just draw a cube
	drawCube( 0.03f, 0.03f, 0.03f );
}

//--------------------------------------------------------------------
Player::Player() : SceneObject() {
	sgSetVec3( vel, 0.0, 0.0, 0.0 );
}

void Player::drawGeom() {
	// base object, just draw a cube
	drawCube( 0.015f, 0.03f, 0.015f );
}

//--------------------------------------------------------------------
IceFloe::IceFloe() : SceneObject() {

	sgSetVec4( dbgDiffColor, 0.4f, 0.8f, 1.0f, 1.0f );
}

void IceFloe::drawGeom() {	
	int i, j;
	float h;

	h = height * TransIcelandicExpress::c_RoadLevel;	

	// draw the sides	
	sgVec3 edge, n, up;
	sgSetVec3( up, 0.0, 1.0, 0.0 );
	glBegin( GL_QUADS );
	for (j = npnts-1, i = 0; i < npnts; j=i++) {
		sgSetVec3( edge, pnts[j][0] - pnts[i][0], 0.0,
						 pnts[j][2] - pnts[i][2] );

		sgNormalizeVec3( edge );
		sgVectorProductVec3( n, edge, up );
	
		glNormal3f( n[0], n[1], n[2] );

		glVertex3f( pnts[i][0], h, pnts[i][2] );
		glVertex3f( pnts[j][0], h, pnts[j][2] );
		glVertex3f( pnts[j][0], 0.0, pnts[j][2] );
		glVertex3f( pnts[i][0], 0.0, pnts[i][2] );						
	}
	glEnd();

	// tesselate the top
	if (!tess.size()) {
		if (!gluTess) {

			gluTess = gluNewTess();
			gluTessCallback( gluTess, GLU_BEGIN,  (void(__stdcall *)(void))tessBegin );
			gluTessCallback( gluTess, GLU_END,    (void(__stdcall *)(void))tessEnd );
			gluTessCallback( gluTess, GLU_VERTEX, (void(__stdcall *)(void))tessVertex );
			gluTessCallback( gluTess, GLU_ERROR,  (void(__stdcall *)(void))tessError );
		}

		printf("Tesselating polygon....\n");
		floeToTess = this;

		gluBeginPolygon( gluTess );
		GLdouble *v;
		for (i = 0; i < npnts; i++) {
			pnts[i][1] = h;

			v = new GLdouble[4];
			v[0] = pnts[i][0]; 
			v[1] = pnts[i][1]; 
			v[2] = pnts[i][2];			

			printf("gluTessVertex %3.4f %3.4f %3.4f\n",
				v[0], v[1], v[2] );
			gluTessVertex( gluTess, v, pnts[i] );
		}
		printf("About to call end....\n");
		gluEndPolygon( gluTess );
		printf("Done.\n");

		floeToTess = NULL;
	} 

	// draw the top
	glNormal3f( 0.0, 1.0, 0.0 );
	glBegin( GL_TRIANGLES );	
	for (std::vector<TessTri*>::iterator ti = tess.begin();
		 ti != tess.end(); ti++ ) {

		glVertex3f( (*ti)->a[0], (*ti)->a[1], (*ti)->a[2] );
		glVertex3f( (*ti)->b[0], (*ti)->b[1], (*ti)->b[2] );
		glVertex3f( (*ti)->c[0], (*ti)->c[1], (*ti)->c[2] );

	}
	glEnd();

}



bool IceFloe::contains( sgVec3 &pp ) {
	bool inside = false;
	int i,j;
	sgVec3 dp, p;
	float t;

	// put p in local coords
	sgSubVec3( p, pp, pos );

	for (j=npnts-1, i = 0; 
		i < npnts; j = i++ ) {			
		sgSubVec3( dp, pnts[j], pnts[i] );
    
		t = (p[2]-pnts[i][2])/dp[2];
		if ((t>=0.0) && (t < 1.0) && ((pnts[i][0] + (dp[0]*t)) >= p[0])) {			
			inside = !inside;
		}
	}	

	return inside;
} 

void IceFloe::build( int nsrcpnt, sgVec2 *srcpnt ) {
	sgVec2 c;	
	sgSetVec2( c, 0.0, 0.0 );
	int n=0,i;

	// find the centeroid
	for ( i = 0; i < nsrcpnt; i++) {

		c[0] += srcpnt[i][0];
		c[1] += srcpnt[i][1];
		n++;
	}

	if (n>0.00001) {
		c[0] /= n;
		c[1] /= n;
	}

	// make the points local
	pnts = new sgVec3[ nsrcpnt ];
	npnts = nsrcpnt;
	for ( i=0; i < nsrcpnt; i++) {				
		sgSetVec3( pnts[i], srcpnt[i][0] - c[0], 0.0, 
					        srcpnt[i][1] - c[1] );		
	}

	// set location
	sgSetVec3( pos, c[0], 0.0, c[1] );
}

void IceFloe::tessBegin(GLenum type) {

	tessMode = type;

	if (type==GL_TRIANGLES) {
		printf("begin triangle\n");		
		stripA = NULL;
		stripB = NULL;
	} else if (type==GL_TRIANGLE_STRIP) {
		printf("begin triangle strip\n");
		stripA = NULL;
		stripB = NULL;
	} else if (type==GL_TRIANGLE_FAN) {
		printf("begin triangle fan\n");
		fanCenter = NULL;
		stripA = NULL;
	}
}

void IceFloe::tessEnd(void) {
	printf("tess end\n");
	stripA = NULL;
	stripB = NULL;
	fanCenter = NULL;
}

void IceFloe::tessError( GLenum error ) {
	printf("Tesselation Error: %s\n", gluErrorString( error ) );
}

void IceFloe::tessVertex( void *data ) {
	sgVec3 *p = (sgVec3*)data;
	printf("tess vtx %p\n", data );
	//printf("tess vtx %3.2f %3.2f %3.2f\n", (*p)[0], (*p)[1],(*p)[2] );

#if 0
	if (tessMode==GL_TRIANGLES) {
		if ((stripA) && (stripB)) {
			TessTri *tri = new TessTri();
			sgCopyVec3( tri->a, *stripA );
			sgCopyVec3( tri->b, *stripB );
			sgCopyVec3( tri->c, *p );

			floeToTess->tess.push_back( tri );

			stripA = NULL;
			stripB = NULL;			
		} else if (stripA) {
			stripB = p;
		} else {
			stripA = p;
		}
	} else if (tessMode==GL_TRIANGLE_FAN) {
		if ((fanCenter) && (stripA)) {
			TessTri *tri = new TessTri();
			sgCopyVec3( tri->a, *fanCenter );
			sgCopyVec3( tri->b, *stripA );
			sgCopyVec3( tri->c, *p );

			stripA = p;

			floeToTess->tess.push_back( tri );
		} else if ((fanCenter)&&(!stripA)) {
			stripA = p;
		} else if (!fanCenter) {
			fanCenter = p;
		}
	} else if (tessMode==GL_TRIANGLE_STRIP) {
		if ((stripA) && (stripB)) {
			TessTri *tri = new TessTri();
			sgCopyVec3( tri->a, *stripA );
			sgCopyVec3( tri->a, *stripB );
			sgCopyVec3( tri->c, *p );
			floeToTess->tess.push_back( tri );

			stripA = stripB;
			stripB = p;
		} else if (stripA) {
			stripB = p;
		} else {
			stripA = p;
		}
	}
#endif
}

//--------------------------------------------------------------------

TransIcelandicExpress::TransIcelandicExpress() {
	// constructor
	music = NULL;
	angle = 0.0;
	dot_init = false;

	sgSetVec3( cameraPos, 0.0f, 0.013f, -0.2f );

	sgSetVec4( light_pos, -1.0f, -1.0f, -1.0f, 0.0f );

	sgSetVec4( light_amb,  0.2f, 0.2f, 0.3f, 1.0f );
	sgSetVec4( light_diff, 1.0f, 1.0f, 0.8f, 1.0f );
	sgSetVec4( light_spec, 1.0f, 1.0f, 1.0f, 1.0f );

	sgSetVec4( ocean_diff, 0.4f, 0.5f, 0.7f, 1.0f );

	player = new Player();

	showHelp = true;

	roadX = 0;
	roadY = 0;

	loadLevel( "./gamedata/levels/tess2.txt" );
	//loadLevel( "./gamedata/levels/SimpleRoad.txt" );
	//loadLevel( "./gamedata/levels/Challenge2.txt" );
}

void TransIcelandicExpress::do_quit()
{
	Mix_CloseAudio();
	SDL_WM_GrabInput( SDL_GRAB_OFF );
	SDL_ShowCursor( 1 );
	SDL_Quit();
	exit(0);
}

void TransIcelandicExpress::loadLevel( const char *filename ) {
	FILE *fp = fopen( filename, "r" );
	char line[1000], cmd[100], type[100];
	IceFloe *floe;
	sgVec2 *pnt;
	float x, y;
	int num, npnt, i, j;

	// clear any floes
	for (std::vector<IceFloe*>::iterator floei=floes.begin();
		floei != floes.end(); floei++ ) {

		delete *floei;
	}
	floes.erase( floes.begin(), floes.end() );


	while (fgets( line, 1000, fp )) {
		if (line[0]=='#') continue;

		sscanf( line, "%s", cmd );
		if (!strcmp(cmd, "location")) {
			sscanf( line, "%*s %f %f %s\n", &x, &y, type );
			if (!strcmp( type, "StartPos" )) {
				sgSetVec3( player->pos, x, 0.05f, y );
				player->pos[1] = getHeight( player->pos );
			} else if (!strcmp( type, "Sheep" )) {
				SceneObject *shp;
				shp = new SceneObject();
				sgSetVec3( shp->pos, x, 0.05f, y );
				shp->pos[1] = getHeight( shp->pos );
				sgSetVec4( shp->dbgDiffColor, 0.8f, 0.8f, 0.8f, 1.0f );
				sheep.push_back( shp );
			} else if (!strcmp( type, "Crate" )) {
				SceneObject *crt;
				crt = new SceneObject();
				sgSetVec3( crt->pos, x, 0.05f, y );
				crt->pos[1] = getHeight( crt->pos );
				sgSetVec4( crt->dbgDiffColor, 1.0f, 0.9f, 0.4f, 1.0f );
				crates.push_back( crt );
			}

		} else if (!strcmp(cmd, "mapsize")) {

			// delete the old map if present
			for (j = 0; j < roadY; j++) {
				for (i = 0; i < roadX; i++) {
					delete road[i][j];
					road[i][j] = NULL;
				}
			}

			sscanf( line, "%*s %d %d", &roadX, &roadY );

			// make a new map
			for (j = 0; j < roadY; j++) {
				for (i = 0; i < roadX; i++) {				
					road[i][j] = new Road();
				}
			}
		} else if (!strcmp(cmd, "road_tiles")) {
			int pylon, roadway;

			sscanf( line, "%*s %d", &num );

			for (int n = 0; n < num; n++ ) {
				fgets( line, 1000, fp );
				sscanf( line, "%s %d %d %d %d", cmd, &i, &j, &roadway, &pylon ); 
				if (strcmp(cmd, "road")) {
					printf( "Error: Expecting 'road' got %s\n", line );
				} else {
					road[i][j]->pylon = pylon;
					road[i][j]->road = roadway;
					road[i][j]->base_height = c_RoadLevel * 0.5;

					sgSetVec3( road[i][j]->pos, float(i) + 0.5f, 0.0f, float(j) + 0.5f);
				}
			}

		} else if (!strcmp(cmd, "num_floes")) {
			sscanf( line, "%*s %d", &num );
			for (i = 0; i < num; i++) {
				fgets( line, 1000, fp );
				floe = new IceFloe();
				sscanf( line, "%s %f %f %d %d %s %d",
					cmd, &(floe->health), &(floe->height),
					&(floe->breakable), &(floe->anchored),
					type, &npnt );

				pnt = new sgVec2[ npnt ];
				for ( j = 0; j < npnt; j++ ) {
					fgets( line, 1000, fp );
					sscanf( line, "%f %f", &x, &y );
					pnt[j][0] = x;
					pnt[j][1] = y;
				}

				floe->build( npnt, pnt );
				delete [] pnt;

				floes.push_back( floe );
			}
		} else {
			printf ("Skipping command %s \n", cmd );
		}
	}
	
}

void TransIcelandicExpress::computeTimeDelta( void ) {	
	static Uint32 last_tick=0;	

	Uint32 tick;
	tick = SDL_GetTicks();	

	if (last_tick==0) {		
		deltaT = 0.001f;
	} else {
		deltaT = (float)(tick - last_tick) / 1000.0f;
	}
	last_tick = tick;

}

void TransIcelandicExpress::simulate() {
	sgVec3 fwd, right, up, frict, v;
	sgVec3 playerForce;
	float cf_accel = 1.0,  // m/s^2 
		  maxVel = 0.5,    // m/2
		  cf_friction_land = 8.0,
		  cf_friction_ice = 5.0;
	float pv, 
		  ff; // friction fudge... more friction for slow objects

	float timestep = 0.01f;
	static float timeleft = 0.0f;	
	timeleft += deltaT;

	sgSetVec3( up, 0.0, 1.0, 0.0 );
	sgSetVec3( playerForce, 0.0, 0.0, 0.0 );	

	while (timeleft > timestep) {
		
		sgCopyVec3( fwd, cameraPos );
		sgNegateVec3( fwd );
		fwd[1] = 0.0;
		sgNormalizeVec3( fwd );
		sgVectorProductVec3( right, fwd, up );

		// todo: if on the ground
		sgScaleVec3( fwd, cf_moveForward );
		sgAddVec3( playerForce, fwd );

		sgScaleVec3( right, cf_moveSideways );
		sgAddVec3( playerForce, right );

		sgScaleVec3( playerForce, cf_accel * timestep );
		sgAddVec3( player->vel, playerForce );

		pv = sgLengthVec3( player->vel ) ;
		ff = (1.0 - ((pv / maxVel)* 0.8));
		ff = ff*ff;

		sgCopyVec3( frict, player->vel );
		sgNegateVec3( frict );
		sgScaleVec3( frict, ff * cf_friction_ice * timestep );	
		sgAddVec3( player->vel, frict );		
		
		dbgVel.push_back( pv );
		if (dbgVel.size() > 100 ) {
			dbgVel.pop_front();
		}
		
		if ( pv > maxVel ) {
			//printf("maxvel!\n" );
			sgNormalizeVec3( player->vel );
			sgScaleVec3( player->vel, maxVel );
		}

		sgCopyVec3( v, player->vel );
		sgScaleVec3( v, timestep );
		sgAddVec3( player->pos, v );
	
		// advance
		timeleft -= timestep;		
	}

	player->pos[1] = getHeight( player->pos );
}

// kind of brute force, could be sped up
float TransIcelandicExpress::getHeight( sgVec3 &pos ) {

	float hite =0.0;

	for (std::vector<IceFloe*>::iterator floei=floes.begin();
		floei != floes.end(); floei++ ) {

		if ( (*floei)->contains( player->pos )) {
			hite = (*floei)->height * c_RoadLevel;
			break;
		}
	}

	return hite;
}


ILuint ImageName;
unsigned int gl_tex_id;
void TransIcelandicExpress::redraw( void ) {	

	static tex_init = 0;	

	
	if (!tex_init) {		

		ilutRenderer( ILUT_OPENGL );

		ilGenImages( 1, &ImageName );

		ilBindImage( ImageName );
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
		if (!ilLoadImage( "ludum48.png" )) {
			printf("Loading image failed\n");
		}

		
		gl_tex_id = ilutGLBindTexImage();
		ilutGLTexImage( 0 );

		tex_init = 1;
	} 

	glBindTexture (GL_TEXTURE_2D, gl_tex_id );

	glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	// 3d part
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 60.0, 800.0/600.0, 0.05, 100.0 );



	gluLookAt(	player->pos[0]+cameraPos[0], 
				player->pos[1]+cameraPos[1]+c_PlayerHeight, 
				player->pos[2]+cameraPos[2],

				player->pos[0], 
				player->pos[1]+ c_PlayerHeight, 
				player->pos[2],

				0.0, 1.0, 0.0 );
				

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// apply some of the camera xform to the light just to make
	// it more shiny
	glPushMatrix();
	float viewHead = atan2( cameraPos[2], cameraPos[0] );	
	glRotated( viewHead * SG_RADIANS_TO_DEGREES, 0.0f, 1.0f, 0.0f );
	setupLights();
	glPopMatrix();

	draw3d();

	// 2d part
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	gluOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_LIGHTING );
	draw2d();

	SDL_GL_SwapBuffers();

}

void TransIcelandicExpress::drawOcean( void ) {
	float sz = 1000.0;
	glPushMatrix();	
	glColor3f( 0.2f, 0.4f, 0.6f );
	//glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ocean_diff );

	glDisable( GL_LIGHTING );
	glDepthMask( 0 );

	glBegin( GL_POLYGON );
	glVertex3f(  sz, 0.0, -sz );
	glVertex3f( -sz, 0.0, -sz );
	glVertex3f( -sz, 0.0,  sz );
	glVertex3f(  sz, 0.0,  sz );
	glEnd();
	glPopMatrix();

	glDepthMask( 1 );
}

void TransIcelandicExpress::setupLights( void ) {	
	// lighting
	glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
	glLightfv( GL_LIGHT0, GL_AMBIENT, light_amb );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light_diff );
	glLightfv( GL_LIGHT0, GL_SPECULAR, light_spec );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
}

void TransIcelandicExpress::draw3d( void ) {	

	
	glPushMatrix();		

	// The ocean
	drawOcean();

	glEnable( GL_LIGHTING );

	// draw the ice floes
	for (std::vector<IceFloe*>::iterator floei=floes.begin();
		floei != floes.end(); floei++ ) {

		(*floei)->draw();
	}

	// draw the sheep
	for (std::vector<SceneObject*>::iterator shpi=sheep.begin();
		shpi != sheep.end(); shpi++ ) {

		(*shpi)->draw();
	}

	// draw the crates
	for (std::vector<SceneObject*>::iterator crti=crates.begin();
		crti != crates.end(); crti++ ) {

		(*crti)->draw();
	}

	// draw the roadways
	for (int j = 0; j < roadY; j++) {
		for (int i=0; i < roadX; i++) {
			road[i][j]->draw();
		}
	}

	// draw the player();
	player->draw();


	glPopMatrix();

#if 0
	// Texture test (spinning logo)
	glEnable( GL_TEXTURE_2D );
	glPushMatrix();

	glTranslated( 0.0, 0.0, -3.0 );
	glRotated( angle, 0.0, 1.0, 0.0 );
	
	angle = angle + deltaT * (360.0f / 4.0f);

	glColor3f( 1.0, 1.0, 1.0 );
	glBegin( GL_QUADS );

	glTexCoord2d( 0.0, 1.0 );
	glVertex3f( -1.0, -1.0, 0.0 );

	glTexCoord2d( 0.0, 0.0 );
	glVertex3f( -1.0,  1.0, 0.0 );

	glTexCoord2d( 1.0, 0.0 );
	glVertex3f(  1.0,  1.0, 0.0 );

	glTexCoord2d( 1.0, 1.0 );
	glVertex3f(  1.0, -1.0, 0.0 );
	glEnd();

	glPopMatrix();
#endif

}	


void TransIcelandicExpress::draw2d( void ) {	
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );

	if (showHelp ) {
		fnt->begin();
	
		glColor3f ( 0.6f, 1.0f, 1.0f ) ;
		fnt->setPointSize ( 40 ) ;
		fnt->start2f ( 20, 550 ) ;	
		fnt->puts( "Trans-Icelandic Expressway" );

		glColor3f ( 1, 1, 1 ) ;
		fnt->setPointSize ( 20 ) ;
		fnt->start2f ( 20, 530 ) ;	
		fnt->puts(	"Ludum Dare 48 hour game competition template\n"
					"Joel Davis (joeld42@yahoo.com)\n"
					"Controls: w,a,s,d - move around\n"
					"          home - save screenshot\n"
					"          m - toggle music\n"
					"          1,2,3 - play sfx\n" 
					"          h - toggle help\n" 
					) ;
		fnt->end();	
	}

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glBegin( GL_LINE_STRIP );
	float v;
	glColor3f( 0.0, 1.0, 0.0 );
	for (int i = 0; i < dbgVel.size(); i++) {
		v = dbgVel[i];
		glVertex3f( i * 8.0f, v*600.0, 0.0 );
	}
	glEnd();
}

void TransIcelandicExpress::initialize() {
	musicOn = false;

	sfx[0] = Mix_LoadWAV("sfx_hit.wav");
	sfx[1] = Mix_LoadWAV("sfx_gunshot.wav");
	sfx[2] = Mix_LoadWAV("sfx_boom.wav");

	music = Mix_LoadMUS( "template_music.ogg" );
	if (!music) {
		printf("Error loading music %s\n", Mix_GetError() );
	} else {
		printf("Music loaded\n");
	}
	//Mix_PlayMusic( music, -1 );
	//musicOn = 1;
}

void TransIcelandicExpress::mouseMotion( int xrel, int yrel ) {
	// rotate the camera
	sgVec3 axis, oldCamPos;
	sgMat4 m;
	float rot_factor = 0.5;

	// change heading
	sgSetVec3( axis, 0.0, 1.0, 0.0 );
	sgMakeRotMat4( m, (float)xrel * rot_factor, axis ) ;

	sgXformVec3( cameraPos, m ) ;

	// change pitch
	sgSetVec3( axis, 1.0, 0.0, 0.0 );
	sgMakeRotMat4( m, (float)yrel * rot_factor, axis ) ;

	sgCopyVec3( oldCamPos, cameraPos );
	sgXformVec3( cameraPos, m ) ;

	// don't let the camera go below player
	if (cameraPos[1] < 0.0 ) {
		sgCopyVec3( cameraPos, oldCamPos );		
	}
}


void TransIcelandicExpress::eventLoop() {
	while (1) {		
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {
		
        switch( event.type ) {
		case SDL_MOUSEMOTION:			
			mouseMotion( event.motion.xrel, event.motion.yrel );
			
			break;
        case SDL_KEYDOWN:
            switch( event.key.keysym.sym ) {
				case SDLK_ESCAPE:
					do_quit();
					break;
				case SDLK_SPACE:
					printf("space\n");
					break;

				case SDLK_1:
					sfx_chan[0] = Mix_PlayChannel(-1, sfx[0], 0);
					break;
				case SDLK_2:
					sfx_chan[1] = Mix_PlayChannel(-1, sfx[1], 0);
					break;
				case SDLK_3:
					sfx_chan[2] = Mix_PlayChannel(-1, sfx[2], 0);
					break;

				case SDLK_HOME:
					ilutGLScreenie();
					break; 

				case SDLK_h:
					showHelp = !showHelp;
					break;

				case SDLK_m:					
					if (!musicOn) {
						printf("Playing\n");
						Mix_PlayMusic( music, -1 );
						musicOn = 1;
					} else {
						printf("Halting\n");
						Mix_HaltMusic();						
						musicOn = 0;
					}
				default:
					break;
				}
			break;

			case SDL_QUIT:
				/* Handle quit requests (like Ctrl-c). */
				do_quit();
				break;
			}	
		}

		// check non-instant keys
		cf_moveForward = 0.0;
		cf_moveSideways = 0.0;

		Uint8 *keys;
		keys = SDL_GetKeyState(NULL);
		if (keys[SDLK_w] && !keys[SDLK_s]) {
			cf_moveForward = 1.0;
		}
		if (!keys[SDLK_w] && keys[SDLK_s]) {
			cf_moveForward = -1.0;
		}
		if (keys[SDLK_a] && !keys[SDLK_d]) {
			cf_moveSideways = -1.0;
		}
		if (!keys[SDLK_a] && keys[SDLK_d]) {
			cf_moveSideways = 1.0;
		}

		computeTimeDelta();
		simulate();
		redraw();
	}
}

int main( int argc, char *argv[]) {

	TransIcelandicExpress *tieGame;

	if ( SDL_Init(SDL_INIT_NOPARACHUTE | SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

	if( SDL_SetVideoMode( 800, 600, 16, SDL_OPENGL /*| SDL_FULLSCREEN */ ) == 0 ) {
		fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
	}

	SDL_WM_SetCaption( "Trans-Icelandic Express", NULL );

#ifdef TAKE_MOUSE
	SDL_WM_GrabInput( SDL_GRAB_ON );
	SDL_ShowCursor( 0 );
#endif

	// Init SDL_Mixer
	if (Mix_OpenAudio( 22050, AUDIO_S16, 2,  4096 )) {
		fprintf(stderr,	"Unable to open audio\n");
        exit(1);
	}

	// Setup libFnt
	fnt = new fntRenderer();
	fntHelv = new fntTexFont( "helvetica_medium.txf");
	
	fnt->setFont( fntHelv ) ;
	fnt->setPointSize ( 20 ) ;
	
	// initialize DevIL
	ilInit();

	tieGame = new TransIcelandicExpress();
	tieGame->initialize();
	
	tieGame->eventLoop();
		
	return 0;
} 