
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/random.hpp"

#include "Input/Input.h"

#include "CivGame.h"

#include "SceneObject.h"
#include "LDJamFile.h"
#include "Camera.h"
#include "Sounds.h"
#include "HexTileBuilder.h"

#include "par_easings.h"

// Military Rules:
// - Build Soldiers and Barracks to increase your STR for each hex
// - Some hexes start with Barbarians on them. You can't build there until you get rid of them.
// - Barbarian STR is (# of barbarians) ** 2
// - Click on a barbarian hex to reduce it by the SUM of the STR of all adjacent hexes.

// Science Rules:
// - The "tech tree" presents 3 (or so) projects you can research.
// - Each has a SCIENCE COST (high value like 40k) and a REWARD
// - Reward is a number of dice, e.g 2D6 + D20
// - You pick a science project to research. All your SCIENCE earned goes towards progress on that.
// - You may abandon it, but you lose all the science you put towards it.
// - On reward, your current TECH is multiplied by the total dice roll

// Goal: Get 1 billion wealth + technology, clear all the barbarians, 
// to build a Space Program and launch a rocket

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"

using namespace Tapnik;
using namespace Oryol;

glm::vec4 colorFromHexCode(const char* hexColor)
{
	int r = 0xff;
	int g = 0xff;
	int b = 0xff;
	int a = 0xff;
	if (strlen(hexColor) == 7) {
		sscanf(hexColor, "#%02x%02x%02x", &r, &g, &b);
	}
	else if (strlen(hexColor) == 9) {
		sscanf(hexColor, "#%02x%02x%02x%02x", &r, &g, &b, &a);
	}

	return glm::vec4((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
}

struct nk_style_item nkStyleFromHexCode(const char* hexColor)
{
	struct nk_style_item result;
	int r = 0xff;
	int g = 0xff;
	int b = 0xff;
	int a = 0xff;
	if (strlen(hexColor) == 7) {
		sscanf(hexColor, "#%02x%02x%02x", &r, &g, &b);
	}
	else if (strlen(hexColor) == 9) {
		sscanf(hexColor, "#%02x%02x%02x%02x", &r, &g, &b, &a);
	}

	result.type = NK_STYLE_ITEM_COLOR;
	result.data.color.r = glm::clamp(r, 0, 255);
	result.data.color.g = glm::clamp(g, 0, 255);
	result.data.color.b = glm::clamp(b, 0, 255);
	result.data.color.a = glm::clamp(a, 0, 255);

	return result;
}


// =========================================================
//  Building Stuff
// =========================================================

BuildingInfo::BuildingInfo(Oryol::String _bldgName,
	Oryol::String _desc) :
	bldgName( _bldgName ),
	desc( _desc ),
	mesh(NULL)
{
	ModelName = _bldgName;
}

// =========================================================
//  GameHex
// =========================================================
GameHex::GameHex(int hx, int hy) : hx(hx), hy(hy), bobTime(0.0), sceneObj( NULL) 
{
	bobTime = glm::linearRand(0.0f, 10.0f );
	bobRate = glm::linearRand(0.9f, 1.5f);
	hoverBump = 0.0f;
	clickBounce = 0.0f;

	sprintf(hexName, "hex_%d_%d", hx, hy);
}

// =========================================================
//  CivGame
// =========================================================
CivGame::CivGame() : scene(NULL), uiAssets(NULL), hoverHex( NULL ), focusHex(NULL)
{
	camFocusPos = glm::vec3(0.0f);
	camRotato = 0.0f;
	camRiseDist = glm::vec2( 0.1f, 0.1f );		 
	gameTime = 0.0f;

	resFood = 0;


	// Initialize Builder
	// TODO: Might want to have multiple to build hexes in background
	hexBuilder = Memory::New<HexTileBuilder>();

	// Initialize buildings

	// Resource Buildings
	glm::vec4 startFoodColor = colorFromHexCode("#65b617");
	glm::vec4 farmColor = colorFromHexCode("#C6D93B");
	glm::vec4 exploreColor = colorFromHexCode("#68badc");
	glm::vec4 mineralColor = colorFromHexCode("#F2B705");
	glm::vec4 barbarianColor = colorFromHexCode("#F23D4C"); 
	glm::vec4 militaryColor = colorFromHexCode("#9B73FF"); 
	glm::vec4 rocketColor = colorFromHexCode("#9a26c0"); 
	glm::vec4 scienceColor = colorFromHexCode("#483bec");

	bldgSpecs.Add(BuildingInfo("Field", "+1 Base Food Production on this space.")
				.withModelName( "cube_sm")
				.withTintColor( farmColor )
				.withBaseFood(1) );

	bldgSpecs.Add(BuildingInfo("Farmer", "A farmer lives the simple life: Harvesting food for you.")
		.withModelName("meeple")		
		.withTintColor(farmColor)
		.withBaseCost(15)
		.withCooldownTime(10.0f));


	bldgSpecs.Add(BuildingInfo("Fisherman", "Gathers the bounty of the sea.")
		.withModelName("meeple")
		.withTintColor(startFoodColor)
		.withBaseCost(15)
		.withType(Type_RESOURCE_OCEAN)
		.withCooldownTime(5.0f));

	bldgSpecs.Add(
		BuildingInfo("Grain Silo", "Doubles food production.")
		.withModelName("cube_big")
		.withTintColor(farmColor)
		.withBaseCost( 500 )
		.withFoodMultiplier( 2.0)
		.withBuildLimit(2) 
	);

	bldgSpecs.Add(BuildingInfo("Plow", "Harvest Faster with Mister Plow.")
		.withModelName("cube_sm")
		.withTintColor(farmColor)
		.withBaseCost( 100 )
		.withCooldownTime(2.0f));

	// Exploration Buildings

	bldgSpecs.Add(BuildingInfo("Scout", "Explores every 8s.")
		.withModelName("meeple")
		.withTintColor(exploreColor)
		.withBaseCost(100)
		.withType( Type_EXPLORE)
		.withCooldownTime(8.0f));

	bldgSpecs.Add(BuildingInfo("Outpost", "Base Explore +1.")
		.withModelName("cube_sm")
		.withTintColor(exploreColor)
		.withBaseCost(100)
		.withBuildLimit(6) 
		.withType(Type_EXPLORE)
		.withBaseExplore(1)
	);

	bldgSpecs.Add(BuildingInfo("Base Camp", "Explore x2.")
		.withModelName("cube_big")
		.withTintColor(exploreColor)
		.withBaseCost(1000)
		.withBuildLimit(6)
		.withType(Type_EXPLORE)
		.withExploreMult(2.0)
	);

	// Military Buildings
	bldgSpecs.Add(BuildingInfo("Soldier", "Might +1")
		.withModelName("meeple")
		.withTintColor(militaryColor)
		.withBaseMilitary(1)
		.withBaseCost(2000)
		.withBuildLimit(3)
		.withType(Type_RESOURCE)
	);

	bldgSpecs.Add(BuildingInfo("Fort", "Might +12")
		.withModelName("cube_big")
		.withTintColor(militaryColor)
		.withBaseMilitary(12)
		.withBaseCost(25000)
		.withBuildLimit(1)
		.withType(Type_RESOURCE)
	);

	// NPC Buildings
	bndx_barbarian = bldgSpecs.Size();
	bldgSpecs.Add(BuildingInfo("Barbarian", "Enemy Barbarian.")
		.withModelName("meeple")
		.withTintColor(barbarianColor)
		.withType(Type_ENEMY)
	);

	bndx_startfood = bldgSpecs.Size();
	bldgSpecs.Add(BuildingInfo("Food", "Food you found on the ground.")
			.withModelName("cube_big")
			.withTintColor(startFoodColor)
			.withType(Type_STARTING_RES )
			.withBaseFood(1)
	);

	/*
	bldgSpecs.Add(BuildingInfo("Cartographer", "Explore x10")
		.withBaseCost(1000)
		.withBuildLimit(2)
	);*/
}

int CivGame::boardNdx(int i, int j)
{
	return (j * BOARD_SZ) + i;
}
GameHex *CivGame::boardHex(int i, int j)
{
	int ndx = boardNdx(i, j);
	if ((ndx >= 0) && (ndx < BOARD_SZ * BOARD_SZ)) {
		return board[ndx];
	}
	return NULL;
}

void CivGame::SetupWithScene(Scene* _scene)
{
	scene = _scene;
	Log::Info("CivGame::SetupWithScene\n");

	SceneObject* groundObj = scene->FindNamedObject("Grid");

	// Hang on to the hex back object
	SceneObject* hexbackObj = scene->FindNamedObject("HexBack");
	
	hexBackMesh = hexbackObj?hexbackObj->mesh:NULL;

	// See if we have building meshes
	for (int i = 0; i < bldgSpecs.Size(); i++) {
		SceneObject *bbObj = scene->FindNamedObject(bldgSpecs[i].ModelName);
		if (bbObj) {
			Log::Info("Found building for %s!\n", bldgSpecs[i].bldgName.AsCStr());
			bldgSpecs[i].mesh = bbObj->mesh;
		}
		else {
			Log::Info("MISSING model mesh '%s' for building '%s'!\n", 
				bldgSpecs[i].ModelName.AsCStr(),
				bldgSpecs[i].bldgName.AsCStr());
			bldgSpecs[i].mesh = NULL;
		}
	}


	// Remove all of the object instances (but keep the meshes)
	while (scene->sceneObjs.Size() > 0) {
		SceneObject* obj = scene->sceneObjs[0];
		Log::Info("Removing object %s\n", obj->objectName.AsCStr());
		scene->sceneObjs.EraseSwapBack(0);
	}

	// Set up the game board
	SetupGameBoard();

	// Now build hex meshes for all of our squares
	float hexSize = 2.0f;
	float hexHite = sqrt(3.0) * hexSize;
	int grid = BOARD_SZ;
	float gridhalf = float(grid - 1) / 2.0f;
	for (int j = 0; j < grid; j++) {
		for (int i = 0; i < grid; i++)
		{
			int boardNdx = (j * BOARD_SZ) + i;
			GameHex* hex = board[boardNdx];
			if (!hex) continue;

			SceneMesh* hexMesh = hexBuilder->BuildHexMesh( scene, hexSize * 0.96);
			SceneObject* hexTile = scene->spawnObject(hexMesh);
			glm::vec3 tilePos = glm::vec3(float(i) - gridhalf, float(j) - gridhalf, 0.0f);
			float offs = ((i & 0x1) == 0) ? 0.0f : hexHite * 0.5;
			glm::vec3 hexPos = (tilePos * glm::vec3(hexSize * 2.0f * 0.75f, hexHite, 1.0f)) + glm::vec3( 0.0f, offs, 0.0f );


			hex->hexWorldPos = hexPos;
			hex->pos = hexPos;
			hex->pos.z = glm::linearRand(-200.0f, -10.0f);
			hex->sceneObj = hexTile;			
			hex->flipAmount = hex->flipAmountTarg = 1.0f;
			hexTile->xform = glm::translate(glm::mat4(1.0f), hexPos);

			if (hexBackMesh) {
				SceneObject* hexBackObj = scene->spawnObject(hexBackMesh);
				hexBackObj->xform = hexTile->xform;
				// TODO: subtle darkening for hexes that need more exploreing?
				hexBackObj->vsParams.decalTint = glm::vec4(1.0f);
				hex->hexBackObj = hexBackObj;				
			} else {
				hex->hexBackObj = NULL;
			}

			ApplyTerrainEffects(hex);
		}
	}
}

// Updates visuals when terrain or explore status changes.
void CivGame::ApplyTerrainEffects(GameHex* hex)
{
	glm::vec4 tileTint = glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);
	if (hex->exploreCount == 0)
	{
		// Explored tiles are face up
		hex->flipAmountTarg = 0.0f;

		// If tile  is explored, show tile
		switch (hex->terrain) {
		case GameHex::TerrainType::Terrain_OCEAN:
			tileTint = colorFromHexCode("#6986e2");
			break;

		case GameHex::TerrainType::Terrain_GRASSLAND:
			tileTint = colorFromHexCode("#F2EFBB");
			break;

		case GameHex::TerrainType::Terrain_FOREST:
			tileTint = colorFromHexCode("#5AB390");
			break;

		case GameHex::TerrainType::Terrain_DESERT:
			tileTint = colorFromHexCode("#F2C641");
			break;

		case GameHex::TerrainType::Terrain_MOUNTAIN:
			tileTint = colorFromHexCode("#A6387F");
			break;
		}
	}
	else {
		// Unexplored, flip it over
		hex->flipAmountTarg = 1.0f;
	}

	// DBG: Don't tint for now
	tileTint = glm::vec4(1.0f);

	hex->sceneObj->vsParams.tintColor = tileTint;

}

void CivGame::SetupGameBoard()
{
	// TODO: allow different size games
	int rowStart[9] = { 3, 1, 0, 0, 0, 0, 0, 2, 4  };
	int rowEnd[9] = { 5, 7, 8, 8, 8, 8, 8, 6, 4 };

	// frequency table for terrain types
	GameHex::TerrainType terrFreq[] = {
		GameHex::TerrainType::Terrain_OCEAN,
		GameHex::TerrainType::Terrain_OCEAN,
		GameHex::TerrainType::Terrain_OCEAN,
		GameHex::TerrainType::Terrain_OCEAN,
		GameHex::TerrainType::Terrain_OCEAN,

		GameHex::TerrainType::Terrain_GRASSLAND,
		GameHex::TerrainType::Terrain_GRASSLAND,
		GameHex::TerrainType::Terrain_GRASSLAND,

		GameHex::TerrainType::Terrain_FOREST,
		GameHex::TerrainType::Terrain_FOREST,

		GameHex::TerrainType::Terrain_DESERT,
		GameHex::TerrainType::Terrain_DESERT,
		GameHex::TerrainType::Terrain_MOUNTAIN
	};
	int numTerrFreq = sizeof(terrFreq) / sizeof(terrFreq[0]);

	int CC = BOARD_SZ / 2;
	for (int i = 0; i < BOARD_SZ; i++) {
		for (int j = 0; j < BOARD_SZ; j++) {
			int boardNdx = (j * BOARD_SZ) + i;
			
			if ( ((i<rowStart[j]) || (i > rowEnd[j]))) {
				// Empty hex
				board[boardNdx] = NULL;
			} else {
				GameHex *hex = Memory::New<GameHex>(i, j);

				//hex->flipAxis = glm::normalize( glm::vec3(glm::diskRand(1.0f), 0.0f) );

				int flipAxisInt = (int)(glm::linearRand(0.0f, 1.0f) * 5);
				float ang = (glm::pi<float>() / 3.0f) * flipAxisInt;
				hex->flipAxis = glm::normalize(glm::vec3(cos(ang), sin(ang), 0.0f));

				// terrain type (todo)
				int tndx = (int)(glm::linearRand(0.0f, 1.0f) * numTerrFreq);
				int explore = (int)(glm::linearRand(0.0f, 1.0f) * 4);
						
				if (tndx >= numTerrFreq) {
					tndx = numTerrFreq - 1;
				}
				hex->terrain = terrFreq[tndx];

				hex->exploreCount = 10;
				for (int i = 0; i < explore; i++)
				{
					hex->exploreCount *= 10;
				}
				hex->exploreCount += (uint64_t)(glm::linearRand(0.0f, 1.0f) * hex->exploreCount);
				hex->exploreCountStart = hex->exploreCount;

				hex->damageCount = 0;

				board[boardNdx] = hex;
			}
		}
	}

	// Add some enemies
	int enemiesLeft = 7;
	while (enemiesLeft > 0) {
		int index = (int)(glm::linearRand(0.0f, 1.0f) * ((BOARD_SZ * BOARD_SZ) - 1));
		GameHex* hex = board[index];
		if ((hex) && (hex->terrain != GameHex::TerrainType::Terrain_OCEAN)) {
			int numEnemies = (int)(glm::linearRand(1.0f, 5.0f));
			for (int i = 0; i < numEnemies; i++) {
				BuildBuilding(hex, &bldgSpecs[bndx_barbarian]);
			}
			enemiesLeft -= 1;
		}
	}

	// Now setup 5 tiles to produce food
	bool foundAnEasyTile = false;
	int tilesLeft = 5;
	while (tilesLeft > 0) {
		int index = (int)(glm::linearRand(0.0f, 1.0f) * ((BOARD_SZ * BOARD_SZ)-1) );
		GameHex* hex = board[index];
		
		// Find an empty hex
		if ((hex) && (hex->bldgs.Size()==0)) {
			BuildBuilding(hex, &bldgSpecs[bndx_startfood]);

			if (hex->exploreCount < 30) {
				foundAnEasyTile = true;
			}

			tilesLeft--;

			// Make sure there's at least one tile that you don't have to click on a zillion times
			if ((tilesLeft == 0) && (!foundAnEasyTile)) {
				hex->exploreCount = 10 + (uint64_t)(glm::linearRand(0.0f, 1.0f) * 20);
				hex->exploreCountStart = hex->exploreCount;
			}

		}
	}

	// Update stats on everything
	UpdateAllHexStats();
	
}

void CivGame::ActivateBulding(float dt, GameHex* hex, Building* bb)
{
	if (hex->exploreCount > 0)
	{
		ExploreHex(hex, false);
	}
	else if (hex->stat_totalFoodHarvest > 0) {
		HarvestHex(hex, false);
	}
}

void CivGame::UpdateHex(float dt, GameHex* hex)
{
	for (int i = 0; i < hex->bldgs.Size(); i++) {

		Building* bb = hex->bldgs[i];
		if (bb->cooldown > -0.1) {
			bb->cooldown -= dt;
			if (bb->cooldown <= 0.0) {
				// cooldown hit, activate this tile!				
				bb->cooldown = bb->info->CooldownTime;
				this->ActivateBulding(dt, hex, bb);
				Log::Info("Activated!\n");
			}
		}
	}
}

void CivGame::UpdateGameSystem(float dt)
{
	for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++) {
		GameHex* hex = board[i];
		if (hex) {
			UpdateHex(dt, hex);
		}
	}
}

void CivGame::HandleInput(float dt, Tapnik::Camera* activeCamera)
{
	// Update input and feedback

	// Right margin
	float RM = uiAssets->fbWidth - 300.0f;

	// Cheats for dev
	if (cheatsEnabled)
	{
		if (Input::KeyDown(Key::N1)) {
			resFood += 1000;
		}
		else if (Input::KeyDown(Key::N4)) {
			// Explore everthing
			for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++) {
				GameHex* hex = board[i];
				if (hex) {
					hex->exploreCount = 0;
					ApplyTerrainEffects(hex);
				}
			}
		}
	}

	if (Input::KeyDown(Key::N0)) {
		cheatsEnabled = !cheatsEnabled;	
	}

	// Handle input
	float vel = 1.0f * dt;
	glm::vec2 move(0.0f);
	if (Input::KeyPressed(Key::W) || Input::KeyPressed(Key::Up)) {
		move.y -= vel;
	}
	if (Input::KeyPressed(Key::S) || Input::KeyPressed(Key::Down)) {
		move.y += vel;
	}
	if (Input::KeyPressed(Key::A) || Input::KeyPressed(Key::Left)) {
		move.x -= vel;
	}
	if (Input::KeyPressed(Key::D) || Input::KeyPressed(Key::Right)) {
		move.x += vel;
	}

	camRotato += move.x;
	camRotato = fmod(camRotato, glm::pi<float>() * 2.0f);

	if (Input::MouseAttached()) {

		glm::vec2 mousePos = Input::MousePosition();

		if (Input::MouseButtonDown(MouseButton::Left)) {
			Log::Info("LMB MouseDown..\n");
		}
		if (Input::MouseButtonUp(MouseButton::Left)) {
			Log::Info("LMB MouseUp..\n");
		}
		if ((!focusHex) || (focusHex == hoverHex))
		{
			// Have they held the mouse down long enough to count as a long press?
			if ((mouseDown) && (!didTriggerLongPress)) {
				mouseDownTime += dt;
				if (mouseDownTime > 0.5f) {
					didTriggerLongPress = true;
					SetFocusHex(hoverHex);
				}
			}

			// Make sure they aren't over the GUI panel			
			if ((mousePos.x > 300) && (mousePos.x < RM)) {
				if (Input::MouseButtonDown(MouseButton::Left)) {


					mouseDown = true;
					didTriggerLongPress = false;
					mouseDownTime = 0.0f;
				}
				else if (Input::MouseButtonUp(MouseButton::Left)) {

					mouseDown = false;
					if (!didTriggerLongPress) {
						ClickForTheGloryOfTheEmpire(hoverHex);
					}
				}

				// Right-click sets focus right away
				if (Input::MouseButtonDown(MouseButton::Right)) {
					SetFocusHex(hoverHex);
				}
			}
		}
		else {
			if ((mousePos.x > 300) && (mousePos.x < RM)) {

				// We have a focus hex, any other click will unfocus it
				if (Input::MouseButtonDown(MouseButton::Left) || Input::MouseButtonDown(MouseButton::Right)) {
					Log::Info("Mouse Button Down, but focused.\n");
					SetFocusHex(NULL);
				}
			}
		}
	}

	glm::vec3 focusTarget(0.0f);
	glm::vec2 camRiseTarget(10.0f, 40.0f);
	if (focusHex) {
		focusTarget = focusHex->hexWorldPos;
		camRiseTarget = glm::vec2(5.0f, 6.0f);
	}
	camFocusPos = mix(camFocusPos, focusTarget, 4.0f * dt);
	camRiseDist = mix(camRiseDist, camRiseTarget, 2.0f * dt);

	// Update the screen positions of all the hexes
	glm::mat4x4 mvp = activeCamera->ViewProj;
	for (int j = 0; j < BOARD_SZ; j++) {
		for (int i = 0; i < BOARD_SZ; i++) {
			GameHex* hex = this->boardHex(i, j);
			if (hex) {
				char buff[10];
				sprintf(buff, "%d,%d", i, j);
				glm::vec4 hexp = glm::vec4(hex->pos, 1.0f);
				glm::vec4 viewportPos = mvp * hexp;

				glm::vec2 screenPos = glm::vec2(
					(((viewportPos.x / viewportPos.w) * 0.5f) + 0.5f) * uiAssets->fbWidth,
					(1.0f - (((viewportPos.y / viewportPos.w) * 0.5f) + 0.5f)) * uiAssets->fbHeight);

				hex->screenPos = screenPos;
			}
		}
	}
}

void CivGame::fixedUpdate(Oryol::Duration fixedDt, Tapnik::Camera* activeCamera)
{
	float dt = fixedDt.AsSeconds();

	// game update
	UpdateGameSystem(dt);

	
	gameTime += dt;

	
}

void CivGame::CountBuildings(GameHex* hex)
{
	for (int i = 0; i < bldgSpecs.Size(); i++) {
		bldgSpecs[i].countOnFocusHex = 0;
	}
	if (hex) {
		for (int i = 0; i < hex->bldgs.Size(); i++) {
			Building* bb = hex->bldgs[i];
			bb->info->countOnFocusHex++;
		}
	}

}

void CivGame::BuildBuilding(GameHex* hex, BuildingInfo* info)
{
	Log::Info("Build %s!\n", info->bldgName.AsCStr() );	

	if ((info->Type != Type_ENEMY) && (info->Type != Type_STARTING_RES))
	{
		if (resFood >= info->BaseCost) {
			resFood -= info->BaseCost;
		}
	}

	Building* bldg = Memory::New<Building>(info);
		
	if (info->CooldownTime >= 0.0f) {
		/// start at a random offset so they don't all fire in sync
		bldg->cooldown = info->CooldownTime * glm::linearRand(0.1f, 1.0f); 
	}
	else {
		bldg->cooldown = info->CooldownTime;
	}

	// Add the building mesh if it has one
	if (info->mesh) {
		bldg->sceneObj = scene->spawnObject(info->mesh);
		bldg->sceneObj->vsParams.tintColor = info->TintColor;
	}

	// for now, just retry a few times to minimze overlaps. In the future, do something better
	bool goodPos = false;
	int attempts = 0;
	while (!goodPos) {
		bldg->localPos = glm::vec3(glm::diskRand(1.4f), glm::linearRand(0.0f, 0.1f));

		// is this position good?
		goodPos = true;
		for (int i = 0; i < hex->bldgs.Size(); i++) {
			float dd = glm::distance(hex->bldgs[i]->localPos, bldg->localPos);
			Log::Info("local dist is %3.3f\n", dd);
			if (dd < 0.6f) {
				goodPos = false;
				break;
			}
		}
		attempts++;
		if (attempts > 20) {
			Log::Info("Couldn't fit building without overlap.\n");
			break;
		}
	}


	bldg->localRot = glm::angleAxis(glm::linearRand(0.0f, glm::pi<float>() * 4.0f), glm::vec3(0.0f, 0.0f, 1.0f) );

	// Add this building
	hex->bldgs.Add(bldg);

	CountBuildings(focusHex);
	UpdateAllHexStats();
}

void CivGame::AttackHex(GameHex* hex, bool actualClick)
{
	UpdateAllHexStats();
	Log::Info("Attack hex\n");

	// See how much damage we do
	uint64_t damage = hex->stat_surroundingMilitary;
	if (damage == 0) {
		// Didn't do anything, play the plonk sound
		sfx->sfxErrBloop.Play();
	}
	else {
		Log::Info("Did %d damage, hp %d/%d\n", damage, hex->damageCount, hex->stat_totalEnemyStr);
		// TODO: Play some attacky sound
		hex->damageCount += damage;
		// Did this get all the enemies??
		if (hex->damageCount > hex->stat_totalEnemyStr) {
			// Yay! nuke them.
			RemoveAllBuildingsOfType(hex, Type_ENEMY );
			UpdateAllHexStats();
		}
	}
}

void CivGame::HarvestHex(GameHex* hex, bool actualClick )
{

	// Make sure our stats are up to date just in case
	UpdateAllHexStats();

	if (hex->stat_totalFoodHarvest > 0) {
		resFood += hex->stat_totalFoodHarvest; 
		// TODO: animated food feedback
	}

	// Always bounce for a real click
	if (actualClick)
	{
		hex->clickBounce = glm::linearRand(0.9f, 1.5f);
		if (hex->stat_totalFoodHarvest <= 0) {
			sfx->sfxErrBloop.Play(); // Let them know this didn't work
		}
	}
	else {
		// Bounce (a little less) for an auto-click.
		hex->clickBounce = glm::linearRand(0.5f, 0.8f);
	}
}

void CivGame::UpdateAllHexStats()
{
	for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++) {
		GameHex* hex = board[i];
		if (hex) {
			UpdateOneHexStats(hex);
		}
	}
	// now the individual hexes are up to date, calculate the surround-based ones
	for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++) {
		GameHex* hex = board[i];
		if (hex) {
			UpdateOneHexSurroundStats(hex);
		}
	}
}

void CivGame::UpdateOneHexSurroundStats(GameHex* hex)
{
	// Military Strength
	int adjX[] = {  0, -1, -1, 0, 1,  1 };
	int adjY[] = { -1, -1,  0, 1, 0, -1 };
	hex->stat_surroundingMilitary = 0;
	for (int i = 0; i < 6; i++) {
		int ii = hex->hx + adjX[i];
		int jj = hex->hy + adjY[i];
		GameHex* nbrHex = boardHex(ii, jj);
		if (nbrHex) {
			hex->stat_surroundingMilitary += nbrHex->stat_totalMilitary;
		}
	}

}

// NOTE: this doesn't update stats which rely on neighbors since
// we have to update all hexes first and then we can apply neighbors
void CivGame::UpdateOneHexStats(GameHex* hex)
{
	// --- Explore Stats

	// Calculate explore amount
	hex->stat_baseExplore = 1; // base explore amount is 1 from the player exploring
	hex->stat_explorerCount = 0;
	for (int i = 0; i < hex->bldgs.Size(); i++) {
		hex->stat_baseExplore += hex->bldgs[i]->info->BaseExplore;
		if (hex->bldgs[i]->info->Type == Type_EXPLORE) {
			hex->stat_explorerCount++;
		}
	}

	// Multiply by explore multiplier
	double exploreAmtF = hex->stat_baseExplore;
	hex->stat_exploreMultiplier = 1.0;
	for (int i = 0; i < hex->bldgs.Size(); i++) {
		if (hex->bldgs[i]->info->ExploreMult != 1.0) {
			exploreAmtF *= hex->bldgs[i]->info->ExploreMult;
			hex->stat_exploreMultiplier *= hex->bldgs[i]->info->ExploreMult;
		}
	}
	hex->stat_totalExplore = (uint64_t)exploreAmtF;

	// --- Food Stats
	bool hasPlayerBuiltFood = false;
	hex->stat_foodProduction = 0;	
	for (int i = 0; i < hex->bldgs.Size(); i++) {
		hex->stat_foodProduction += hex->bldgs[i]->info->BaseFood;
		if ((hex->bldgs[i]->info->Type != Type_STARTING_RES) && (hex->bldgs[i]->info->BaseFood > 0)) {
			hasPlayerBuiltFood = true;
		}
	}
	hex->hasPlayerBuiltFood = hasPlayerBuiltFood;

	// Multiply by food multiplier	
	double foodProductionF = hex->stat_foodProduction;
	hex->stat_foodMultiplier = 1.0;
	for (int i = 0; i < hex->bldgs.Size(); i++) {
		if (hex->bldgs[i]->info->FoodMultiplier != 1.0) {
			foodProductionF *= hex->bldgs[i]->info->FoodMultiplier;
			hex->stat_foodMultiplier *= hex->bldgs[i]->info->FoodMultiplier;
		}
	}
	hex->stat_totalFoodHarvest = (uint64_t)foodProductionF;

	// -- Military stats
	hex->stat_baseMilitary = 0;
	for (int i = 0; i < hex->bldgs.Size(); i++) {
		hex->stat_baseMilitary += hex->bldgs[i]->info->BaseMilitary;
	}
	// no multipliers on military yet
	hex->stat_totalMilitary = hex->stat_baseMilitary;

	// --- Enemy and combat stats
	hex->stat_numEnemies = 0;
	for (int i = 0; i < hex->bldgs.Size(); i++) {
		if (hex->bldgs[i]->info->Type == Type_ENEMY) {
			hex->stat_numEnemies++;
		}
	}
	// Enemy strenght is 10 * N^2
	hex->stat_totalEnemyStr = (hex->stat_numEnemies * hex->stat_numEnemies) * 10;
}

void CivGame::ExploreHex(GameHex* hex, bool actualClick)
{
	if (hex->exploreCount > 0) {
		hex->clickBounce = glm::linearRand(0.9f, 1.5f);
		
		// Make sure our stats are up to date just in case
		UpdateAllHexStats();

		if (hex->exploreCount > hex->stat_totalExplore) {
			hex->exploreCount -= hex->stat_totalExplore;
		}
		else {
			hex->exploreCount = 0;
			ApplyTerrainEffects(hex);

			// HERE: Big reveal, flip hex and stuff.
			if (hex->stat_totalEnemyStr == 0) {
				sfx->sfxWhoosh.Play();
			}
			else {
				sfx->sfxStomp.Play();
			}
			
			// Also can nuke all Explore buildings in the tile
			RemoveAllBuildingsOfType(hex, Type_EXPLORE);
		}
	}
	Log::Info("Explore: %zu\n", hex->exploreCount);
}

void CivGame::RemoveAllBuildingsOfType(GameHex *hex, int type)
{
	for (int i = hex->bldgs.Size() - 1; i >= 0; i--) {
		if (hex->bldgs[i]->info->Type == type ) {

			Building* bb = hex->bldgs[i];
			scene->destroyObject(bb->sceneObj);
			Memory::Free(bb);

			hex->bldgs.EraseSwapBack(i);
		}
	}
}

void CivGame::ClickForTheGloryOfTheEmpire(GameHex* hex)
{
	Log::Info("CLICK!!!\n");
	if (hex) {
		// Is tile explored?
		if (hex->exploreCount > 0) {
			ExploreHex(hex, true);
		}
		else if (hex->stat_numEnemies > 0) {
			AttackHex(hex, true);
		}
		else {
			HarvestHex(hex, true);
		}
	}
}

void CivGame::SetFocusHex(GameHex* hex)
{
	if (!focusHex) {
		focusHex = hoverHex;		
	}
	else {
		focusHex = NULL;
	}
	CountBuildings(focusHex);
}

void CivGame::dynamicUpdate(Oryol::Duration frameDt, Tapnik::Camera * activeCamera)
{
	float dt = frameDt.AsSeconds();

	// Update mouse ray and ground cursor
	glm::vec2 mousePos = Input::MousePosition();
	mouseRay = activeCamera->getCameraRay(mousePos.x, mousePos.y);

	// find where the ray hits the ground plane
	float t = mouseRay.pos.z / -mouseRay.dir.z;
	groundCursor = mouseRay.pos + (mouseRay.dir * t);


	// Update the hexes
	float hexSize = 2.0f;
	GameHex* closestHex = NULL;
	float closestHexDist;
	glm::vec2 groundCursor2D = glm::vec2(groundCursor.x, groundCursor.y);
	for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++) {
		GameHex *hex = board[i];
		if (hex) {			
			// idle bob anim
			hex->bobTime += dt;
			hex->targetPos = glm::vec3(hex->hexWorldPos.x, hex->hexWorldPos.y, sin(hex->bobTime * 0.5 * hex->bobRate ) * 0.1 + hex->hoverBump );

			// lerp target pos			
			hex->pos = glm::mix(hex->pos, hex->targetPos, 0.5f);

			// Click bounce is special sauce
			float cb = sin(gameTime * 50.0) * hex->clickBounce;
			hex->clickBounce = glm::max<float>(0.0f, hex->clickBounce - dt*3.0f);

			// If we're focused, tone down the bounce cause it's pretty intense
			if (focusHex) {
				cb *= 0.2f;
			}

			// Update hover dist
			glm::vec2 hexPos2d = glm::vec2(hex->hexWorldPos.x, hex->hexWorldPos.y);
			float dd = glm::distance(hexPos2d, groundCursor2D);
			if (dd < hexSize*1.2) {
				if ((!closestHex) || (dd < closestHexDist)) {
					closestHex = hex;
					closestHexDist = dd;
				}
			}
	
			// Update the transform and flip
			float needFlipAnim = (hex->flipAmountTarg - hex->flipAmount);
			if (fabs(needFlipAnim) > 0.001f) {
				float flipDir = needFlipAnim > 0.0f ? 1.0f : -1.0f;
				hex->flipAmount += dt * flipDir * 2.0f;
				// Don't bound when flipping cause it looks weird
				cb = 0.0f;
				
				// clamp
				hex->flipAmount = glm::clamp(hex->flipAmount, 0.0f, 1.0f);
			}
			else {
				hex->flipAmount = hex->flipAmountTarg;
			}
			
			//hex->flipAmount += dt;
			//hex->flipAmount = fmod(hex->flipAmount, 1.0f);

			float easeFlip = par_easings_in_bounce(hex->flipAmount);
			glm::quat hexFlip = glm::angleAxis(easeFlip * glm::pi<float>(),hex->flipAxis );

			//glm::mat4x4 hexXform = 
			glm::mat4x4 hexXform = glm::translate(glm::mat4x4(), hex->pos + glm::vec3(0.0f, 0.0f, cb));

			hex->sceneObj->xform = hexXform * glm::toMat4(hexFlip);
			if (hex->hexBackObj) {
				hex->hexBackObj->xform = hex->sceneObj->xform;
			}

			// Update any building transforms
			for (int i = 0; i < hex->bldgs.Size(); i++)
			{
				Building* bb = hex->bldgs[i];
				SceneObject* bbObj = bb->sceneObj;
				if (bbObj) {

					bool isExplore = (bb->info->Type == Type_EXPLORE);
					glm::vec3 offs(0.0f);
					if (isExplore) {
						offs.z = 0.38f;
					}

					glm::mat4x4 bldgXform = glm::translate(glm::mat4x4(), bb->localPos + offs);
					glm::mat4x4 bldgRot = glm::toMat4(bb->localRot);					
					
					if (!isExplore) {
						bbObj->xform = hex->sceneObj->xform * (bldgXform * bldgRot);
					}
					else {
						// Apply the xform without the flip
						bbObj->xform = hexXform * (bldgXform * bldgRot);
					}
				}
			}
		}
	}	

	if (hoverHex != closestHex)
	{
		if (hoverHex) {
			hoverHex->hoverBump = 0.0f;
		}
		hoverHex = closestHex;
		if ((hoverHex) && (hoverHex != focusHex))
		{
			hoverHex->hoverBump = 0.2f;
			//play click sound
			// TODO: don't play when camera is moving
			sfx->sfxClick.Play();
		}
	}
	
	// Handle Mouse Input
	HandleInput(dt, activeCamera);

}

float CivGame::CalcIconOffset(GameHex* hex)
{
	GameHex* nbr = boardHex(hex->hx, hex->hy - 1);
	if (!nbr) {
		nbr = boardHex(hex->hx, hex->hy + 1);
	}

	// should have a neigbhor on at least one side top or bottom
	// but just in case
	if (!nbr) return 0.0f;
	
	float d = glm::distance(hex->screenPos, nbr->screenPos);

	return d * 0.25;
}

void CivGame::interfaceScreens(Tapnik::UIAssets* uiAssets)
{
	// Draw UI Layer
	nk_context* ctx = NKUI::NewFrame();
	uiAssets->storeDefaultStyle(ctx);

	nk_style_set_font(ctx, &(uiAssets->font_14->handle));
	uiAssets->buttonStyleNormal(ctx);

	DoGameUI_Gameplay(ctx, uiAssets);


	NKUI::Draw();
	nk_clear(ctx);
}

// Tries to fit a number into approx 4-5 chars
void shortNumStr(char* dest, uint64_t num)
{
	if (num < 1000) {
		sprintf(dest, "%zu", num );
	}
	else {
		// round it off
		double nn = (double)num;
		if (nn < 1000000.0) {
			sprintf(dest, "%3.1lfK", nn / 1000.0 );
		}
		else {
			sprintf(dest, "%3.1lfM", nn / 1000000.0);
		}
	}
}

void CivGame::DoGameUI_HexResourceIcons(nk_context* ctx, GameHex* hex, Tapnik::UIAssets *uiAssets )
{
	float offs = CalcIconOffset(hex);
	bool showBig = false;
	glm::vec2 hp = hex->screenPos + glm::vec2(0.0, offs);

	if ((hex == focusHex) && (offs > 50)) {
		showBig = true;
	}

	if (hex->exploreCount > 0) {
		ctx->style.window.fixed_background.data.color = nk_rgba(0, 0, 0, 0);

		// Still need to explore this hex.
		glm::vec2 hpb = hex->screenPos;
		if ((hpb.x > 0) && (hpb.x < uiAssets->fbWidth) &&
			(hpb.y > 0) && (hpb.y < uiAssets->fbHeight))
		{
			if (hex->exploreCount != hex->exploreCountStart) {

				if (!showBig) {
					// Started exploring, show a progress.
					if (nk_begin(ctx, hex->hexName, nk_rect(hpb.x - 60, hpb.y - 30, 120, 60), NK_WINDOW_NO_SCROLLBAR /*|NK_WINDOW_BORDER*/)) {
						nk_layout_row_static(ctx, 40, 100, 1);

						ctx->style.text.color = nk_rgb(40, 40, 50);
						nk_style_set_font(ctx, &(uiAssets->font_30->handle));
						char buff[20];
						shortNumStr(buff, hex->exploreCount);
						nk_label(ctx, buff, NK_TEXT_CENTERED);
					}
					nk_end(ctx);
				}
				else {

					if (nk_begin(ctx, hex->hexName, nk_rect(hpb.x - 125, hpb.y - 30, 250, 60), NK_WINDOW_NO_SCROLLBAR /*| NK_WINDOW_BORDER */)) {
						nk_layout_row_static(ctx, 40, 220, 1);

						ctx->style.text.color = nk_rgb(40, 40, 50);
						nk_style_set_font(ctx, &(uiAssets->font_30->handle));
						char buff[20];
						sprintf(buff, "%zu", hex->exploreCount);
						nk_label(ctx, buff, NK_TEXT_CENTERED);
					}
					nk_end(ctx);
				}
			}
		}
	}
	
	if ((hex->stat_totalFoodHarvest > 0) || 
		(hex->stat_totalEnemyStr > 0) || 
		(hex->stat_totalExplore > 1) || 
		(hex->stat_explorerCount > 0) || 
		(hex->exploreCount != hex->exploreCountStart))
	{
		
		if ((hp.x > 0) && (hp.x < uiAssets->fbWidth) &&
			(hp.y > 0) && (hp.y < uiAssets->fbHeight))
		{
			ctx->style.window.fixed_background.data.color = nk_rgba(0, 0, 0, 0);

			if (!showBig) {

				bool drawIcons = true;
				struct nk_image icon;
				if ((hex->exploreCount > 0) && ((hex->stat_totalExplore > 1) || (hex->stat_explorerCount > 0)) ) {
					// We've started to explore this hex
					icon = uiAssets->icon_explore;
				}
				else if (hex->exploreCount == 0) {

					if (hex->stat_totalEnemyStr > 0) {
						icon = uiAssets->icon_enemy;
					}
					else {
						if (hex->stat_foodProduction > 0)
						{
							if (hex->hasPlayerBuiltFood) {
								icon = uiAssets->icon_food;
							}
							else {
								icon = uiAssets->icon_startfood;
							}
						}
						else {
							drawIcons = false;
						}
					}
				}
				else {
					drawIcons = false;
				}

				if (drawIcons) {
					// Small resource icons
					char buff[20];
					sprintf(buff, "%s-smres", hex->hexName);
					if (nk_begin(ctx, buff, nk_rect(hp.x - 30, hp.y - 16, 65, 34), NK_WINDOW_NO_SCROLLBAR /*| NK_WINDOW_BORDER */)) {
						nk_layout_space_begin(ctx, NK_STATIC, 101 /*height*/, 1 /*numchilds*/);

						nk_layout_space_push(ctx, nk_rect(22, 0, 16, 16));
						nk_image(ctx, icon);

						nk_layout_space_end(ctx);
					}
					nk_end(ctx);
				}
			}
			else {

				bool drawIcons = true;
				struct nk_image icon;
				char numText[20];
				if (hex->exploreCount > 0) {
					
					// Always show explore stats for unexplored hex if we've started it
					if (hex->exploreCount != hex->exploreCountStart)
					{
						icon = uiAssets->icon_explore_big;
						sprintf(numText, "%zu", hex->stat_totalExplore);
						ctx->style.text.color = nkStyleFromHexCode("#68badc").data.color;
					}
					else {
						drawIcons = false;
					}
				}
				else {
					if (hex->stat_totalEnemyStr > 0) {
						uint64_t hpRemain = hex->stat_totalEnemyStr - hex->damageCount;
						icon = uiAssets->icon_enemy_big;
						sprintf(numText, "%zu", hpRemain );
						ctx->style.text.color = nkStyleFromHexCode("#F23D4C").data.color;
					}
					else {
						if (hex->hasPlayerBuiltFood) {
							icon = uiAssets->icon_food_big;
							ctx->style.text.color = nkStyleFromHexCode("#C6D93B").data.color;
						}
						else {
							icon = uiAssets->icon_startfood_big;
							ctx->style.text.color = nkStyleFromHexCode("#65b617").data.color;
						}
						sprintf(numText, "%zu", hex->stat_totalFoodHarvest);
					}
				}

				if (drawIcons)
				{
					// Big resource icons
					char buff[20];
					sprintf(buff, "%s-bigres", hex->hexName);
					if (nk_begin(ctx, buff, nk_rect(hp.x - 100, hp.y - 25, 200, 75), NK_WINDOW_NO_SCROLLBAR /* | NK_WINDOW_BORDER*/)) {

						nk_layout_space_begin(ctx, NK_STATIC, 65 /*height*/, 2 /*numchilds*/);

						nk_layout_space_push(ctx, nk_rect(0, 0, 64, 64));
						nk_image(ctx, icon);

						nk_style_set_font(ctx, &(uiAssets->font_30->handle));

						nk_layout_space_push(ctx, nk_rect(80, 0, 200 - 80, 64));
						nk_label(ctx, numText, NK_TEXT_LEFT);

						nk_layout_space_end(ctx);
					}
					nk_end(ctx);
				}
			}
		}
	}
}

void CivGame::DoGameUI_Gameplay(nk_context* ctx, Tapnik::UIAssets* uiAssets)
{
	struct nk_panel layout = {};

	uiAssets->restoreDefaultStyle( ctx );
	ctx->style.window.fixed_background.data.color = nk_rgba(0, 0, 0, 100);

	float W = uiAssets->fbWidth;
	float H = uiAssets->fbHeight;

	// Do all the hexes 
	for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++)
	{
		GameHex* hex = board[i];
		if (hex) {	
			DoGameUI_HexResourceIcons(ctx, hex, uiAssets);
		}
	}

	// Civ Panel
	ctx->style.window.fixed_background.data.color = nk_rgba(0, 0, 0, 100);
	if (nk_begin(ctx, "civ_panel", nk_rect(0, 0, 300, uiAssets->fbHeight), NK_WINDOW_NO_SCROLLBAR ))
	{
		nk_layout_row_static(ctx, 30, 300, 1);
		ctx->style.text.color = nk_rgb(255, 255, 255);
		nk_style_set_font(ctx, &(uiAssets->font_30->handle));
		nk_label(ctx, "Click of Ages", NK_TEXT_ALIGN_CENTERED);

		
		nk_style_set_font(ctx, &(uiAssets->font_30->handle));
		
		char buff[20];

		// Food Resource
		nk_layout_row_begin(ctx, NK_STATIC, 64, 2);
		nk_layout_row_push(ctx, 64);		
		nk_image(ctx, uiAssets->icon_food_big);		
		nk_layout_row_push(ctx, 300-64);
		ctx->style.text.color = nk_rgb(200, 255, 150);		
		sprintf(buff, "%zu", resFood);
		nk_label(ctx, buff, NK_TEXT_LEFT);
		nk_layout_row_end(ctx );

		// Mineral Resource
		/*
		nk_layout_row_begin(ctx, NK_STATIC, 64, 2);
		nk_layout_row_push(ctx, 64);
		nk_image(ctx, uiAssets->icon_startfood_big);
		nk_layout_row_push(ctx, 300 - 64);
		ctx->style.text.color = nk_rgb(200, 255, 150);
		sprintf(buff, "%zu", (uint64_t)12345678 );
		nk_label(ctx, buff, NK_TEXT_LEFT);
		nk_layout_row_end(ctx);
	*/
		
	}
	nk_end(ctx);

	// Current Hex Panel
	if (nk_begin(ctx, "hex_panel", nk_rect(W-300, 0, 300, uiAssets->fbHeight), NK_WINDOW_NO_SCROLLBAR))
	{
		
		

		nk_style_set_font(ctx, &(uiAssets->font_30->handle));
		
		if (!focusHex) {
			nk_layout_row_static(ctx, 60, 300, 1);

			ctx->style.text.color = nk_rgb(255, 255, 255);
			nk_label(ctx, "Your Civilization", NK_TEXT_ALIGN_CENTERED);
		}

		else if (focusHex->exploreCount > 0) {
			nk_layout_row_static(ctx, 120, 300, 1);

			ctx->style.text.color = nkStyleFromHexCode("#68badc").data.color;
			nk_label(ctx, "Unexplored", NK_TEXT_ALIGN_CENTERED);

			ctx->style.text.color = nk_rgb(255, 255, 255);
			nk_style_set_font(ctx, &(uiAssets->font_20->handle));

			nk_label_wrap(ctx, "This hex has not been explored yet. Click to explore. Create units and buildings to explore it faster." );
		}
		else {			

			nk_layout_row_static(ctx, 60, 300, 1);

			switch (focusHex->terrain) {
				
			case GameHex::TerrainType::Terrain_OCEAN:
				ctx->style.text.color = nkStyleFromHexCode("#6986e2").data.color;
				nk_label(ctx, "Ocean", NK_TEXT_ALIGN_CENTERED);
				break;

			case GameHex::TerrainType::Terrain_GRASSLAND:
				ctx->style.text.color = nkStyleFromHexCode("#F2EFBB").data.color;
				nk_label(ctx, "Grassland", NK_TEXT_ALIGN_CENTERED);
				break;

			case GameHex::TerrainType::Terrain_FOREST:
				ctx->style.text.color = nkStyleFromHexCode("#5AB390").data.color;
				nk_label(ctx, "Forest", NK_TEXT_ALIGN_CENTERED);
				break;

			case GameHex::TerrainType::Terrain_DESERT:
				ctx->style.text.color = nkStyleFromHexCode("#F2C641").data.color;
				nk_label(ctx, "Desert", NK_TEXT_ALIGN_CENTERED);
				break;

			case GameHex::TerrainType::Terrain_MOUNTAIN:
				ctx->style.text.color = nkStyleFromHexCode("#A6387F").data.color;
				nk_label(ctx, "Mountain", NK_TEXT_ALIGN_CENTERED);
				break;
			}

			// TODO: Show terrain modifiers
			ctx->style.text.color = nk_rgb(255, 255, 255);
			nk_style_set_font(ctx, &(uiAssets->font_20->handle));
			nk_labelf(ctx, NK_TEXT_ALIGN_CENTERED, "%d/%d", focusHex->bldgs.Size(), 12);
		}

		if (!focusHex) {
			ctx->style.text.color = nk_rgb(255, 255, 255);
			nk_style_set_font(ctx, &(uiAssets->font_20->handle));
			
			/*
			char buff[30];
			sprintf(buff, "Shadow Near (%3.2f)", shadNear);
			nk_label(ctx, buff, NK_TEXT_LEFT);
			nk_slider_float(ctx, 0, &shadNear, 10.0f, 0.0001f);
			
			sprintf(buff, "Shadow Far (%3.2f)", shadFar);
			nk_label(ctx, buff, NK_TEXT_LEFT);
			nk_slider_float(ctx, 0, &shadFar, 100.0f, 1.f);
			*/

			nk_layout_row_static(ctx, 60, 250, 1);
			nk_label_wrap(ctx, "Click on an unexplored hex to explore it.");
			nk_label_wrap(ctx, "Click a hex with resources to produce.");
			nk_label_wrap(ctx, "Click and Hold or Right-Click to inspect and build improvements on a hex." );
			//nk_layout_row_end(ctx);

			nk_layout_row_static(ctx, 400, 300, 1);
			nk_style_set_font(ctx, &(uiAssets->font_30->handle));
			//nk_label(ctx, "TODO: Rocket", NK_TEXT_ALIGN_CENTERED);
			//nk_layout_row_end(ctx);

		} else {							

			if ((focusHex) && (focusHex->exploreCount==0) && (focusHex->stat_numEnemies > 0)) {
				
				// Can't build here, there are enemies. 
				//nk_layout_space_begin(ctx, NK_STATIC, 101 /*height*/, 6 /*numchilds*/);

				ctx->style.text.color = nk_rgb(255, 255, 255);
				nk_style_set_font(ctx, &(uiAssets->font_30->handle));				
				nk_label(ctx, "Barbarians!", NK_TEXT_LEFT);


				//nk_layout_space_push(ctx, nk_rect(0, 0, 300, 151));
				//nk_image(ctx, uiAssets->img_panel_enemies);
				nk_style_set_font(ctx, &(uiAssets->font_20->handle));
				nk_label_wrap(ctx, "Unfortunately I ran out of time to finish implementing barbarians." );
				
				//nk_layout_space_end( ctx );
			}
			else {

				// This also filters out enemy type buildings
				int currentType = focusHex->terrain == GameHex::TerrainType::Terrain_OCEAN?Type_RESOURCE_OCEAN : Type_RESOURCE;
				if ((focusHex) && (focusHex->exploreCount > 0)) {
					currentType = Type_EXPLORE;
				}

				for (int i = 0; i < bldgSpecs.Size(); i++)
				{
					BuildingInfo& bldg = bldgSpecs[i];

					if (bldg.Type != currentType) continue;

					nk_layout_space_begin(ctx, NK_STATIC, 101 /*height*/, 6 /*numchilds*/);

					nk_layout_space_push(ctx, nk_rect(0, 0, 300, 100));
					nk_image(ctx, uiAssets->img_panel_bldg_food);

					ctx->style.text.color = nk_rgb(255, 255, 255);
					nk_style_set_font(ctx, &(uiAssets->font_30->handle));
					nk_layout_space_push(ctx, nk_rect(43, 0, 257, 32));
					nk_label(ctx, bldg.bldgName.AsCStr(), NK_TEXT_LEFT);

					ctx->style.text.color = nk_rgb(40, 40, 40);
					nk_style_set_font(ctx, &(uiAssets->font_14->handle));
					nk_layout_space_push(ctx, nk_rect(105, 38, 183, 47));
					nk_label_wrap(ctx, bldg.desc.AsCStr());
					
					char buff[32];
					nk_layout_space_push(ctx, nk_rect(163, 0, 90, 16));
					if (bldg.countOnFocusHex == 0) {
						sprintf(buff, "Build Limit: %d", bldg.BuildLimit);
					}
					else {
						if (bldg.countOnFocusHex >= bldg.BuildLimit) {
							ctx->style.text.color = nk_rgb(100, 80, 40);
						}
						sprintf(buff, "Built %d of %d", bldg.countOnFocusHex, bldg.BuildLimit);
					}
					nk_label(ctx, buff, NK_TEXT_LEFT);

					ctx->style.text.color = nk_rgb(2, 100, 30);
					nk_layout_space_push(ctx, nk_rect(133, 78, 120, 17));
					sprintf(buff, "Cost: %zu", bldg.BaseCost);
					nk_label(ctx, buff, NK_TEXT_LEFT);

					const char* buyPrompt = "Buy!";
					bool canBuild = true;
					if (resFood < bldg.BaseCost) {
						buyPrompt = "Can't Afford";
						canBuild = false;
					}
					else if (bldg.countOnFocusHex >= bldg.BuildLimit) {
						buyPrompt = "Built Limit";
						canBuild = false;
					}
					else if (focusHex->bldgs.Size() >= 12) {
						buyPrompt = "Full";
						canBuild = false;
					}

					if (canBuild) {
						uiAssets->buttonStyleNormal(ctx);
					}
					else {
						uiAssets->buttonStyleDisabled(ctx);
					}

					nk_layout_space_push(ctx, nk_rect(229, 76, 60, 17));
					if (nk_button_label(ctx, buyPrompt) && canBuild) {
						BuildBuilding(focusHex, &bldg);
					}

					nk_layout_space_end(ctx);
				}
			}
		}
	}
	nk_end(ctx);
}




