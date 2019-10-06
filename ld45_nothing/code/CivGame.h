#ifndef LD45_CIVGAME_H
#define LD45_CIVGAME_H

#include "Core/Types.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include "Core/Time/Clock.h"

#include "Camera.h"
#include "SceneObject.h"
#include "UIAssets.h"
#include "Ray.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"


#define BOARD_SZ (9)

#define BUILDING_PROPERTY( T, name, default ) \
	T name = default; \
	BuildingInfo with##name##( T _##name## ) { ##name## = _##name##; return *this; }

class BuildingInfo
{
public:
	BuildingInfo::BuildingInfo(Oryol::String _bldgName,
		Oryol::String _desc);

	Oryol::String bldgName;
	Oryol::String desc;

	BUILDING_PROPERTY(uint64_t, BaseCost, 10);
	BUILDING_PROPERTY(double, CostMultiplier, 1.15 );
	BUILDING_PROPERTY(int, BuildLimit, 10 );
	BUILDING_PROPERTY(int, BaseFood, 0);
	BUILDING_PROPERTY(double, FoodMultiplier, 1.0);
	BUILDING_PROPERTY(double, HarvestTime, -1.0 );	

	// these are just cached for the current focus building
	int countOnFocusHex = 0;
};

class Building
{
public:
	Building(BuildingInfo* info) :info(info) {}
	
	BuildingInfo* info;
	Tapnik::SceneObject* sceneObj;
	float cooldown;
};

class GameHex
{
public:
	GameHex(int hx, int hy);
	int hx;
	int hy;

	float bobTime;
	float bobRate;

	glm::vec3 hexWorldPos; // unmodified board pos
	glm::vec3 targetPos;   // target pos with bounces, etc
	glm::vec3 pos;  // actual current pos
	float hoverBump;
	glm::vec2 screenPos;
	Tapnik::SceneObject* sceneObj;
	char hexName[12];
	float clickBounce;

	// Gameplay stuff
	int baseFood; // Some tiles naturally produce food
	int farmCount;

	float harvestWait;

	Oryol::Array<Building*> bldgs;
};

class CivGame
{
public:
	CivGame();
	void SetupWithScene(Tapnik::Scene* scene);

	Tapnik::SceneMesh* BuildHexMesh( float hexSize );

	void SetupGameBoard();

	void fixedUpdate(Oryol::Duration fixedDt, Tapnik::Camera* activeCamera);
	void dynamicUpdate(Oryol::Duration frameDt, Tapnik::Camera* activeCamera);

	void ClickForTheGloryOfTheEmpire(GameHex* hex);
	void SetFocusHex(GameHex* hex);

	void interfaceScreens(Tapnik::UIAssets* uiAssets);
	void DoGameUI_Gameplay(nk_context* ctx, Tapnik::UIAssets* uiAssets);

	int boardNdx(int i, int j);
	GameHex *boardHex(int i, int j);
	float CalcIconOffset(GameHex* hex);

	void UpdateGameSystem(float dt);
	void HarvestHex(GameHex* hex, bool actualClick);
	void UpdateHex(float dt, GameHex* hex);
	void BuildBuilding(GameHex* hex, BuildingInfo* info);
	void CountBuildings(GameHex* hex);

	// Gameplay info
	Oryol::Array<BuildingInfo> bldgSpecs;
	uint64_t resFood = 0;
	uint64_t resWealth =0;
	uint64_t resScience =0;
	uint64_t resMight =0;

	// UI stuff
	Tapnik::UIAssets* uiAssets; 

	Tapnik::Scene* scene;
	Oryol::Id testTex;
	GameHex *board[BOARD_SZ*BOARD_SZ];

	float gameTime;

	GameHex* hoverHex;
	GameHex* focusHex;

	Tapnik::Ray mouseRay;
	glm::vec3 groundCursor;

	glm::vec3 camFocusPos;
	float camRotato;
	glm::vec2 camRiseDist;

	// for detecting long-press
	bool mouseDown;
	float mouseDownTime;
	bool didTriggerLongPress;
};

#endif