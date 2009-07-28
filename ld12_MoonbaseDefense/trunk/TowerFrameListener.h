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

#ifndef __TowerFrameListener_H__
#define __TowerFrameListener_H__

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS/OIS.h>

class TowerGameApp;

using namespace Ogre;

class TowerFrameListener: public FrameListener, public WindowEventListener
{
protected:
	void updateStats(void);

public:
	// Constructor takes a RenderWindow because it uses that to determine input context
	TowerFrameListener( TowerGameApp *game, RenderWindow* win, Camera* cam, 
						bool bufferedKeys = false, bool bufferedMouse = false,
						bool bufferedJoy = false );

	//Adjust mouse clipping area
	virtual void windowResized(RenderWindow* rw);	

	//Unattach OIS before window shutdown (very important under Linux)
	virtual void windowClosed(RenderWindow* rw);	

	virtual ~TowerFrameListener();	

	virtual bool processUnbufferedKeyInput(const FrameEvent& evt);
	
	bool processUnbufferedMouseInput(const FrameEvent& evt);	

	void moveCamera();
	
	void showDebugOverlay(bool show);	

	// Override frameStarted event to process that (don't care about frameEnded)
	bool frameStarted(const FrameEvent& evt);	
	bool frameEnded(const FrameEvent& evt);


//protected:
	Camera* mCamera;

	Vector3 mTranslateVector;
	Vector3 mScrollMapVector;
	RenderWindow* mWindow;
	bool mStatsOn;

	std::string mDebugText;

	unsigned int mNumScreenShots;
	float mMoveScale;
	Degree mRotScale;
	// just to stop toggles flipping too fast
	Real mTimeUntilNextToggle ;
	Radian mRotX, mRotY;
	TextureFilterOptions mFiltering;
	int mAniso;

	int mSceneDetailIndex ;
	Real mMoveSpeed;
	Degree mRotateSpeed;
	Overlay* mDebugOverlay;
	Overlay* m_gameInfoOverlay;
	Overlay* m_gameOverOverlay;

	float m_power;
	bool m_charging;
	float m_chargeSgn;

	// heheh.. ganked the loading bar for a power meter
	Overlay *m_powerOverlay;

	//OIS Input devices
	OIS::InputManager* mInputManager;
	OIS::Mouse*    mMouse;
	OIS::Keyboard* mKeyboard;
	OIS::JoyStick* mJoy;

	// The game
	TowerGameApp *m_theGame;
};

#endif
