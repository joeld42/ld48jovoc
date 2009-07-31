#include <deque>
#include <vector>
#include <list>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#ifdef WIN32
# include <windows.h>
# include <crtdbg.h>
#endif

#include <math.h>
#include <GL/glut.h>

#include <prmath/vector3.hpp>

#include <png.h>
#include "zillaGame.h"

using namespace prmath;

#ifndef M_PI
#  define M_PI (3.1415926535897932384626433832795)
#endif
#define D2R (M_PI/180.0)
#define R2D (180.0/M_PI)

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
	camPos = vec3f( 8.0, 4.0, 0.0 );
    camLookat = vec3f( 0, 1.2, 0 );

	currAngle = 0.0;
	currHeight = 1.5;
	currZoom = 3.0;

	dataLoaded = false;

#if ENABLE_FONT
	fnt = NULL;
#endif
	levelNdx = 0;
	follow = true;
	drawpath = false;

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
			camLookat = vec3f( godzilla->pos[0], 1.0, godzilla->pos[1] );
		}

		camPos = vec3f( 
				 camLookat[0] + cos( D2R * currAngle) * currZoom,
			currHeight, camLookat[2] + sin( D2R * currAngle) * currZoom );

		glutPostRedisplay();
	}

	// Update all of the current actors
	for (std::vector<Actor*>::iterator ai = actors.begin();
		ai != actors.end(); ai++ ) {
		(*ai)->update( level, timeDelta );	
	}

	// figure out where the mouse hit the ground
	GLdouble x, y, z, x2,y2,z2,sx, sy;
        
	// unproject some point to get a ray
	sx = (float)cursor2d[0];
	sy = (float)(c_Height-cursor2d[1]); 
	gluUnProject( sx, sy, 0.01, modelview, proj, viewport, &x, &y, &z );
	gluUnProject( sx, sy, 0.5, modelview, proj, viewport,  &x2, &y2, &z2 );
        
	// make a ray from cam pos and the point        
	vec3f dir;
	float t;
	dir = vec3f( x-x2, y-y2, z-z2 );
    dir.Normalize();
    
	// find out where it hits the ground    
	t = y2 / dir[1];        
    cursor3d = vec3f( x2 - (dir[0]*t), y2-(dir[1]*t), z2-(dir[2])*t );

	// is the cursor on the map?
	int tx, ty;
	level->worldToMap( cursor3d[0], cursor3d[2],tx, ty );
	

	if ((tx>=0) && (tx < level->w) &&
		(ty>=0) && (ty < level->h) ) {
		onMap = true;
		mapcursorX = tx; 
		mapcursorY = ty;		
	} else {
		onMap = false;
	}


	// mouse scroll
	vec2f camdir;
	float moveAmt = 5.0*timeDelta;

	if (cursor2d[0] < 10) {
		camdir = vec2f( -moveAmt, 0.0 );
		moveCamera( camdir );
	} else if (cursor2d[0] > 790 ) {
		camdir = vec2f( moveAmt, 0.0 );
		moveCamera( camdir );
	}


	if (cursor2d[1] < 10) {
		camdir = vec2f( 0.0, moveAmt );
		moveCamera( camdir );
	} else if (cursor2d[1] > 590 ) {
		camdir = vec2f( 0.0, -moveAmt );
		moveCamera( camdir );
	}	

}

void 
ZillaGame::UpdateTitle( float timeDelta )
{
	// do nothing
}

void 
ZillaGame::moveCamera( vec2f dir ) 
{
	vec3f front, right, up;	

    front = camLookat - camPos;    
	front[1] = 0.0;
    front.Normalize();
    up = vec3f( 0.0, 1.0, 0.0 );    
    right = CrossProduct( front, up );
    
    right.Normalize();
    
    front *= dir[1];
    right *= dir[0];

    camLookat = camLookat + front;
    camLookat = camLookat + right;
}

void
ZillaGame::Special( int key, int x, int y ) {
	vec2f camdir;
	float moveAmt = 0.2;

	switch( key ) {
		case GLUT_KEY_UP:			
			camdir = vec2f( 0.0, moveAmt );
			moveCamera( camdir );
			break;
		case GLUT_KEY_DOWN:
			camdir = vec2f( 0.0, -moveAmt );
			moveCamera( camdir );			
			break;
		case GLUT_KEY_RIGHT:
			camdir = vec2f(moveAmt, 0.0 );
			moveCamera( camdir );
			break;
		case GLUT_KEY_LEFT:
			camdir = vec2f( -moveAmt, 0.0 );
			moveCamera( camdir );
			break;
	}

	// DBG
    camPos = vec3f(
			camLookat[0] + cos( D2R * currAngle) * currZoom,
			currHeight, camLookat[2] + sin( D2R * currAngle) * currZoom );
	//_RPT3( _CRT_WARN, "camPos is %f %f %f\n", camPos[0], camPos[1], camPos[2] );

}

void
ZillaGame::Motion( int mx, int my ) {
    cursor2d = vec2f( mx, my );
}

void
ZillaGame::Mouse( int button, int state, int mx, int my ) {
	
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {

		if (level->map[mapcursorX][mapcursorY]->tankpassable) {
			// add a tank
			vec2f tankPos ( cursor3d[0], cursor3d[2] );

			Godzilla *tank = new Godzilla( tankMesh, tankPos );
			tank->speed = 0.3;
			tank->start = false;
			actors.push_back( tank );		
		}
	} else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
        godzilla->target = vec2f( cursor3d[0], cursor3d[2] );
	}

}

void
ZillaGame::Key( unsigned char key, int x, int y )
{
//	_RPT1( _CRT_WARN, "Key pressed: %c\n", key );

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
	case 'r': //reset godzilla
		godzilla->state = Actor::Idle;
		godzilla->start = true;
		break;
	case 'w':
		currHeight = currHeight + 0.1;
		break;
	case 'P':
		drawpath = !drawpath;
		break;
	case 'n': // next level
		levelNdx++;
		if (levelNdx==levels.size()) levelNdx = 0;
		level = levels[levelNdx];
		godzilla->state = Actor::Idle;
		godzilla->start = true;
		break;
		break;
	case 'p': // prev level
		levelNdx--;
		if (levelNdx==-1) levelNdx = levels.size()-1;
		level = levels[levelNdx];
		godzilla->state = Actor::Idle;
		godzilla->start = true;
		break;
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
	int passable, breakable, tankpassable;
	Tile *tile;

	assert(fp);

	while (!feof(fp) ) {
		fgets( line, 1024, fp );
		if (line[0]=='#') continue;
		sscanf( line, "%s %s %d %d %d", name, sym, &passable, &breakable, &tankpassable );		

		tile = new Tile( name, sym[0], passable==1, breakable==1, tankpassable==1 );
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
	
#if ENABLE_FONT
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
	
#endif

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

	glPopMatrix();

	// store the camera xform for later use
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, proj );
	glGetIntegerv( GL_VIEWPORT, viewport );   

	// draw the click point for debugging
	/*
	glDisable( GL_TEXTURE_2D );

	glPushMatrix();
	glColor3f( 1.0, 0.0, 0.0 );	
	glTranslated( dbgClickPnt[0], dbgClickPnt[1], dbgClickPnt[2] );
	glutSolidSphere( 0.1, 5, 5);
	glPopMatrix();

	glPushMatrix();
	glColor3f( 1.0, 0.0, 1.0 );	
	glTranslated( dbgClickPnt2[0], dbgClickPnt2[1], dbgClickPnt2[2] );
	glutSolidSphere( 0.1, 5, 5);
	glPopMatrix();
*/
	//godzilla->draw();
	for (std::vector<Actor*>::iterator ai = actors.begin();
		ai != actors.end(); ai++ ) {
		(*ai)->draw();		
	}

	// draw the cursor
	if (onMap) {		
		cursor->bindTexture();

		if (level->map[mapcursorX][mapcursorY]->tankpassable) {
			glColor3f( 0.0, 1.0, 0.0 );
		} else {
			glColor3f( 1.0, 0.0, 0.0 );
		}

		glBegin( GL_QUADS );
	
		float cursorHite = 0.1, sz = 0.5;
		glTexCoord2d( 0.0, 1.0 );		
		glVertex3f( cursor3d[0]-sz, cursorHite, cursor3d[2]-sz );
		glTexCoord2d( 0.0, 0.0 );	
		glVertex3f( cursor3d[0]-sz, cursorHite, cursor3d[2]+sz );
		glTexCoord2f( 1.0, 0.0 );	
		glVertex3f( cursor3d[0]+sz, cursorHite, cursor3d[2]+sz );
		glTexCoord2f( 1.0, 1.0 );	
		glVertex3f( cursor3d[0]+sz, cursorHite, cursor3d[2]-sz );
	
		glEnd();
	}	

	// draw path for testing
	glDisable( GL_TEXTURE_2D);
	glColor3f( 1.0, 0.0, 1.0 );
	glLineWidth( 3.0 );
	glBegin( GL_LINE_STRIP );
	float px, py;
	if (drawpath) {		
		for (std::list<vec2f*>::iterator pi = godzilla->waypoints.begin();
				pi != godzilla->waypoints.end(); *pi++) {
	
				px = (*(*pi))[0];
				py = (*(*pi))[1];
				glVertex3f( px, 0.5, py );
		}
	}
	glEnd();

}

void
ZillaGame::Display2D()
{
	char s[1024];

	// display buttons
	btntank->bindTexture();
	glColor4f( 1.0, 1.0, 1.0, 0.75 );

	glBegin( GL_QUADS );
	
	glTexCoord2d( 0.0, 1.0 );
	glVertex2f( 20, 20 );

	glTexCoord2d( 0.0, 0.0 );
	glVertex2f( 20, 120 );

	glTexCoord2f( 1.0, 0.0 );
	glVertex2i( 120, 120 );

	glTexCoord2f( 1.0, 1.0 );
	glVertex2i( 120, 20 );
	
	glEnd();


	glDisable( GL_LIGHTING ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glEnable( GL_BLEND ) ;
	glAlphaFunc( GL_GREATER, 0.1 ) ;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;

#if ENABLE_FONT	
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
#endif

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
ZillaGame::cbMotion( int x, int y)
{
	assert(theGame);
	theGame->Motion( x, y );
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
    long curTick = glutGet( GLUT_ELAPSED_TIME );
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
#if ENABLE_FONT
	fnt = new fntRenderer();

	fntHand = new fntTexFont( "gamedata/fonts/sorority.txf");
	fntHelv = new fntTexFont( "gamedata/fonts/helvetica_bold.txf");
	fntHelvBold = new fntTexFont( "gamedata/fonts/helvetica_bold.txf");
#endif	

/*
	testMesh = new Mesh( "gamedata/models/tile_basic_split.am");
	testMesh2 = new Mesh( "gamedata/models/tile_lightForest_split.am");
	testMesh3 = new Mesh( "gamedata/models/tile_residential_ns_split.am");

	testTex = new Texture( "gamedata/textures/testmap2.png", true );
	testTex2 = new Texture( "gamedata/textures/testmap3.png", false );
*/	
	skybox = new Mesh( "gamedata/models/sky.am");

	titleScreen = new Texture( "gamedata/textures/titlescreen.png", false );
	cursor = new Texture( "gamedata/textures/cursor.png", true );
	btntank = new Texture( "gamedata/textures/btntank.png", true );

    // FIXME: missing tank mesh
	//tankMesh = new Mesh( "gamedata/models/tank.am");
    tankMesh = new Mesh( "gamedata/models/house.am");

	vec2f startpos;
	startpos = vec2f( 0.0, 0.0 );
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
Tile::Tile(char *_name, char _symbol, bool _passable, bool _destructible, bool _tankpassable ) :
	symbol(_symbol), passable( _passable ), destructible( _destructible ), tankpassable( _tankpassable )
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
	
	//worldX = ((worldX+w*0.75) / (w*1.5))*w;
	//worldZ = ((worldZ+h*0.75) / (h*1.5))*h + 1.0;	
	worldX = (worldX/1.5)+(0.5*w);
	worldZ = (worldZ/1.5)+(0.5*h) + 1.0;

	mapX = (int)worldX;
	mapY = (int)worldZ;
}

void 
Level::mapToWorld( int mapX, int mapY,
				  float &worldX, float &worldZ ) {
	
	worldX = mapX; worldZ = mapY;

	worldX = (1.5*worldX) - (0.75*w) + 0.75;
	worldZ = (1.5*worldZ) - (0.75*h) - 0.75;
}

// path is returned by filling 'waypoints'
bool 
Level::pathTo( int startx, int starty, 
				int targx,  int targy,
				PathMode mode, 
				std::list<vec2f*> &waypoints )
{
	int i, j;

	// initialize the pathfinding map
	for (j = 0; j < h; j++ ) {
		for (i =0; i < w; i++) {
			pathMap[i][j].visited = false;
			pathMap[i][j].cost = 1.0;
			pathMap[i][j].pathcost = 0.0;

			if (mode==ModeZilla) {
				pathMap[i][j].pass = map[i][j]->passable;
			} else if (mode==ModeTank) {
				pathMap[i][j].pass = map[i][j]->tankpassable;
			}
		}
	}

	// BFS the map
	std::deque<MapPos> mapq;
	MapPos curr;
	int nx, ny;
	mapq.push_back( MapPos( startx, starty) );
	while(mapq.size()) {
		curr = (*mapq.begin());
		mapq.pop_front();
		
		assert( mapq.size() < (w*h) );
		printf( "Queuesize %d Curr %d %d\n", mapq.size(), curr.x, curr.y );

		pathMap[curr.x][curr.y].visited = true;
		if ((curr.x==targx)&&(curr.y==targy)) {
			
			for (j = 0; j < h;  j++) {
				for (i = 0; i < w; i++) {
					if (pathMap[i][j].visited) {
						printf( " %02.0f", pathMap[i][j].pathcost );
					} else {
						printf(" --" );
					}
				}
				printf("\n" );
			}
			
			// build path			
			buildPath( startx, starty, targx, targy, waypoints );
			
			return true;
		}

		// add reachable neighbors
		printf( "at %d %d\n", curr.x, curr.y );
		for (i=-1; i <= 1; i++) {
			for (j=-1; j <= 1; j++) {
				

				if ((curr.x+i>=0) && (curr.y+j>=0) &&
					(curr.x+i<w) && (curr.y+j<h)) {
					nx = curr.x+i; 
					ny = curr.y+j;
					if ((!pathMap[nx][ny].visited)&&(pathMap[nx][ny].pass)) {

						printf( "Adding %d %d\n", nx, ny );

						pathMap[nx][ny].visited = true;
						pathMap[nx][ny].pathcost = pathMap[curr.x][curr.y].pathcost + 
												   pathMap[nx][ny].cost;

						mapq.push_back( MapPos(nx, ny) );
					}
				}
			}
		}

	}

	// not reachable
	return false;
}

void Level::buildPath( int startx, int starty, 
			   int targx,  int targy,				 
			   std::list<vec2f*> &waypoints )
{
	// trace path backward
	int i,j,currx, curry,nx,ny;
	currx = targx; curry = targy;
	vec2f *p;

	waypoints.erase( waypoints.begin(), waypoints.end() );

	while ((currx!=startx)&&(curry!=starty)) {

		float mincost = pathMap[currx][curry].pathcost, maxncost=0.0;

		for (i=-1; i <= 1; i++) {
			for (j=-1; j <=1; j++) {
				//if ((i==0)&&(j=0)) continue;
				if ((currx+i>=0) && (curry+j>=0) &&
					(currx+i<w) && (curry+j<h)) {
					nx = currx+i;
					ny = curry+j;
					if ( (pathMap[nx][ny].visited) && 
						(pathMap[nx][ny].pathcost < mincost) ) { //&&
						//(pathMap[nx][ny].pathcost >= maxncost) ) {
						currx = nx;
						curry = ny;
						mincost = pathMap[nx][ny].pathcost;
						//maxncost = pathMap[nx][ny].pathcost;
					}
				}
			}
		}

		p = new vec2f[1];		
		mapToWorld( currx, curry, (*p)[0], (*p)[1] );
		printf( "path %d %d %f %f\n", currx, curry, (*p)[0], (*p)[1] );

		waypoints.push_front( p );
	}
}


bool 
Level::pickRandomMatch( const char *syms, int &resultx, int &resulty )
{
	// count how many occurances of this tile are in the map
	int ntile = 0, pick;
	bool done;
	for (int j = 0; j < h; j++) {
		for (int i=0; i < w; i++) {
			if (strchr(syms,map[i][j]->symbol)) {
				ntile++;
			}
		}
	}
	
	// no tiles match
	if (ntile==0) return false;

	// pick a random number and find that occurance of the tile
	pick = int( ((float)rand()/(float)RAND_MAX)*float(ntile) );
	printf( "pick is %d\n", pick );
	ntile = 0;
	done = false;
	for ( int j = 0; j < h; j++) {
		for (int i=0; i < w; i++) {
			if (strchr(syms,map[i][j]->symbol) ) {				
				printf("at %d %d (%c) ntile %d\n", i, j, map[i][j]->symbol, ntile );
				if (ntile==pick) {					
					resultx = i;
					resulty = j;
					done = true;
					break;
				}
				ntile++;
			}
		}
		if (done) break;
	}

	printf( "pick sym %s found %d %d\n", syms, resultx, resulty );
	return true;
}

/////////////////////////////////////////////
Actor::Actor(Mesh *_mesh, vec2f _pos )
{
	mesh = _mesh;	
    pos = _pos;
    
	state = Actor::Idle;

	speed = 1.0; // units/sec
}

void Actor::draw()
{
	glPushMatrix();	
	glTranslated( pos[0], 0.0, pos[1] );	
	glRotated( R2D * heading, 0.0, 1.0, 0.0 );
	mesh->draw();
	glPopMatrix();
}

void Actor::update( Level *lvl, float t )
{
	switch (state) {
	case Actor::Idle:
		// pick the next waypoint
		if (waypoints.size()) {
			//sgSetVec2f( target, (((float)rand() / (float)RAND_MAX) * (1.5*lvl->w)) - (0.75*lvl->w),
			//(((float)rand() / (float)RAND_MAX) * (1.5*lvl->h)) - (0.75*lvl->h) );

            target = vec2f ( (*(*waypoints.begin()))[0], (*(*waypoints.begin()))[1] ) ;
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
	int x=0, y=0;

	switch (state) {
	case Actor::Idle:

		if (start) {
			start = false;

			// start somewhere in the water			
			lvl->pickRandomMatch( "~", x, y );
			lvl->mapToWorld( x, y, pos[0], pos[1] );

			// find a path to the powerplant.
			int px, py;
			if (lvl->pickRandomMatch( "P", px, py )) {
                printf("Px py %d, %d\n", px, py );                
				
                if (lvl->pathTo( x, y, px, py, Level::ModeZilla, waypoints ) )
                {
                    //lvl->mapToWorld( px, py, target[0], target[1] );
                    target = vec2f( (*(*waypoints.begin()))[0], (*(*waypoints.begin()))[1] );
                    waypoints.pop_front();
                }
                

			} else {
				printf( "No powerplants\n" );
			}


		} else {

			if (!waypoints.empty()) {
				target = vec2f ( *(*waypoints.begin())[0], (*(*waypoints.begin()))[1] );
				waypoints.pop_front();

				if (moveTowardTarget( t ) ) {
					state = Actor::Idle;
				}

			} else {

				// pick a random house
				lvl->pickRandomMatch( "h", x, y );
				lvl->mapToWorld( x, y, target[0], target[1] );

			}
			//sgSetVec2f( target, (((float)rand() / (float)RAND_MAX) * (1.5*lvl->w)) - (0.75*lvl->w),
			//	(((float)rand() / (float)RAND_MAX) * (1.5*lvl->h)) - (0.75*lvl->h) );

		}
		
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
	vec2f dir;
	float dist, amt;
    dir = target - pos;    

    dist = LengthSquared( dir );
    
	if (dist < 0.001) {
		return true;
	}
	dist = sqrt(dist );
	/*
	_RPT3( _CRT_WARN, "targ %f %f dist %f\n", target[0], target[1], dist );
	_RPT3( _CRT_WARN, "pos %f %f dist %f\n", pos[0], pos[1], dist );
	*/

    dir.Normalize();
    
	heading = M_PI + atan2( dir[0], dir[1] );

	// don't overshoot the target
	amt = std::min( dist, t*speed );
	

    dir *= amt;
    pos = pos + dir;
    
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

    if (!fp)
    {
        printf("Can't load mesh %s\n", filename );        
    }
    
	
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

	vtx = new vec3f[num];
	nVtx = num;

	fscanf( fp, "%*s" ); // {

	// read the vertices
	for (int i = 0; i < num; i++) {
		fscanf( fp, "%s %s %s", sx, sy, sz );

		// remove , and ; after numbers
		sx[strlen(sx)-1] = 0;
		sy[strlen(sy)-1] = 0;
		sz[strlen(sz)-1] = 0;

		vtx[i] = vec3f( atof( sx ), atof( sy ), atof( sz ) );
	}

	fscanf( fp, "%*s" ); // }
}

void Mesh::skaReadNormalSection( FILE *fp, int num )
{
	char sx[100], sy[100], sz[100];

	nrm = new vec3f[num];

	fscanf( fp, "%*s" ); // {

	// read the vertices
	for (int i = 0; i < num; i++) {
		fscanf( fp, "%s %s %s", sx, sy, sz );

		// remove , and ; after numbers
		sx[strlen(sx)-1] = 0;
		sy[strlen(sy)-1] = 0;
		sz[strlen(sz)-1] = 0;

		nrm[i] = vec3f( atof( sx ), atof( sy ), atof( sz ) );
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
	st = new vec2f[nVtx];

	// remove "; from end of mapname
	mapname[ strlen(mapname)-2 ] = 0;

	sprintf(texname, "gamedata/textures/%s.png", mapname+1 );
	alpha = mapname[ strlen(mapname)-1 ] =='A';

	for (std::list<Texture*>::iterator ti = texCache.begin(); 
		ti != texCache.end(); ti++ ) {
			if ( !strcmp( (*ti)->name, texname ) ) {
				printf( "Found %s in the texture cache\n", texname );
				tex = (*ti);
				break;
			}
	}

	if (!tex) {
		printf("Loading %s\n", texname );
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
		
		st[i] = vec2f( atof( ss ), atof( stt ) );
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
int main( int argc, char *argv[]) {
	ZillaGame *game;

	glutInitWindowPosition ( 0, 0 ) ;
	glutInitWindowSize( ZillaGame::c_Width, ZillaGame::c_Height ) ;
	glutInit( &argc, argv ) ;
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;

	glutCreateWindow( "GodZILLA"  ) ;
	//glutGameModeString("800x600:16@60");
    //glutEnterGameMode();

	glClearColor( 0.5, 0.5, 1, 1.0 );

	glutDisplayFunc( ZillaGame::cbDisplay ) ;
	glutIdleFunc( ZillaGame::cbIdle  ) ;
	glutSpecialFunc( ZillaGame::cbSpecial ) ;
	glutMouseFunc( ZillaGame::cbMouse ) ;

	glEnable( GL_DEPTH_TEST );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	glutPassiveMotionFunc( ZillaGame::cbMotion ) ;
	glutKeyboardFunc( ZillaGame::cbKey ) ;

	game = new ZillaGame();

	glutMainLoop () ;	
    return 1;
    
}
