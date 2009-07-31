#ifndef GAME_VIEW_GL_H
#define GAME_VIEW_GL_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include "GameState.h"
#include "AVar.h"

#include "Mesh.h"

class GameViewGL
{
public:
	GameViewGL( GameState *game );

	void update( float dt );
	void redraw( float dt );// todo: move all the animation stuff to update

	void mouseMove( int x, int y );
	void mouseDown();
	void mouseUp();
	
protected:
	void initResources();

	// Draw the board
	void setupLighting();

	void setupDrawPass( int drawPass );
	void drawTile( const Tile &t, int drawPass, float i, float j );
	void drawQuad( float i, float j, float yval );

	GameState *m_game;

	vec2f m_mousePos2d;
	vec3f m_mousePosGround;
	double m_modelview[16], m_proj[16];
	int m_viewport[4];

	// test
	anim_float dbg_bgBlue;

	GLuint m_idTile;
	GLuint m_idTileBlocked;
	GLuint m_idCar;

	ILuint m_ilTileFontId;
	GLuint m_glTileFontTexId;
	GLuint m_glTileTexId;
	GLuint m_glBlockTexId;
	GLuint m_glGrassTexId;
	GLuint m_glCarTexId;	

	GLuint loadTexture( const char *filename );

	// drop stuff
	bool m_dragging;
	int m_dragTileIndex;
};

#endif
