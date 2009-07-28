#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include <vector>
#include <deque>
#include <allegro.h>
class TileMap;

void message( const char *message );

class GameObj
{
public:
	GameObj();

	virtual void update(TileMap &map,std::vector<GameObj*> &objs );

	BITMAP *m_bmp;
	int m_x, m_y;
};

class Player : public GameObj
{
public:
	Player();

	virtual void update( TileMap &map, std::vector<GameObj*> &objs );

	int walk_x, walk_y;
	int walk_c;

	std::deque<int> old_x;
	std::deque<int> old_y;
};

class BubbleObj : public GameObj
{
public:
	BubbleObj();

	TileMap *m_bub;
};

// Critter 
class CritterObj : public GameObj
{
public:
	CritterObj();
	int walk_c;

	void update( TileMap &map, std::vector<GameObj*> &objs );

};

// NPC 
class NpcObj : public GameObj
{
public:
	NpcObj();	
	bool in_party;
};

#endif