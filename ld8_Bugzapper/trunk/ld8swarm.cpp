// ld8swarm.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <crtdbg.h>
#include <math.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include "gamefontgl.h"

//#include <Cg/cg.h>
//#include <Cg/cgGL.h>

#include <vector>

#include <prmath/prmath.hpp>
using namespace prmath;

// external stuff that i'm to lazy to make a header file for
GLuint loadObj( const char *filename );
GLuint loadTexture( const char *filename );

// constants
#define UPDATE_TIME (0.01f)   // 100 updates per sec

#define MAX_SWING (6.0) // in degrees
#define DAMAGE_AMT (0.0055f)

#define GLOW_RES (32)

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795f)
#endif

float frand();
float frand( float min, float max );
float frand( float max );

// globals
bool dbgDrawNorms = false;
bool dbgDrawMode = false;  // debug mode or beauty mode
float g_viewAngle = 0.0f;
float g_viewAngleMove = 0.0f;

bool g_drawGlowMode = true;

int g_score = 0;
int g_level = 1;
int g_goalBugs = 0;
int g_numSwarms = 0;
char *g_levelText;
char highLevelText[100];

enum {
	GAMESTATE_MENU,
	GAMESTATE_GAME,
	GAMESTATE_PASSLEVEL,
	GAMESTATE_GAMEOVER,
};

int g_gameState = GAMESTATE_MENU;
int g_zappersLeft = 4;

// text output
ILuint ilFontId;
GLuint glFontTexId, fntFontId;

// models
GLuint idZapper, idZapGlow, idZapColor;

// textures
GLuint idTexGlow, idTexBug, idTexTitleScreen;

enum {
		COLOR_RED,
		COLOR_GREEN,
		COLOR_YELLOW,
		COLOR_PURPLE
	};

struct Zapper {

	int m_points;
	float m_level;		
	int color;
	bool m_alive;

	float m_health; 
	bool m_charged;

	// graphics stuff
	vec3f m_anchorPos;
	vec3f m_vel; // only used for falling effect
	vec3f m_goalPos;

	// swinging animation
	vec3f m_swingAng, m_swingVal;
	static float s_fSwingTime;

	// current world to zapper matrix
	matrix4x4f m_worldToZap, m_zapToWorld;


	Zapper() { init(); }
	void init() {
		m_points = 0;
		m_level = 0.0f;
		m_swingAng = vec3f( 0.0f, 0.0f, 0.0f );
		m_swingVal = vec3f( 0.0f, 0.0f, 0.0f );
		m_vel = vec3f( 0.0f, 0.0f, 0.0f );
		m_health = 1.0f;
		m_alive = true;
	}

	void update_sim( float dt);

	void draw();	
	void drawPanel();

	void toggle() {
		
		if (m_health <= 0.0f) return; // off

		if (m_level< 0.5f) {
			m_level =  10.0f;
		} else {
			m_level = 0;
		}
	}
};
float Zapper::s_fSwingTime = 0.0f;

std::vector<Zapper> zap(4);

struct Bug {
	vec3f pos;
	vec3f f, goaldir;
	vec3f fc, faway, fzap; // dbg
	int size;
	bool dead;
	bool active;

	Bug() {
		dead = false;
		active = true;

		fc = vec3f( 0.0f, 0.0f, 0.0f );
		faway = vec3f( 0.0f, 0.0f, 0.0f );
		fzap = vec3f( 0.0f, 0.0f, 0.0f );
		goaldir = vec3f( 0.0f, 0.0f, 0.0f );
		f = vec3f( 0.0f, 0.0f, 0.0f );

	}

	void spawn( vec3f swarm_pos, float rad);
};

struct Swarm {	

	int color;
	int type;
	int numActive;
	std::vector<Bug> bug;

	vec3f m_center;
	static matrix4x4f m_proj; // used to draw

	void spawn();
	void draw();
	void update( float dt );
	void checkCollision();
	void update_swarm_forces();
};
std::vector<Swarm*> swarms;
matrix4x4f Swarm::m_proj;


//----------------------------------------------------------------------
void drawCube( float xs, float ys, float zs ) {
	
	float y0 = 0.0, y1 = -ys;
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

float frand() 
{
	return (float)rand()/ (float)RAND_MAX;
}

float frand( float min, float max ) 
{
	return (frand()*(max-min)) + min;
}

float frand( float max )
{
	return frand( 0.0f, max );
}

vec3f getColor( int color ) 
{
	switch (color) {
		case COLOR_RED:
			return vec3f( 1.0f, 0.0f, 0.0f );
			break;
		case COLOR_GREEN:
			return vec3f( 0.0f, 1.0f, 0.0f );
			break;
		case COLOR_YELLOW:
			return vec3f( 1.0f, 1.0f, 0.0f );
			break;
		case COLOR_PURPLE:
			return vec3f( 1.0f, 0.0f, 1.0f );
			break;		
	}

	return vec3f( 0.5f, 0.5f, 0.5f );
}

//----------------------------------------------------------

void Zapper::draw() 
{
	//glowColorDim = 58, 88, 93;
	//globrite     = 73,232,232
	float t = ((float)m_level / 10.0f);
	vec3f glowColorDim( 58.0f/255.0f, 88.0f/255.0f, 93.0f/255.0f );
	vec3f glowColorBrite( 73.0f/255.0f, 232.0f/255.0f,232.0f/255.0f );
	vec3f glowColor = (glowColorDim*(1.0f-t)) + (glowColorBrite*t);

	glPushMatrix();
	glTranslated( m_anchorPos.x, m_anchorPos.y, m_anchorPos.z );

//	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	
	// Draw rope 
	glLineWidth( 3.0f );
	glColor3f( 0.0, 0.0, 0.0 );
	float ropeStep = -0.3f;
	for (int i=0; i < 3; i++) {
		glRotated( m_swingAng.z * MAX_SWING * m_health, 0.0f, 0.0f, 1.0f );
		glRotated( m_swingAng.y * MAX_SWING * m_health, 0.0f, 1.0f, 0.0f );
		glRotated( m_swingAng.x * MAX_SWING * m_health, 1.0f, 0.0f, 0.0f );
		glBegin( GL_LINES );
		glVertex3f( 0.0, 0.0, 0.0 );
		glVertex3f( 0.0, ropeStep, 0.0 );
		glEnd();

		glTranslated( 0.0, ropeStep, 0.0 );
	}

	glColor3f( glowColor.r, glowColor.g, glowColor.b );	

	if (dbgDrawMode) {
		drawCube( 1.0, 2.0, 1.0 );
	} else {
		if (!g_drawGlowMode) {		
			glCallList( idZapGlow );
		} else if (m_level> 0.5f) {
			glPushMatrix();
			glScalef( 1.1f, 1.1f, 1.1f );
			glCallList( idZapGlow );
			glPopMatrix();
		}
	}

	vec3f zapColor = getColor( color ) * m_health;
	glColor3f( zapColor.r, zapColor.g, zapColor.b );

	if (dbgDrawMode) {
		drawCube( 0.25, -0.25, 0.25 ); // top cube
	} else {
		glCallList( idZapColor );

		if (!g_drawGlowMode) {
			glEnable( GL_LIGHTING );
			glColor3f( 0.75, 0.75, 0.75 );
		} else {
			glDisable( GL_LIGHTING );
			glColor3f( 0.0f, 0.0f, 0.0f );
		}
		glCallList( idZapper );

		glDisable( GL_LIGHTING );
	}

	// remember matrix state for this zapper
	glGetFloatv( GL_MODELVIEW_MATRIX, m_zapToWorld.m16 );
	m_worldToZap = m_zapToWorld;
	m_worldToZap.Inverse();

	if (dbgDrawMode) {
		glTranslated( 0.0f, -2.0f, 0.0f );	
		drawCube( 0.25, 0.25, 0.25 ); // bottom cube
	}

	// draw bottom cube
//	glTranslated( 0.0, ropeStep, 0.0 );

	// test matrix
	//vec3f testPos = m_anchorPos * m_worldToZap;
	//printf("anchor pos %f %f %f in zap space %f %f %f\n",
	//			m_anchorPos.x, m_anchorPos.y,  m_anchorPos.z, 
	//			testPos.x, testPos.y, testPos.z );

	m_goalPos = vec3f( 0.0f, -1.0f, 0.0f) * m_zapToWorld;
	


	// restore matrix state
	glPopMatrix();
}

void Zapper::drawPanel()
{
	static GLuint idTexPanel, idTexDial;
	static bool stuffLoaded = false;
	if (!stuffLoaded) {
		idTexPanel = loadTexture( "gamedata\\hud_bg.jpg" );
		idTexDial  = loadTexture( "gamedata\\dial.png"  );
		stuffLoaded = true;
	}

	// draw the background
	glColor3f( 1.0f, 1.0f, 1.0f );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, idTexPanel );

	glBegin( GL_QUADS );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f( 0.0f, 0.0f, 0.0f );

	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f( 190.0f, 0.0f, 0.0f );

	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f( 190.0f, 100.0f, 0.0f );

	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f( 0.0f, 100.0f, 0.0f );
	glEnd();

	glDisable( GL_TEXTURE_2D );

	// draw the dial
	if (dbgDrawMode) {		
		glLineWidth( 5.0f );
		glColor3f( 0.3f, 0.3f, 0.3f );
		glBegin(GL_POLYGON );
		for (float ang=0; ang < 2.0f*M_PI; ang +=  2.0f*M_PI/40.0f ) {
			glVertex3f( 50 + cos(ang)*20, 
						50 + sin(ang)*20, 0.0f );
		}	
		glEnd();

		float ang = m_level * M_PI/10.0f;
		glBegin(GL_LINES);
		glVertex3f( 50, 50, 0 );
		glVertex3f( 50 - cos(ang)*30, 
					50 + sin(ang)*30, 0.0f );
		glEnd();
	} else {
		glAlphaFunc( GL_GREATER, 0.9f );
		glEnable( GL_ALPHA_TEST );

		glPushMatrix();
		glColor3f( 1.0f, 1.0f, 1.0f );

		glTranslated( 50.0f, 50.0f, 0.0f );
		
		glRotatef( -((m_level*18.0f)-90.0f), 0.0f, 0.0f, 1.0f );
		glTranslated( -32.0f, -32.0f, 0.0f );

		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, idTexDial );

		glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3f( 0.0f, 0.0f, 0.0f );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex3f( 64.0, 0.0f, 0.0f );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex3f( 64.0f, 64.0f, 0.0f );

		glTexCoord2f( 0.0f, 0.0f );
		glVertex3f( 0.0f, 64.0f, 0.0f );
		glEnd();

		glDisable( GL_ALPHA_TEST );
		glPopMatrix();
	}

	// draw the fuze
	vec3f c = getColor( color ) * m_health;
	glColor3f(c.r,c.g,c.b);		
	glTranslated( 100, 25, 0 );
	glBegin( GL_QUADS );
	glVertex3f( 0.0f, 10.0f, 0.0f );
	glVertex3f( 0.0f, 40.0f, 0.0f );
	glVertex3f( 50.0f, 40.0f, 0.0f );
	glVertex3f( 50.0f, 10.0f, 0.0f );
	glEnd();		

	

}

void Zapper::update_sim( float dt)
{
	if (m_anchorPos.y < -6.0f ) {
		// dead and fallen, ignore it
		return;
	}

	// charging?
	if (m_charged) {
		m_level += 50.0f*dt;
		if (m_level > 10.0f) m_level = 10.0f;
	} else {
		m_level -= 50.0f*dt;
		if (m_level < 0.0f) m_level = 0.0f;
	}

	// is zapper dead?
	if (m_health < 0.001f) {

		// record zapper death
		if (m_alive) {
			m_alive = false;
			g_zappersLeft -= 1;
		}

		m_vel += vec3f( 0.0f, -5.0f*dt, 0.0f );
		m_anchorPos += m_vel * dt;
		m_health = 0.0f; // clamp 
		m_level = 0; // make sure it's off

		if ((g_zappersLeft==0) && (m_anchorPos.y < -4.0f )) {
			// everythings dead
			g_gameState = GAMESTATE_GAMEOVER;
			g_level = 0;
		}
	}
}

//----------------------------------------------------------------------
void Bug::spawn( vec3f swarm_pos, float rad)
 {
	f = vec3f(frand(-1,1), frand(-1,1), frand(-1,1));
	f.Normalize();
	f *= 0.8f;

	pos = vec3f( frand(-1.0f, 1.0f), frand(-1.0f, 1.0f), frand(-1.0f, 1.0f));
	//pos.Normalize();
	pos = swarm_pos + (pos * rad);
	size = (int)frand( 1.0f, 5.0f );
	dead = false;
	active = true;
		
	assert( fabs( pos.x ) < 1000.0f );
}

//----------------------------------------------------------------------
void Swarm::spawn()
{
	// random swarm pos (outside of play area)
	vec3f pos;
	pos = vec3f( frand(-1.0f, 1.0f), frand(0.0f, 1.0f), frand(-1.0f, 1.0f));
	pos = Normalize(pos);
	pos *= 15.0f;

//	printf("Spawn pos %f %f %f\n", pos.x, pos.y, pos.z );

	// random color
	color = (int)frand(4);
	
	// for now just make a bunch	
	bug.erase( bug.begin(), bug.end() );
	Bug b;
	int NUM_BUGS = (int)frand(30, 120);
	for (int i=0; i < NUM_BUGS; i++) {
		b.spawn( pos, 2.0f );
		bug.push_back( b );
	}
	
}

void Swarm::draw()
{
	vec3f swarmCol( 0.1f, 0.1f, 0.1f );
	swarmCol = getColor( color );

	vec3f vright, vup, vr, vu;
	vright = m_proj.GetX();
	vup = m_proj.GetY();

	vup = Normalize( vup );
	vright = Normalize( vright );
	

	// draw buggies	
	if (dbgDrawMode) {
		glLineWidth( 1.0f );
		for (size_t i=0; i < bug.size(); i++) {
			Bug &b = bug[i];		

			if (!b.active) continue;

			if (b.dead) {
				glColor3f( 0.0f, 0.0f, 0.0f );			
			} else {
				glColor3f(swarmCol.r, swarmCol.g, swarmCol.b );			
			}
			glPointSize( (float)b.size );	
			glBegin( GL_POINTS );			
			glVertex3f( b.pos.x, b.pos.y, b.pos.z );		
			glEnd();

			if (dbgDrawNorms) {
				glBegin( GL_LINES );
			
				glColor3f( 0.0f, 1.0f, 1.0f );
				glVertex3f( b.pos.x, b.pos.y, b.pos.z );
				glVertex3f( b.pos.x + b.f.x, 
							b.pos.y + b.f.y, 
							b.pos.z + b.f.z );

				glColor3f( 1.0f, 0.0f, 0.0f );
				glVertex3f( b.pos.x, b.pos.y, b.pos.z );
				glVertex3f( b.pos.x + b.goaldir.x, 
							b.pos.y + b.goaldir.y, 
							b.pos.z + b.goaldir.z );		

				glColor3f( 0.5f, 0.5f, 1.0f );
				glVertex3f( b.pos.x, b.pos.y, b.pos.z );
				glVertex3f( b.pos.x + b.fzap.x, 
							b.pos.y + b.fzap.y, 
							b.pos.z + b.fzap.z );		

				glEnd();
			}
		}
	} else {
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glDepthMask( 0 );
		//glBlendFunc( GL_ONE, GL_ONE );
		glBindTexture( GL_TEXTURE_2D, idTexBug );
		glBegin( GL_QUADS );

		for (size_t i=0; i < bug.size(); i++) {
			Bug &b = bug[i];		

			if (!b.active) continue;

			if (b.dead) {
				glColor3f( 0.0f, 0.0f, 0.0f );			
			} else {
				glColor3f(swarmCol.r, swarmCol.g, swarmCol.b );			
			}

			vr = vright * b.size * 0.1;
			vu = vup * b.size * 0.1;
			vec3f p;
			p = b.pos - vu - vr;
			glTexCoord2f( 0.0f, 1.0f );
			glVertex3f( p.x, p.y, p.z );

			p = b.pos + vu - vr;
			glTexCoord2f( 0.0f, 0.0f );
			glVertex3f( p.x, p.y, p.z );
			
			p = b.pos + vu + vr;
			glTexCoord2f( 1.0f, 0.0f );
			glVertex3f( p.x, p.y, p.z );

			p = b.pos - vu + vr;
			glTexCoord2f( 1.0f, 1.0f );
			glVertex3f( p.x, p.y, p.z );
			
		}

		
		glEnd();
		glDisable( GL_BLEND );
		glDepthMask( 1 );
	}

}

void Swarm::update( float dt )
{
	static vec3f gravity(0.0f, -9.8f, 0.0f );
	for (size_t bndx=0; bndx < bug.size(); bndx++)
	{
		Bug &b = bug[bndx];

		if (!b.active) continue;

		if (b.dead) {
			
			// "ballistic motion" heheh
			b.f += gravity*dt;
			
			b.pos += b.f*dt;			
		} else {
			b.f = b.f + (b.goaldir*dt);
			b.pos += b.f *dt;

			// Max bug speed
			float d;
			float maxBugSpeed = 2.0f;
			if ((d = Length(b.f)) > maxBugSpeed) {
				b.f /= d;
				b.f *= maxBugSpeed;
			}
		}

		assert (fabs(b.pos.x) < 1000.0f );
	}

}

void Swarm::checkCollision()
{
	for (size_t zi=0; zi < zap.size(); zi++) {
		Zapper &z = zap[zi];
		vec3f bugZapPos;
		for (size_t bndx=0; bndx < bug.size(); bndx++) {
			bugZapPos = bug[bndx].pos * z.m_worldToZap;
			if ( (bugZapPos.x > -0.5f) && (bugZapPos.x < 0.5f) &&
				 (bugZapPos.z > -0.5f) && (bugZapPos.z < 0.5f) &&
				 (bugZapPos.y > -2.0f) && (bugZapPos.y < 0.0f) ) {

				// ZZaappp!
			    //todo: give zapper points
				//printf("ZZZapapp!\n" );
				
				// any bug, give points
				g_score++;
				
				// did they pass the level?
				if (g_score >= g_goalBugs) {
					g_level++;
					g_gameState = GAMESTATE_PASSLEVEL;
				}

				if (color!=z.color) {				
					// didn't match, damage zapper
					z.m_health -= DAMAGE_AMT;					
				}

				// mark bug as dead
				bug[bndx].dead = true;

				// make it look like the bug "bounces off"
				bug[bndx].f = bug[bndx].f * -2.0f;
			} 
		}
	}

	// check if any dead bugs have hit the ground and replace them
	numActive = 0;
	for (size_t bndx=0; bndx < bug.size(); bndx++) {
		Bug &b = bug[bndx];
		if ((b.active) && (b.dead) && (b.pos.y < -2.0f)) {
			b.active = false;
		} else {
			if (b.active) numActive++;
		}
	}



	if (numActive<5) {
		//printf("Whole swarm is dead\n" );
		spawn();
	}// else {
	//	printf("numActive %d\n", numActive );
	//}

}

void Swarm::update_swarm_forces() 
{
	// Update bugs
	vec3f center( 0.0f, 0.0f, 0.0f ); 
	vec3f groupDir( 0.0f, 0.0f, 0.0f ); 
	vec3f faway; // average away direction
	vec3f fc, n;
	float fRepelRad = 0.5f;
	float fFlockRad = 3.0f;
	float fZapSeekRad = 2.0f; // will still attract past this, but increase closer

	// find centeroid & group direction
	size_t numBugs = 0;
	for (size_t bndx=0; bndx < bug.size(); bndx++)
	{
		if (bug[bndx].dead) continue;
		center += bug[bndx].pos;

		vec3f v = bug[bndx].f;
		Normalize(v);
		groupDir += v;

		numBugs++;
	}
	if (numBugs) {
		center /= (float)numBugs;

		groupDir /= (float)numBugs;
		Normalize( groupDir );
	} 	
	m_center = center;		

	if (fabs(m_center.x) > 100.0f) {
		printf("---- BAD SWARM --- %3.2f %3.2f %3.2f\n",
				 m_center.x, m_center.y, m_center.z );
		for (size_t bndx=0; bndx < bug.size(); bndx++) {
			printf("Bug Pos: %3.2f %3.2f %3.2f\n", 
				bug[bndx].pos.x,
				bug[bndx].pos.y,
				bug[bndx].pos.z );
		}
		assert(0);
	}

	// update forces
	for (size_t bndx=0; bndx < bug.size(); bndx++)
	{
		float d;
		Bug &b = bug[bndx];

		// dead bugs get updated in update()
		if (b.dead) continue;

		fc = center - b.pos;
		d = Length(fc);
		if (d > 0.001) {
			fc /= d;

			// center factor
			if (d > fFlockRad) { d=fFlockRad;}
			fc *= pow( d/fFlockRad,10.0f );
		}

		// get average away dir
		faway = vec3f(0.0f,0.0f,0.0f);		

		for (size_t jndx=0; jndx<bug.size(); jndx++) {
			if ((bndx==jndx) ||( bug[jndx].dead)) continue;

			n = b.pos - bug[jndx].pos;
			d = Length(n);
			if ((d>0.0001)&&(d < fRepelRad)) {
				n /= d;

				// away factor			
				if (d > fRepelRad) { d=fRepelRad;}
				n *= pow( 1.0f - (d/fRepelRad), 3.0f );

				faway += n;
			}
		}		
		//faway /= bug.size();

		// update the zapper attraction forces
		vec3f fzapper = vec3f( 0.0f, 0.0f, 0.0f );		
		int zappersOn = 0;
		for (size_t zi=0; zi < zap.size(); zi++) {		
			Zapper &z = zap[zi];
			vec3f fz = z.m_goalPos - b.pos;
					
#if 1
			// OLD
			fz = Normalize(fz);
			fz *= ((float)z.m_level / 10.0f );

#else
			// NEW
			d = Length(fz);
			if (d > 0.0001) {
				fz /= d;
				if (d > fZapSeekRad) d = fZapSeekRad;
				fz *= (0.5f + pow( 1.0f - (d/fZapSeekRad), 3.0f )*1.0f );
			}

			// attenuate attrators
			fz *= ((float)z.m_level / 10.0f ); 
#endif
			
			fzapper += fz;
			if (z.m_level>0.5f) {
				zappersOn++;
			}
		}		 		

		// if no zappers, go for the center (weakly)
		if (zappersOn==0) {
			fzapper = vec3f( 0.0f, 4.0f, 0.0f) - b.pos;
			fzapper = Normalize(fzapper);
			fzapper *= 0.2;
		}


		// update bug force towards f
		vec3f fresult = ((fc * 2.0f)+ 
			             (faway * 8.0f) + 
						 (fzapper*4.0f) +
						 (groupDir* 1.2f)
						 ); 		

		b.goaldir = fresult;
		b.fc = fc;
		b.faway = faway;
		b.fzap = fzapper;

		assert( fabs(fresult.x) < 1000.0f );
		assert( fabs(fresult.y) < 1000.0f );
		assert( fabs(fresult.z) < 1000.0f );		
	}
}

//----------------------------------------------------------------------
void do_quit()
{
    // Do cleanup here
    exit(0);
}

void initGame() 
{
	srand( SDL_GetTicks() );

	vec3f zapperPos[4];
	zapperPos[0] = vec3f( -4.0, 6.0f,  4.5f );
	zapperPos[1] = vec3f( -4.0, 6.0f, -4.5f );
	zapperPos[2] = vec3f(  4.0, 6.0f, -4.5f );
	zapperPos[3] = vec3f(  4.0, 6.0f,  4.5f );

	//printf("InitGame\n");
	g_score = 0;

	// Initialize zappers
	g_zappersLeft = 4;
	for (size_t i=0; i < zap.size(); i++) {
		zap[i].init();
		
		//zap[i].m_anchorPos = vec3f(  (i*5.0f) - 7.5f, 6.0f, -3.0f );
		zap[i].m_anchorPos = zapperPos[i];
		
		zap[i].m_swingVal = vec3f( frand( 0.9f, 1.2f ), 
								   frand( 0.9f, 1.2f ), 
								   frand( 0.9f, 1.2f ) );
		// zapper color
		zap[i].color = (int)i;
	}

	// delete any existing swarms
	for (int i=0; i < swarms.size(); i++) {
		delete swarms[i];
	}

	// Init swarms
	swarms.erase( swarms.begin(), swarms.end() );
	for (int i=0; i < g_numSwarms; i++) {
		Swarm *s = new Swarm;
		swarms.push_back( s );
		s->spawn();
	}
	
	
}

void draw2d_frontend()
{

	static bool stuffLoaded = false;
	if (!stuffLoaded) {
		idTexTitleScreen = loadTexture ("gamedata\\titlescrn.jpg" );
		stuffLoaded = true;
	}

	// Draw menu screen
	glEnable( GL_TEXTURE );
	glPushMatrix();	

	if (g_gameState == GAMESTATE_MENU )
	{
		glColor3f( 1.0f, 1.0f, 1.0f );

		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, idTexTitleScreen );

		glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3f( 0.0f, 0.0f, 0.0f );
	
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3f( 800.0f, 0.0f, 0.0f );
	
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3f( 800.0f, 600.0f, 0.0f );

		glTexCoord2f( 0.0f, 0.0f );
		glVertex3f( 0.0f, 600.0f, 0.0f );
		glEnd();			

		gfEnableFont( fntFontId, 20 );	
		gfBeginText();	
		glTranslated( 75, 30, 0 );
		gfDrawString( "Press SPACE to start Zapping" );
		gfEndText();

	} else if (g_gameState == GAMESTATE_PASSLEVEL ){
		glColor3f( 0.0f, 1.0f, 0.0f );
		gfEnableFont( fntFontId, 20 );	
		gfBeginText();	

		switch ( g_level ) {
			case 1:
				g_levelText = "Zapping 101";
				g_numSwarms = 1;
				g_goalBugs = 500;
				break;
			case 2:
				g_levelText = "Zap Carefully";
				g_numSwarms = 2;
				g_goalBugs = 500;
				break;
			case 3:
				g_levelText = "Plethora";
				g_numSwarms = 2;
				g_goalBugs = 1000;
				break;
			case 4:
				g_levelText = "Tricky Bugs";
				g_numSwarms = 3;
				g_goalBugs = 1000;
				break;
			case 5:
				g_levelText = "Swarm-tastic";
				g_numSwarms = 3;
				g_goalBugs = 1200;
				break;
			case 6:
				g_levelText = "Time for an Onslaught";
				g_numSwarms = 4;
				g_goalBugs = 500;
				break;
			case 7:
				g_levelText = "Bug Bonanza";
				g_numSwarms = 4;
				g_goalBugs = 1000;
				break;
			case 8:
				g_levelText = "Bugzilla";
				g_numSwarms = 4;
				g_goalBugs = 1500;
				break;
			case 9:
				g_levelText = "Ludumdare ate my brain";
				g_numSwarms = 5;
				g_goalBugs = 1000;
				break;
			default:
				g_levelText = highLevelText;
				sprintf(highLevelText, "Insanity %d\n", g_level-9 );

				g_numSwarms = 6;
				g_goalBugs = 500 * (g_level-9);
				break;
		}
		
		glTranslated( 200, 400, 0 );
		gfDrawStringFmt( "LEVEL %d -- %s", g_level, g_levelText );

		glColor3f( 1.0f, 1.0f, 1.0f );
		glTranslated( 0, -40, 0 );
		gfDrawStringFmt( "You are facing %d swarms.", g_numSwarms );

		glTranslated( 0, -20, 0 );
		gfDrawStringFmt( "Zap %d bugs to pass", g_goalBugs );


		glTranslated( 0, -40, 0 );
		gfDrawString( "Use 1,2,3,4 keys to activate zappers." );
		glTranslated( 0, -20, 0 );
		gfDrawString( "Use left/right arrows to change camera." );
		glTranslated( -150, -60, 0 );
		gfDrawString( "Warning: Zapping bugs that don't match the" );
		glTranslated( 0, -20, 0 );
		gfDrawString( "zapper color will damage zapper and void your warranty!" );
			

		gfEndText();

	} else if (g_gameState == GAMESTATE_GAMEOVER ){
		glColor3f( 1.0f, 0.5f, 0.5f );
		gfEnableFont( fntFontId, 72 );	
		gfBeginText();	
		glTranslated( 200, 400, 0 );
		gfDrawString( "game over" );
		gfEndText();
	}
}

void draw2d() 
{	
	
	
	// Draw score
	glEnable( GL_TEXTURE );
	glPushMatrix();

	glColor3f( 1.0f, 1.0f, 1.0f );

	gfEnableFont( fntFontId, 20 );	
	gfBeginText();	
	glTranslated( 10, 550, 0 );
	gfDrawStringFmt( "Zapped: %d/%d", g_score, g_goalBugs );
	gfEndText();
	

	// draw dbg
#if 0
	gfEnableFont( fntFontId, 12 );
	gfBeginText();
	
	glPushMatrix();
	glTranslated( 10, 500, 0 );
	gfDrawStringFmt("%d swarms", swarms.size() );
	glPopMatrix();


	for (size_t i=0; i < swarms.size(); i++) {
		glPushMatrix();
		glTranslated( 10, 480-i*15, 0 );
		gfDrawStringFmt("%d) %d bugs, center %3.2f %3.2f %3.2f", 
					i, swarms[i]->numActive,
					swarms[i]->m_center.x,
					swarms[i]->m_center.y,
					swarms[i]->m_center.z );
		glPopMatrix();
	}
	gfEndText();
#endif

	glPopMatrix();
	glDisable( GL_TEXTURE );

	// Draw zapper control panels	
	for (size_t zi=0; zi < zap.size(); zi++) {
		Zapper &z = zap[zi];
		
		glPushMatrix();
		glTranslated( 10+zi*200, 10, 0 );
		z.drawPanel();
		glPopMatrix();

		// panel text
		glColor3f( 1.0f, 1.0f, 1.0f );
		gfEnableFont( fntFontId, 20 );	
		gfBeginText();		
		glTranslated( 110+zi*200, 80, 0 );
		gfDrawStringFmt( "%d %s", zi+1, ((z.m_level>0.5f)?"ON":"OFF"));
		gfEndText();

		gfEnableFont( fntFontId, 12 );	
		gfBeginText();
		glColor3f( 1.0f, 1.0f, 1.0f );
		glTranslated( 85+zi*200, 30, 0 );
		gfDrawStringFmt( "Health: %d", (int)(z.m_health*100) );
		gfEndText();
	}
	


}

GLuint loadTexture( const char *filename )
{
	ILuint ilImgId;
	ilGenImages( 1, &ilImgId );
	ilBindImage( ilImgId );		
	
	if (!ilLoadImage( ILstring(filename) )) {
		printf("Loading image %s failed.\n", filename);
	}
	
	// Make a GL texture for it	
	return ilutGLBindTexImage();
}

void draw3d() 
{
	static bool stuffLoaded = false;
	static GLuint idGazebo, idGround, idSky;
	static GLuint idLmapGround, idLmapGazebo, idTexSky, idTexSkyGlow;
	if (!stuffLoaded) {
		// Load stuff
		idGround = loadObj( "gamedata\\ground.obj" );
		idLmapGround = loadTexture("gamedata\\ground_good.png" );

		idGazebo = loadObj( "gamedata\\gazebo.obj" );
		idLmapGazebo = loadTexture("gamedata\\gazebo_good.png" );

		idSky = loadObj( "gamedata\\sky.obj" );
		idTexSky = loadTexture("gamedata\\skybox.jpg" );
		idTexSkyGlow = loadTexture("gamedata\\skybox_glo.jpg" );

		idZapper = loadObj( "gamedata\\zapper_case.obj" );
		idZapGlow = loadObj( "gamedata\\zapper_glow.obj" );
		idZapColor = loadObj( "gamedata\\zapper_color.obj" );

		idTexGlow = loadTexture( "gamedata\\glow_dummy.png" );

		idTexBug = loadTexture("gameData\\bug.png" );

		// also set up lighting
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );

		vec3f lightColor(247.0f, 243.0f, 180.0f), 
			  specColor(1.0f, 1.0f, 1.0f );
		lightColor /= 255.0f;
		float fv[3];
		fv[0] = lightColor.x; fv[1] = lightColor.y; fv[2] = lightColor.z;
		glLightfv( GL_LIGHT0, GL_DIFFUSE, fv );

		fv[0] = specColor.x; fv[1] = specColor.y; fv[2] = specColor.z;
		glLightfv( GL_LIGHT0, GL_SPECULAR, fv );
		
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, fv );		

		fv[0] = 25.0;
		glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, fv );

		glDisable( GL_LIGHTING );

		stuffLoaded = true;
	}

	// setup
	if (g_drawGlowMode) {
		glViewport( 0, 0, GLOW_RES, GLOW_RES );
	} else {
		glViewport( 0, 0, 800, 600 );
	}

	glShadeModel( GL_SMOOTH );
	
	// draw world
	if (dbgDrawMode) {
		// draw a cube
		glLineWidth( 1.0f );
		glColor3f( 0.5f, 0.5f, 0.5f );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glPushMatrix();
		glTranslated( 0.0f, 8.0f, 0.0f );
		drawCube( 10.0f, 8.0f, 8.0f );
		glPopMatrix();
	} else {
		// draw the world objects
		
		
		// ground
		
		glDisable( GL_TEXTURE_2D );
		glPushMatrix();
		if (!g_drawGlowMode) {
			glColor3f( 198.0f/255.0f, 174.0f/255.0f, 148.0f/255.0f );
		} else {
			glColor3f( 0.0f, 0.0f, 0.0f );
		}
		glTranslated( 0.0f, -0.1f, 0.0f);
		glScalef( 50.0f, 50.0f, 50.0f );
		glCallList( idGround );
		glPopMatrix();

		if (!g_drawGlowMode) {
			glColor3f( 1.0f, 1.0f, 1.0f  );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, idLmapGround );		
			glCallList( idGround );
		}
		

		
		// gazebo structure	
		if (g_drawGlowMode) {
			glColor3f( 0.0f, 0.0f, 0.0f );
			glDisable( GL_TEXTURE_2D );
		}
		glBindTexture( GL_TEXTURE_2D, idLmapGazebo );		
		glCallList( idGazebo );
		
		// skybox
		if (g_drawGlowMode) {
			glColor3f( 1.0f, 1.0f, 1.0f );
			glEnable( GL_TEXTURE_2D );
		}
		glPushMatrix();
		glScalef( 100.0f, 100.0f, 100.0f );
		if (g_drawGlowMode) {
			glBindTexture( GL_TEXTURE_2D, idTexSkyGlow );
		} else {
			glBindTexture( GL_TEXTURE_2D, idTexSky );
		}

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP); 

		glMatrixMode( GL_TEXTURE );
		glPushMatrix();
		glScalef( 3.0f, 2.0f, 1.0f );
		glTranslated( 0.0, -0.39, 0.0 );

		glCallList( idSky );

		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );

		glPopMatrix();

		glDisable( GL_TEXTURE_2D );
	}

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// axis
	if (dbgDrawMode) {
		glLineWidth( 2.0f );
	
		glBegin( GL_LINES );
		for (int i=0; i < 3; i++) {
			glColor3f( i==0?1.0f:0.0f, i==1?1.0f:0.0f, i==2?1.0f:0.0f );
			glVertex3f( 0.0f, 0.0f, 0.0f );
			glVertex3f( i==0?1.0f:0.0f, i==1?1.0f:0.0f, i==2?1.0f:0.0f );
		}
		glEnd();
	}

	// Draw bugzappers
	for (size_t i=0; i < zap.size(); i++) {
		zap[i].draw();
	}

	//draw zapper goals
#if 0
	glColor3f( 1.0f, 0.0f, 1.0f );
	glPointSize( 5.0f );
	glBegin( GL_POINTS );
	for (size_t i=0; i < zap.size(); i++) {
		glVertex3f(	zap[i].m_goalPos.x, 
					zap[i].m_goalPos.y, 
					zap[i].m_goalPos.z );
	}
	glEnd();
#endif

	// draw swarms
	if (!g_drawGlowMode) {
		glGetFloatv( GL_PROJECTION_MATRIX, Swarm::m_proj.m16 );
		for (size_t i=0; i < swarms.size(); i++) {
			swarms[i]->draw();
		}
	}
	
	
	
}

void redraw( void ) {	

	static bool initgl =false;
	if (!initgl) {
		initgl = true;
		glEnable( GL_DEPTH_TEST );
	}

	if ((!g_drawGlowMode)&&(g_gameState==GAMESTATE_GAME)) {
		glClearColor( 0.2f, 0.4f, 0.2f, 1.0f );    
	} else {
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );    
	}
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// 3d draw
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	//glOrtho( 0, 800, 0, 600,-1, 1 );
	//if (!g_drawGlowMode) {
		gluPerspective( 65.0f, 800.0f/600.0f, 0.1, 500.0f );
	//} else {
	//	gluPerspective( 65.0f, 1.0f, 0.1, 500.0f );
	//}
	
	//glTranslated( 0.0f, -4.0f, -9.0f );
	//glRotated( g_viewAngle, 0.0, 1.0, 0.0 );
	vec3f camPos, camLookat;
	camLookat = vec3f( 0.0f, 4.0f, 0.0f );
	camPos = vec3f( 0.0f, 0.0f, 14.0f );
	matrix4x4f camRot;
	camRot.RotateY( g_viewAngle * (M_PI/180.0f));
	camPos *= camRot;
	camPos += camLookat;
	gluLookAt( camPos.x, camPos.y, camPos.z,
			   camLookat.x, camLookat.y, camLookat.z,
			   0.0f, 1.0f, 0.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	if (g_gameState==GAMESTATE_GAME) {
		draw3d();
	}

	
	// 2d draw
	
	glDisable( GL_CULL_FACE );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho( 0, 800, 0, 600,-1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );
	
	if (g_gameState==GAMESTATE_GAME) {

		if (!g_drawGlowMode) {

#if 1
			glColor3f( 1.0f, 1.0f, 1.0f );
			glBindTexture( GL_TEXTURE_2D, idTexGlow );
			glEnable( GL_TEXTURE_2D );
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE );

			// draw the glow overlay
			glBegin( GL_QUADS );
			glTexCoord2d( 0.0, 0.0 );
			glVertex3f( 0, 0, 0.0f );			

			glTexCoord2d( 1.0, 0.0 );
			glVertex3f( 800.0f, 0, 0.0f );

			glTexCoord2d( 1.0, 1.0 );
			glVertex3f( 800.0f, 600.0f, 0.0f );

			glTexCoord2d( 0.0, 1.0 );
			glVertex3f( 0, 600.0f, 0.0f );
			glEnd();
			
			glDisable( GL_BLEND );
			glDisable( GL_TEXTURE_2D );
#endif
			draw2d();
		}
	} else {
		draw2d_frontend();
	}	

	
	if (g_drawGlowMode) {
		glFlush();
		glBindTexture( GL_TEXTURE_2D, idTexGlow );		
		glReadBuffer( GL_BACK );
		glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 0,0, GLOW_RES, GLOW_RES, 0 );
		
	} else {	
		SDL_GL_SwapBuffers();	
	}
}

// physics update
void update( float dt ) {	

	// did they pass the level
	if (g_score >= g_goalBugs) {
		g_level++;
		g_gameState = GAMESTATE_PASSLEVEL;
	}

	// update camera
	g_viewAngle += 30.0f*g_viewAngleMove*dt;

	// Update buggy motion	
	for (size_t i=0; i < swarms.size(); i++) {
		swarms[i]->update( dt );
	}

	

	// Update pendulum swing
	//vec3f m_swingAng, m_swingVal;
	//static float s_fSwingTime;
	Zapper::s_fSwingTime += dt;
	for (size_t zi=0; zi < zap.size(); zi++) {
		Zapper &z = zap[zi];
		
		// update the zapper ai
		z.update_sim( dt );

		// update the swinging		
		z.m_swingAng.x = sin( z.m_swingVal.x * z.s_fSwingTime );
		z.m_swingAng.y = sin( z.m_swingVal.y * z.s_fSwingTime );
		z.m_swingAng.z = sin( z.m_swingVal.z * z.s_fSwingTime );
	}
	
	// Check for bug/zapper collision	
	for (size_t i=0; i < swarms.size(); i++) {
		swarms[i]->checkCollision();
	}

	

}

// swarm forces--------------




int main(int argc, char* argv[])
{
	if ( SDL_Init(SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE | SDL_INIT_AUDIO|SDL_INIT_VIDEO ) < 0 ) {	
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    if( SDL_SetVideoMode( 800, 600, 16, SDL_OPENGL /*| SDL_FULLSCREEN */ ) == 0 ) {
	//if( SDL_SetVideoMode( 800, 600, 16, SDL_OPENGL| SDL_FULLSCREEN  ) == 0 ) {
	fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }
    
    SDL_WM_SetCaption( "Bugzapper", NULL );

	// initialize DevIL
    ilInit();
    ilutRenderer( ILUT_OPENGL );

	// Load the font image
	ilGenImages( 1, &ilFontId );
	ilBindImage( ilFontId );		
	
	if (!ilLoadImage( ILstring("gamedata/lucidia-console.png") )) {
		printf("Loading font image failed\n");
	}
	
	// Make a GL texture for it
	glFontTexId = ilutGLBindTexImage();

	// Create a font by passing in an opengl texture id
	fntFontId = gfCreateFont( glFontTexId );

	// load font metrics, note: has 12, 20 and 72 pts	
	gfLoadFontMetrics( fntFontId, "gamedata/lucidia-console.finfo");

	printf("font has %d chars\n", gfGetFontMetric( fntFontId, GF_FONT_NUMCHARS ) );
	gfEnableFont( fntFontId, 20 );	

	glViewport( 0, 0, 800, 600 );	

	SDL_Event event;
    while (1) {		
		while( SDL_PollEvent( &event ) ) {	    
			switch( event.type ) {

			case SDL_QUIT:			    
				do_quit();
				break;			

			case SDL_KEYDOWN:
				switch( event.key.keysym.sym ) {
					case SDLK_ESCAPE:
						do_quit();
						break;
#if 0
					case SDLK_1:
						zap[0].toggle();
						break;
					case SDLK_2:						
						zap[1].toggle();
						break;
					case SDLK_3:						
						zap[2].toggle();
						break;
					case SDLK_4:						
						zap[3].toggle();
						break;					
#endif

					// DBG STUFF
#ifndef NDEBUG
					case SDLK_TAB:
						dbgDrawMode = !dbgDrawMode;
						break;

					case SDLK_z:					
						dbgDrawNorms = !dbgDrawNorms;
						break;		
					case SDLK_x:
						// cheat: pass level
						g_score = g_goalBugs;
						break;
#endif

					case SDLK_SPACE:
						if (g_gameState==GAMESTATE_MENU) {
							g_gameState = GAMESTATE_PASSLEVEL;
						} else if (g_gameState==GAMESTATE_PASSLEVEL) {
							initGame();
							g_gameState = GAMESTATE_GAME;
						} else if (g_gameState==GAMESTATE_GAMEOVER) {
							g_gameState = GAMESTATE_MENU;
						}
						break;			
#if 0
					case SDLK_LEFT:
						g_viewAngle -= 10.0;
						break;

					case SDLK_RIGHT:
						g_viewAngle += 10.0f;
						break;
#endif

				}
				break;
			}
		}

		Uint8 *keys;
		keys = SDL_GetKeyState(NULL);
		g_viewAngleMove = 0.0f;
		if (keys[SDLK_LEFT] && !keys[SDLK_RIGHT]) {
			g_viewAngleMove = -1.0f;
		}
		if (keys[SDLK_RIGHT] && !keys[SDLK_LEFT]) {
			g_viewAngleMove = 1.0f;
		}

		for (int z=0; z < 4; z++) {
			if (keys[SDLK_1+z]) {
				zap[z].m_charged = true;
			} else {
				zap[z].m_charged = false;
			}
		}
		

		// draw screen ---------------------------------------
		g_drawGlowMode = true;
		redraw();		

		g_drawGlowMode = false;
		redraw();		

		// timing update --------------------------------------		
		static Uint32 last_ticks = 0;
		Uint32 ticks = SDL_GetTicks();		
		static float time_left = 0.0f;

		if (last_ticks>0) {			
			time_left += (ticks - last_ticks)/1000.0f;
		}
		last_ticks = ticks;		

		// Update 
		while (time_left > UPDATE_TIME) {
			time_left -= UPDATE_TIME;
			if (g_gameState==GAMESTATE_GAME) {
				update( UPDATE_TIME );
			}
		}

		// update swarm forces
		for (size_t i=0; i < swarms.size(); i++) {
			swarms[i]->update_swarm_forces();
		}
	
    }


	return 0;
}

