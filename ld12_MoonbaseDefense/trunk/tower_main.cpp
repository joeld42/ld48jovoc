#include "ExampleApplication.h"

#include "towergame.h"
#include "TowerGameApp.h"

int Tower::m_towerHighIndex = 0;
int Crawler::m_crawlerIndex = 0;

Tower::Tower() : m_connectNode( NULL ), m_upstreamTower( NULL ) 
{
	m_towerIndex = m_towerHighIndex++;
};

void Tower::connectSubtower( SceneManager *scene, Tower *subtower )
{
	char name[30];
	sprintf( name, "Connector%d_%d\n", m_towerIndex, subtower->m_towerIndex );
	Entity *ent1 = scene->createEntity( name, "tube.mesh");

	sprintf( name, "Connector%d_%d\n", m_towerIndex, subtower->m_towerIndex );
	subtower->m_connectNode = scene->getRootSceneNode()->createChildSceneNode( name );
	subtower->m_connectNode->attachObject( ent1 );	

	Vector3 pos = getNode()->getPosition();
	Vector3 subPos = subtower->getNode()->getPosition();

	// align connector
	Vector3 connectPos = pos + subPos;
	connectPos *= 0.5;
	connectPos.y = 1.0;
	subtower->m_connectNode->setPosition( connectPos);

	Vector3 connectDir = pos - subPos;
	subtower->m_connectNode->setScale( 1.0, 1.0, connectDir.length() );
	connectDir.normalise();
	subtower->m_connectNode->setDirection( connectDir );

	subtower->m_upstreamTower = this;
}

BaseTower::BaseTower()
: Tower(), m_base( NULL )
{
}

void BaseTower::Build( SceneManager *scene )
{
	char name[30];
	sprintf( name, "Base%d\n", m_towerIndex );
	Entity *ent1 = scene->createEntity( name, "tower_base.mesh");

	sprintf( name, "BaseNode%d\n", m_towerIndex );
	m_base = scene->getRootSceneNode()->createChildSceneNode( name );
	m_base->attachObject( ent1 );	

	ent1->setQueryFlags( MASK_OBSTACLE | MASK_TOWER );

	m_sceneMgr = scene;
}


WeaponTower::WeaponTower( TowerType type ) :
	Tower(), m_type( type )
{
	m_timeout = 0.0;


	if (m_type == Type_NODULE)
	{
		// nodules don't do anything		
		m_sensorRange = 10.0; //60.0;
		m_reloadTime = 1.1;
		m_damage = 1.0;
	}
	else if (m_type == Type_GUN)
	{
		m_sensorRange = 300.0; //60.0;
		m_reloadTime = 0.1;
		m_damage = 20.0;
	}
	else if (m_type == Type_MISSILE)
	{
		m_sensorRange = 300.0;
		m_reloadTime = 2.0;
		m_damage = 100.0;
	}
	else if (m_type == Type_FLAMETHROWER)
	{
		m_sensorRange = 70.0;
		m_reloadTime = 0.0;		
		m_damage = 60.0; // per second for splash
	}

}

	void WeaponTower::Build( SceneManager *scene )
	{
		char name[30];
		m_sceneMgr = scene;

		// Nodules don't have weapons
		if (m_type == Type_NODULE )
		{
			sprintf( name, "Nodule%d\n", m_towerIndex );
			Entity *ent1 = scene->createEntity( name, "tower_nodule.mesh");

			sprintf( name, "NoduleNode%d\n", m_towerIndex );
			m_platform = scene->getRootSceneNode()->createChildSceneNode( name );
			m_platform->attachObject( ent1 );

			ent1->setQueryFlags( MASK_OBSTACLE | MASK_TOWER );
		}
		else
		{
			sprintf( name, "Platform%d\n", m_towerIndex );
			Entity *ent1 = scene->createEntity( name, "tower_platform.mesh");

			sprintf( name, "PlatformNode%d\n", m_towerIndex );
			m_platform = scene->getRootSceneNode()->createChildSceneNode( name );
			m_platform->attachObject( ent1 );

			ent1->setQueryFlags( MASK_OBSTACLE | MASK_TOWER );

			sprintf( name, "Bracket%d\n", m_towerIndex );
			Entity *ent2 = scene->createEntity( name, "tower_bracket.mesh");

			sprintf( name, "BracketNode%d\n", m_towerIndex );
			m_bracket = m_platform->createChildSceneNode( name, Vector3( 0.0, 5.0, 0.0 ) );
			m_bracket->attachObject( ent2 );

			sprintf( name, "Gun%d\n", m_towerIndex );
			Entity *ent3 = scene->createEntity( name, "tower_gun.mesh");

			sprintf( name, "GunNode%d\n", m_towerIndex );
			m_gun = m_bracket->createChildSceneNode( name, Vector3( 0.0, 2.0, 0.0) );
			m_gun->attachObject( ent3 );


			if (true) //(m_towerHighIndex == 1)
			{				
				if ( m_type==Type_FLAMETHROWER)
				{
					sprintf( name, "FlameThower%d\n", m_towerIndex );
					m_psys = scene->createParticleSystem( name,"Tower/Flamethrower");
				}
				else
				{
					sprintf( name, "TracerFire%d\n", m_towerIndex );
					m_psys = scene->createParticleSystem( name,"Tower/TracerFire");
				}

				// Point the fountain at an angle
				m_psysNode = m_gun->createChildSceneNode();
				m_psysNode->translate(0,0,-10.0);
				m_psysNode->rotate(Vector3::UNIT_X, Degree(90));
				m_psysNode->attachObject(m_psys);
			}	
		}
	}

void WeaponTower::update( Crawler *closestEnemy, float dist , float dt, TowerGameApp *game )
{
	if (m_type == Type_NODULE)
	{
		// nodules dont do anything
		return;
	}
	

	// don't update if out of range or no bug
	ParticleEmitter *emitter = m_psys->getEmitter( 0 );
	if ((dist > m_sensorRange) || (!closestEnemy)) 
	{
		emitter->setEnabled( false );
		return;
	}

	// turn on the juice
	emitter->setEnabled( true );	

	// aim at the bug
	Vector3 myPos = m_platform->getPosition();
	Vector3 crawlerPos = closestEnemy->getNode()->getPosition();
	Vector3 dir = crawlerPos - myPos;
	dir.y = 0.0;
	dir.normalise();

	//Radian targetHeading = Ogre::Math::ATan2( dir.x, dir.z );	
	//m_bracket->yaw( targetHeading );
	m_bracket->setDirection( dir, Ogre::Node::TS_WORLD );

	dir = crawlerPos - (myPos + Vector3( 0.0, 7.0, 0.0));
	//float adj = sqrt((dir.x * dir.x) + ( dir.z * dir.z));
	//Radian angle = Ogre::Math::ATan2( dir.y, adj );
	//m_gun->pitch( angle, Ogre::Node::TS_LOCAL);
	m_gun->setDirection( dir, Ogre::Node::TS_WORLD );

	//game->mFrameListener->mDebugText = " blarg blarg";

	// Ok.. now actually apply damage	
	if (m_timeout < 0.001)
	{		

#if 0
		Ray shooty( m_gun->getWorldPosition(),
				m_gun->getWorldOrientation().zAxis() );
		RaySceneQuery *rayQuery = m_sceneMgr ->createRayQuery( shooty );
		rayQuery->setQueryMask( MASK_CRAWLER );

		RaySceneQueryResult &result = rayQuery->execute();
		RaySceneQueryResult::iterator itr = result.begin();

		/// did we hit a crawler
		game->mFrameListener->mDebugText = String("Hit Test: ");
		if (itr != result.end() && ( itr->movable))
        {
			game->mFrameListener->mDebugText += String(" movable: ") + itr->movable->getName();

			SceneNode *node = itr->movable->getParentSceneNode();
			if (node)
			{
				game->mFrameListener->mDebugText +=  node->getName();

				// did we hit a crawler??
				for (std::vector<Crawler*>::iterator ci = game->m_crawlers.begin();
					ci != game->m_crawlers.end(); ++ci )
				{
					if ( (*ci)->getNode() == node )
					{
						// hit!
						(*ci)->m_health -= m_damage;
					}
				}
			}
		}

		 m_sceneMgr->destroyQuery(rayQuery);
#endif

		 // forget all of that nonsense... just hit the
		 // closest crawler
		 if (m_type == Type_FLAMETHROWER)
		 {
			 // TODO: should be all enemies within range
			closestEnemy->m_health -= m_damage * dt;
		 }
		 else
		 {
			 closestEnemy->m_health -= m_damage;
		 }

		// reset timeout
		m_timeout = m_reloadTime;
	}
	else
	{
		// wait to 'recharge'
		m_timeout -= dt;
	}


}

//============================================================================
Crawler::Crawler( SceneManager *scene )
{
	char name[30];
	sprintf( name, "Crawler%d", m_crawlerIndex );
	Entity *ent1 = scene->createEntity( name, "crawler.mesh");
	ent1->setQueryFlags( MASK_CRAWLER );

	sprintf( name, "CrawlerNode%d\n", m_crawlerIndex );
	m_crawler = scene->getRootSceneNode()->createChildSceneNode( name );
	m_crawler->attachObject( ent1 );

	m_spinRate = Ogre::Math::RangeRandom(-360.0, 360.0);

	m_crawlerIndex++;
	
	m_health = 100.0;
}

SceneNode *Crawler::getNode()
{
	return m_crawler;
}
	
void Crawler::update( float dt )
{
	// rotate
	m_crawler->yaw( dt * Degree(m_spinRate) );

	// Move towards the center
	Vector3 dir = -m_crawler->getPosition();
	dir.y = 0.0;
	dir.normalise();	

	if (m_health > 0.0)
	{
		m_crawler->translate( 10.0 * dir * dt);
	}
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    TowerGameApp app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 
        MessageBoxA( NULL, e.what(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occurred: %s\n",
                e.what());
#endif
    }

    return 0;
}