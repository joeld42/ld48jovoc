#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include <vector>

// This is like model-view-controller except without
// all the careful planning. The "main loop" takes the
// role of the controller.

class GameState;

class GameView {
public:
	GameView( GameState *state );

	virtual void initGraphics( unsigned int fontId ) { m_fontId = fontId;}
	virtual void redraw( bool paused ) {}

	// run the game forward a bit
	virtual void simulate( float dt );

	// stations are where you go to do things
	virtual void homeStation();
	virtual void nextStation();
	virtual void prevStation();

	virtual void activateStation();	

	virtual int getStation() { return m_station; }

	// the gamestate calls this when a combo
	// happens. ndxlist contains the indices of
	// the pumpkins which are about to be removed
	virtual void notifyAboutCombo( Combo &c, std::vector<int> ndxlist ) {}

	// prefs
	void toggleAnim() { m_anim = !m_anim; }

protected:
	GameState *m_game;

	float m_turn; // fractional part of turn... 0..1

	// station
	std::vector<int> m_stationNdxList;
	int m_station;  // what station we are at		

	// really shouldn't be here but oh well
	unsigned int m_fontId;

	// prefs & debug stuff
	bool m_anim;

};

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif


#endif