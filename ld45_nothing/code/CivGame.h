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

#include "Sounds.h"

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"


#define BOARD_SZ (9)

#define BUILDING_PROPERTY( T, name, default ) \
	T name = default; \
	BuildingInfo with##name##( T _##name## ) { ##name## = _##name##; return *this; }

// Same but no default
#define BUILDING_PROPERTY_ND( T, name ) \
	T name; \
	BuildingInfo with##name##( T _##name## ) { ##name## = _##name##; return *this; }


enum {
	Type_EXPLORE,
	Type_RESOURCE,
	Type_RESOURCE_OCEAN,
	
	Type_ENEMY,
	Type_STARTING_RES
};

class BuildingInfo
{
public:
	BuildingInfo::BuildingInfo(Oryol::String _bldgName,
		Oryol::String _desc);

	Oryol::String bldgName;
	Oryol::String desc;

	BUILDING_PROPERTY_ND(Oryol::String, ModelName );
	BUILDING_PROPERTY(glm::vec4, TintColor, glm::vec4(1.0f) );

	BUILDING_PROPERTY(uint64_t, BaseCost, 10);
	BUILDING_PROPERTY(double, CostMultiplier, 1.15 );
	BUILDING_PROPERTY(int, BuildLimit, 10 );
	BUILDING_PROPERTY(int, Type, Type_RESOURCE );
	BUILDING_PROPERTY(int, BaseFood, 0);
	BUILDING_PROPERTY(int, BaseExplore, 0);
	BUILDING_PROPERTY(double, ExploreMult, 1.0);
	BUILDING_PROPERTY(double, FoodMultiplier, 1.0);
	BUILDING_PROPERTY(double, CooldownTime, -1.0 );	

	// these are just cached for the current focus building
	int countOnFocusHex = 0;

	Tapnik::SceneMesh* mesh;
};

class Building
{
public:
	Building(BuildingInfo* info) :info(info), sceneObj(NULL) {}
	
	BuildingInfo* info;
	Tapnik::SceneObject* sceneObj;	
	float cooldown;

	glm::vec3 localPos;
	glm::quat localRot;
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
	Tapnik::SceneObject* hexBackObj;
	char hexName[12];
	float clickBounce;
	
	float flipAmountTarg;
	float flipAmount;
	glm::vec3 flipAxis;

	enum TerrainType {
		Terrain_OCEAN,
		Terrain_GRASSLAND,
		Terrain_FOREST,
		Terrain_DESERT,
		Terrain_MOUNTAIN,

		NUM_TERRAIN
	};

	// Gameplay stuff
	uint64_t exploreCountStart;
	uint64_t exploreCount;
	TerrainType terrain;	

	// how much has this been damaged
	uint64_t damageCount;

	// stats (can be recalcuated from buildings)
	uint64_t stat_foodProduction;
	uint64_t stat_foodMultiplier;
	uint64_t stat_totalFoodHarvest;
	bool hasPlayerBuiltFood;
	// TODO: Food per second

	uint64_t stat_baseExplore;
	uint64_t stat_explorerCount;
	uint64_t stat_exploreMultiplier;
	uint64_t stat_totalExplore;

	uint64_t stat_numEnemies;
	uint64_t stat_totalEnemyStr;


	Oryol::Array<Building*> bldgs;
};

class CivGame
{
public:
	CivGame();
	void SetupWithScene(Tapnik::Scene* scene);

	Tapnik::SceneMesh* BuildHexMesh( float hexSize );

	void SetupGameBoard();
	void HandleInput(float dt, Tapnik::Camera* activeCamera);

	void fixedUpdate(Oryol::Duration fixedDt, Tapnik::Camera* activeCamera);
	void dynamicUpdate(Oryol::Duration frameDt, Tapnik::Camera* activeCamera);

	void ClickForTheGloryOfTheEmpire(GameHex* hex);
	void SetFocusHex(GameHex* hex);

	void interfaceScreens(Tapnik::UIAssets* uiAssets);
	void DoGameUI_Gameplay(nk_context* ctx, Tapnik::UIAssets* uiAssets);
	void DoGameUI_HexResourceIcons(nk_context* ctx, GameHex* hex, Tapnik::UIAssets* uiAssets);

	int boardNdx(int i, int j);
	GameHex *boardHex(int i, int j);
	float CalcIconOffset(GameHex* hex);

	void UpdateGameSystem(float dt);
	void HarvestHex(GameHex* hex, bool actualClick);
	void ExploreHex(GameHex* hex, bool actualClick);
	void UpdateHex(float dt, GameHex* hex);
	void BuildBuilding(GameHex* hex, BuildingInfo* info);
	void ActivateBulding(float dt, GameHex* hex, Building* bb);
	void CountBuildings(GameHex* hex);

	void UpdateHexStats(GameHex* hex);

	void ApplyTerrainEffects(GameHex* hex);

	// Audio
	SoundMaker* sfx = NULL;
	
	// Gameplay info
	Oryol::Array<BuildingInfo> bldgSpecs;
	uint64_t resFood = 0;
	uint64_t resWealth =0;
	uint64_t resScience =0;
	uint64_t resMight =0;

	bool cheatsEnabled = false;

	// UI stuff
	Tapnik::UIAssets* uiAssets; 

	float shadNear = 1.0f;
	float shadFar = 100.0f;

	int bndx_barbarian = 0;
	int bndx_startfood = 0;

	Tapnik::Scene* scene;
	Oryol::Id testTex;
	Tapnik::SceneMesh* hexBackMesh;

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