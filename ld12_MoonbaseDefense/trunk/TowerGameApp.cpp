#include "Ogre.h"
#include "OgreConfigFile.h"

#include "TowerGameApp.h"
#include "TowerFrameListener.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

// This function will locate the path to our application on OS X,
// unlike windows you can not rely on the curent working directory
// for locating your configuration files and resources.
std::string macBundlePath()
{
	char path[1024];
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	assert(mainBundle);

	CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
	assert(mainBundleURL);

	CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
	assert(cfStringRef);

	CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);

	CFRelease(mainBundleURL);
	CFRelease(cfStringRef);

	return std::string(path);
}
#endif

using namespace Ogre;

TowerGameApp::TowerGameApp()
{
	mFrameListener = 0;
	mRoot = 0;
	
	m_launchTower = NULL;
	m_cursorTurnAmt = 0;
	
	m_money = 100;
	m_currTower = Type_NODULE;

	m_waveTimeleft = 30.0;
	m_waveNum = 1;
	m_nextWaveBugs = 20;

	m_gameOver = false;

	// Provide a nice cross platform solution for locating the configuration files
	// On windows files are searched for in the current working directory, on OS X however
	// you must provide the full path, the helper function macBundlePath does this for us.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
	mResourcePath = macBundlePath() + "/Contents/Resources/";
#else
	mResourcePath = "";
#endif

	m_selTowerNdx = 0;
	m_cubeLaunched = false;	
}

/// Standard destructor
TowerGameApp::~TowerGameApp()
{
	if (mFrameListener)
		delete mFrameListener;
	if (mRoot)
		delete mRoot;
}

/// Start the example
void TowerGameApp::go(void)
{
	if (!setup())
		return;

	mRoot->startRendering();

	// clean up
	destroyScene();
}

void TowerGameApp::selectNextTower()
{
	if (m_selTowerNdx == m_towers.size() -1 )
	{
		doSelectTower(0);
	}
	else
	{
		doSelectTower(m_selTowerNdx+1 );
	}
}

void TowerGameApp::selectPrevTower()
{
	if (m_selTowerNdx == 0)
	{
		doSelectTower( m_towers.size()-1 );
	}
	else
	{
		doSelectTower(m_selTowerNdx-1 );
	}
}

void TowerGameApp::doSelectTower( int ndx )
{
	Vector3 oldTowerPos = m_towers[m_selTowerNdx]->getNode()->getPosition();

	m_selTowerNdx = ndx;
	SceneNode *selTower = m_towers[ndx]->getNode();

	Vector3 towerPos = selTower->getPosition();
	m_cursorNode->setPosition( towerPos + Vector3( 0.0, 0.5, 0.0 ));

	// look at selected
	Vector3 camOffs = mCamera->getPosition() - oldTowerPos;
	mCamera->setPosition( towerPos + camOffs );
	mCamera->lookAt( towerPos );

}

bool TowerGameApp::canAffordBuild()
{
	// can we affort it??
	// BAD: setting cost as a side effect
	switch( m_currTower)
	{
		case Type_NODULE: m_cost = 10; break;
		case Type_GUN: m_cost = 50; break;
		case Type_MISSILE: m_cost = 75; break;
		case Type_FLAMETHROWER: m_cost = 100; break;
	}

	return (m_money > m_cost);
}

void TowerGameApp::launchBuildCube( float power )
{
	// cube already in flite
	if (m_cubeLaunched) return;

	if (!canAffordBuild()) return;
	
	// build it		
	m_money -= m_cost;
	m_builtTower = m_currTower;

	m_buildCube->setVisible( true );
	m_cubeTrail->setVisible( true );
	//m_cubeTrail->resetAllTrails();

	m_launchTower = m_towers[ m_selTowerNdx ];
	SceneNode *selTower = m_launchTower->getNode();

	
	m_buildCube->setPosition( selTower->getPosition() );

	m_cubeVel = m_cursorNode->getOrientation().zAxis();
	
	m_cubeVel.y = 0.0;
	m_cubeVel.normalise();
	
	Vector3 launchVel = m_cubeVel;
	launchVel *= 40.0;
	launchVel.y = 25.0;

	// min vel
	m_cubeVel *= 15.0;
	m_cubeVel.y = 5.0;
	
	m_cubeVel += (launchVel * power);

	m_cubeLaunched = true;
}

bool TowerGameApp::setup(void)
{

	String pluginsPath;
	// only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
	pluginsPath = mResourcePath + "plugins.cfg";
#endif

	mRoot = new Root(pluginsPath, 
		mResourcePath + "ogre.cfg", mResourcePath + "Ogre.log");

	setupResources();

	bool carryOn = configure();
	if (!carryOn) return false;

	chooseSceneManager();
	createCamera();
	createViewports();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Create any resource listeners (for loading screens)
	createResourceListener();
	// Load resources
	loadResources();

	// Create the scene
	createScene();

	createFrameListener();

	return true;

}


bool TowerGameApp::configure(void)
{
	// Show the configuration dialog and initialise the system
	// You can skip this and use root.restoreConfig() to load configuration
	// settings if you were sure there are valid ones saved in ogre.cfg
	if(mRoot->showConfigDialog())
	{
		// If returned true, user clicked OK so initialise
		// Here we choose to let the system create a default rendering window by passing 'true'
		mWindow = mRoot->initialise(true);
		return true;
	}
	else
	{
		return false;
	}
}

void TowerGameApp::chooseSceneManager(void)
{
	// Create the SceneManager, in this case a generic one
	mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "ExampleSMInstance");
}
void TowerGameApp::createCamera(void)
{
	// Create the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");

	// Position it at 500 in Z direction
	mCamera->setPosition(Vector3(0,20,50));        
	mCamera->lookAt(Vector3(0,0,0));

	mCamera->setNearClipDistance(5);

}
void TowerGameApp::createFrameListener(void)
{
	mFrameListener= new TowerFrameListener(this, mWindow, mCamera);
	mFrameListener->showDebugOverlay(true);
	mRoot->addFrameListener(mFrameListener);
}

void TowerGameApp::destroyScene(void){}    // Optional to override this

void TowerGameApp::createViewports(void)
{
	// Create one viewport, entire window
	Viewport* vp = mWindow->addViewport(mCamera);
	vp->setBackgroundColour(ColourValue(0,0,0));

	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(
		Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
}

/// Method which will define the source of resources (other than current folder)
void TowerGameApp::setupResources(void)
{
	// Load resource paths from config file
	ConfigFile cf;
	cf.load(mResourcePath + "resources.cfg");

	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			// OS X does not set the working directory relative to the app,
			// In order to make things portable on OS X we need to provide
			// the loading with it's own bundle path location
			ResourceGroupManager::getSingleton().addResourceLocation(
				String(macBundlePath() + "/" + archName), typeName, secName);
#else
			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
#endif
		}
	}
}

/// Optional override method where you can create resource listeners (e.g. for loading screens)
void TowerGameApp::createResourceListener(void)
{

}

/// Optional override method where you can perform resource group loading
/// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
void TowerGameApp::loadResources(void)
{
	// Initialise, parse scripts etc
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

}

void TowerGameApp::createScene(void)
{

	// Create a skydome
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

	// ligthing
	mSceneMgr->setAmbientLight( ColourValue( 0.1, 0.1, 0.1 ) );
	Vector3 dir(-1, -1, 0.5);
	dir.normalise();

	Light* l = mSceneMgr->createLight("SunLight");
	l->setType(Light::LT_DIRECTIONAL);
	l->setDirection(dir);

	// create some background geo
	//Entity *ent1 = mSceneMgr->createEntity( "GroundShape", "ground.mesh" );
	//SceneNode *nodeGround = mSceneMgr->getRootSceneNode()->createChildSceneNode( "GroundNode" );
	//nodeGround->attachObject( ent1 );

	// Define a floor plane mesh
	Plane p;
	p.normal = Vector3::UNIT_Y;
	p.d = 0;
	MeshManager::getSingleton().createPlane(
            "GroundPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
            p, 200000, 200000, 20, 20, true, 1, 150, 150, Vector3::UNIT_Z);

	// Create an entity (the floor)
	Entity *ent = mSceneMgr->createEntity("floor", "GroundPlane");
	ent->setMaterialName("groundMtl");

	// Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

	Entity *ent1 = mSceneMgr->createEntity( "BuildCube", "build_cube.mesh" );
	m_buildCube = mSceneMgr->getRootSceneNode()->createChildSceneNode( "BuildCubeNode" );
	m_buildCube->attachObject( ent1 );
	m_buildCube->setVisible( false );

	NameValuePairList pairList;
		pairList["numberOfChains"] = "2";
		pairList["maxElements"] = "80";

	m_cubeTrail = static_cast<RibbonTrail*>(
		mSceneMgr->createMovableObject("1", "RibbonTrail", &pairList));
	m_cubeTrail->setMaterialName("Examples/LightRibbonTrail");
	m_cubeTrail->setTrailLength(400);
	
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject( m_cubeTrail );

	m_cubeTrail->setInitialColour(0, 1.0, 0.8, 0);
	m_cubeTrail->setColourChange(0, 0.5, 0.5, 0.5, 0.5);
	m_cubeTrail->setInitialWidth(0, 5);
	m_cubeTrail->addNode( m_buildCube );

	m_cubeTrail->setVisible( false );

	// Make the starting tower
	Tower *baseTower = new BaseTower();
	baseTower->Build( mSceneMgr);
	m_towers.push_back( baseTower );

#if 0
	// make some random weapon towers
	for ( int i=0; i < 10; i++) 
	{
		// create a test weapon tower
		Tower *weaponTower = new WeaponTower( WeaponTower::Type_GUN );
		weaponTower->Build( mSceneMgr );
		m_towers.push_back( weaponTower );
	
		SceneNode *n = weaponTower->getNode();
		
		Vector3 towerPos = Vector3( 0.0, 0.0, 0.0 );
		towerPos.x = Ogre::Math::RangeRandom(-100.0, 100.0);
		towerPos.z = Ogre::Math::RangeRandom(-100.0, 100.0);
		n->setPosition( towerPos );
	}
#endif

	// make some random enemies
	//attackWaveCrawlers();

	// set up "cursor"
	//m_cursorNode = mSceneMgr->getRootSceneNode()->createChildSceneNode( "CursorNode", Vector3( 0.0, 5.0, 0.0) );
	//m_cursorSet = mSceneMgr->createBillboardSet("CursorSet");
	//m_cursorSet->setBillboardType( BBT_ORIENTED_COMMON );
	//m_cursorSet->setDefaultDimensions( 5.0, 5.0 );
	//m_cursorSet->setCommonDirection( Vector3( 0.0, -1.0, 0.0 ) );
	//m_cursorSet->setMaterialName("Tower/Cursor");
	//m_cursorNode->attachObject( m_cursorSet );
	//m_cursorBB = m_cursorSet->createBillboard( Vector3( 0.0, 1.0, 0.0 ) );

	MeshManager::getSingleton().createPlane(
            "Cursor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            p, 20.0, 20.0, 1, 1, true, 1, 1, 1, Vector3::UNIT_Z);
	Entity *entCursor = mSceneMgr->createEntity("cursor", "Cursor");
	entCursor->setMaterialName("Tower/Cursor");

	m_cursorNode = mSceneMgr->getRootSceneNode()->createChildSceneNode( "CursorNode", 
		Vector3( 0.0, 0.5, 0.0) );
	m_cursorNode->attachObject(entCursor);

	//m_cursorBB->setColour(ColourValue::Black);
	
#if 0
	// Okay placeholder stuff... make the level
	Vector3 towerLocs[5];
	towerLocs[0] = Vector3( -429, 0, -123 );
	towerLocs[1] = Vector3(-6, 0, -379 );
	towerLocs[2] = Vector3( 371, 0, -100 );
	towerLocs[3] = Vector3( 198, 0, 358 );
	towerLocs[4] = Vector3(-300, 0, 345 );

	for (int i=0; i < 5; i++) 
	{		
		float height = 0.0;
		for (int level=0; level < 4; level++)
		{
			char name[1024], nodeName[1024];
			sprintf( name, "Tower%d_lvl%dShape", i, level );
			sprintf( nodeName, "Tower%d_lvl%d", i, level );


			Entity *ent2 = NULL;
			int part = rand() % 3;
			if (part==0)
			{
				ent2 = mSceneMgr->createEntity( name, "part_tall.mesh" );
			}
			else if (part==1)
			{
				ent2 = mSceneMgr->createEntity( name, "part_medium.mesh" );
			}
			else
			{
				ent2 = mSceneMgr->createEntity( name, "part_short_unshaded.mesh" );
			}

			Vector3 partLoc = towerLocs[i] + Vector3( 0.0, height, 0.0 );
			SceneNode *node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode( nodeName, partLoc);
			node2->attachObject( ent2 );

			const AxisAlignedBox &bbox = ent2->getWorldBoundingBox( true );
			height += bbox.getSize().y;
		}
	}
#endif

	// throw in some fog
	//mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);

}

void TowerGameApp::updateStuff( const FrameEvent& evt)
{
	// update cursor
	m_cursorNode->yaw( Degree(180.0) * evt.timeSinceLastFrame * m_cursorTurnAmt );

	// update build cube
	if (m_cubeLaunched)
	{
		Vector3 cubePos = m_buildCube->getPosition();
		cubePos += m_cubeVel * evt.timeSinceLastFrame;
		m_cubeVel.y += -9.8 * evt.timeSinceLastFrame;

		m_buildCube->setPosition( cubePos );
		if (cubePos.y < 0.0)
		{
			m_cubeLaunched = false;
			m_buildCube->setVisible( false );
			m_cubeTrail->setVisible( false );

			// create a new weapon tower
			Tower *weaponTower = new WeaponTower( m_builtTower );
			weaponTower->Build( mSceneMgr );
			m_towers.push_back( weaponTower );
	
			SceneNode *n = weaponTower->getNode();
		
			Vector3 towerPos = Vector3( 0.0, 0.0, 0.0 );
			towerPos.x = cubePos.x;
			towerPos.z = cubePos.z;
			n->setPosition( towerPos );

			m_launchTower->connectSubtower( mSceneMgr, weaponTower );
			m_launchTower = NULL;
			//doSelectTower( m_towers.size() -1 );
		}
	}

	// Is it time for an onslaught?
	m_waveTimeleft -= evt.timeSinceLastFrame;
	if (m_waveTimeleft <= 0.0)
	{
		attackWaveCrawlers( m_nextWaveBugs);

		m_waveNum++;
		m_nextWaveBugs = 20 + (10.0 * m_waveNum);

		m_waveTimeleft = 30.0;
	}

	// Update towers
	for (std::vector<Tower*>::iterator ti = m_towers.begin();
		ti != m_towers.end(); ++ti )
	{
		SceneNode *twrNode = (*ti)->getNode();
		Vector3 tPos = twrNode->getPosition();

		// find the closest enemy -- probably a much more ogre-riffic 
		// way to do this
		Crawler *closest = NULL;
		float dMin = -1.0;		

		for (std::vector<Crawler*>::iterator ci = m_crawlers.begin();
			ci != m_crawlers.end(); ++ci )
		{
			Vector3 cPos = (*ci)->getNode()->getPosition();
			float d2 = cPos.squaredDistance( tPos );

			if ((dMin < 0.0) || (d2 < dMin))
			{
				dMin = d2;
				closest = (*ci);
			}
		}

		// update the tower
		(*ti)->update( closest, sqrt( dMin ), evt.timeSinceLastFrame, this );
		
	}	

	// Update the crawlers
	std::vector<Crawler*> liveCrawlers;
	for (std::vector<Crawler*>::iterator ci = m_crawlers.begin();
			ci != m_crawlers.end(); ++ci )
	{

		// Did this crawler git us?
		if ( (*ci)->getNode()->getPosition().length() < 2.0)
		{
			m_gameOver = true;
		}

		// Is this crawler dead??
		if ((*ci)->m_health <= 0.001 )
		{
			// clean up
			mSceneMgr->destroyMovableObject( (*ci)->getNode()->detachObject( (unsigned short)0 ) );
			(*ci)->getNode()->removeAndDestroyAllChildren();
			delete (*ci);

			// Get some cash
			m_money += 10;

			// TODO: nice big explosion
		}
		else
		{
			// nope, keep on truckin
			(*ci)->update( evt.timeSinceLastFrame );
			liveCrawlers.push_back( (*ci) );
		}
	}
	m_crawlers = liveCrawlers;
}


void TowerGameApp::attackWaveCrawlers( int numBugs )
{
	int numCrawlers = Ogre::Math::RangeRandom( 20, 30);
	Vector3 waveCenter;

	waveCenter.x = Ogre::Math::RangeRandom(-100.0, 100.0);
	waveCenter.y = 0.0f;
	waveCenter.z = Ogre::Math::RangeRandom(-100.0, 100.0);
	waveCenter.normalise();
	waveCenter *= 500.0;

	for ( int i=0; i < numBugs; i++) 
	{
		Crawler *crawler = new Crawler( mSceneMgr );

		SceneNode *n = crawler->getNode();
		
		Vector3 crawlerPos = Vector3( 0.0, 0.0, 0.0 );		
		crawlerPos.x = Ogre::Math::RangeRandom(-50.0, 50.0);
		crawlerPos.z = Ogre::Math::RangeRandom(-50.0, 50.0);		
		crawlerPos.y = 1.0;
		crawlerPos += waveCenter;

		n->setPosition( crawlerPos );	

		m_crawlers.push_back( crawler );
	}

}
