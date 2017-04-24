#pragma once

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Assets/Gfx/TextureLoader.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "shaders.h"
#include "IsosurfaceBuilder.h"
#include "SceneObject.h"

enum PlayerType {
    Player_NONE,  // Not playing
    Player_HUMAN, // Human player
    Player_CPU    // Computer player
};

class TeamInfo
{
public:
    PlayerType playerType;
    const char *teamName;
    glm::vec4 teamColor;
    int selectedWeapon;
    int ammoSupply[20]; // current ammo
    Oryol::Array<const char*> names;
    
};

void MakeDefaultTeams( Oryol::Array<TeamInfo> &teams );

class AmmoInfo {
public:
    AmmoInfo( const char *name );
    const char *name;
    
    const char *meshName;
    const char *textureName;
    
    float damageRadius; // how much dirt to blast, kills anything within this radius outright
    float fatalRadius;  // kills anything within this radius outright
    float splashRadius; // Does damage out here
    
    float craterNoise; // How noisy is the crater?
    
    int defaultSupply; // -1 = unlimited
    int currentSupply; // -1 = unlimited
    
    float ammoScale;
    
    float boomAge;  // at what time to explode if we haven't hit something already
    float splitAge; // at what time to split
    float splitNum; 
    
    bool wackyGravity;
};

void MakeDefaultAmmos( Oryol::Array<AmmoInfo> &ammos );

class Cannon
{
public:

    Cannon() {}
    Cannon( Scene *scene, TeamInfo *_team, glm::vec3 anchorPos, glm::vec3 upDir );
    
    const char *name;
    
    TeamInfo *team;
    
    
    // Gameplay fields
    int health;
    int maxHealth;
    float aimHeading; // heading, -180.0 to 180.0
    float cannonAngle; // tilt, 0 to 180.0
    float power;
    
    // internals for placeing stuff
    float _placementAngle;
    glm::vec3 _anchorPoint;
    glm::vec3 _shootyPoint;
    glm::vec3 _shootyDir;
    glm::vec3 _upDir;
    
    void place( glm::vec3 anchorPos, glm::vec3 upDir );
    void updatePlacement();
    
    void update( float dt, float animT, bool active );
            
    int blinker;
    
    void makeDead();
    
    glm::vec3 calcProjectileVel();
    
    float showDamageTimer;
    
    SceneObject *objBase;
    SceneObject *objBushing;
    SceneObject *objBarrel;
};


class Shot
{
public:
    Shot( SceneObject *shotObj, AmmoInfo *_ammo, glm::vec3 pos, glm::vec3 startVel );
    
    float age;
    glm::vec3 vel;
    AmmoInfo *ammo;
    SceneObject *objShot;
    
    bool splitDone;
    
    void updateBallistic(float dt, glm::vec3 gravity );
};
