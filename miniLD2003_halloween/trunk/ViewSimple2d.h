#ifndef VIEW_SIMPLE_2D_H
#define VIEW_SIMPLE_2D_H

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

#include "SoundFx.h"

#include "GameView.h"

enum {
	SHAPE_LINE,
	SHAPE_FILL,
	SHAPE_LINE_AND_FILL
};

struct PumpkinAnim : public Pumpkin {

	PumpkinAnim &operator=( const Pumpkin &other ) {
		Pumpkin::operator=( other );
		return *this;
	}

	vec2f m_pos;
	vec2f m_vel;	
};

// If I planned ahead, some of this could share with
// animated pumpkins...
struct Particle {
	vec2f m_pos;
	vec2f m_vel;
	float m_rot, m_rot_vel;
	int m_type;
	float m_age;
};

enum {
	PART_FIRE,
	PART_FROST,
	PART_SMOKE,
	PART_STAR
};

class ViewSimple2D : public GameView {
public:
	ViewSimple2D( GameState *game );

	virtual void initGraphics( unsigned int fontId );
	virtual void redraw( bool paused );

	virtual void activateStation();

	virtual void notifyAboutCombo( Combo &c, std::vector<int> ndxlist );

	virtual void simulate( float dt );

protected:

	vec2f calcPosFromNdx( int ndx );

	void drawCircle( vec2f ctr, float rad, 
								int mode,
								vec4f clr_fill, vec4f clr_line, 
								int steps );
	void drawRect( vec2f min, vec2f max, int mode,
				   vec4f clr_fill, vec4f clr_line );
	
	void drawPumpkin( vec2f pos, const Pumpkin &p, float sz, bool doColor=true );

	// particle man, particle man, triangle man hates particle man
	void drawParts();
	void updateParts( float dt );
	void emitParts( int num, vec2f emitter_pos, int type );

	ILuint m_ilPumpkinIcons;
	GLuint m_texPumpkinIcons;

	ILuint m_ilParticles;
	GLuint m_texParticles;

	std::vector<vec3f> m_groupColor;
	std::vector<PumpkinAnim> m_panim;

	float m_burntime;
	

	std::vector<Particle> m_parts;

	Combo m_lastCombo;
	float m_lastComboAge;
};

#endif	