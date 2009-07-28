#ifndef GAME_VIEW_H
#define GAME_VIEW_H

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

#include "GlacierGame.h"

// An iceblock is a dropped
// tetromino instance
struct IceBlock
{
	color3f m_iceCol;
	vec3f m_pos;
	vec3f m_sinkRot; // sinking rotate axis
	float m_sinkRotAmt;

	int m_rot;
	Tetromino *m_tet;
};

// A land block is a block of land
struct LandBlock
{
	LandBlock()
	{
		m_hite = 1;		
	}

	void drawTop();
	void drawBottom();
	void shade();
	
	std::vector<vec2f> m_pnts;
	int m_hite;	
};

class GameView	
{
public:
	GameView( GlacierGame *game );

	void update( float dt );
	void redraw( int state );

	void mouseMove( int x, int y );
	void mouseDown();
	void mouseUp();

//protected:
	GLuint loadTexture( const char *filename );
	void loadResources();
	void setupLighting();

	GlacierGame *m_game;
	int m_screenX, m_screenY;

	std::vector<std::string> m_levelNames;
	int m_levelNdx;

	// ice blocks
	std::vector<IceBlock> m_blocks;

	// mouse stuff
	void updateMouse();
	int m_mouseX, m_mouseY;
	vec2f m_mousePos2d;
	vec3f m_mousePosGround;
	double m_modelview[16], m_proj[16];
	int m_viewport[4];

	float m_oceanAnim;
	int m_editHite;
	bool m_zoomOut;

	int m_currMenuItem;
	
	LandBlock *m_editBlock;

	// dbg stuff
	bool dbgDrawHeight;

	// graphics resource	
	ILuint m_ilFontId;
	GLuint m_glFontTexId, m_fntFontId;

	GLuint m_glLogoTexId;	

	GLuint m_glGroundTexId;	
	GLuint m_glGlacierTexId;	
	GLuint m_glGroundSideTexId;
	GLuint m_glOceanTexId;	
	GLuint m_glCritterTexId;	
	
	GLuint m_idTetBlock; // single block

	GLuint m_idTetLine;  // tetris pieces
	GLuint m_idTetZee1;
	GLuint m_idTetZee2;
	GLuint m_idTetSquare;
	GLuint m_idTetEll1;
	GLuint m_idTetEll2;
	GLuint m_idTetTee1;
	GLuint m_idTetTee2;
};

#endif