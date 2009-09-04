#include <stdio.h>

#include <allegro.h>
#include <tilemap.h>
#include <game_obj.h>

// panic has set in, screw encapsulation
extern std::vector<TileMap*> bubbles;
extern std::vector<GameObj*> npcs;

GameObj::GameObj() : m_bmp( NULL ), m_x(0), m_y(0)
{
}

void GameObj::update( TileMap &map, std::vector<GameObj*> &objs )
{
}

Player::Player() : GameObj()
{
	walk_x = 0;
	walk_y = 0;
	walk_c = 0;
}

void Player::update( TileMap &map, std::vector<GameObj*> &objs )
{
	// Walk
	if (walk_c==0)
	{
		int x2,y2;
		x2 = m_x + walk_x;
		y2 = m_y + walk_y;
		MapCell &c = map.map( x2, y2 );
		if ((c.m_tile) && (c.m_tile->m_code == Tile_LAND))
		{
			if ((x2 != m_x) || (y2 != m_y))
			{					
				// update followers				
				for (int i=0; i < npcs.size(); i++)
				{					
					npcs[i]->m_x = old_x[i];
					npcs[i]->m_y = old_y[i];
				}				

				// move
				old_x.push_front( m_x );
				old_y.push_front( m_y );
				if (old_x.size() > 20 )
				{
					old_x.pop_back();
					old_y.pop_back();
				}
				m_x = x2; m_y = y2;
			}
		}

		walk_c = 3;
		//printf("WALK: %d %d %d %d\n", m_x, m_y, walk_x, walk_y );		
	}
	else
	{
		walk_c--;
	}

	// Are we standing on something interesting???
	for (int i=0; i < objs.size(); i++)
	{
		GameObj *o = objs[i];

		if (!o) continue; // skip nulls
		if (o==this) continue; // skip self

		if ((o->m_x == m_x) && (o->m_y == m_y))
		{
			BubbleObj *bubObj = dynamic_cast<BubbleObj*>( o );
			if (bubObj)
			{
				// it's a bubble!
				if (bubbles.size() < 5)
				{
					bubbles.push_back( bubObj->m_bub );
					objs[i] = NULL;
					delete bubObj;
				}
			} else {			
				NpcObj *npc = dynamic_cast<NpcObj*>( o );
				if ((npc) && (npc->in_party == false))
				{
					message( "I gained a follower." );
					npcs.push_back( npc );			
					npc->in_party = true;
				}
			}
		}
	}

}

BubbleObj::BubbleObj() : GameObj()
{
	m_bub = NULL;
}

CritterObj::CritterObj() : GameObj()
{
	// pause before walking
	walk_c = 30;
}

void CritterObj::update( TileMap &map, std::vector<GameObj*> &objs )
{
	// Walk
	if (walk_c==0)
	{
		int x2,y2;
		x2 = m_x + (rand() % 3) - 1;
		y2 = m_y + (rand() % 3) - 1;
		MapCell &c = map.map( x2, y2 );
		if ((c.m_tile) && (c.m_tile->m_code == Tile_LAND))
		{
			m_x = x2; m_y = y2;
		}

		walk_c = 8 + (rand() % 15);
		//printf("WALK: %d %d %d %d\n", m_x, m_y, walk_x, walk_y );		
	}
	else
	{
		walk_c--;
	}
}

NpcObj::NpcObj() : GameObj()
{
	in_party = false;
}
