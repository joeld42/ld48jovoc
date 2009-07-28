#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <deque>

#include <prmath/prmath.hpp>

///////////////////////
// Gameplay parameters
///////////////////////

// Number of slots in the machine
#define MACHINE_SLOTS (30)

// start of the 5 indices that ar checked for a set
#define DEST_SLOT_NDX (13) 

// Places where you can swap from
#define SWAP_NDX1 (8)
#define SWAP_NDX2 (21)
#define SWAP_NDX3 (29)

#define FREEZER_NDX (5)
#define TORCHY_NDX (1)

// production
#define PRODUCTION_NDX (0)
#define PRODUCTION_CAPACITY (6)
#define DEFAULT_PRODUCTION_RATE (4)

// really a view param but keep it all together
#define TURN_TIME_SECS (0.8) 

// how many pumpkins can be frozen?
#define FREEZER_CAPACITY (5)

// how long (# turns) it takes a pumpkin to burn
#define PUMPKIN_BURN_RATE (13)

// using the torch costs points.. 
#define TORCH_COST (10)

// Pumpkin 
enum {
	// Nothing is here..
	EMPTY,

	//pumpkin things
	PUMPKIN,
	PUMPKIN_ORANGE,
	PUMPKIN_YELLOW,
	PUMPKIN_RED,

	// The non-pumpkin things
	SKULL,
	FISH_HEAD,
	BLACKBIRD,

	// the combo uses these
	ANY_PUMPKIN,
	ANY_ITEM,  

};

// The view
class GameView;

// All things are pumpkins, even if they
// are skulls or fish heads.
struct Pumpkin {
	Pumpkin() {
		m_type = EMPTY;
		m_burning = false;
		m_burnval = 0;
	}

	Pumpkin( const Pumpkin &other ) {
		m_type = other.m_type;
		m_burning = other.m_burning;
		m_burnval = other.m_burnval;
	}

	const Pumpkin &operator= ( const Pumpkin &other ) {
		m_type = other.m_type;
		m_burning = other.m_burning;
		m_burnval = other.m_burnval;
		return *this;
	}
	
	int m_type;
	bool m_burning;
	int m_burnval;
	
};

// Combos are good....
struct ComboGroup {
	int m_item; // what to match
	int m_count;
};

struct Combo {
	Combo() {}

	Combo( int value, bool secret=false );

	bool m_secret;   // is this a secret combo?	
	int m_value; // points! Yay!
	std::vector<ComboGroup> m_groups; // what you need to get

	std::string m_name;
	std::string m_desc;

	void addGroup( int item, int count );	

	bool match( std::vector<Pumpkin> &plist, bool *used, int gndx=0 );
};

/////////////////////////////////
struct Level {
	std::string m_group, m_title, m_desc;
	int m_prodrate;
	std::vector<Combo> m_combos;
};

/////////////////////////////////
class GameState {
public:
	GameState();
	
	void init();

	void loadLevels( const std::string &filename );

	// view stuff
	void setView( GameView *view );

	// Access to pumpkins
	Pumpkin getPumpkin( int ndx );
	Pumpkin getCenterPumpkin() { return m_center; }

	int getFreezerUsage() { return m_freezer.size(); }
	Pumpkin getFrozenFood( int ndx ) { return m_freezer[(m_freezer.size()-1)-ndx]; }

	int getProdQueueSize() { return m_production_queue.size(); }
	Pumpkin getQueuedPumpkin( int ndx ) { return m_production_queue[ndx]; }

	// access to combos
	int getNumCombos( bool incSecret=false );
	Combo &getCombo( int ndx, bool incSecret=false );

	// for debugging
	void dbgNextPumpkin( int ptype );
	void dbgClearQueue( );

	// gameplay
	void doTurn(); // advance the game a turn

	// swap with the center Pumpkin
	void Swap( int ndx ); 

	// put or take out from the freezer
	void Freeze( int ndx ); 

	// Fire! Fire! 
	void Torch( int ndx );

	// Stuff you set up between levels
	void resetLevel( int levelIndex);
	void setProductionRate( int rate ) { m_productionRate = rate; }
	void addCombo( Combo c );

	// For the level selection menu
	int getNumLevels() { return m_levels.size(); }
	const std::string &getLevelGroup( int ndx ) { return m_levels[ndx].m_group; }
	const std::string &getLevelDesc( int ndx ) { return m_levels[ndx].m_desc; }
	const std::string &getLevelTitle( int ndx ) { return m_levels[ndx].m_title; }

	void forceGameOver() { m_gameOver = true; }
	bool isGameOver() { return m_gameOver; }

	int getScore() { return m_score; }
	int getLevelScore() { return m_levelscore; }

protected:
	void ProducePumpkins();

	Pumpkin makeRandomPumpkin();

	// combo stuff
	std::vector<int> evalBestComboValue( int startndx, Combo &c );	

protected:
	std::deque<Pumpkin> m_pumpkins;
	std::vector<Pumpkin> m_freezer;

	GameView *m_view;

	Pumpkin m_center;

	// Production rate -- how often are new
	// pumpkins added??
	int m_productionRate;
	int m_turnsLeftUntilProduction;
	std::deque<Pumpkin> m_production_queue;

	// combos and levels
	std::vector<Combo> m_allCombos;
	std::vector<Level> m_levels;

	// combos for this level
	std::vector<Combo> m_combos;

	// the always popular score
	int m_levelscore;
	int m_score;

	// game status
	bool m_gameOver;
};

#endif