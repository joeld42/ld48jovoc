#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include <prmath/prmath.hpp>

#include <SDL_mixer.h>

#include "gamefontgl.h"
#include "GameState.h"

#include "ViewSimple2D.h"

#define COMBO_DISPLAY_TIME (3.0)

#define BURN_ADD_TIME (0.3) // how often to add burny bits

// random (0..1)
#define FRAND ((float)rand()/(float)RAND_MAX)

// random (-1..1)
#define SFRAND ((((float)rand()/(float)RAND_MAX)*2.0)-1.0)


ViewSimple2D::ViewSimple2D( GameState *game ) :
	GameView( game )
{

}	

void ViewSimple2D::initGraphics( unsigned int fontId )
{
	GameView::initGraphics( fontId );

	// Load the images for the Pumpkin icons
	ilGenImages( 1, &m_ilPumpkinIcons );
	if (!ilLoadImage( "gamedata/pumpkin-icons.png" )) 
		printf("Loading pumpkin-icons.png image failed\n");
		
	m_texPumpkinIcons = ilutGLBindTexImage();	

	// Load the images for the particles
	ilGenImages( 1, &m_ilParticles );
	if (!ilLoadImage( "gamedata/particles.png" )) 
		printf("Loading particles.png image failed\n");
		
	m_texParticles = ilutGLBindTexImage();	

	// group colors
	m_groupColor.push_back( vec3f( 1.0, 217.0/255.0, 147.0/255.0 ) );
	m_groupColor.push_back( vec3f( 232.0/255.0, 192.0/255.0, 1.0 ) );
	m_groupColor.push_back( vec3f( 0.5, 1.0, 1.0 ) );
	m_groupColor.push_back( vec3f( 1.0, 1.0, 1.0 ) );
	m_groupColor.push_back( vec3f( 1.0, 0.5, 0.5 ) );

	m_burntime = 0.0;
	m_lastComboAge = 100.0;
}

void ViewSimple2D::simulate( float dt )
{
	int i;

	GameView::simulate( dt );

	// update display text
	if (m_lastComboAge < COMBO_DISPLAY_TIME) {
		m_lastComboAge += dt;
	}

	// update animated pumpkins	-- TODO: make this its own func.
	std::vector<PumpkinAnim> keep;
	for (i = 0; i < m_panim.size(); i++) {
		m_panim[i].m_pos += m_panim[i].m_vel * dt;
		m_panim[i].m_vel += vec2f( 0.0, -400.0) * dt; // gravity

		// remove offscreen panim
		if ( (m_panim[i].m_pos[0] < -100) ||
			 (m_panim[i].m_pos[0] > 900) ||
			 (m_panim[i].m_pos[1] < -100) ||
			 (m_panim[i].m_pos[1] > 800) ) {
			// don't keep
		} else {
			keep.push_back( m_panim[i] );
		}
	}

	// Add some burny bits
	m_burntime += dt;
	if (m_burntime > BURN_ADD_TIME) {
		m_burntime -= BURN_ADD_TIME;
		for (int i=0; i < MACHINE_SLOTS; i++) {
			if (m_game->getPumpkin(i).m_burning) {
				emitParts( 4, calcPosFromNdx( i ), PART_FIRE );
			}
		}
	}

	// update particles
	updateParts( dt );

	m_panim = keep;
}

void ViewSimple2D::drawCircle( vec2f ctr, float rad, 
								int mode,
								vec4f clr_fill, vec4f clr_line, int steps )
{
	float t;
	glPushMatrix();
	
	glTranslated( ctr[0], ctr[1], 0.0);

	// draw solid part
	if ((mode==SHAPE_FILL)||(mode==SHAPE_LINE_AND_FILL)) {		

		glColor4f( clr_fill[0], clr_fill[1],  clr_fill[2],  clr_fill[3] );
		glBegin( GL_POLYGON );
		
		for (int i = 0; i < steps; i++) {
			t = ((2.0*M_PI)/steps) * i;
			glVertex3f( cos(t) * rad, sin(t)*rad, 0.0 );
		}
		glEnd();
		
	}


	// draw outline part
	if ((mode==SHAPE_LINE)||(mode==SHAPE_LINE_AND_FILL)) {

		glColor4f( clr_line[0], clr_line[1],  clr_line[2],  clr_line[3] );
		glBegin( GL_LINE_LOOP );
		
		for (int i = 0; i < steps; i++) {
			t = ((2.0*M_PI)/steps) * i;
			glVertex3f( cos(t) * rad, sin(t)*rad, 0.0 );
		}
		glEnd();
		
	}


	glPopMatrix();
}

void ViewSimple2D::drawPumpkin( vec2f pos, const Pumpkin &p, float sz, bool doColor )
{
	float s1, s2, t1, t2, x=pos[0], y=pos[1];
	int ndx = p.m_type-1;

	s1 = (ndx % 8) * 1.0/8.0;		
	s2 = s1 + 1.0/8.0;

	t2 = (ndx / 8) * 0.5;
	t1 = t2 - 0.5;

	if (doColor) {
		if (p.m_burning) {
			float b = p.m_burnval / (float)PUMPKIN_BURN_RATE;			
			glColor3f( b, b*0.5, 0 );
		} else {
			glColor3f( 1.0, 1.0, 1.0 );
		}
	}

	glBegin( GL_QUADS );
	glTexCoord2d( s1, t1 );   glVertex3f( x,y, 0.0 );
	glTexCoord2d( s1, t2 );   glVertex3f( x, y+sz, 0.0 );
	glTexCoord2d( s2, t2 );  glVertex3f( x+sz,y+sz, 0.0 );
	glTexCoord2d( s2, t1 );  glVertex3f( x+sz, y, 0.0 );
	glEnd();
}

void ViewSimple2D::drawRect( vec2f min, vec2f max, int mode, vec4f clr_fill, vec4f clr_line )
{
	// draw solid part
	if ((mode==SHAPE_FILL)||(mode==SHAPE_LINE_AND_FILL)) {		

		glColor4f( clr_fill[0], clr_fill[1],  clr_fill[2],  clr_fill[3] );
		glBegin( GL_QUADS );
		
		glVertex3f( min[0], min[1], 0.0 );
		glVertex3f( min[0], max[1], 0.0 );
		glVertex3f( max[0], max[1], 0.0 );
		glVertex3f( max[0], min[1], 0.0 );
		
		glEnd();
		
	}

	// draw outline part
	if ((mode==SHAPE_LINE)||(mode==SHAPE_LINE_AND_FILL)) {

		glColor4f( clr_line[0], clr_line[1],  clr_line[2],  clr_line[3] );
		glBegin( GL_LINE_LOOP );
		
		glVertex3f( min[0], min[1], 0.0 );
		glVertex3f( min[0], max[1], 0.0 );
		glVertex3f( max[0], max[1], 0.0 );
		glVertex3f( max[0], min[1], 0.0 );

		glEnd();		
	}

}

vec2f ViewSimple2D::calcPosFromNdx( int ndx )
{
	vec2f ctr;	
	ctr = vec2f( 300, 300 );
	float ii,t;

	if ((m_anim)&&(!m_game->isGameOver()) ) {
		ii = (float)(ndx) - 0.5f + m_turn;
	} else {
		ii = (float)(ndx);
	}
	t = (2.0*M_PI)/MACHINE_SLOTS * ii;		
		
	return vec2f( ctr[0] + cos(t)*250.0,
			      ctr[1] - sin(t)*250.0 ); 
}

void ViewSimple2D::notifyAboutCombo( Combo &c, std::vector<int> ndxlist )
{
	if (c.m_secret) {
		playSound( SFX_SECRET );
	} else {
		playSound( SFX_HARP );
	}

	for (int i = 0; i < ndxlist.size(); i++) {
		PumpkinAnim pa;
		pa = m_game->getPumpkin( ndxlist[i] );
		pa.m_pos = calcPosFromNdx( ndxlist[i] );
		pa.m_vel = (-0.5, (float)rand()/(float)RAND_MAX );
		//pa.m_vel.Normalize();
		//printf("vel %3.2f %3.2f\n", pa.m_vel[0], pa.m_vel[1] );
		pa.m_vel *= 200.0; // pxls/sec		

		m_panim.push_back( pa );
	}

	// you made a combo.. you're a superstar! That's
	// what you are. a superstar... uh... 
	emitParts( c.m_value, vec2f( 300, 300 ), PART_STAR );

	m_lastCombo = c;
	m_lastComboAge = 0.0;
}

void ViewSimple2D::redraw( bool paused )
{
	vec2f ctr;	
	ctr = vec2f( 300, 300 );
	
	// 2d part
	glMatrixMode( GL_PROJECTION );	
	glLoadIdentity();	
	gluOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );	
	glLoadIdentity();

	// Draw pumpkins in a ring
	int i, j, k;
	float t, sz=60;
	int x, y;
	Pumpkin p;


	// the "board"
	glDisable( GL_TEXTURE_2D );
	glLineWidth( 3.0 );

	vec4f fill = vec4f( 0.5f, 0.7f, 0.6f, 0.5f ),
		  line = vec4f( 0.8f, 0.3f, 0.8f, 1.0f ),
		  fill_dest = vec4f( 0.9f, 0.4f, 0.5f, 0.5f ),
		  line_dest = vec4f( 0.2f, 1.0f, 0.2f, 1.0f );

	// The center "storage ring"
	drawCircle( ctr, 50, SHAPE_LINE_AND_FILL, 
				fill, line, 100 );


	// The outer ring
	drawCircle( ctr, 250, SHAPE_LINE, 
				fill, line, 100 );

	gfEnableFont( m_fontId, 25 );

	// a bunch of dots for the board
	vec2f freeze_pos, freeze_dir;
	vec2f queue_pos, queue_dir;
	vec2f station_pos;
	vec4f color, linecolor;
	char *label = NULL;
	for (i = 0; i < MACHINE_SLOTS; i++) {		
		t = (2.0*M_PI)/MACHINE_SLOTS * i;
		x = ctr[0] + cos(t)*250.0; 
		y = ctr[1] - sin(t)*250.0; 

		label = NULL;

		if ((i >= DEST_SLOT_NDX) && (i < DEST_SLOT_NDX+5)) {
			color = fill_dest;
			linecolor = line_dest;
			if (i==DEST_SLOT_NDX+2) label = "Destination";
		} else if (i==FREEZER_NDX) {
			color = vec4f( 0.0f, 1.0f, 1.0f, 0.6f );
			linecolor = vec4f( 1.0f, 1.0f, 1.0f, 1.0f );
			label = "Freeze";

			// draw freezer slots						
			freeze_dir = vec2f( cos(t), -sin(t) );
			freeze_pos = vec2f( x, y ) - freeze_dir*sz;

			for (int j=0; j < FREEZER_CAPACITY; j++) {

				drawCircle( freeze_pos - (freeze_dir*30*j), 
					15, SHAPE_LINE_AND_FILL, 
					vec4f( 1.0, 1.0, 1.0, 1.0 ), 
					vec4f( 0.0, 1.0, 1.0, 1.0), 12 );
			}

		} else if (i==PRODUCTION_NDX) {			
			color = fill;
			linecolor = line;

			// draw production slots			
			queue_dir = vec2f( cos(t), -sin(t) );
			queue_pos = vec2f( x, y ) + queue_dir*sz*0.6f;
			
			queue_dir = vec2f( 0.0, 1.0 ); // vertical queue

			for (int j=0; j < PRODUCTION_CAPACITY; j++) {

				drawCircle( queue_pos + (queue_dir*30*j), 
					15, SHAPE_LINE_AND_FILL, 
					vec4f( 1.0, 1.0, 1.0, 1.0 ), 
					vec4f( 0.5, 0.5, 0.5, 1.0), 12 );
			}


		} else if (i==TORCHY_NDX) {
			color = vec4f( 1.0f, 0.0f, 0.0f, 0.6f );
			linecolor = vec4f( 1.0f, 1.0f, 0.0f, 1.0f );
			label = "Torchy";
		} else {
			color = fill;
			linecolor = line;
		}		

		// draw swap lines
		if ((i==SWAP_NDX1)||(i==SWAP_NDX2)||(i==SWAP_NDX3)) {
			glColor4f( linecolor[0], linecolor[1],  linecolor[2],  linecolor[3] );
			glBegin( GL_LINES );
			glVertex3f( ctr[0], ctr[1], 0 );
			glVertex3f( x, y, 0 );
			glEnd();

			label = "Swap";
		}

		// draw circle
		drawCircle( vec2f( x, y ), 25, SHAPE_LINE_AND_FILL, 
				color, linecolor, 8 );

		// draw current station
		if (m_station==i) {
			station_pos = vec2f(x, y );			
		}

		// draw label
		if (label) {
			glColor4f( linecolor[0], linecolor[1],  linecolor[2],  linecolor[3] );
			gfBeginText();
			float lx = x - (gfGetStringWidth( label )/2.0);
			glTranslated( (lx<5.0)?5.0:lx, y, 0.0 );
			gfDrawString( label );
			gfEndText();
			glDisable( GL_TEXTURE_2D );
		}

	}

	// draw combos
	color = fill;
	linecolor = line;

	vec2f rmin(610,545), rmax(795, 595);
	for (i=0; i < m_game->getNumCombos(); i++)
	{
		Combo c = m_game->getCombo( i );

		glDisable( GL_TEXTURE_2D );
		drawRect( rmin, rmax, SHAPE_LINE_AND_FILL, 
				   color, vec4f( 0.0, 0.9, 0.9, 1.0) );
		
		gfEnableFont( m_fontId, 15 );
		gfBeginText();
		glTranslated( (rmin[0] + (rmax[0]-rmin[0])/2.0) - 
						gfGetStringWidth( c.m_name.c_str() )/2.0 , rmax[1]-12, 0.0 );
		gfDrawString( c.m_name.c_str() );
		gfEndText();

		// draw combo Value		
		gfEnableFont( m_fontId, 25 );
		gfBeginText();
		glTranslated( rmax[0]-30 , rmax[1]-35, 0.0 );
		gfDrawStringFmt( "%d", c.m_value );
		gfEndText();

		// draw combo pumpkins
		glBindTexture( GL_TEXTURE_2D, m_texPumpkinIcons );		
		vec2f px(rmin[0]+5, rmin[1]+5);		
		for (j=0; j < c.m_groups.size(); j++) {
			p = Pumpkin();
			p.m_type = c.m_groups[j].m_item;
			
			if ( (p.m_type == ANY_PUMPKIN) || (p.m_type == ANY_ITEM) ) {
				glColor3f( m_groupColor[j][0], m_groupColor[j][1], m_groupColor[j][2] );
			} else {
				glColor3f( 1.0, 1.0, 1.0 );
			}
			
			for (k=0; k < c.m_groups[j].m_count; k++ ) {
				drawPumpkin( px, p, sz*0.5, false );
				px[0] = px[0] + sz*0.5;
			}
		}

		rmin[1] = rmin[1] - 55;
		rmax[1] = rmax[1] - 55;

	}


	gfEnableFont( m_fontId, 25 );

	if (!paused) {
		
		// Draw pumpkins in the machine
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, m_texPumpkinIcons );
		
		glColor3f( 1.0, 1.0, 1.0 );	
		
		vec2f v;
		for (i = 0; i < MACHINE_SLOTS; i++) {
			p = m_game->getPumpkin( i );
			
			if (p.m_type == EMPTY) continue;
			
			v = calcPosFromNdx( i );		
			x = v[0] - (sz/2);
			y = v[1] - (sz/2);
			
			drawPumpkin( vec2f( x, y ), p, sz );		
		}
		
		
		// draw frozen pumpkins
		for ( j=0; j < m_game->getFreezerUsage(); j++) {	
			
			p = m_game->getFrozenFood( j );
			// shouldn't be empty, but...
			if (p.m_type != EMPTY) {
				drawPumpkin( (freeze_pos+ - (freeze_dir*30*j)) - vec2f(sz*0.25,sz*0.25), p, sz*0.5 );
			}
		}	
		
		// draw queued pumpkins
		for ( j=0; j < m_game->getProdQueueSize(); j++) {	
			
			p = m_game->getQueuedPumpkin( j );
			// shouldn't be empty, but...
			if (p.m_type != EMPTY) {
				drawPumpkin( (queue_pos+ (queue_dir*30*j)) - vec2f(sz*0.25,sz*0.25), p, sz*0.5 );
			}
		}	
		
		// draw center pumpkin
		p = m_game->getCenterPumpkin();
		if (p.m_type != EMPTY) {
			drawPumpkin( vec2f( ctr[0]-(sz*1.5/2), ctr[1]-(sz*1.5/2)), p, sz*1.5 );
		}
		
	}

	// Draw last combo acheived
	if (m_lastComboAge < COMBO_DISPLAY_TIME) {
		char buff[200];
		t = 1.0 - (m_lastComboAge/COMBO_DISPLAY_TIME);
		gfEnableFont( m_fontId, 25 );
		gfBeginText();

		sprintf( buff, "%d - %s - %d", m_lastCombo.m_value, 
					   			   m_lastCombo.m_name.c_str(), 
								   m_lastCombo.m_value );

		
		glColor3f( 1.0, t, t );		
		glTranslated( ctr[0] - gfGetStringWidth( buff )/2, 
				ctr[1]+50-(50.0*t)+j*5, 0.0 );
		gfDrawString( buff );			

		gfEndText();
	}

	glDisable( GL_TEXTURE_2D );

	// draw cursor
	drawCircle( station_pos, 30, SHAPE_LINE, 
				color, vec4f( 0.0, 1.0, 1.0, 1.0), 8 );

	drawCircle( station_pos, 35, SHAPE_LINE, 
				color, vec4f( 0.0f, 0.8f, 0.8f, 1.0f), 8 );

	// Draw animated pumpkins
	if (!paused) {
		
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, m_texPumpkinIcons );
		
		PumpkinAnim pa;
		for (i = 0; i < m_panim.size(); i++) {
			pa = m_panim[i];
			glPushMatrix();
			glTranslated( pa.m_pos[0] - sz/2, pa.m_pos[1]-sz/2, 0.0 );
			drawPumpkin( vec2f( 0.0, 0.0 ), pa, sz );
			glPopMatrix();
		}
		
	}

	// draw particles
	drawParts();
	
}

void ViewSimple2D::drawParts() {
	float s1, s2, x, y, sz;

	glBindTexture( GL_TEXTURE_2D, m_texParticles );	
	glBegin( GL_QUADS );
	for (int i = 0; i < m_parts.size(); i++) {

		if (m_parts[i].m_age < 2.0) {
			glColor3f( 1.0, 1.0, 1.0 );
		} else {
			glColor4f( 1.0, 1.0, 1.0, 1.0-(m_parts[i].m_age-2.0) );
		}

		sz = 16 * m_parts[i].m_age;
		if (sz < 16) sz=16;
		x = m_parts[i].m_pos[0] - sz;
		y = m_parts[i].m_pos[1] - sz;
		s1 = m_parts[i].m_type * (1.0/8.0);
		s2 = s1 + (1.0/8.0);
		glTexCoord2d( s1, 0 );   glVertex3f( x,y, 0.0 );
		glTexCoord2d( s1, 1 );   glVertex3f( x, y+sz, 0.0 );
		glTexCoord2d( s2, 1 );  glVertex3f( x+sz,y+sz, 0.0 );
		glTexCoord2d( s2, 0 );  glVertex3f( x+sz, y, 0.0 );
	}
	glEnd();
}

void ViewSimple2D::updateParts( float dt ) {

	std::vector<Particle> keep;
	for (int i = 0; i < m_parts.size(); i++) {
		m_parts[i].m_age += dt;
		m_parts[i].m_pos += m_parts[i].m_vel * dt;

		if ((m_parts[i].m_type==PART_FIRE)||(m_parts[i].m_type==PART_SMOKE)) {
			m_parts[i].m_vel += vec2f( 0.0, 20.0) * dt; // upwards gravity :)

			if ((m_parts[i].m_type==PART_FIRE) && 
				(m_parts[i].m_age > 1.0)) {
				m_parts[i].m_type=PART_SMOKE;
			}

		} else {
			m_parts[i].m_vel += vec2f( 0.0, -400.0) * dt; // gravity
		}

		// remove offscreen panim
		if ( (m_parts[i].m_age > 3.0 ) ||
			 (m_parts[i].m_pos[0] < -100) ||
			 (m_parts[i].m_pos[0] > 900) ||
			 (m_parts[i].m_pos[1] < -100) ||
			 (m_parts[i].m_pos[1] > 800) ) {
			// don't keep
		} else {
			keep.push_back( m_parts[i] );
		}
	}

	m_parts = keep;
}

void ViewSimple2D::emitParts( int num, vec2f pos, int type ) {

	float sz = 50.0;

	Particle p;
	for (int i=0; i < num; i++) {
		p.m_type = type;
		p.m_age = 0.0;
		switch (type) {
			case PART_FROST: 
				{
					float a = SFRAND * (2.0*M_PI);
					p.m_pos = pos + (vec2f( cos(a), sin(a) ) * FRAND * sz/2.0);
					p.m_vel = vec2f( cos(a), sin(a) ) * (300*FRAND);
					p.m_rot = FRAND;
					p.m_rot_vel = SFRAND;
				}
				break;

			case PART_STAR: 
			{
				float a = SFRAND * (2.0*M_PI);
				p.m_pos = pos + (vec2f( cos(a), sin(a) ) * FRAND * sz/2.0);
				p.m_vel = vec2f( cos(a), sin(a) ) * (200+(100*FRAND));
				p.m_rot = FRAND;
				p.m_rot_vel = SFRAND;
			}
			break;

			case PART_FIRE: 
				{					
					p.m_pos = pos + SFRAND * (sz/2.0);
					p.m_vel = vec2f( SFRAND*0.2, FRAND ) * (50*FRAND);
					p.m_rot = 0.0;
					p.m_rot_vel = 0.0;
				}
				break;

		}

		m_parts.push_back( p );
	}
}

void ViewSimple2D::activateStation()
{
	// Do fx for this station?
	switch (getStation()) {
		case FREEZER_NDX:
			if ( (m_game->getFreezerUsage()) ||
				(m_game->getPumpkin(FREEZER_NDX).m_type != EMPTY) ) {
				emitParts( 150, calcPosFromNdx( FREEZER_NDX ), PART_FROST );
			}
	}
	GameView::activateStation();
}