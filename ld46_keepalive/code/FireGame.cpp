#include "FireGame.h"

using namespace Oryol;
using namespace Tapnik;

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Dbg/Dbg.h"
#include "Input/Input.h"
#include "Core/Time/Clock.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/random.hpp"

#define kLogFuel (60.0f)

void FireGame::Setup(Tapnik::Scene* scene)
{
	_scene = scene;

	_playerPos = glm::vec3(0.0f, 0.0f, 1000.0f);
	_playerDir = glm::vec3(0.0f, 1.0f, 0.0f);

	_holdingObj = NULL;
	_playerObj = _scene->FindNamedObject("Player");
	if (_playerObj == NULL) {
		Log::Info("Couldn't find Player Object!\n");
	}
	else {
		_playerPos = glm::vec3(_playerObj->xform[3].x, _playerObj->xform[3].y, 1000.0f);
	}
	_playerStartPos = _playerPos;

	_groundObj = _scene->FindNamedObject("Ground");
	if (_groundObj == NULL) {
		Log::Info("Couldn't find Ground Object!\n");
	}	

	// find meshes for game objs
	_logObj = _scene->FindNamedObject("Log");
	_logMesh = _logObj->mesh;

	SceneObject *fireObj = _scene->FindNamedObject("Fire");
	_fireMesh = fireObj->mesh;
	_scene->destroyObject(fireObj);

	// make items for shrines
	_numShrines = 0;
	for (int i = 0; i < _scene->sceneObjs.Size(); i++) {
		SceneObject* obj = _scene->sceneObjs[i];
		if (obj->mesh->meshName == "shrine_mesh") {
			GameItem shrineItem = {};
			shrineItem._obj = obj;
			shrineItem._type = GameItem_SHRINE;			
			_items.Add(shrineItem);
			_numShrines++;
		}
	}

	// Make a pickup for the "dummy" log, we could just remove it from the scene instead
	GameItem logItem;
	logItem._obj = _logObj;
	logItem._type = GameItem_LOG;
	_items.Add(logItem);

	_numLogsSpawned = 1;
	_spawnLogTimeout = 3.0f;

	_cameraZoom = 10.0f;
	_cameraTilt = 45.0f;
	_cameraHeading = 0.0f;

	_flameAmt = 5.0f;	
}

bool FireGame::PickupWorldItem(int itemNdx)
{
	GameItem& item = _items[itemNdx];

	if (item._type == GameItem_LOG)
	{
		//  it's a log 
		Log::Info("Log Info pickup log %d...\n", itemNdx);		
		if (_holdingObj == NULL) {
			//_scene->destroyObject(item._obj);
			_holdingObj = item._obj;

			// Remove it from the item list
			_items.EraseSwap(itemNdx);
			_numLogsSpawned -= 1;
			
			sfx->sfxYoink.Play();
		}
		else {
			Log::Info("Already carrying something...\n");
		}

		return true;
	}
	else if (item._type == GameItem_SHRINE)
	{
		// add fuel to the shrine
		if (_holdingObj)
		{
			Log::Info("Activate shrine %d\n", itemNdx);

			if (item._flame == NULL) {
				item._fuel = 0.0f;
				item._flame = _scene->spawnObject(_fireMesh);
			}
			item._fuel = glm::clamp(item._fuel + kLogFuel, 0.0f, kLogFuel * 2.0f);

			_scene->destroyObject(_holdingObj);
			_holdingObj = NULL;

			sfx->sfxFwoosh.Play();
		}
		else {
			Log::Info("Shrine %d: you don't have anything use the shrine\n", itemNdx);
		}
		return true;
	}
	return false;
	
}

void FireGame::SpawnLogItem()
{
	Log::Info("Spawn Log!\n");
	SceneObject* trees[50];
	int numTrees = 0;
	for (int i = 0; i < _scene->sceneObjs.Size(); i++) {
		SceneObject* obj = _scene->sceneObjs[i];
		if (obj->mesh->meshName == "tree_mesh") {
			trees[numTrees++] = obj;
			if (numTrees == 50) {
				Log::Info("WARNING: Too many trees.\n");
				break;
			}
		}		
		
	} 

	if (numTrees > 0) {
		int ndx = glm::linearRand(0, numTrees - 1);
		float dist = glm::linearRand(1.0f, 5.0f);
		float ang = glm::linearRand(0.0f, float(M_PI) * 2.0f);
		glm::vec3 treePos = glm::vec3(trees[ndx]->xform[3]);
		glm::vec3 logPos = treePos + glm::vec3(cos(ang), sin(ang), 0.0f) * dist;
			
		//trees[ndx]->hidden = true;

		bool logValid = true;
		float logGroundHite = calcLowerGroundHeight(logPos, logValid);
		logPos.z = logGroundHite;

		GameItem logItem;
		SceneObject *logObj = _scene->spawnObject(_logMesh);
		glm::mat4 xform = glm::translate(glm::mat4(1.0f), logPos);
		Log::Info("Spawn log at pos %3.2f, %3.2f, %3.2f\n", logPos.x, logPos.y, logPos.z);
		logObj->xform = xform;
		logItem._obj = logObj;
		logItem._type = GameItem_LOG;
		_items.Add(logItem);
		_numLogsSpawned += 1;
	}
	else {
		Log::Info("WARNING: No Trees found\n");
	}
	
}

void FireGame::fixedUpdate(Oryol::Duration fixedDt)
{
	float dt = fixedDt.AsSeconds();
	const float kPlayerMoveSpeed = 50.0f;
	const float kPlayerTurnSpeed = 1.0f;

	// Update world spawns
	if (_numLogsSpawned < 10)
	{
		_spawnLogTimeout -= dt;
		if (_spawnLogTimeout < 0.0f) {
			_spawnLogTimeout = glm::linearRand(5.0f, 10.0f);
			_spawnLogTimeout = 2.0f; // DBG
			SpawnLogItem();
		}
	}

	if (Input::KeyDown(Key::Space) || (Input::KeyDown(Key::E)) ) {
		
		// Check for pickup object		
		bool didPickup = false;
		for (int i = 0; i < _items.Size(); i++) {
			GameItem& item = _items[i];
			glm::vec3 itemPos = glm::vec3( item._obj->xform[3] );
			float d = glm::distance(itemPos, _playerPos);
			Log::Info("Distance to item %d is %3.2f\n", i, d );
			if (d < 4.0f) {				
				didPickup = PickupWorldItem(i);
				if (didPickup) {
					break;
				}
			}
		}
		if (!didPickup) {
			// Play pickup-fail sound fx
			sfx->sfxHuh.Play();
		}
	}


	// Update all the shrines
	float totalFuel = 0.0f;
	for (int i = 0; i < _items.Size(); i++) {
		GameItem& item = _items[i];
		if (item._type == GameItem_SHRINE) {
			if ((item._fuel > 0.0f) && (item._flame)) {
				totalFuel += item._fuel;

				item._fuel -= dt;
				if (item._fuel <= 0.0f) {
					Log::Info("A shrine went out!\n");
					_scene->destroyObject(item._flame);
					item._flame = NULL;

					sfx->sfxFireOut.Play();
				}
			}

			if (item._flame) {
				float size = glm::clamp((item._fuel / kLogFuel), 0.001f, 1.5f);
				
				// TODO
				float gameTime = 0.0f;
				glm::mat4 flameRot = glm::rotate( glm::mat4(1.0f), 
					glm::linearRand(0.0f, 2.0f * float(M_PI)), glm::vec3(0.0f, 0.0f, 1.0f));

				glm::mat4 flameOffs = glm::scale(glm::translate(glm::mat4(1.0f),
					glm::vec3(0.6f, 0.0f, 3.5f + sin(gameTime) * 0.4f)),
					glm::vec3(size, size, size));

				item._flame->xform = item._obj->xform * (flameRot * flameOffs);
			}
		}
	}

	
	//Log::Info("Total Fuel: %3.2f\n", totalFuel);
	_targFlameAmt = glm::clamp(totalFuel / (_numShrines * 60.0f), 0.0f, 1.0f );
	_flameAmt = glm::mix(_flameAmt, _targFlameAmt, 0.1f);

	// DBG for tuning flame size
	//_flameAmt = Input::MousePosition().y / 720.0;

	// TODO: Gamepad support
	float fwd = 0.0f;
	float strafe = 0.0f;
	if (Input::KeyboardAttached()) {
		if (Input::KeyPressed(Key::W) || Input::KeyPressed(Key::Up)) {
			fwd += 1.0f;
		}

		if (Input::KeyPressed(Key::S) || Input::KeyPressed(Key::Down)) {
			fwd -= 1.0f;
		}

		if (Input::KeyPressed(Key::A) || Input::KeyPressed(Key::Left)) {
			strafe -= 1.0f;
		}

		if (Input::KeyPressed(Key::D) || Input::KeyPressed(Key::Right)) {
			strafe += 1.0f;
		}

	}

	if (Input::KeyDown(Key::N0)) {
		_playerPos = _playerStartPos;
		
	}

	glm::vec3 nextPlayerPos;
	_playerDir = _cameraHeadingDir;
	nextPlayerPos = _playerPos + 
					(_cameraHeadingDir * (fwd * kPlayerMoveSpeed * dt) ) +
					(_cameraRightDir  * (strafe * kPlayerMoveSpeed * dt));

	// Get ground height
	bool validMove = true;
	float groundHite = calcLowerGroundHeight(nextPlayerPos + glm::vec3(0.0f, 0.0f, 1.0f), validMove );
	//groundHite = 100.0f;
	if ((validMove) && (groundHite > 0.0f))  {
		nextPlayerPos.z = groundHite;
	}
	else {
		// disallow the move
		nextPlayerPos = _playerPos;
	}
	
	glm::vec3 move = nextPlayerPos - _playerPos;	
	float moveLen = glm::length(move);
	if ( moveLen > 0.01f) {
		_playerFacingTarg = glm::normalize(move);
		_playerTravel += moveLen;
	}
	_playerFacing = glm::mix(_playerFacing, _playerFacingTarg, 0.1f);
	_playerAngle = atan2(-_playerFacing.x, _playerFacing.y);
	_playerPos = nextPlayerPos;

	// Update the player transform
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), _playerAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 xlate = glm::translate(glm::mat4(1.0f), _playerPos);
	_playerObj->xform = xlate * rot;
	//Log::Info("strafe %3.2f Player Pos %3.2f %3.2f %3.2f\n", strafe, _playerPos.x, _playerPos.y, _playerPos.z);

	// Update what the player is carrying if they are carrying something
	if (_holdingObj) {
		glm::mat4 holdingOffs = glm::translate(glm::mat4(1.0f), 
			glm::vec3(0.6f, 0.0f, 1.2f + fabs(sin(_playerTravel * 0.2f)) * 0.4f ) );
		_holdingObj->xform = _playerObj->xform * holdingOffs;
	}
	
	// Update the camera
	if (_gameCamera) {

		//_camera
		if (Input::KeyDown(Key::K)) {
			dbgPrintMatrix("gameCamera", _gameCamera->Model);
		}

		//float mx = (Input::MousePosition().x / 1280.0f) - 0.5f;
		//float my = (Input::MousePosition().y / 720.0f) - 0.5f;		
		//glm::vec3 camOrbitPos = glm::normalize(glm::vec3(mx, my, 1.0f - sqrtf(mx * mx + my * my))) * 10.0f;

		if (Input::MouseButtonPressed(MouseButton::Right))
		{
			glm::vec2 mm = Input::MouseMovement();
			_cameraTilt = glm::clamp(_cameraTilt + mm.y, 0.0f, 90.0f);

			_cameraHeading = _cameraHeading + mm.x;
			if (_cameraHeading > 360.0f) _cameraHeading -= 360.0f;
			if (_cameraHeading < 0.0f) _cameraHeading += 360.0f;
		}

		glm::vec2 ms = Input::MouseScroll();
		_cameraZoom = glm::clamp(_cameraZoom - ms.y * 10.0f, 8.0f, 50.0f );
		

		float hr = glm::radians(_cameraHeading);
		float tr = glm::radians(_cameraTilt);


		_cameraHeadingDir = glm::vec3( -cos(hr), -sin(hr), 0.0f);
		_cameraRightDir = glm::vec3(-cos(hr-M_PI/2.0), -sin(hr-M_PI/2.0), 0.0f);
		glm::vec3 camOrbitPos = glm::vec3(cos(hr), sin(hr), sin(tr)) * _cameraZoom;
		
		_cameraPos = _playerPos + camOrbitPos;

		bool validMove = true;
		float cameraGroundHite = calcLowerGroundHeight( _cameraPos + glm::vec3( 0.0f, 0.0f, 1.0f), validMove) + 0.3f;
		
		// don't let camera go underwater		
		if (validMove) {
			//Log::Info("Camera ground hite %3.2f %3.2f\n", cameraGroundHite, _cameraPos.z);
			if (cameraGroundHite < 0.5f) {
				cameraGroundHite = 0.5f;
			}

			if (cameraGroundHite > _cameraPos.z) {				
				_cameraPos.z = cameraGroundHite;
			}
		}


		glm::vec3 camShift = glm::vec3(0.0f, 0.0f, 4.0f);
		glm::mat4 mlookAt = glm::inverse( 
			glm::lookAt(
				_cameraPos + camShift, 
				_playerPos + camShift, 
			glm::vec3(0.0f, 0.0f, 1.0f)) );

		_gameCamera->UpdateModel(mlookAt);

		// Dbg box at desired camera pos
		//const ddVec3 boxColor = { 1.0f, 0.8f, 0.01f };		
		//float boxSize = 1.0f;
		//dd::box( glm::value_ptr(_cameraPos), boxColor, boxSize, boxSize, boxSize);
		//dd::cross(glm::value_ptr(cameraTarg), 1.0f);

		//glm::mat4 camXform = glm::mat4(1.0);
		//glm::mat4 camXform = glm::lookAt(
			//glm::vec3(0.0f, 0.0f, 0.0f),
			//glm::vec3(0.0f, 0.0f, 0.0f),
			//glm::vec3(0.0f, 1.0f, 0.0f)); // should give the GL z identity
		
		// rotate up to our y-front
		//float my = Input::MousePosition().y / 720.0f;
		//Log::Info("Tilt %f\n", my);
		//glm::mat4 rot = glm::rotate(glm::mat4(1.0f), ((float)(M_PI) / 2.0f) * my, glm::vec3(1.0f, 0.0f, 0.0f));
		//camXform = camXform * rot;

		//dbgPrintMatrix("lookAt", camXform);

		
		
	}

	
}

void FireGame::DoGameUI_Gameplay(nk_context* ctx, Tapnik::UIAssets* uiAssets)
{
	struct nk_panel layout = {};
	struct nk_rect bounds;
	const struct nk_input* in = &ctx->input;

	ctx->style.window.fixed_background = nk_style_item_image(uiAssets->img_top_graphic);
	if (nk_begin(ctx, "top_status_bar", nk_rect(0, 0, 1280, 74), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_space_begin(ctx, NK_STATIC, 74 /*height*/, 1 + _numShrines /*numchilds*/);

		// holding icon
		int itemIndex = 0;
		if (_holdingObj) {
			itemIndex = 1;
		}
		int x = (itemIndex % 4) * 64;
		int y = (itemIndex / 4) * 64;
		struct nk_image pic_img = nk_subimage_handle(uiAssets->img_item_icons.handle,
			256, 256, nk_rect(x, y, 64, 64));
		nk_layout_space_push(ctx, nk_rect(27, 4, 64, 64));
		nk_image(ctx, pic_img);

		// Shrine icons
		int sx = 640 - (_numShrines * 25);
		for (int i = 0; i < _items.Size(); i++) {
			GameItem& item = _items[i];
			if (item._type != GameItem_SHRINE) continue;

			// flame is out
			int flameNdx = 0;
			if (item._flame != NULL) {
				flameNdx = 1 + int((item._fuel / (kLogFuel * 1.5)) * 7);
				if (flameNdx > 7) flameNdx = 7;
			}
			struct nk_image pic_img = nk_subimage_handle(uiAssets->img_shrine_icons.handle,
				400, 34, nk_rect( flameNdx * 50, 0, 50, 34));
			nk_layout_space_push(ctx, nk_rect(sx, 16, 50, 34));
			nk_image(ctx, pic_img);
			
			sx += 50;
		}
	}
	nk_end(ctx);
}

void FireGame::interfaceScreens(Tapnik::UIAssets* uiAssets)
{
	// Draw UI Layer
	nk_context* ctx = NKUI::NewFrame();
	uiAssets->storeDefaultStyle(ctx);

	nk_style_set_font(ctx, &(uiAssets->font_14->handle));
	uiAssets->buttonStyleNormal(ctx);

	DoGameUI_Gameplay(ctx, uiAssets);


	NKUI::Draw();

	// DONT DO THIS it recreates all the windows every frame
	//nk_clear(ctx);
}

float FireGame::calcLowerGroundHeight(glm::vec3 pos, bool& valid)
{
	if ((!_groundObj)||(!_groundObj->mesh)) {
		valid = false;
		return pos.z;
	}

	float bestHite = -10000.0f;
	SceneMesh* groundMesh = _groundObj->mesh;	
	for (int i = 0; i < groundMesh->groundTris.Size(); i++)
	{
		GroundTriangle gtri = groundMesh->groundTris[i];

		//Log::Info("%d -- A %3.2f %3.2f %3.2f B %3.2f %3.2f %3.2f C %3.2f %3.2f %3.2f\n",
		//	i,
		//	gtri.a.x, gtri.a.y, gtri.a.z,
		//	gtri.b.x, gtri.b.y, gtri.b.z,
		//	gtri.c.x, gtri.c.y, gtri.c.z);

		//if (i > 4) break;
		float u, v, w;
		gtri.GroundBarycentric(pos, u, v, w);
		if ((u >= 0.0f) && (u < 1.0f) &&
			(v >= 0.0f) && (v < 1.0f) &&
			(w >= 0.0f) && (w < 1.0f)) {
			
			// Is inside
			float result = (u * gtri.a.z) + (v * gtri.b.z) + (w * gtri.c.z);
			
			// only count surfaces below the player
			if ((result < pos.z) && (result > bestHite)) {
				//Log::Info("hite is %f\n", result);		
				bestHite = result;
				valid = true;
			}
		}
	}

	if (bestHite < -9000) {
		bestHite = pos.z;
		valid = false;
	}
	
	return bestHite;
}
