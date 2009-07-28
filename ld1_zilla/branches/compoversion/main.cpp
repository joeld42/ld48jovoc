#include <vector>
#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <windows.h>
#include <crtdbg.h>

#include <math.h>
#include <GL/glut.h>

#include <plib/sg.h>
#include <plib/fnt.h>

#include <png.h>
#include "zillaGame.h"

// quasi-globals
ZillaGame *ZillaGame::theGame = NULL;
int ZillaGame::c_Width = 800;
int ZillaGame::c_Height = 600;
clock_t ZillaGame::lastTick = 0, ZillaGame::lastFpsTick = 0;
int ZillaGame::c_fpsUpdateNum = 50;
int ZillaGame::fpsFrameCount = ZillaGame::c_fpsUpdateNum;
float ZillaGame::fps = 0.0;

// texture id stuff
GLuint Texture::texIdPool[MAX_TEXTURES];
int Texture::currTexId = -1;
Texture *Texture::currentBound = NULL;

// the texture cache
std::list<Texture*> texCache;

// mesh stuff
int Mesh::nextDisplayList = 1;

ZillaGame::ZillaGame()
{
	assert(!theGame);
	theGame = this;

	// camera init
	sgSetVec3( camPos, 8.0, 4.0, 0.0 );
	sgSetVec3( camLookat, 0, 1.2, 0 );

	currAngle = 0.0;
	currHeight = 1.5;
	currZoom = 3.0;

	dataLoaded = false;

	fnt = NULL;

	levelNdx = 0;
	follow = true;

	state = ZillaGame::TitleScreen;
}

void 
ZillaGame::Update( float timeDelta )
{
	if (!dataLoaded) {
		loadGameData();
		dataLoaded = 1;
	} else {

		if (state==TitleScreen) {
			UpdateTitle( timeDelta );
			glutPostRedisplay();
			return;
		}

		//currAngle += 30.0 * timeDelta;
		//if (currAngle > 360.0) currAngle -= 360.0;
		//_RPT1( _CRT_WARN, "currangle %f\n", currAngle );

		if (follow){
			sgSetVec3( camLookat, godzilla->pos[0], 1.0, godzilla->pos[1] );
		}

		sgSetVec3( camPos,  
				 camLookat[0] + cos( SG_DEGREES_TO_RADIANS * currAngle) * currZoom,
			currHeight, camLookat[2] + sin( SG_DEGREES_TO_RADIANS * currAngle) * currZoom );

		glutPostRedisplay();
	}

	// Update all of the current actors
	for (std::vector<Actor*>::iterator ai = actors.begin();
		ai != actors.end(); ai++ ) {
		(*ai)->update( level, timeDelta );	
	}
}

void 
ZillaGame::UpdateTitle( float timeDelta )
{
	// do nothing
}

void
ZillaGame::Special( int key, int x, int y ) {
	sgVec3 front, right, up;

	_RPT2( _CRT_WARN, "In special key is %d up is %d\n", key, GLUT_KEY_UP );

	sgSubVec3( front, camLookat, camPos );
	front[1] = 0.0;
	sgNormalizeVec3( front );
	_RPT3( _CRT_WARN, "front %f %f %f\n", front[0], front[1], front[2] );

	sgSetVec3( up, 0.0, 1.0, 0.0 );
	sgVectorProductVec3( right, front, up  );
	sgNormalizeVec3( right );

	sgScaleVec3( front, 0.2 );
	sgScaleVec3( right, 0.2 );

	switch( key ) {
		case GLUT_KEY_UP:
			sgAddVec3( camLookat, camLookat, front );
			break;
		case GLUT_KEY_DOWN:
			sgSubVec3( camLookat, camLookat, front );
			break;
		case GLUT_KEY_RIGHT:
			sgAddVec3( camLookat, camLookat, right );
			break;
		case GLUT_KEY_LEFT:
			sgSubVec3( camLookat, camLookat, right );
			break;
	}

	// DBG
	sgSetVec3( camPos,  
			camLookat[0] + cos( SG_DEGREES_TO_RADIANS * currAngle) * currZoom,
			currHeight, camLookat[2] + sin( SG_DEGREES_TO_RADIANS * currAngle) * currZoom );
	_RPT3( _CRT_WARN, "camPos is %f %f %f\n", camPos[0], camPos[1], camPos[2] );

}


// HISTORICAL NOTE: I was trick to fix this function when it came around
// to 5:45... so you can click and put down tanks. but i want time to
// package it up so I am stopping here.
void
ZillaGame::Mouse( int button, int state, int mx, int my ) {
	
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {

		// figure out where the mouse hit the ground
		GLdouble x, y, z, sx, sy;			

		// unproject some point
		sx = (float)mx;
		sy = (float)(c_Height-my); 
		gluUnProject( sx, sy, 0.01,
					  modelview, proj, viewport, 
					  &x, &y, &z );

		sgSetVec3( dbgClickPnt, x, y, z );

		// undo the level transform
		x = x - (level->w * 0.75);
		y = y -(level->h * 0.75);

		_RPT3( _CRT_WARN, "Click %f %f %f\n", x, y, z );

		// make a ray from cam pos and the point		
		sgVec3 dir;
		float t;
		sgSetVec3( dir, camPos[0]-x, camPos[1]-y,camPos[2]-z );
		sgNormalizeVec3( dir );

		// find out where it hits the ground
		_RPT3( _CRT_WARN, "dir is %f %f %f\n", dir[0], dir[1], dir[2] );
		_RPT3( _CRT_WARN, "camPos is %f %f %f\n", camPos[0], camPos[1], camPos[2] );
		
		t = camPos[1] / dir[1];
		_RPT1( _CRT_WARN, "t is %f\n", t );

	}

}

void
ZillaGame::Key( unsigned char key, int x, int y )
{
	_RPT1( _CRT_WARN, "Key pressed: %c\n", key );

	switch( key ) {
	case 27:
		exit(0);
	case ' ':
		if (state==TitleScreen) state=InGame;
		break;
	case 'a':
		currAngle = currAngle - 10.0;
		break;
	case 'd':
		currAngle = currAngle + 10.0;
		break;
	case 'q':
		currZoom = currZoom - 0.1;
		if (currZoom < 1.0) currZoom=1.0;
		break;
	case 'f':
		follow = !follow;
		break;
	case 'e':
		currZoom = currZoom + 0.1;
		break;
	case 's':
		currHeight = currHeight - 0.1;
		if (currHeight < 0.0) currHeight=0.0;
		break;
	case 'w':
		currHeight = currHeight + 0.1;
		break;
	case 'n': // next level
		levelNdx++;
		if (levelNdx==levels.size()) levelNdx = 0;
		level = levels[levelNdx];
		break;
	case 'p': // prev level
		levelNdx--;
		if (levelNdx==-1) levelNdx = levels.size()-1;
		level = levels[levelNdx];
		break;
	}
}

void
ZillaGame::Display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( 90, (float)c_Width / (float)c_Height, 0.1, 1000.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	gluLookAt( camPos[0], camPos[1], camPos[2],
			camLookat[0], camLookat[1], camLookat[2],
			0.0, 1.0, 0.0 );

	Texture::resetBound();

	if (state==TitleScreen) {
		DisplayTitle3D();
	} else if (state==InGame) {
		Display3D();
	}

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluOrtho2D( 0, c_Width, 0, c_Height ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	Texture::resetBound();

	if (state==TitleScreen) {
		DisplayTitle2D();
	} else if (state==InGame) {
		Display2D();
	}

	glutSwapBuffers();

}

void 
ZillaGame::loadTileset( const char *filename )
{
	FILE *fp = fopen( filename, "r" );
	char line[1024], name[1024], sym[100];
	int passable, breakable;
	Tile *tile;

	assert(fp);

	while (!feof(fp) ) {
		fgets( line, 1024, fp );
		if (line[0]=='#') continue;
		sscanf( line, "%s %s %d %d", name, sym, &passable, &breakable );

		tile = new Tile( name, sym[0], passable==1, breakable==1 );
		tileset.push_front( tile );
	}
}

void
ZillaGame::DisplayTitle2D()
{
	char s[1024];

	// display title screen
	titleScreen->bindTexture();
	glColor3f( 1.0, 1.0, 1.0 );

	glBegin( GL_QUADS );
	
	glTexCoord2d( 0.0, 0.0 );
	glVertex2f( 0, 600 );

	glTexCoord2d( 0.0, 1.0 );
	glVertex2f( 0, 0 );

	glTexCoord2f( 1.0, 1.0 );
	glVertex2i( 800, 0 );

	glTexCoord2f( 1.0, 0.0 );
	glVertex2i( 800, 600 );
	
	glEnd();

	// write words
	glDisable( GL_LIGHTING ) ;
	glDisable( GL_DEPTH_TEST ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glEnable( GL_BLEND ) ;
	glAlphaFunc( GL_GREATER, 0.1 ) ;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
	
	fnt->setFont( fntHelvBold ) ;
	fnt->setPointSize ( 20 ) ;
	
	fnt->begin();
	glColor3f ( 1, 1, 1 ) ;
    fnt->start2f ( 20, 400 ) ;	
    fnt->puts( "Controls: q,w,e,a,s,d rotate/zoom view\n"
			   "f - toggle follow mode\n"
			   "arrows - move (not in follow mode)\n"
			   "press n/p to select next/prev level\n"
			   "and press SPACE to begin" ) ;

	
	for (int i = 0; i < levels.size(); i++) {

		glColor3f( 0.0, 0.0, 0.0 );
		fnt->start2f ( 100+3, 250-(i*20)-3 );
		sprintf( s, "%s -- %s", levels[i]->name, levels[i]->desc );
		fnt->puts( s ) ;
		fnt->end();	

		if ((i==levelNdx)&&((fpsFrameCount/5)&0x1)) {
			glColor3f( 1.0, 1.0, 1.0 );
		} else {
			glColor3f( 1.0, 0.5, 1.0 );
		}

		fnt->start2f ( 100, 250-(i*20) );
		sprintf( s, "%s -- %s", levels[i]->name, levels[i]->desc );
		fnt->puts( s ) ;
		fnt->end();	
	}


	glColor3f( 0.0, 1.0, 0.0 );
	fnt->setFont( fntHand ) ;
	fnt->setPointSize ( 15 ) ;
	fnt->start2f ( 10, 20 );	
	fnt->puts( "by Joel Davis (joeld42@yahoo.com) for the ludumdare 48 hour game contest" ) ;
	fnt->end();	
	

	glColor3f ( 1, 1, 0 ) ;

	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_LIGHTING ) ;	
}

void
ZillaGame::DisplayTitle3D()
{

}

void
ZillaGame::Display3D()
{
#if 0 
	// draw grid
	glBegin (GL_LINES);
	for (int i = -100; i < 100; i+= 10 ){
		glVertex3f( i, 0.0, -100.0 );
		glVertex3f( i, 0.0,  100.0 );

		glVertex3f( -100, 0.0, i );
		glVertex3f(  100, 0.0, i );
	}
	glEnd();
#endif

	// draw a skybox
	glDisable( GL_LIGHTING );
	glPushMatrix();
	glScaled( 500.0, 500.0, 500.0 );
	skybox->draw();
	glPopMatrix();

	// set up the lights
	//glEnable( GL_LIGHTING );	
	
	// draw the level
	glPushMatrix();
	glTranslated( -(level->w * 0.75), 0.0, -(level->h * 0.75) );
	level->draw();

	// store the camera xform for later use
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, proj );
	glGetIntegerv( GL_VIEWPORT, viewport );   

	// draw the click point for debugging
	glColor3f( 1.0, 0.0, 0.0 );	
	glDisable( GL_TEXTURE_2D );
	glPushMatrix();
	glTranslated( dbgClickPnt[0], dbgClickPnt[1], dbgClickPnt[2] );
	glutSolidSphere( 0.01, 5, 5);
	glPopMatrix();

	glPopMatrix();

	//godzilla->draw();
	for (std::vector<Actor*>::iterator ai = actors.begin();
		ai != actors.end(); ai++ ) {
		(*ai)->draw();		
	}

}

void
ZillaGame::Display2D()
{
	char s[1024];

	glDisable( GL_LIGHTING ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glEnable( GL_BLEND ) ;
	glAlphaFunc( GL_GREATER, 0.1 ) ;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
	
	fnt->setFont( fntHelv ) ;
	fnt->setPointSize ( 20 ) ;
	
	fnt->begin();
	glColor3f ( 1, 1, 0 ) ;
    fnt->start2f ( 10, 580 ) ;
	sprintf( s, "FPS: %3.2f", fps );
    fnt->puts( s ) ;

	glColor3f ( 1, 1, 1 ) ;
	fnt->start2f ( 10, 560 ) ;
	sprintf( s, "Level: %s (%s)", level->name, level->desc );
    fnt->puts( s ) ;

	fnt->end();	

	glColor3f ( 1, 1, 0 ) ;

	glDisable( GL_BLEND );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_ALPHA_TEST );
	//glEnable( GL_LIGHTING ) ;
}

//-- static callbacks

void
ZillaGame::cbDisplay()
{
	assert(theGame);
	theGame->Display();
}

void
ZillaGame::cbKey( unsigned char key, int x, int y)
{
	assert(theGame);
	theGame->Key( key, x, y );
}

void
ZillaGame::cbMouse( int button, int state, int x, int y)
{
	assert(theGame);
	theGame->Mouse( button, state, x, y );
}

void
ZillaGame::cbSpecial( int key, int x, int y)
{
	assert(theGame);
	theGame->Special( key, x, y );
}

void
ZillaGame::cbIdle()
{
	assert(theGame);
	clock_t curTick = GetTickCount();
	float t;

	// get the elaspsed time
	if ((lastTick==0)||(curTick<lastTick)) {
		// just ignore the tricky cases
		t = 0.001;		
	} else {
		// usual case, calc the time difference between last frame and this frame
		t = (float)(curTick - lastTick) / 1000.0f;
	}

	// update the fps-o-meter every fpsUpdateNum frames 
	if (fpsFrameCount==c_fpsUpdateNum) 
	{
		fpsFrameCount = 0;
		fps = ((float)c_fpsUpdateNum)/((float)(curTick - lastFpsTick) / 1000.0f);
		lastFpsTick = curTick;
	}
	fpsFrameCount++;

	//_RPT4( _CRT_WARN, " %d %d %d %10f\n", lastTick, curTick, curTick - lastTick, t );
	

	lastTick = curTick;
	theGame->Update(t);
}

void 
ZillaGame::loadGameData()
{	
	fnt = new fntRenderer();

	fntHand = new fntTexFont( "gamedata/fonts/sorority.txf");
	fntHelv = new fntTexFont( "gamedata/fonts/helvetica_bold.txf");
	fntHelvBold = new fntTexFont( "gamedata/fonts/helvetica_bold.txf");
	
/*
	testMesh = new Mesh( "gamedata/models/tile_basic_split.am");
	testMesh2 = new Mesh( "gamedata/models/tile_lightForest_split.am");
	testMesh3 = new Mesh( "gamedata/models/tile_residential_ns_split.am");

	testTex = new Texture( "gamedata/textures/testmap2.png", true );
	testTex2 = new Texture( "gamedata/textures/testmap3.png", false );
*/	
	skybox = new Mesh( "gamedata/models/sky.am");

	titleScreen = new Texture( "gamedata/textures/titlescreen.png", false );

	sgVec2 startpos;
	sgSetVec2( startpos, 0.0, 0.0 );
	godzilla = new Godzilla( new Mesh( "gamedata/models/godzilla.am"), startpos );
	godzilla->speed = 3.0;
	actors.push_back( godzilla );

	loadTileset( "gamedata/tileset.txt" );
	loadLevels( "gamedata/levels.txt" );


}

void 
ZillaGame::loadLevels( const char *filename )
{
	Level *l;	
	Tile *pickfrom[100];
	char line[1024], name[1024], desc[1024], *ch;
	int w, h, ntile, pick;

	FILE *fp = fopen( filename, "r" );
	while (!feof(fp)) {
		fgets( line, 1024, fp );
		if (strlen(line) < 2) continue;
		
		if(line[strlen(line)-1]=='\n') line[strlen(line)-1]=0;

		sscanf( line, "%d %d %s", &w, &h, name );
		if (ch = strchr( line, ':' )) {
			strcpy (desc, ch+1 );
		} else {
			strcpy( desc, "No description available" );
		}

		l = new Level(name, desc, w, h );	
		//_RPT4( _CRT_WARN, " level %s (%s) w %d h %d\n", name, desc, w, h );

		for (int j = 0; j < h; j++) {
			fgets( line, 1024, fp );
			for (int i = 0; i < w; i++) {

				//_RPT3( _CRT_WARN, "tile %d %d is %c\n", i, j, line[i] );

				// multiple tiles can have the same symbol (i.e. all ground tiles are '.')
				// this is not the most efficient way to pick one, end up rebuild the
				// picklists over and over, but it should work.
				ntile = 0;
				for( std::list<Tile*>::iterator ti = tileset.begin();
					ti != tileset.end(); ti++ ) {
						if ( (*ti)->symbol == line[i] ) {
							pickfrom[ntile++] = (*ti);
						}
				}

				pick = (int)( ((float)rand() / (float)RAND_MAX) * ntile);
				l->setTile( i,j, pickfrom[pick] );
			}
		}

		levels.push_back( l );		
	}
	levelNdx = 0;
	level = levels[levelNdx];

}

/////////////////////////////////////////////
Tile::Tile(char *_name, char _symbol, bool _passable, bool _destructible ) :
	symbol(_symbol), passable( _passable ), destructible( _destructible )
{
		char meshname[1024];

		name = strdup( _name );

		sprintf( meshname, "gamedata/models/%s.am", name );
		mesh = new Mesh( meshname );
}

/////////////////////////////////////////////
Level::Level( char *_name, char *_desc, int _w, int _h  )
{
	name = strdup( _name );
	desc = strdup( _desc );
	w = _w; h = _h;
}

void
Level::setTile( int x, int y, Tile *tile )
{
	assert (x < w );
	assert (y < h );

	map[x][y] = tile;
}

void
Level::draw()
{
	for (int j = 0; j < h; j++) {
		for (int i=0; i < w; i++) {
			glPushMatrix();
			glTranslated( i*1.5, 0.0, j*1.5 );
			map[i][j]->mesh->draw();
			glPopMatrix();
		}
	}
}

void 
Level::worldToMap( float worldX, float worldZ,
				  int &mapX, int &mapY ) {
	worldX = worldX - (w * 0.75);
	worldZ = worldZ - (h * 0.75);

	mapX = int(worldX);
	mapY = int(worldZ);
}

void 
Level::mapToWorld( int mapX, int mapY,
				  float &worldX, float &worldZ ) {
}

/////////////////////////////////////////////
Actor::Actor(Mesh *_mesh, sgVec2 _pos )
{
	mesh = _mesh;	
	sgCopyVec2( pos, _pos );
	state = Actor::Idle;

	speed = 1.0; // units/sec
}

void Actor::draw()
{
	glPushMatrix();	
	glTranslated( pos[0], 0.0, pos[1] );	
	glRotated( SG_RADIANS_TO_DEGREES * heading, 0.0, 1.0, 0.0 );
	mesh->draw();
	glPopMatrix();
}

void Actor::update( Level *lvl, float t )
{
	switch (state) {
	case Actor::Idle:
		// pick the next waypoint
		if (waypoints.size()) {
			//sgSetVec2( target, (((float)rand() / (float)RAND_MAX) * (1.5*lvl->w)) - (0.75*lvl->w),
			//(((float)rand() / (float)RAND_MAX) * (1.5*lvl->h)) - (0.75*lvl->h) );

			sgSetVec2( target, (*(*waypoints.begin()))[0], (*(*waypoints.begin()))[1] );
			waypoints.pop_front();

			state = Actor::Moving;
			targetRad = 1.0;
		} else {
			// no waypoints, do nothing
		}		
		break;
	case Actor::Moving:
		if (moveTowardTarget( t ) ) {
			state = Actor::Idle;
		}
		
		break;
	case Actor::Attacking:
		break;
	}
}

void
Godzilla::update( Level *lvl, float t )
{
	switch (state) {
	case Actor::Idle:

		if (start) {
			start = false;
			// start at a random loaction in the water
			//

			// find a path to the powerplant.
			
		} else {

		}
		// pick a random destination
		sgSetVec2( target, (((float)rand() / (float)RAND_MAX) * (1.5*lvl->w)) - (0.75*lvl->w),
			(((float)rand() / (float)RAND_MAX) * (1.5*lvl->h)) - (0.75*lvl->h) );
		state = Actor::Moving;
		targetRad = 1.0;
		break;
	case Actor::Moving:
		if (moveTowardTarget( t ) ) {
			state = Actor::Idle;
		}
		
		break;
	case Actor::Attacking:
		break;
	}
}

bool
Actor::moveTowardTarget( float t )
{
	sgVec2 dir;
	float dist, amt;
	sgSubVec2( dir, target, pos );

	dist = sgLengthSquaredVec2( dir );
	if (dist < 0.001) {
		return true;
	}
	dist = sqrt(dist );
	/*
	_RPT3( _CRT_WARN, "targ %f %f dist %f\n", target[0], target[1], dist );
	_RPT3( _CRT_WARN, "pos %f %f dist %f\n", pos[0], pos[1], dist );
	*/

	sgNormalizeVec2( dir );

	heading = SG_PI + atan2( dir[0], dir[1] );

	// don't overshoot the target
	amt = min( dist, t*speed );
	

	sgScaleVec2( dir, amt );
	sgAddVec2( pos, pos, dir );

	return false;
}

/////////////////////////////////////////////
Mesh::Mesh( const char *filename)
{
	tex = NULL;
	loadFile( filename );
	dlistId = 0;
}

void Mesh::draw()
{
#if 0
	// draw points
	glColor3f( 1.0, 0.0, 1.0 );
	glPointSize( 2 );
	glBegin( GL_POINTS );
	
	for (int i=0; i < nVtx; i++) {
		glVertex3f( vtx[i][0], vtx[i][1], vtx[i][2] );
	}

	glEnd();
	glColor3f( 1.0, 1.0, 1.0 );
#endif

	if (tex) {
		glColor3f( 1.0, 1.0, 1.0 );
		tex->bindTexture();
	}

	if (!dlistId) {
		

		glNewList( nextDisplayList, GL_COMPILE_AND_EXECUTE );
		dlistId = nextDisplayList++;
				
		
		// draw triangles
		MeshPart *p;
		int ndx;
		for (std::vector<MeshPart*>::iterator pi = part.begin();
		pi != part.end(); pi++) {
			p = (*pi);
			
			glBegin( GL_TRIANGLES );
			for (int j =0; j < p->nTri; j++) {
				
				for (int v=0; v < 3; v++) {
					ndx = p->tri[j][v];
					glNormal3f( nrm[ ndx ][0], nrm[ ndx ][1], nrm[ ndx ][2] );
					if (tex) {
						glTexCoord2f( st[ ndx ][0], st[ ndx ][1] );
					}
					glVertex3f( vtx[ ndx ][0], vtx[ ndx ][1], vtx[ ndx ][2] );				
				}
				
			}
			glEnd();
		}
		
		glEndList();

	} else {		
		glCallList( dlistId );
	}
}

void Mesh::loadFile( const char *filename )
{
	FILE *fp = fopen( filename, "rt" );
	char section[1024];
	int num;
	bool done = false;
	
	// SE_MESH 0.1;
	fscanf( fp, "%*s %*s" );

	while (!done) {
		fscanf( fp, "%s %d", &section, &num );
		if ( !strcmp( section, "VERTICES") ) {
			skaReadVertexSection( fp, num );
		} else if ( !strcmp( section, "NORMALS") ) {
			skaReadNormalSection( fp, num );
		} else if ( !strcmp( section, "SURFACES") ) {
			skaReadSurfaceSection( fp, num );
		} else if ( !strcmp( section, "UVMAPS") ) {
			skaReadUVmapSection( fp, num );
		} else if ( !strcmp( section, "SE_MESH_END;") ) {
			done = 1;
		} else {
			printf("Skipping unknown section %s\n", section );
			skaSkipUnknownSection( fp );
		}
	}

	fclose(fp);
}

void Mesh::skaReadVertexSection( FILE *fp, int num )
{
	char sx[100], sy[100], sz[100];

	vtx = new sgVec3[num];
	nVtx = num;

	fscanf( fp, "%*s" ); // {

	// read the vertices
	for (int i = 0; i < num; i++) {
		fscanf( fp, "%s %s %s", sx, sy, sz );

		// remove , and ; after numbers
		sx[strlen(sx)-1] = 0;
		sy[strlen(sy)-1] = 0;
		sz[strlen(sz)-1] = 0;

		sgSetVec3( vtx[i], atof( sx ), atof( sy ), atof( sz ) );
	}

	fscanf( fp, "%*s" ); // }
}

void Mesh::skaReadNormalSection( FILE *fp, int num )
{
	char sx[100], sy[100], sz[100];

	nrm = new sgVec3[num];

	fscanf( fp, "%*s" ); // {

	// read the vertices
	for (int i = 0; i < num; i++) {
		fscanf( fp, "%s %s %s", sx, sy, sz );

		// remove , and ; after numbers
		sx[strlen(sx)-1] = 0;
		sy[strlen(sy)-1] = 0;
		sz[strlen(sz)-1] = 0;

		sgSetVec3( nrm[i], atof( sx ), atof( sy ), atof( sz ) );
	}

	fscanf( fp, "%*s" ); // }
}

void Mesh::skaReadUVmapSection( FILE *fp, int num )
{
	char ss[100], stt[100], mapname[1024], texname[1024];
	int numcoord;	
	bool alpha;

	fscanf( fp, "%*s" ); // {

	// only support single maps
	assert( num==1 );

	fscanf( fp, "%*s" ); // {
	
	fscanf( fp, "%*s %s", mapname ); 
	fscanf( fp, "%*s %d", &numcoord); 

	assert( numcoord== nVtx );
	st = new sgVec2[nVtx];

	// remove "; from end of mapname
	mapname[ strlen(mapname)-2 ] = 0;

	sprintf(texname, "gamedata/textures/%s.png", mapname+1 );
	alpha = mapname[ strlen(mapname)-1 ] =='A';

	for (std::list<Texture*>::iterator ti = texCache.begin(); 
		ti != texCache.end(); ti++ ) {
			if ( !strcmp( (*ti)->name, texname ) ) {
				_RPT1( _CRT_WARN, "Found %s in the texture cache\n", texname );
				tex = (*ti);
				break;
			}
	}

	if (!tex) {
		_RPT1( _CRT_WARN, "Loading %s\n", texname );
		tex = new Texture( texname, alpha );	
		texCache.push_front( tex );
	}
	
	fscanf( fp, "%*s" ); // {
	
	// read the coords
	for (int i = 0; i < nVtx; i++) {
		fscanf( fp, "%s %s", ss, stt );
		
		// remove , and ; after numbers
		ss[strlen(ss)-1] = 0;
		stt[strlen(stt)-1] = 0;		
		
		sgSetVec2( st[i], atof( ss ), atof( stt ) );
	}
	
	fscanf( fp, "%*s" ); // }
	
	fscanf( fp, "%*s" ); // }
	

	fscanf( fp, "%*s" ); // }
}

void Mesh::skaReadSurfaceSection( FILE *fp, int num ) 
{
	char partname[1024], sa[100], sb[100], sc[100];
	int numtris;

	fscanf( fp, "%*s" ); // {
	for (int surf = 0; surf < num; surf++) {
		MeshPart *p = new MeshPart();

		fscanf( fp, "%*s" ); // {
		fscanf( fp, "%*s %s", partname ); // NAME "name";
	
		partname[strlen(partname)-2] = 0; // remove ";
		p->name = strdup( partname+1 );   // remove first quote and dup

		//_RPT1( _CRT_WARN, "Reading %s", p->name );

		fscanf( fp, "%*s %d", &numtris ); // TRIANGLE_SET num
		fscanf( fp, "%*s" ); // {

		p->tri = new Tri[numtris];
		p->nTri = numtris;
		for (int i = 0; i < numtris; i++) {
			//_RPT2( _CRT_WARN, "Reading triangle %d of %d\n", i, numtris );
			fscanf( fp, "%s %s %s", sa, sb, sc );
		
			// remove , and ;
			sa[strlen(sa)-1] = 0;
			sb[strlen(sb)-1] = 0;
			sc[strlen(sc)-1] = 0;

			//_RPT3( _CRT_WARN, "%s %s %s\n", sa, sb, sc);

			p->tri[i][0] = atoi( sa );
			p->tri[i][1] = atoi( sb );
			p->tri[i][2] = atoi( sc );
		}
		fscanf( fp, "%*s" ); // }

		part.push_back( p );
		fscanf( fp, "%*s" ); // }
	}
	fscanf( fp, "%*s" ); // {
}

void Mesh::skaSkipUnknownSection( FILE *fp )
{
	int count = 0;
	bool start = true;
	char token[1024];
	// loop until we've found some braces (!start) and 
	// we've consumed them all (when count is 0)
	while ((start)||(count)) {
		fscanf( fp, "%s", token );
		if (token[0]=='{') {
			start = false;
			count++;
		} else if (token[0]=='}') {
			count--;
		} // otherwise ignore the token
	}
}


////////////////////////////////////////////
Texture::Texture( const char *filename, bool alpha)
{
	hasAlpha = alpha;
	name = strdup( filename );

	id = ID_UNASSIGNED;

	if (hasAlpha) {
		loadPngRGBA( filename );
	} else {
		loadPngRGB( filename );
	}
	
}

// the font stuff changes the textures, so call this to 
// invalidate the texture state
void 
Texture::resetBound()
{
	currentBound = NULL;
}


void
Texture::bindTexture()
{	

	if (this==currentBound) return;
	
	if (id==ID_UNASSIGNED) {

		if (currTexId==-1) {
			glEnable( GL_TEXTURE_2D );
			glGenTextures( MAX_TEXTURES, texIdPool );
			currTexId = 0;
		}
      
		id = currTexId++;
		glBindTexture( GL_TEXTURE_2D, texIdPool[id] );
		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);    

		if (hasAlpha){
			glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
			glTexImage2D( GL_TEXTURE_2D, 0, 4, w, h,
				0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			
			glEnable( GL_TEXTURE_2D );
			
		} else {
			glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		
			glTexImage2D( GL_TEXTURE_2D, 0, 3, w, h,
				0, GL_RGB, GL_UNSIGNED_BYTE, data );
			
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			
			glDisable( GL_BLEND );		
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			
			glEnable( GL_TEXTURE_2D );	
		}

	} else {

		glEnable( GL_TEXTURE_2D );
		if (hasAlpha) {
			glEnable( GL_BLEND );
		} else {
			glDisable( GL_BLEND );
		}
		glBindTexture (GL_TEXTURE_2D, texIdPool[id] );
	}

	currentBound = this;
}

////////////////////////////////////////////
void main( int argc, char *argv[]) {
	ZillaGame *game;

	glutInitWindowPosition ( 0, 0 ) ;
	glutInitWindowSize( ZillaGame::c_Width, ZillaGame::c_Height ) ;
	glutInit( &argc, argv ) ;
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;

	//glutCreateWindow( "GodZILLA"  ) ;
	glutGameModeString("800x600:16@60");
    glutEnterGameMode();

	glClearColor( 0.5, 0.5, 1, 1.0 );

	glutDisplayFunc( ZillaGame::cbDisplay ) ;
	glutIdleFunc( ZillaGame::cbIdle  ) ;
	glutSpecialFunc( ZillaGame::cbSpecial ) ;
	glutMouseFunc( ZillaGame::cbMouse ) ;

	glEnable( GL_DEPTH_TEST );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	//glutMotionFunc      ( motionfn  ) ;
	glutKeyboardFunc    ( ZillaGame::cbKey ) ;

	game = new ZillaGame();

	glutMainLoop () ;	
}