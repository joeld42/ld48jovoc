#ifndef GLOBALS_H
#define GLOBALS_H

#include <allegro.h>
#include <alfont.h>

#include <vector>

// common fonts
extern ALFONT_FONT *g_fontTitlePDA;
extern ALFONT_FONT *g_fontTextPDA;

// gamestate
enum {
	GAMESTATE_PDA,
	GAMESTATE_GAME,
	GAMESTATE_TILETEST
};
extern int gamestate;

// Levels
struct Map;
extern Map *g_map;

// Cost to go to next planet
extern int TICKET_PRICE;

// useful funcs
int irand( int min, int max );
float frand( float min, float max );

float clamp( float n );
int iclamp( int n );

#define min3(a, b, c) ((a)<(b) ? ((a)<(c) ? (a) : (c)) : ((b)<(c) ? (b) : (c)))
#define max3(a, b, c) ((a)>(b) ? ((a)>(c) ? (a) : (c)) : ((b)>(c) ? (b) : (c)))

#define lerp(a, b, t) ((t)*(a) + (1.0-t)*(b))

class Player {
public:
    Player() {vy = vx = 0.0f;}
    void  setX(float x) {m_fx = x; m_x = static_cast<int>(x);}
    void  setY(float y) {m_fy = y; m_y = static_cast<int>(y);}
    int   x() const {return m_x;}
    int   y() const {return m_y;}
    float xF() const {return m_fx;}
    float yF() const {return m_fy;}

    float vy;
    float vx;
	int f, ff;
	int walkdir;
	int iswalking;
	int airborne;
	int jetpack;

	int inv[10];
	int cash;

private:
    float m_fy;
    float m_fx;
	int   m_x;
    int   m_y;
};

extern Player g_player;
extern BITMAP *bmpItems[10];
extern int gameOn, atRocket;
extern BITMAP *bmpTrain;

enum {
	ITEM_MUSHROOM,
	ITEM_APPLE,
	ITEM_BATTERY,
	ITEM_BULB,
	ITEM_FISH,
	ITEM_SAX,
	ITEM_CHEESE,
	ITEM_TILE,
	ITEM_HAMMER,
	ITEM_LATTE
};

struct Store {
	int price[10], stock[10];
};

enum {
	STATE_S1,
	STATE_S2,
	STATE_MOVING_S1_TO_S2,
	STATE_MOVING_S2_TO_S1
};
struct TrainFSM 
{
	int x, y;  // train loc
	int s1x, s2x; // station locs

	int state;
	int stime,    // time this stays at station
		timeleft; // time until departure

	Store store1, store2;
};
extern std::vector<TrainFSM> g_trains;

// is the player at a store?
extern Store *g_atStore;
extern int g_simResume;

void initLevel();

#endif