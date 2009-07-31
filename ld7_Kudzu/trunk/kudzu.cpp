#ifdef WIN32
#   include <windows.h>
#endif

#include <math.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <prmath/prmath.hpp>

#include "kudzu.h"
#include "klevel.h"

// globals
#define SIM_FRAME_TIME (33)  // every 33ms, for 30 fps sim loop
#define SIM_STEP ((float)SIM_FRAME_TIME / 1000.0f)

#define ZOOM_AMT (0.95)
#define BUNCH_SPC (0.8)
#define MIN_VEL (0.0001f)

// camera
struct Camera {
	vec3f pos, lookat;
	float heading, tilt, dist;	

	Camera() {
		heading = 0.0f;
		tilt = 20.0f;
		dist = 20.0f;
		recalc();
	}

	void recalc();
};
Camera g_cam;

// Debug flags
struct DebugFlags {
	bool drawDbgView;
	bool drawGameView;
	bool drawSimView;
	int dbgTriNdx;
	vec2f dbgBaryCursor;

	// mouse position on sim map
	float mx, my;

	// defaults
	DebugFlags() {
		drawDbgView = true;
		drawGameView = true;
		drawSimView = false;
		dbgTriNdx = 0;
		dbgBaryCursor = vec2f( 0.5, 0.5 );
		
	}
};
DebugFlags g_dbg;

// Game Stuff
struct KudzuGame {
	KLevel *currLevel;

	// aiming stuff
	// index of kudzu node currently active
	int activeNode;
	float aimdir; // angle	

	bool isUpdating;
	bool isCharging;
	float charge_amt;


	// controls
	float cfAimDir;

	KudzuGame() {
		currLevel = NULL;
		reset();
	}

	void reset() {
		if (currLevel) {
			delete currLevel;
		}
		currLevel = NULL;
		activeNode = 0;
		aimdir = 0.0f;		
		isCharging =false;
		charge_amt=0.0f;
	}

	KNode &GetActNode() { return currLevel->m_nodes[ activeNode ]; }
	
};
KudzuGame g_game;

void fire_shot();

//-----------------------------------------------------------------------
void Camera::recalc()
{
	vec3f cam;
	matrix4x4f mat, m2;

	cam = vec3f( 0.0f, 0.0f, dist );
	mat.RotateY( (float)(D2R*heading) );

	cam *= mat;
	printf("Cam %f %f %f\n", cam.x, cam.y, cam.z );
	pos = lookat + cam;
	pos.y = 10.0;	
}

// clean up and quit
void do_quit()
{	
	SDL_Quit();
	exit(0);
}

// compute 1/30th of a second
void update_sim()
{
	//printf("Sim Update\n" );
	if (!g_game.currLevel) {
		// no level loaded, don't update
		return;
	}

	// move aim thinggy based on cfHeading
	g_game.aimdir += g_game.cfAimDir * 10.0f;
	if (g_game.aimdir < 0.0f) g_game.aimdir += 360.0f;
	if (g_game.aimdir > 360.0f) g_game.aimdir -= 360.0f;	

	// update power charge
	if (g_game.isCharging) {
		g_game.charge_amt += SIM_STEP;
		if (g_game.charge_amt > 2.0f) {
			g_game.charge_amt = 2.0f;
			fire_shot();
			g_game.isCharging = false;
		}
	}

	// update any moving nodes
	bool needCleanup = false;
	g_game.isUpdating = false;
	for (size_t ndx=0; ndx < g_game.currLevel->m_nodes.size(); ndx++) {
			KNode &n = g_game.currLevel->m_nodes[ndx];
			if (n.v > MIN_VEL) {

				// flag that something is being updated
				g_game.isUpdating = true;

				// update in kudzu space (bad)
				//n.pos += (n.dir * n.v);
				vec3f pos2;
				vec3f pos, dir, nrm, nrm2, bary,b2;
				KSimTri tri;
				if (!g_game.currLevel->getSimInfo( n.pos, n.dir, 
						pos, nrm, dir, tri, bary ) ) {
					// node went out of bounds, mark it for
					// removal					
					needCleanup = true;
					n.erase_flag = true;
				} else {					
					n.erase_flag = false;

					// update
					vec3f oldpos = pos;
					pos	= pos +	dir	* (n.v * SIM_STEP);

					// Add a bunch every BUNCH_SPC					
					float d = prmath::Length( pos - oldpos );
					n.bdist += d;
					if (n.bdist > BUNCH_SPC) {
						n.bdist -= BUNCH_SPC;
						KBunch b;
						b.pos = n.pos;
						b.pos3 = oldpos;
						b.age = 0.0f;
						g_game.currLevel->m_bunches.push_back( b );						
					}

					// make pos2 a little further for boundry crossing thing
					pos2 = pos + dir * (n.v	* (SIM_STEP*5)); 

					// remember	pos3
					n.pos3 = pos;

					// convert pos back	to uv space
					b2 = tri.pos2bary( pos );				
					n.pos =	tri.bary2uv( b2	);

					// apply drag
					n.v	-= 0.02;

					// if new pos is outside of	triangle, 
					// update dir so it	will still appear straight
					if ((b2.x <	0) || (b2.y	< 0) ||	(b2.z <	0))	{
						// b2 went outside of tri, get the new tri
						g_game.currLevel->getSimInfo( n.pos, n.dir,	
							pos, nrm2, dir,	tri, bary );

						// find	what uv's the new tri would	give for the old position					
						b2 = tri.pos2bary( pos2	);
						vec2f uv2 =	tri.bary2uv( b2	);

						// get new heading from	uv2	-> uv, if triangles	
						// are near	flat
						float olddir = n.dir;
						float ang =	acos( prmath::DotProduct( nrm, nrm2	));
						printf("Angle: %f\n", R2D*ang );
						if (ang	< D2R*45 ) {
							n.dir =	R2D	* atan2( uv2.y - n.pos.y, uv2.x	- n.pos.x  );					
							printf("DBG: Adjusting:	Old	dir	was	%f new dir %f\n", olddir, n.dir	);
						} else {
							// give	the	angle a	random nudge to	hide the
							// distortion in mapping
							n.dir += (((float)rand() / (float)RAND_MAX)- 0.5f) * 45.0;
							printf("DBG: Jittering:	Old	dir	was	%f new dir %f\n", olddir, n.dir	);
						}
					}					
					
					// kill anything that runs into a bunch
					for (int i=0; i < g_game.currLevel->m_bunches.size(); i++) {

						// ignore freshly planted bunches
						if (g_game.currLevel->m_bunches[i].age < 2.0f) continue;

						float d = prmath::Length(n.pos3 - g_game.currLevel->m_bunches[i].pos3);					
						if (d < BUNCH_SPC* 0.5) {
							printf("KILL node %d Dist %f %f\n", 
								ndx, d, BUNCH_SPC*0.25 );
							n.erase_flag = true;
							needCleanup = true;
							break;
						}
					}					

					
			} // end of update
		} else {
			// node is stationary
			if (n.explode_flag) {
				needCleanup = true;
				n.erase_flag = true;
			}
		}

	} // end of node loop


	// safety: kill any bunches next to a not moving node
	std::vector<KBunch> newbunch;
	for (int i=0; i < g_game.currLevel->m_bunches.size(); i++) {
		KBunch &b = g_game.currLevel->m_bunches[i];
		bool touching = false;
		for (int j=0; j < g_game.currLevel->m_nodes.size(); j++) {
			KNode &n = g_game.currLevel->m_nodes[j];
			// skip moving nodes
			if (n.v > MIN_VEL) continue;

			if (prmath::Length( b.pos3 - n.pos3 ) < BUNCH_SPC * 0.55 ) {
				touching = true;
				break;
			}
		}

		if (!touching) {
			newbunch.push_back( b );
		}
	}
	g_game.currLevel->m_bunches = newbunch;

	// age all of the bunches
	for (int i=0; i < g_game.currLevel->m_bunches.size(); i++) {
		g_game.currLevel->m_bunches[i].age += SIM_STEP;
	}

	// rebuild node list if there were baddies
	if (needCleanup) {

		// reset active node to 0
		g_game.activeNode = 0;

		// Remove any nodes with the erase_flag set
		std::vector<KNode> newlist;
		for (int i=0; i < g_game.currLevel->m_nodes.size(); i++) {
			KNode &n = g_game.currLevel->m_nodes[i];
			if (!n.erase_flag) {
				newlist.push_back( n );
			}
		}
		g_game.currLevel->m_nodes = newlist;

	}
	
}

void redraw_dbg_view_3d(){

	// draw an axis at the origin
	vec3f axis[3];
	axis[0] = vec3f( 1.0f, 0.0f, 0.0f );
	axis[1] = vec3f( 0.0f, 1.0f, 0.0f );
	axis[2] = vec3f( 0.0f, 0.0f, 1.0f );
	
	glBegin( GL_LINES );
	for (int andx = 0; andx < 3; andx++) {
		glColor3f( axis[andx].x, axis[andx].y, axis[andx].z );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( axis[andx].x, axis[andx].y, axis[andx].z );
	}
	glEnd();

	// Draw the sim mesh for the current level if there is one
	if (g_game.currLevel) {
		vec3f edgeColor, badEdgeColor;
		edgeColor = vec3f( 0.8f, 0.8f, 0.8f );
		badEdgeColor = vec3f( 1.0f, 0.0f, 0.0f );
		for (size_t ndx=0; ndx < g_game.currLevel->m_tri.size(); ndx++) {
			KSimTri tri = g_game.currLevel->m_tri[ndx];
			glBegin( GL_LINES );

			// Edge AB
			if( tri.ablink >=0 ) {
				glColor3f( edgeColor.x, edgeColor.y, edgeColor.z );
			} else {
				glColor3f( badEdgeColor.x, badEdgeColor.y, badEdgeColor.z );
			}
			glVertex3f( tri.a.x, tri.a.y, tri.a.z );
			glVertex3f( tri.b.x, tri.b.y, tri.b.z );

			// Edge BC
			if( tri.bclink >=0 ) {
				glColor3f( edgeColor.x, edgeColor.y, edgeColor.z );
			} else {
				glColor3f( badEdgeColor.x, badEdgeColor.y, badEdgeColor.z );
			}			
			glVertex3f( tri.b.x, tri.b.y, tri.b.z );
			glVertex3f( tri.c.x, tri.c.y, tri.c.z );

			// Edge CA
			if( tri.calink >=0 ) {
				glColor3f( edgeColor.x, edgeColor.y, edgeColor.z );
			} else {
				glColor3f( badEdgeColor.x, badEdgeColor.y, badEdgeColor.z );
			}
			glVertex3f( tri.c.x, tri.c.y, tri.c.z );
			glVertex3f( tri.a.x, tri.a.y, tri.a.z );
			
			
			glEnd();
		}

		// Draw kudzu nodes
		glColor3f( 0.6f, 1.0f, 0.6f );
		glPointSize( 8.0 );
		glBegin( GL_POINTS );
		for (size_t ndx=0; ndx < g_game.currLevel->m_nodes.size(); ndx++) {
			KNode n = g_game.currLevel->m_nodes[ndx];

			//vec3f p3d = g_game.currLevel->k2pos( n.pos );
			vec3f p3d = n.pos3;
			glVertex3f(p3d.x, p3d.y, p3d.z);				
			
		}
		glEnd();

		// draw "active" firing cursor
		KNode n = g_game.currLevel->m_nodes[ g_game.activeNode ];
		vec3f p3d, nrm, dir, bary;
		KSimTri tri;

		// get firing direction in uv space
		g_game.currLevel->getSimInfo( n.pos, g_game.aimdir,
									  p3d, nrm, dir, tri, bary );

		glColor3f( 0.0f, 1.0f, 0.0f );
		glBegin( GL_LINES );
		glVertex3f( p3d.x, p3d.y, p3d.z );
		p3d += dir;
		glVertex3f( p3d.x, p3d.y, p3d.z );
		glEnd();


#if 0
		// Draw kudzu vines
		glColor3f( 0.0f, 0.4f, 0.0f );
		glLineWidth( 2.0 );
		glBegin( GL_LINES );
		for (size_t ndx=0; ndx < g_game.currLevel->m_nodes.size(); ndx++) {
			KNode &n = g_game.currLevel->m_nodes[ndx];
			for (size_t ndx2=0; ndx2 < n.links.size(); ndx2++) {
				KNode &n2 = g_game.currLevel->m_nodes[ n.links[ndx2] ];

				if ((n.pos3.x > -20.0) && (n.pos3.x < 20.0) &&
					(n2.pos3.x > -20.0) && (n2.pos3.x < 20.0) ) {
					
					glVertex3f( n.pos3.x, n.pos3.y, n.pos3.z );
					glVertex3f( n2.pos3.x, n2.pos3.y, n2.pos3.z );

				}
			}
		}
		glEnd();
		glLineWidth( 1.0 );
#endif

		// draw dbg "cursor"		
		glColor3f( 1.0f, 1.0f, 0.0f );
		glPointSize( 5.0f );
		glBegin( GL_POINTS );
		p3d = g_game.currLevel->k2pos( g_dbg.dbgBaryCursor );
		glVertex3f(p3d.x, p3d.y, p3d.z);
		glEnd();		

		// Draw kudzu bunches
		glColor3f( 0.3f, 0.9f, 0.3f );
		glPointSize( 3.0 );
		glBegin( GL_POINTS );
		for (size_t ndx=0; ndx < g_game.currLevel->m_bunches.size(); ndx++) {
			KBunch b = g_game.currLevel->m_bunches[ndx];

			//vec3f p3d = g_game.currLevel->k2pos( n.pos );
			vec3f p3d = b.pos3;
			glVertex3f(p3d.x, p3d.y, p3d.z);				
			
		}
		glEnd();

	} // if currLevel
}

void redraw_dbg_view_2d() {
}

void redraw_game_view_3d() {

}


// map to screen for sim view
float sx( float x ) { return 110+(x*580.0f); }
float sy( float y ) { return 10+(y*580.0f); }
vec2f sp( vec2f p ) {
	vec2f sp;
	sp.x = sx( p.x );
	sp.y = sy( p.y );
	return sp;
}

void redraw_game_view_2d() {

	float power;

	if (g_game.isCharging) {
		if (g_game.charge_amt <= 1.0) {
			power = g_game.charge_amt;
		} else {
			power = 1.0f - (g_game.charge_amt-1.0);
		}

		// power bar
		float p = 100.0f + power*600.0f;
		glColor3f( 1.0f, 0.0f, 1.0f );
		glBegin( GL_QUADS );
		glVertex2d( 100.0f, 20.0f );
		glVertex2d( p, 20.0f );
		glVertex2d( p, 80.0f );
		glVertex2d( 100.0f, 80.0f );
		glEnd();
	}

}


void redraw_sim_view_2d() {

	// background
	glColor4f( 0.5f, 0.4f, 0.3f, 0.8f );
	glBegin( GL_QUADS );
	glVertex2d( sx( 0.0f ), sy( 0.0f ) );
	glVertex2d( sx( 1.0f ), sy( 0.0f ) );
	glVertex2d( sx( 1.0f ), sy( 1.0f ) );
	glVertex2d( sx( 0.0f ), sy( 1.0f ) );
	glEnd();

	// Draw the sim mesh for the current level if there is one
	if (g_game.currLevel) {
		vec3f edgeColor, badEdgeColor;

		for (size_t ndx=0; ndx < g_game.currLevel->m_tri.size(); ndx++) {
			KSimTri tri = g_game.currLevel->m_tri[ndx];

			// highlight containing triangles
			bool inside = false;
			if ((g_dbg.mx >= 0.0) && (g_dbg.mx <= 1.0) &&
				(g_dbg.my >= 0.0) && (g_dbg.my <= 1.0) ) {

				if (pointInTri( vec2f( g_dbg.mx, g_dbg.my), tri )) {
					inside = true;
				}
			}

			if (inside) {
				edgeColor = vec3f( 1.0f, 1.0f, 1.0f );
				badEdgeColor = vec3f( 1.0f, 0.5f, 0.5f );
				glLineWidth( 3.0f );
			} else {
				edgeColor = vec3f( 0.8f, 0.8f, 0.8f );
				badEdgeColor = vec3f( 1.0f, 0.0f, 0.0f );
				glLineWidth( 1.0f );
			}


			glBegin( GL_LINES );

			// Edge AB
			if( tri.ablink >=0 ) {
				glColor3f( edgeColor.x, edgeColor.y, edgeColor.z );
			} else {
				glColor3f( badEdgeColor.x, badEdgeColor.y, badEdgeColor.z );
			}
			glVertex2f( sx(tri.ka.x), sy(tri.ka.y) );
			glVertex2f( sx(tri.kb.x), sy(tri.kb.y) );

			// Edge BC
			if( tri.bclink  >=0) {
				glColor3f( edgeColor.x, edgeColor.y, edgeColor.z );
			} else {
				glColor3f( badEdgeColor.x, badEdgeColor.y, badEdgeColor.z );
			}			
			glVertex2f( sx(tri.kb.x), sy(tri.kb.y) );
			glVertex2f( sx(tri.kc.x), sy(tri.kc.y) );

			// Edge CA
			if( tri.calink >=0 ) {
				glColor3f( edgeColor.x, edgeColor.y, edgeColor.z );
			} else {
				glColor3f( badEdgeColor.x, badEdgeColor.y, badEdgeColor.z );
			}
			glVertex2f( sx(tri.kc.x), sy(tri.kc.y) );
			glVertex2f( sx(tri.ka.x), sy(tri.ka.y) );
						
			glEnd();

			// drag triangle links
			if (ndx == g_dbg.dbgTriNdx) {
				KSimTri t2;
				float cx, cy;								
				glBegin( GL_LINES);

				// EDGE AB				
				cx = (tri.ka.x + tri.kb.x) / 2.0;
				cy = (tri.ka.y + tri.kb.y) / 2.0;
				if (tri.ablink >= 0) {						
					glColor3f( 0.0f, 1.0f, 0.0f );
					glVertex2f( sx(cx), sy(cy) );
					t2 = g_game.currLevel->m_tri[tri.ablink];
					glVertex2f( sx( (t2.ka.x + t2.kb.x + t2.kc.x) / 3.0f ), 
								sy( (t2.ka.y + t2.kb.y + t2.kc.y) / 3.0f ) );
				} else {
					glColor3f( 1.0f, 0.0f, 0.0f );
					glVertex2f( sx(cx)+5, sy(cy)+5 );
					glVertex2f( sx(cx)-5, sy(cy)-5 );
					glVertex2f( sx(cx)+5, sy(cy)-5 );
					glVertex2f( sx(cx)-5, sy(cy)+5 );
				}

				// EDGE BC				
				cx = (tri.kc.x + tri.kb.x) / 2.0;
				cy = (tri.kc.y + tri.kb.y) / 2.0;
				if (tri.bclink >= 0) {						
					glColor3f( 0.0f, 1.0f, 0.0f );
					glVertex2f( sx(cx), sy(cy) );
					t2 = g_game.currLevel->m_tri[tri.bclink];
					glVertex2f( sx( (t2.ka.x + t2.kb.x + t2.kc.x) / 3.0f ), 
								sy( (t2.ka.y + t2.kb.y + t2.kc.y) / 3.0f ) );
				} else {
					glColor3f( 1.0f, 0.0f, 0.0f );
					glVertex2f( sx(cx)+5, sy(cy)+5 );
					glVertex2f( sx(cx)-5, sy(cy)-5 );
					glVertex2f( sx(cx)+5, sy(cy)-5 );
					glVertex2f( sx(cx)-5, sy(cy)+5 );
				}

				// EDGE CA			
				cx = (tri.kc.x + tri.ka.x) / 2.0;
				cy = (tri.kc.y + tri.ka.y) / 2.0;
				if (tri.calink >= 0) {						
					glColor3f( 0.0f, 1.0f, 0.0f );
					glVertex2f( sx(cx), sy(cy) );
					t2 = g_game.currLevel->m_tri[tri.calink];
					glVertex2f( sx( (t2.ka.x + t2.kb.x + t2.kc.x) / 3.0f ), 
								sy( (t2.ka.y + t2.kb.y + t2.kc.y) / 3.0f ) );
				} else {
					glColor3f( 1.0f, 0.0f, 0.0f );
					glVertex2f( sx(cx)+5, sy(cy)+5 );
					glVertex2f( sx(cx)-5, sy(cy)-5 );
					glVertex2f( sx(cx)+5, sy(cy)-5 );
					glVertex2f( sx(cx)-5, sy(cy)+5 );
				}

				

				glEnd();
			}
		}

#if 0
		// Draw kudzu vines
		glColor3f( 0.0f, 0.4f, 0.0f );
		glLineWidth( 2.0 );
		glBegin( GL_LINES );
		for (size_t ndx=0; ndx < g_game.currLevel->m_nodes.size(); ndx++) {
			KNode &n = g_game.currLevel->m_nodes[ndx];
			for (size_t ndx2=0; ndx2 < n.links.size(); ndx2++) {
				KNode &n2 = g_game.currLevel->m_nodes[ n.links[ndx2] ];				
					
					glVertex2f( sx(n.pos.x), sy(n.pos.y) );
					glVertex2f( sx(n2.pos.x), sy(n2.pos.y) );
				
			}
		}
		glEnd();
		glLineWidth( 1.0 );
#endif

		// Draw kudzu nodes
		for (size_t ndx=0; ndx < g_game.currLevel->m_nodes.size(); ndx++) {
			KNode n = g_game.currLevel->m_nodes[ndx];

			glColor3f( 0.6f, 1.0f, 0.6f );
			glBegin( GL_LINES );
			for (float a=0.0f; a < M_PI*2.0f; a += M_PI/8.0f) {
				glVertex2f( sx(n.pos.x), sy(n.pos.y) );
				glVertex2f( sx(n.pos.x + cos(a)*0.01), sy(n.pos.y + sin(a)*0.01) );
			}
			glEnd();
		}

		// draw mouse pos thinggy
		if ((g_dbg.mx >= 0.0) && (g_dbg.mx <= 1.0) &&
			(g_dbg.my >= 0.0) && (g_dbg.my <= 1.0) ) {

			glColor3f( 1.0f, 0.0f, 1.0f );
			glPointSize( 5.0f );
			glBegin( GL_POINTS );
			glVertex2f( sx(g_dbg.mx), sy(g_dbg.my) );
			glEnd();
		}

		// bary debug marker
		glColor3f( 1.0f, 1.0f, 0.0f );
		glPointSize( 5.0f );
		glBegin( GL_POINTS );
		vec3f p3d = g_game.currLevel->k2pos( g_dbg.dbgBaryCursor );
		glVertex2f( sx( g_dbg.dbgBaryCursor.x ), sy( g_dbg.dbgBaryCursor.y ) );
		glEnd();		

	} // if currLevel

}

void redraw_scene_3d()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, 800.0/600.0, 1.0, 1000.0 );
	
	gluLookAt( g_cam.pos.x, g_cam.pos.y, g_cam.pos.z,
				g_cam.lookat.x, g_cam.lookat.y, g_cam.lookat.z,
				0.0f, 1.0f, 0.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	if (g_dbg.drawDbgView) {
		redraw_dbg_view_3d();
	}

	if (g_dbg.drawGameView) {
		redraw_game_view_3d();
	}
}

void redraw_scene_2d()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	gluOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();	

	if (g_dbg.drawGameView) {
		redraw_game_view_2d();
	}

	if (g_dbg.drawDbgView) {
		redraw_dbg_view_2d();
	}

	if (g_dbg.drawSimView) {
		redraw_sim_view_2d();
	}
}

void redraw() {
	static bool initgfx = false;

	glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if (!initgfx) {
		initgfx = true;
	}

	// draw the 3d part
	redraw_scene_3d();

	// draw the 2d part
	redraw_scene_2d();

	SDL_GL_SwapBuffers();
}

void fire_shot()
{	
	float power;

	if (g_game.charge_amt <= 1.0) {
		power = g_game.charge_amt;
	} else {
		power = 1.0f - (g_game.charge_amt-1.0);
	}

	KNode n;
	n.pos = g_game.GetActNode().pos;						
	n.dir = g_game.aimdir;
	n.v = 1.0f + 3.0f * power;
	n.explode_flag = false;
	n.bdist = 0.0f;
				
	g_game.currLevel->m_nodes.push_back( n );	

	g_game.isCharging  = false;
	g_game.charge_amt = 0.0f;
}

// main loop
int main( int argc, char *argv[] )
{
		if ( SDL_Init(SDL_INIT_NOPARACHUTE |SDL_INIT_VIDEO |SDL_INIT_JOYSTICK) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

	if( SDL_SetVideoMode( 800, 600, 16, SDL_OPENGL /*| SDL_FULLSCREEN */ ) == 0 ) {
		fprintf(stderr,	"Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
	}

	SDL_WM_SetCaption( "Kudzu Commander", NULL );


	while (1) {		
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {
		
        switch( event.type ) {
        case SDL_KEYDOWN:
            switch( event.key.keysym.sym ) {
				case SDLK_ESCAPE:
					do_quit();
					break;				

				case SDLK_SPACE:
					if (!g_game.isUpdating) {
						g_game.isCharging = true;
						g_game.charge_amt = 0.0;
					}
					break;

				case SDLK_RETURN:
					// EXPLODE node
					// don't fire a shot until current one is done, 
					// and can't explode if there is only one node left
					if ((!g_game.isUpdating) && (g_game.currLevel->m_nodes.size() > 1 )) {
						// TODO: energy system								
						int num_explode = 20;
						for (int i = 0; i < num_explode; i++ ) {
							KNode n;
							n.pos = g_game.GetActNode().pos;						
							n.dir = ((float)i / float(num_explode)) * 360.0f;
							n.v = 2.0;
							n.bdist = ( (float)rand() / (float)RAND_MAX) * BUNCH_SPC;
							n.explode_flag = true;
						
							g_game.currLevel->m_nodes.push_back( n );
						}
						g_game.GetActNode().erase_flag = true;
						g_game.activeNode = 0;
					}
					break;				


				case SDLK_s:
					//ilutGLScreenie();
					break; 

				case SDLK_1:
					g_dbg.drawGameView = !g_dbg.drawGameView;
					printf("Game View is now: %s\n", g_dbg.drawGameView?"enabled":"disabled" );
					break;

				case SDLK_2:
					g_dbg.drawDbgView = !g_dbg.drawDbgView;
					printf("Debug View is now: %s\n", g_dbg.drawDbgView?"enabled":"disabled" );
					break;				

				case SDLK_3:
					g_dbg.drawSimView = !g_dbg.drawSimView;
					printf("Sim View is now: %s\n", g_dbg.drawSimView?"enabled":"disabled" );
					break;

				case SDLK_l: 
					{
					// init level
					printf("Init level\n");
					g_game.currLevel = new KLevel();
					// TODO: load this from a level info file
					g_game.currLevel->loadSimMesh( "./gamedata/house_sim.obj" );
					//g_game.currLevel->loadSimMesh( "./gamedata/grid_sim.obj" );

					// staring node
					KNode n;
					n.pos = vec2f( 0.1f, 0.5f );
					n.pos3 = g_game.currLevel->k2pos( n.pos );
					n.erase_flag = false;
					n.explode_flag = false;
					g_game.currLevel->m_nodes.push_back( n );
					}
					break;				

				case SDLK_q:
					if (g_game.currLevel) {
						g_dbg.dbgTriNdx--;
						if (g_dbg.dbgTriNdx < 0 ) {
							g_dbg.dbgTriNdx = g_game.currLevel->m_tri.size()-1;
						}

						printf("Triangle %d, links ab %d bc %d ca %d\n",
							g_dbg.dbgTriNdx,
							g_game.currLevel->m_tri[g_dbg.dbgTriNdx].ablink,
							g_game.currLevel->m_tri[g_dbg.dbgTriNdx].bclink,
							g_game.currLevel->m_tri[g_dbg.dbgTriNdx].calink );
					}
					break;

				case SDLK_w:
					if (g_game.currLevel) {
						g_dbg.dbgTriNdx++;
						if (g_dbg.dbgTriNdx >= g_game.currLevel->m_tri.size() ) {
							g_dbg.dbgTriNdx = 0;
						}
					}
					break;				

				case SDLK_UP:
					if (g_game.currLevel) {
						g_game.activeNode++;
						if (g_game.activeNode == g_game.currLevel->m_nodes.size()) {
							g_game.activeNode = 0;
						}
					}
					break;

				case SDLK_DOWN:
					if (g_game.currLevel) {
						g_game.activeNode--;
						if (g_game.activeNode < 0) {
							g_game.activeNode = g_game.currLevel->m_nodes.size() - 1;
						}
					}
					break;
				
				default:
					break;
				}
			break;

			case SDL_KEYUP:
				switch( event.key.keysym.sym ) {					
					case SDLK_SPACE:
					if ((!g_game.isUpdating) && (g_game.isCharging)) {
						fire_shot();
					}					

					break;		
				}

			case SDL_MOUSEMOTION:
				if (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(3)) {
					// dragging RMB
					printf("Dragging %d %d\n", 
						event.motion.xrel, event.motion.yrel );
					g_cam.heading += -((float)event.motion.xrel / 800.0f) * 90.0f;
					printf("g_cam.heading %f\n", g_cam.heading );
					if (g_cam.heading < 0.0f) g_cam.heading += 360.0f;
					if (g_cam.heading > 360.0f) g_cam.heading -= 360.0f;
					g_cam.recalc();
				}

				// update mouse pos on sim view
				if ( (g_dbg.drawSimView) || (g_dbg.drawDbgView) ) {
					g_dbg.mx = (float)(event.motion.x - 110) / 580.0;
					g_dbg.my = 1.0f - ((float)(event.motion.y - 10) / 580.0);
				}

				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button==4) {
					// wheel up
					g_cam.dist *= ZOOM_AMT;
					g_cam.recalc();
				} else if (event.button.button==5) {
					// wheel down
					g_cam.dist *= (1.0f / ZOOM_AMT);
					g_cam.recalc();
				} else if (event.button.button==2) {
					if ((g_dbg.mx >= 0.0) && (g_dbg.mx <= 1.0) &&
						(g_dbg.my >= 0.0) && (g_dbg.my <= 1.0) ) {
						g_dbg.dbgBaryCursor = vec2f( g_dbg.mx, g_dbg.my );
					}
				}
				printf("Mouse button %d pressed at (%d,%d)\n",
                       event.button.button, event.button.x, event.button.y);
				break;
		
			case SDL_QUIT:
				/* Handle quit requests (like Ctrl-c). */
				do_quit();
				break;
			}	
		}

		// Continuous (not one shot) keypresses
		g_game.cfAimDir = 0.0f;
		Uint8 *keys;
		keys = SDL_GetKeyState(NULL);
		if (keys[SDLK_LEFT] && !keys[SDLK_RIGHT]) {
			g_game.cfAimDir = 1.0;
		}
		if (!keys[SDLK_LEFT] && keys[SDLK_RIGHT]) {
			g_game.cfAimDir = -1.0;
		}

		// Update game loop	
		static Uint32 last_tick=0, sim_ticks = 0;		
		Uint32 tick;
		tick = SDL_GetTicks();
		Uint32 deltaT;

		// get elapsed time
		if (last_tick==0) {		
			deltaT = 0;
		} else {
			deltaT = tick - last_tick;
		}
		last_tick = tick;

		// update every SIM_FRAME_TIME ticks 
		sim_ticks += deltaT;
		while (sim_ticks > SIM_FRAME_TIME) {
			sim_ticks -= SIM_FRAME_TIME;
			update_sim();
		}

		// Redraw the screen
		redraw();
	}


	return 0;
}
