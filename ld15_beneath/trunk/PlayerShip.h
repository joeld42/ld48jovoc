#ifndef PLAYERSHIP_H
#define PLAYERSHIP_H

#include <GameObj.h>

class PlayerShip : public GameObj
{
public:
	PlayerShip( Shape *shape );
	
	std::string m_playerName;
	vec2f m_pos;
	vec2f m_vel;
	float m_angle;
	float m_radius;

	// controls
	int m_forward;
	int m_reverse;
	int m_rotleft;
	int m_rotright;
	int m_latleft;
	int m_latright;
	
	bool m_readyToStart;
};


#endif