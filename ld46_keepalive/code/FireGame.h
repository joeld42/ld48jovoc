#ifndef LD46_FIREGAME_H
#define LD46_FIREGAME_H

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Dbg/Dbg.h"
#include "Input/Input.h"
#include "Core/Time/Clock.h"

#include "Camera.h"
#include "SceneObject.h"
#include "LD46KeepAliveApp.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

enum ItemType {
	GameItem_LOG,
	GameItem_TORCH,
	
	GameItem_SHRINE,
};
struct GameItem
{
	ItemType _type;
	Tapnik::SceneObject* _obj;
	
	Tapnik::SceneObject* _flame; // for shrines
	float _fuel;
};

class FireGame
{
public:
	void Setup( Tapnik::Scene *scene );
	void fixedUpdate(Oryol::Duration fixedDt);

	float calcLowerGroundHeight(glm::vec3 pos, bool &valid );

	bool PickupWorldItem(int itemNdx);
	void SpawnLogItem();

	void interfaceScreens(Tapnik::UIAssets* uiAssets);
	void DoGameUI_Gameplay(nk_context* ctx, Tapnik::UIAssets* uiAssets);

	Tapnik::Scene* _scene;
	Tapnik::SceneObject* _playerObj;
	Tapnik::SceneObject* _groundObj;
	Tapnik::SceneObject* _holdingObj;
	
	SoundMaker* sfx;

	// Game Item Stuff
	Tapnik::SceneObject* _logObj;
	Tapnik::SceneMesh* _logMesh;
	Tapnik::SceneMesh* _fireMesh;
	Oryol::Array<GameItem> _items;
	int _numLogsSpawned;
	float _spawnLogTimeout;
	int _numShrines;

	Tapnik::Camera* _gameCamera;

	float _cameraTilt;
	float _cameraHeading;
	float _cameraZoom;
	glm::vec3 _cameraHeadingDir;
	glm::vec3 _cameraRightDir;
	glm::vec3 _cameraPos;

	glm::vec3 _playerPos;	
	glm::vec3 _playerStartPos;
	glm::vec3 _playerDir;
	glm::vec3 _playerFacing;
	glm::vec3 _playerFacingTarg;
	float _playerTravel;
	float _playerAngle;

	float _flameAmt;
	float _targFlameAmt;
};

#endif

