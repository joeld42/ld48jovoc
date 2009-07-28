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
Filename:    TowerFrameListener.h
Description: Defines an example frame listener which responds to frame events.
This frame listener just moves a specified camera around based on
keyboard and mouse movements.
Mouse:    Freelook
W or Up:  Forward
S or Down:Backward
A:        Step left
D:        Step right
PgUp:     Move upwards
PgDown:   Move downwards
F:        Toggle frame rate stats on/off
R:        Render mode
T:        Cycle texture filtering
Bilinear, Trilinear, Anisotropic(8)
P:        Toggle on/off display of camera position / orientation
-----------------------------------------------------------------------------
*/

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "TowerFrameListener.h"
#include "TowerGameApp.h"

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS/OIS.h>

using namespace Ogre;

void TowerFrameListener::updateStats(void)
{
	static String currFps = "Current FPS: ";
	static String avgFps = "Average FPS: ";
	static String bestFps = "Best FPS: ";
	static String worstFps = "Worst FPS: ";
	static String tris = "Triangle Count: ";
	static String batches = "Batch Count: ";

	static String money = "Money: ";
	static String angle = "Angle: ";
	static String build = "Build: ";
	static String nextwave;	

	// update stats when necessary
	try {
		OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
		OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
		OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
		OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

		const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
		guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
		guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
			+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
			+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

		OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
		guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

		OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
		guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

		OverlayElement* guiMoney = OverlayManager::getSingleton().getOverlayElement("Tower/Money");
		guiMoney->setCaption(money + StringConverter::toString( m_theGame->m_money ));

		OverlayElement* guiAngle = OverlayManager::getSingleton().getOverlayElement("Tower/Angle");
		Radian yaw = m_theGame->m_cursorNode->getOrientation().getYaw();
		guiAngle ->setCaption(angle + StringConverter::toString( yaw ));

		OverlayElement* guiBuild = OverlayManager::getSingleton().getOverlayElement("Tower/Build");
		String buildCaption = build;
		if (m_theGame->m_currTower == Type_NODULE)
		{
			buildCaption = buildCaption + "Nodule (10)";
		}
		else if (m_theGame->m_currTower == Type_GUN)
		{
			buildCaption = buildCaption + "Gun Tower (50)";
		}
		else if (m_theGame->m_currTower == Type_MISSILE)
		{
			buildCaption = buildCaption + "Missile Tower (75)";
		}
		else if (m_theGame->m_currTower == Type_FLAMETHROWER)
		{
			buildCaption = buildCaption + "Flamethrower (100)";
		}
		guiBuild ->setCaption( buildCaption );

		OverlayElement* guiNextWave = OverlayManager::getSingleton().getOverlayElement("Tower/NextWave");
		char next[100];
		sprintf( next, "%3.0f until wave #%d (%d bugs)", 
					m_theGame->m_waveTimeleft,
					m_theGame->m_waveNum,
					m_theGame->m_nextWaveBugs );
		nextwave = next;
		guiNextWave->setCaption( nextwave );

		OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
		guiDbg->setCaption(mDebugText);
			
	}
	catch(...) { /* ignore */ }
}

// Constructor takes a RenderWindow because it uses that to determine input context
TowerFrameListener::TowerFrameListener( TowerGameApp *game, RenderWindow* win, Camera* cam, 
									   bool bufferedKeys, bool bufferedMouse ,
									   bool bufferedJoy ) :
mCamera(cam), 
mTranslateVector(Vector3::ZERO),
mScrollMapVector(Vector3::ZERO),
mWindow(win), mStatsOn(true), mNumScreenShots(0),
mMoveScale(0.0f), mRotScale(0.0f), mTimeUntilNextToggle(0), mFiltering(TFO_BILINEAR),
mAniso(1), mSceneDetailIndex(0), mMoveSpeed(100), mRotateSpeed(36), mDebugOverlay(0),
mInputManager(0), mMouse(0), mKeyboard(0), mJoy(0), m_theGame( game ), m_gameInfoOverlay( NULL)
{
	using namespace OIS;

	mDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	m_gameInfoOverlay = OverlayManager::getSingleton().getByName("Tower/GameInfo");
	m_gameInfoOverlay->show();

	m_powerOverlay = OverlayManager::getSingleton().getByName("Tower/PowerOverlay");
	
	m_gameOverOverlay = OverlayManager::getSingleton().getByName("Tower/GameOver");
	m_gameOverOverlay->hide();
	//if (m_powerOverlay) m_powerOverlay->show();

	LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	m_power = 0;
	m_charging = false;

	win->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	mInputManager = InputManager::createInputSystem( pl );

	//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
	mKeyboard = static_cast<Keyboard*>(mInputManager->createInputObject( OISKeyboard, bufferedKeys ));
	mMouse = static_cast<Mouse*>(mInputManager->createInputObject( OISMouse, bufferedMouse ));
	try {
		mJoy = static_cast<JoyStick*>(mInputManager->createInputObject( OISJoyStick, bufferedJoy ));
	}
	catch(...) {
		mJoy = 0;
	}

	//Set initial mouse clipping size
	windowResized(mWindow);

	showDebugOverlay(true);

	//Register as a Window listener
	WindowEventUtilities::addWindowEventListener(mWindow, this);
}

//Adjust mouse clipping area
void TowerFrameListener::windowResized(RenderWindow* rw)
{
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void TowerFrameListener::windowClosed(RenderWindow* rw)
{
	//Only close for window that created OIS (the main window in these demos)
	if( rw == mWindow )
	{
		if( mInputManager )
		{
			mInputManager->destroyInputObject( mMouse );
			mInputManager->destroyInputObject( mKeyboard );
			mInputManager->destroyInputObject( mJoy );

			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}
	}
}

TowerFrameListener::~TowerFrameListener()
{
	//Remove ourself as a Window listener
	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
}

bool TowerFrameListener::processUnbufferedKeyInput(const FrameEvent& evt)
{
	using namespace OIS;

	if(mKeyboard->isKeyDown(KC_A))
		mTranslateVector.x = -mMoveScale;	// Move camera left

	if(mKeyboard->isKeyDown(KC_D))
		mTranslateVector.x = mMoveScale;	// Move camera RIGHT

	if( mKeyboard->isKeyDown(KC_W) )
		mTranslateVector.z = -mMoveScale;	// Move camera forward

	if( mKeyboard->isKeyDown(KC_S) )
		mTranslateVector.z = mMoveScale;	// Move camera backward

	if(mKeyboard->isKeyDown(KC_PGUP))
		mTranslateVector.y = mMoveScale;	// Move camera up

	if(mKeyboard->isKeyDown(KC_PGDOWN))
		mTranslateVector.y = -mMoveScale;	// Move camera down

	//if(mKeyboard->isKeyDown(KC_RIGHT))
	//	mCamera->yaw(-mRotScale);

	//if(mKeyboard->isKeyDown(KC_LEFT))
	//	mCamera->yaw(mRotScale);

	if( mKeyboard->isKeyDown(KC_ESCAPE) || mKeyboard->isKeyDown(KC_Q) )
		return false;

	// Charge launcher
	if (mKeyboard->isKeyDown(KC_SPACE) && (!m_charging) && (m_theGame->canAffordBuild())
		&& (!m_theGame->m_cubeLaunched) && (!m_theGame->m_gameOver) )
	{
		m_power = 0.0;
		m_chargeSgn = 1.0;
		m_powerOverlay->show();

		m_charging = true;
	}
	else if (!mKeyboard->isKeyDown(KC_SPACE) && (m_charging) )
	{
		m_theGame->launchBuildCube( m_power );
		m_powerOverlay->hide();

		m_charging = false;
	}

	// DBG: Laucnch a wave of crawlers
	if( mKeyboard->isKeyDown(KC_C) && mTimeUntilNextToggle <= 0 )
	{
		m_theGame->attackWaveCrawlers( 30);
		mTimeUntilNextToggle = 2;
	}

	// DBG: Cheat: get monies
	if( mKeyboard->isKeyDown(KC_M) && mTimeUntilNextToggle <= 0 )
	{
		m_theGame->m_money = 1000;
		mTimeUntilNextToggle = 2;
	}

	

	// pick towers
	if( mKeyboard->isKeyDown(KC_1))
	{
		m_theGame->m_currTower = Type_NODULE;		
	}
	if( mKeyboard->isKeyDown(KC_2))
	{
		m_theGame->m_currTower = Type_GUN;		
	}
	if( mKeyboard->isKeyDown(KC_3))
	{
		m_theGame->m_currTower = Type_MISSILE;		
	}
	if( mKeyboard->isKeyDown(KC_4))
	{
		m_theGame->m_currTower = Type_FLAMETHROWER;		
	}
	

	if( mKeyboard->isKeyDown(KC_F) && mTimeUntilNextToggle <= 0 )
	{
		mStatsOn = !mStatsOn;
		showDebugOverlay(mStatsOn);
		mTimeUntilNextToggle = 1;
	}

	if( mKeyboard->isKeyDown(KC_T) && mTimeUntilNextToggle <= 0 )
	{
		switch(mFiltering)
		{
		case TFO_BILINEAR:
			mFiltering = TFO_TRILINEAR;
			mAniso = 1;
			break;
		case TFO_TRILINEAR:
			mFiltering = TFO_ANISOTROPIC;
			mAniso = 8;
			break;
		case TFO_ANISOTROPIC:
			mFiltering = TFO_BILINEAR;
			mAniso = 1;
			break;
		default: break;
		}
		MaterialManager::getSingleton().setDefaultTextureFiltering(mFiltering);
		MaterialManager::getSingleton().setDefaultAnisotropy(mAniso);

		showDebugOverlay(mStatsOn);
		mTimeUntilNextToggle = 1;
	}

	if(mKeyboard->isKeyDown(KC_SYSRQ) && mTimeUntilNextToggle <= 0)
	{
		std::ostringstream ss;
		ss << "screenshot_" << ++mNumScreenShots << ".png";
		mWindow->writeContentsToFile(ss.str());
		mTimeUntilNextToggle = 0.5;
		mDebugText = "Saved: " + ss.str();
	}

	if(mKeyboard->isKeyDown(KC_R) && mTimeUntilNextToggle <=0)
	{
		mSceneDetailIndex = (mSceneDetailIndex+1)%3 ;
		switch(mSceneDetailIndex) {
				case 0 : mCamera->setPolygonMode(PM_SOLID); break;
				case 1 : mCamera->setPolygonMode(PM_WIREFRAME); break;
				case 2 : mCamera->setPolygonMode(PM_POINTS); break;
		}
		mTimeUntilNextToggle = 0.5;
	}

	static bool displayCameraDetails = false;
	if(mKeyboard->isKeyDown(KC_P) && mTimeUntilNextToggle <= 0)
	{
		displayCameraDetails = !displayCameraDetails;
		mTimeUntilNextToggle = 0.5;
		if (!displayCameraDetails)
			mDebugText = "";
	}
	
	// tower cursor
	if( mKeyboard->isKeyDown(KC_UP) && mTimeUntilNextToggle <= 0 )
	{
		m_theGame->selectNextTower();
		mTimeUntilNextToggle = 0.3;
	}

	if( mKeyboard->isKeyDown(KC_DOWN) && mTimeUntilNextToggle <= 0 )
	{
		m_theGame->selectPrevTower();
		mTimeUntilNextToggle = 0.3;
	}

	// rotate cursor
	if (( mKeyboard->isKeyDown(KC_LEFT)) && ( !mKeyboard->isKeyDown(KC_RIGHT)) )
	{
		m_theGame->m_cursorTurnAmt = -1.0;
	}
	else if (( !mKeyboard->isKeyDown(KC_LEFT)) && ( mKeyboard->isKeyDown(KC_RIGHT)) )
	{
		m_theGame->m_cursorTurnAmt = 1.0;
	}
	else
	{
		m_theGame->m_cursorTurnAmt = 0.0;
	}

	// Print camera details
	if(displayCameraDetails)
		mDebugText = "P: " + StringConverter::toString(mCamera->getDerivedPosition()) +
		" " + "O: " + StringConverter::toString(mCamera->getDerivedOrientation());

	// Return true to continue rendering
	return true;
}

bool TowerFrameListener::processUnbufferedMouseInput(const FrameEvent& evt)
{
	using namespace OIS;

	// Rotation factors, may not be used if the second mouse button is pressed
	// 2nd mouse button - slide, otherwise rotate
	const MouseState &ms = mMouse->getMouseState();
	if( !ms.buttonDown( MB_Right ) )
	{
		mScrollMapVector.x += ms.X.rel * 0.13;
		mScrollMapVector.z -= ms.Y.rel * 0.13;
	}
	else
	{
		mRotX = Degree(-ms.X.rel * 0.13);
		mRotY = Degree(-ms.Y.rel * 0.13);
	}

	return true;
}

void TowerFrameListener::moveCamera()
{
	// Make all the changes to the camera
	// Note that YAW direction is around a fixed axis (freelook style) rather than a natural YAW
	//(e.g. airplane)
	mCamera->yaw(mRotX);
	mCamera->pitch(mRotY);
	mCamera->moveRelative(mTranslateVector);

	// Move in XZ
	Vector3 camDir = mCamera->getDirection();
	Vector3 upDir = Vector3( 0.0, 1.0, 0.0 );
	Vector3 rightDir = camDir.crossProduct( upDir );
	rightDir.y = 0.0;
	rightDir.normalise();

	camDir = upDir.crossProduct( rightDir );
	camDir.normalise();


	mCamera->move( (rightDir * mScrollMapVector.x) + 
		(camDir * mScrollMapVector.z) );


	// Camera limits
	Vector3 camPos = mCamera->getPosition();
	if ( camPos.y < 5.0 )
	{
	//	camPos.y = 5.0;
	}
	if (camPos.y > 100.0)
	{
		camPos.y = 100.0;		
	}
	mCamera->setPosition( camPos );
} 

void TowerFrameListener::showDebugOverlay(bool show)
{
	if (mDebugOverlay)
	{
		if (show)
			mDebugOverlay->show();
		else
			mDebugOverlay->hide();
	}
}

// Override frameStarted event to process that (don't care about frameEnded)
bool TowerFrameListener::frameStarted(const FrameEvent& evt)
{
	using namespace OIS;

	if(mWindow->isClosed())	return false;

	//Need to capture/update each device
	mKeyboard->capture();
	mMouse->capture();
	if( mJoy ) mJoy->capture();

	bool buffJ = (mJoy) ? mJoy->buffered() : true;

	//Check if one of the devices is not buffered
	if( !mMouse->buffered() || !mKeyboard->buffered() || !buffJ )
	{
		// one of the input modes is immediate, so setup what is needed for immediate movement
		if (mTimeUntilNextToggle >= 0)
			mTimeUntilNextToggle -= evt.timeSinceLastFrame;

		// Move about 100 units per second
		mMoveScale = mMoveSpeed * evt.timeSinceLastFrame;
		// Take about 10 seconds for full rotation
		mRotScale = mRotateSpeed * evt.timeSinceLastFrame;

		mRotX = 0;
		mRotY = 0;
		mScrollMapVector = Ogre::Vector3::ZERO;			
		mTranslateVector = Ogre::Vector3::ZERO;			
	}

	//Check to see which device is not buffered, and handle it
	if( !mKeyboard->buffered() )
		if( processUnbufferedKeyInput(evt) == false )
			return false;
	if( !mMouse->buffered() )
		if( processUnbufferedMouseInput(evt) == false )
			return false;

	if( !mMouse->buffered() || !mKeyboard->buffered() || !buffJ )
		moveCamera();

	// Update game stuff
	if (m_charging)
	{
		m_power += evt.timeSinceLastFrame * m_chargeSgn;
		if (m_power > 1.0)
		{
			m_power = 1.0;
			m_chargeSgn = -1.0;
		}
		else if (m_power < 0.0)
		{
			m_power = 0.0;
			m_chargeSgn = 1.0;
		}

		OverlayElement* powerBar = OverlayManager::getSingleton().getOverlayElement("Tower/PowerPanel/Bar/Progress");
		OverlayElement* barContainer = OverlayManager::getSingleton().getOverlayElement("Tower/PowerPanel/Bar");

		powerBar->setWidth( m_power * barContainer->getWidth() );		
	}
	

	if (!m_theGame->m_gameOver)
	{		
		m_theGame->updateStuff( evt );
	}
	else
	{
		m_gameOverOverlay->show();
	}

	return true;
}

bool TowerFrameListener::frameEnded(const FrameEvent& evt)
{
	updateStats();
	return true;
}


