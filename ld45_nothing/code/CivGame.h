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

#define BOARD_SZ (9)

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

	// Gameplay stuff
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


	int boardNdx(int i, int j);
	GameHex *boardHex(int i, int j);

	Tapnik::UIAssets* uiAssets; 

	Tapnik::Scene* scene;
	Oryol::Id testTex;
	GameHex *board[BOARD_SZ*BOARD_SZ];

	GameHex* hoverHex;
	GameHex* focusHex;

	Tapnik::Ray mouseRay;
	glm::vec3 groundCursor;
};

#endif