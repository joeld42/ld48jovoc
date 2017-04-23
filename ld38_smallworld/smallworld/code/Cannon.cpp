#include "Cannon.h"


Cannon::Cannon( Scene *scene, glm::vec3 anchorPos, glm::vec3 upDir )
{
    
    int teamNum = rand() % 4;
    if (teamNum==0) {
        teamColor = glm::vec4(0.84,0.38,0.29, 1.0);
    } else if (teamNum==1) {
        teamColor = glm::vec4(0.16,0.68,0.68, 1.0);
    } else if (teamNum==2) {
        teamColor = glm::vec4(0.52,0.67,0.20,1.0);
    } else if (teamNum==3) {
        teamColor = glm::vec4(0.68,0.40,0.74, 1.0);
    }
    
    // Initialize gameplay stuff
    aimHeading = 0.0;
    cannonAngle = glm::linearRand( 0.0f, 180.0f  );
    power = 1.0f;
    
    health = 6;
    maxHealth=6;
    
    objBase = scene->addObject( "msh:cannon_base.omsh", "tex:cannon_basecolor.dds");
    objBushing = scene->addObject( "msh:cannon_bushing.omsh", "tex:cannon_basecolor.dds");
    objBarrel = scene->addObject( "msh:cannon_barrel.omsh", "tex:cannon_basecolor.dds");
    
    _placementAngle = glm::linearRand( 0.0f, 360.0f );
    place( anchorPos, upDir );
}

void Cannon::place( glm::vec3 anchorPos, glm::vec3 upDir )
{
    _anchorPoint = anchorPos;
    _upDir = upDir;
    updatePlacement();
}

void Cannon::updatePlacement()
{
    glm::vec3 modelUpDir ( 0.0, 1.0, 0.0 );
    glm::vec3 modelShootyDir ( -1.0, 0.0, 0.0 );
    
    glm::quat qrotUpright( modelUpDir, _upDir );
    
    glm::quat placementRot = glm::rotate( glm::quat(), glm::radians( _placementAngle + aimHeading ), _upDir  );
    glm::quat baseRot = placementRot * qrotUpright;
    
    glm::quat tiltRot = glm::rotate( glm::quat(), glm::radians( cannonAngle), glm::vec3( 0.0, 0.0, -1.0) );
    
    objBase->pos = _anchorPoint;
    objBase->rot = qrotUpright;
    
    objBushing->pos = _anchorPoint;
    objBushing->rot = baseRot;
    
    _shootyPoint =  _anchorPoint + (_upDir * 191.6f);
    objBarrel->pos = _shootyPoint;
    objBarrel->rot = baseRot * tiltRot;
    
    _shootyDir = objBarrel->rot * modelShootyDir;
}

glm::vec3 Cannon::calcProjectileVel()
{
    return _shootyDir * (power*5000.0f);
}

void Cannon::applyTeamColor()
{
    // TODO: Pulse active color
    glm::vec4 color = teamColor;
    objBase->fsParams.TintColor = color;
    objBarrel->fsParams.TintColor = color;
    objBushing->fsParams.TintColor = color;
    
}

void Cannon::pulseActive( float t )
{
    glm::vec4 highlightColor = glm::vec4(1.00,0.86,0.48,1.0);
    glm::vec4 color = glm::mix( teamColor, highlightColor, fabs(sinf( t * 10.0 )) );
    
    objBase->fsParams.TintColor = color;
    objBarrel->fsParams.TintColor = color;
    objBushing->fsParams.TintColor = color;
}

// ========================================================
//   Shot
// ========================================================

Shot::Shot( SceneObject *shotObj, glm::vec3 pos, glm::vec3 startVel )
{
    objShot = shotObj;
    objShot->pos = pos;
    vel = startVel;
    age = 8.0f;
    
    // TODO: random spin
}

void Shot::updateBallistic(float dt, glm::vec3 gravity )
{
    vel += gravity * dt;
    vel *= 0.99;  // space drag from cosmic dust
    
    objShot->pos += vel * dt;
    
    age -= dt;
}


