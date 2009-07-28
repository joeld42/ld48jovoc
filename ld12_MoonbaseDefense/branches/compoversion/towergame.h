#ifndef TOWERGAME_H
#define TOWERGAME_H

#include "Ogre.h"

#include <vector>

class TowerGameApp;
struct Crawler;

// scene masks
enum 
{
	MASK_OBSTACLE  = 0x01,
	MASK_CRAWLER   = 0x02,
	MASK_TOWER	   = 0x04,
	MASK_CONNECTOR = 0x08
};

//================================
struct Tower
{
public:
	Tower();

	virtual void Build( SceneManager *scene )=0;
	virtual SceneNode *getNode()=0;

	virtual void update( Crawler *closestEnemy, float dist, float dt, TowerGameApp *game ) {};

	virtual void connectSubtower( SceneManager *scene, Tower *subtower );
	SceneNode * m_connectNode; // connector TO PARENT

	SceneManager *m_sceneMgr;

	std::vector<Tower*> m_subtowers;
	Tower *m_upstreamTower;

	int m_towerIndex;
	static int m_towerHighIndex;

};

struct BaseTower : public Tower
{
	BaseTower();

	virtual void Build( SceneManager *scene );
	virtual SceneNode *getNode() { return m_base; }

	virtual void update( Crawler *closestEnemy, float dist , float dt, TowerGameApp *game ) {}

	SceneNode *m_base;	
};

enum TowerType {
	Type_NODULE,	// no weapon, but can branch
	Type_GUN,	 // fast, low damage
	Type_MISSILE, // slow, high damage
	Type_FLAMETHROWER // short range but splash damage
};

struct WeaponTower : public Tower
{

	WeaponTower( TowerType type );

	virtual void Build( SceneManager *scene );
	virtual SceneNode *getNode() { return m_platform; }

	virtual void update( Crawler *closestEnemy, float dist, float dt, TowerGameApp *game );

	TowerType m_type;
	
	float m_sensorRange;
	float m_reloadTime;
	float m_damage;

	ParticleSystem *m_psys;
	SceneNode *m_platform, *m_bracket, *m_gun, *m_psysNode;

	// timeout 
	float m_timeout;
};

//================================
struct Crawler 
{
	Crawler( SceneManager *scene );

	void update( float dt );

	SceneNode *getNode();
	SceneNode *m_crawler;

	float m_spinRate;
	float m_health;
	static int m_crawlerIndex;
};

#endif