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

class Cannon
{
public:

    Cannon() {}
    Cannon( Scene *scene, glm::vec3 anchorPos, glm::vec3 upDir );
        
    glm::vec3 teamColor;
    
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
    
    void updatePlacement();
    
    glm::vec3 calcProjectileVel();
    
    SceneObject *objBase;
    SceneObject *objBushing;
    SceneObject *objBarrel;
};


class Shot
{
public:
    Shot( SceneObject *shotObj, glm::vec3 pos, glm::vec3 startVel );
    
    float age;
    glm::vec3 vel;
    SceneObject *objShot;
    
    void updateBallistic(float dt, glm::vec3 gravity );
};
