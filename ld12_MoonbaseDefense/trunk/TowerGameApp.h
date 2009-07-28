/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    TowerGameApp.h
Description: Base class for all the OGRE examples
-----------------------------------------------------------------------------
*/

#ifndef __TowerGameApp_H__
#define __TowerGameApp_H__

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "TowerFrameListener.h"

#include "TowerGame.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

// This function will locate the path to our application on OS X,
// unlike windows you can not rely on the curent working directory
// for locating your configuration files and resources.
std::string macBundlePath();

#endif

using namespace Ogre;

/** Base class which manages the standard startup of an Ogre application.
    Designed to be subclassed for specific examples if required.
*/
class TowerGameApp
{
public:
    /// Standard constructor
    TowerGameApp();
    
    /// Standard destructor
    virtual ~TowerGameApp();    

    /// Start the example
    virtual void go(void);    

	// gameplay stuff
	virtual void updateStuff( const FrameEvent& evt);

	// sends a wave of crawlers in
	void attackWaveCrawlers( int numBugs );

	void selectNextTower();
	void selectPrevTower();
	void doSelectTower( int ndx );

	void launchBuildCube( float power );

	bool canAffordBuild();

	float m_cursorTurnAmt;

	// build stuff
	int m_money;
	int m_cost;
	TowerType m_currTower;

//protected:
    Root *mRoot;
    Camera* mCamera;
    SceneManager* mSceneMgr;
    TowerFrameListener* mFrameListener;
    RenderWindow* mWindow;
	Ogre::String mResourcePath;

	// Gameplay stuff
	std::vector<Tower*> m_towers;
	std::vector<Crawler*> m_crawlers;
	
	// selected tower	
	SceneNode *m_cursorNode;
	int m_selTowerNdx;	
	
	// next attack
	float m_waveTimeleft;
	int m_waveNum;
	int m_nextWaveBugs;
	bool m_gameOver;

	// the build cube
	SceneNode *m_buildCube;
	Vector3 m_cubeVel;
	bool m_cubeLaunched;
	Tower *m_launchTower;
	RibbonTrail *m_cubeTrail;
	TowerType m_builtTower;



    // These internal methods package up the stages in the startup process
    /** Sets up the application - returns false if the user chooses to abandon configuration. */
    virtual bool setup(void);
    
    /** Configures the application - returns false if the user chooses to abandon configuration. */
    virtual bool configure(void);

    virtual void chooseSceneManager(void);

    virtual void createCamera(void);
    
    virtual void createFrameListener(void);

    virtual void createScene(void);

    virtual void destroyScene(void);    // Optional to override this

    virtual void createViewports(void);
    

    /// Method which will define the source of resources (other than current folder)
    virtual void setupResources(void);
    
	/// Optional override method where you can create resource listeners (e.g. for loading screens)
	virtual void createResourceListener(void);
	
	/// Optional override method where you can perform resource group loading
	/// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	virtual void loadResources(void);



};


#endif
