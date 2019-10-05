
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"

#include "Input/Input.h"

#include "CivGame.h"

#include "SceneObject.h"
#include "LDJamFile.h"
#include "Camera.h"

using namespace Tapnik;
using namespace Oryol;

// =========================================================
//  GameHex
// =========================================================
GameHex::GameHex(int hx, int hy) : hx(hx), hy(hy), bobTime(0.0)
{
	bobTime = glm::linearRand(0.0f, 10.0f );
	bobRate = glm::linearRand(0.9f, 1.5f);
	hoverBump = 0.0f;
}

// =========================================================
//  CivGame
// =========================================================
CivGame::CivGame() : scene(NULL), uiAssets(NULL), hoverHex( NULL ), focusHex(NULL)
{
	
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
	printf("CivGame::SetupWithScene\n");

	testTex = scene->sceneObjs[0]->mesh->texture;

	// Remove all of the object instances (but keep the meshes)
	while (scene->sceneObjs.Size() > 0) {
		SceneObject* obj = scene->sceneObjs[0];
		printf("Removing object %s\n", obj->objectName.AsCStr());
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

			SceneMesh* hexMesh = BuildHexMesh(hexSize * 0.96);
			SceneObject* hexTile = scene->spawnObject(hexMesh);
			glm::vec3 tilePos = glm::vec3(float(i) - gridhalf, float(j) - gridhalf, 0.0f);
			float offs = ((i & 0x1) == 0) ? 0.0f : hexHite * 0.5;
			glm::vec3 hexPos = (tilePos * glm::vec3(hexSize * 2.0f * 0.75f, hexHite, 1.0f)) + glm::vec3( 0.0f, offs, 0.0f );

			hex->hexWorldPos = hexPos;
			hex->pos = hexPos;
			hex->sceneObj = hexTile;
			hexTile->xform = glm::translate(glm::mat4(1.0f), hexPos);			
		}
	}
}

void CivGame::SetupGameBoard()
{
	// TODO: allow different size games
	int rowStart[9] = { 3, 1, 0, 0, 0, 0, 0, 2, 4  };
	int rowEnd[9] = { 5, 7, 8, 8, 8, 8, 8, 6, 4 };


	int CC = BOARD_SZ / 2;
	for (int i = 0; i < BOARD_SZ; i++) {
		for (int j = 0; j < BOARD_SZ; j++) {
			int boardNdx = (j * BOARD_SZ) + i;
			
			if ( ((i<rowStart[j]) || (i > rowEnd[j]))) {
				// Empty hex
				board[boardNdx] = NULL;
			} else {
				board[boardNdx] = Memory::New<GameHex>(i, j);
			}
		}
	}
}

void CivGame::fixedUpdate(Oryol::Duration fixedDt, Tapnik::Camera *activeCamera )
{

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
			hex->pos = glm::vec3(hex->hexWorldPos.x, hex->hexWorldPos.y, sin(hex->bobTime * 0.5 * hex->bobRate ) * 0.1 + hex->hoverBump );

			// Update hover dist
			glm::vec2 hexPos2d = glm::vec2(hex->hexWorldPos.x, hex->hexWorldPos.y);
			float dd = glm::distance(hexPos2d, groundCursor2D);
			//if (dd < hexSize*1.2) {
				if ((!closestHex) || (dd < closestHexDist)) {
					closestHex = hex;
					closestHexDist = dd;
				}
			//}
	
			// Update the transform
			hex->sceneObj->xform = glm::translate(glm::mat4(1.0f), hex->pos );
		}
	}

	if (hoverHex != closestHex)
	{
		if (hoverHex) {
			hoverHex->hoverBump = 0.0f;
		}
		hoverHex = closestHex;
		if (hoverHex)
		{
			hoverHex->hoverBump = 0.2f;
			// TODO: play click sound
		}
	}


}

Tapnik::SceneMesh* CivGame::BuildHexMesh( float hexSize )
{
	int numCapVerts = 7; // six + center
	int numCapTris = 6;

	size_t vertBufferSize = sizeof(LDJamFileVertex) * numCapVerts;	
	size_t indexBufferSize = sizeof(uint16_t) * numCapTris * 3;
	size_t geomBufferSize = vertBufferSize + indexBufferSize;
	
	
	float r = 2.0f;
	float s3 = (sqrt(3.0)*r) / 2.0;
	float r2 = r / 2.0f;
	uint8_t *geomBuffer = (uint8_t*)Memory::Alloc(geomBufferSize);
	LDJamFileVertex* verts = (LDJamFileVertex*)geomBuffer;
	verts[0].m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	
	verts[1].m_pos = glm::vec3(r, 0.0f, 0.0f);
	verts[2].m_pos = glm::vec3(r2, s3, 0.0f);
	verts[3].m_pos = glm::vec3( -r2, s3, 0.0f);
	verts[4].m_pos = glm::vec3(-r, 0.0f, 0.0f);
	verts[5].m_pos = glm::vec3(-r2, -s3, 0.0f);
	verts[6].m_pos = glm::vec3(r2,-s3, 0.0f);

	for (int i = 0; i < numCapVerts; i++) {
		glm::vec2 st = glm::vec2(verts[i].m_pos) * glm::vec2(0.25f, -0.25f);
		verts[i].m_nrm = glm::vec3(0.f, 0.f, 1.f);
		verts[i].m_st0 = st + glm::vec2( 0.5f, 0.5f);

		verts[i].m_pos *= hexSize/2.0;
	}

	uint16_t* indexes = (uint16_t*)(geomBuffer + vertBufferSize);
	indexes[0] = 0;
	indexes[1] = 1;
	indexes[2] = 2;

	indexes[3] = 0;
	indexes[4] = 2;
	indexes[5] = 3;

	indexes[6] = 0;
	indexes[7] = 3;
	indexes[8] = 4;

	indexes[9] = 0;
	indexes[10] = 4;
	indexes[11] = 5;

	indexes[12] = 0;
	indexes[13] = 5;
	indexes[14] = 6;

	indexes[15] = 0;
	indexes[16] = 6;
	indexes[17] = 1;

	SceneMesh* hexMesh = scene->BuildMesh("Hex_MESH", testTex, geomBuffer,
		numCapVerts, numCapTris * 3);

	return hexMesh;
}

