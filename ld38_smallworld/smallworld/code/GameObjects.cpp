#include "GameObjects.h"

// ========================================================
//  Team Infos
// ========================================================


void MakeDefaultTeams( Oryol::Array<TeamInfo> &teams )
{
    
    TeamInfo teamVegans;
    teamVegans.teamName = "Militant Vegans";
    teamVegans.teamColor = glm::vec4(0.52,0.67,0.20,1.0);
    teamVegans.playerType = Player_NONE;
    
    teamVegans.names.Add( "Locust" );
    teamVegans.names.Add( "Shaylee" );
    teamVegans.names.Add( "Orinda" );
    teamVegans.names.Add( "Trey" );
    teamVegans.names.Add( "Trent" );
    teamVegans.names.Add( "Trey" );
    teamVegans.names.Add( "Blaze" );
    teamVegans.names.Add( "Mogshade" );
    teamVegans.names.Add( "Areola" );
    teamVegans.names.Add( "Azalae" );
    teamVegans.names.Add( "Aurora" );
    teamVegans.names.Add( "Loam" );
    teamVegans.names.Add( "Compostia" );
    teamVegans.names.Add( "Creche" );
    teamVegans.names.Add( "Banana" );
    teamVegans.names.Add( "Winter" );
    teamVegans.names.Add( "Produce" );
    teamVegans.names.Add( "Tomilla" );
    teams.Add( teamVegans );
    
    
    TeamInfo scotsmen;
    scotsmen.teamName = "True Scotsmen";
    scotsmen.teamColor = glm::vec4(0.68,0.40,0.74, 1.0);
    scotsmen.playerType = Player_NONE;
    scotsmen.names.Add( "Angus" );
    scotsmen.names.Add( "Calum" );
    scotsmen.names.Add( "Duff" );
    scotsmen.names.Add( "Fingal" );
    scotsmen.names.Add( "Hamish" );
    scotsmen.names.Add( "Nevin" );
    scotsmen.names.Add( "Torquil" );
    scotsmen.names.Add( "Ranald" );
    scotsmen.names.Add( "Dubh Glas" );
    scotsmen.names.Add( "Ewan" );
    scotsmen.names.Add( "Aileen" );
    scotsmen.names.Add( "Bonnie" );
    scotsmen.names.Add( "Effie" );
    scotsmen.names.Add( "Fiona" );
    scotsmen.names.Add( "Gillian" );
    scotsmen.names.Add( "Jinty" );
    scotsmen.names.Add( "Senga" );
    scotsmen.names.Add( "Shona" );
    scotsmen.names.Add( "Agnes" );
    scotsmen.names.Add( "Nessie" );
    teams.Add( scotsmen );
    
    TeamInfo deptOfAg;
    deptOfAg.teamName = "Dept. of Agriculture";
    deptOfAg.teamColor = glm::vec4(0.84,0.38,0.29, 1.0);
    deptOfAg.playerType = Player_NONE;
    deptOfAg.names.Add( "Donald" );
    deptOfAg.names.Add( "Jared" );
    deptOfAg.names.Add( "Hillary" );
    deptOfAg.names.Add( "Ivanka" );
    deptOfAg.names.Add( "Barack" );
    deptOfAg.names.Add( "Joe" );
    deptOfAg.names.Add( "Dubya" );
    deptOfAg.names.Add( "Betsy" ); // Ross or DeVos, depends whether you hate teaching kids
    deptOfAg.names.Add( "Putin" );
    deptOfAg.names.Add( "Paulryan" );
    deptOfAg.names.Add( "Chuck" );
    deptOfAg.names.Add( "Bernie" ); // would've won
    deptOfAg.names.Add( "Socks" );
    deptOfAg.names.Add( "Strangelove" );
    deptOfAg.names.Add( "Mandrake" );
    deptOfAg.names.Add( "Brexit" ); // i think this is a british politician??? ?
    deptOfAg.names.Add( "Mike" ); // Mike Young (I had to look it up on wikipedia)
    deptOfAg.names.Add( "Orville" );
    teams.Add( deptOfAg );
    
    TeamInfo chefs;
    chefs.teamName = "Epicurian Mafia";
    chefs.teamColor = glm::vec4(0.16,0.68,0.68, 1.0);
    chefs.playerType = Player_NONE;
    chefs.names.Add( "Gordon" );
    chefs.names.Add( "Boyardee" );
    chefs.names.Add( "Guy" );
    chefs.names.Add( "Fieri" ); // so good he has to be in there twice
    chefs.names.Add( "Emeril" );
    chefs.names.Add( "Wolfgang" );
    chefs.names.Add( "Mario" );
    chefs.names.Add( "Deen" );
    chefs.names.Add( "Julia" );
    chefs.names.Add( "Alton" );
    chefs.names.Add( "Ming" );
    chefs.names.Add( "Bork" );
    chefs.names.Add( "Remy" );
    teams.Add( chefs );
    
    // Shuffle teams
    TeamInfo temp;
    for (int i=0; i < 10; i++) {
        int a = rand() % teams.Size();
        int b = rand() % teams.Size();
        if (a==b) continue;
        
        temp = teams[a];
        teams[a] = teams[b];
        teams[b] = temp;
    }
    
    // Default is player vs cpu
    teams[0].playerType = Player_HUMAN;
    teams[2].playerType = Player_CPU;
}
// ========================================================
//  AmmoInfo
// ========================================================
AmmoInfo::AmmoInfo( const char *_name )
{
    name = _name;
    fatalRadius = 500.0f;
    damageRadius = 500.0f;
    splashRadius = 800.0f;
    wackyGravity = false;
    craterNoise = 0.0f;
    defaultSupply = -1;
}

void MakeDefaultAmmos( Oryol::Array<AmmoInfo> &ammos )
{
    // Basic bomb, not much damage
    AmmoInfo ammo = AmmoInfo( "Pea Shooter");
    ammos.Add( ammo );

    // Large bomb, but does little damage
    ammo = AmmoInfo( "Pumpkin Eater");
    ammo.fatalRadius = 350.0;
    ammo.damageRadius = 1000.0;
    ammo.splashRadius = 200.0;
    ammo.craterNoise = 0.5;
    ammo.defaultSupply = 10;
    ammos.Add( ammo );

    ammo = AmmoInfo( "Eggplant");
    ammo.defaultSupply = 20;
    ammos.Add( ammo );

    ammo = AmmoInfo( "Fertilizer");
    ammo.defaultSupply = 5;
    ammos.Add( ammo );


//    "Pea Shooter",
//    "Pumpkin Eater",
//    "Grapeshot",
//    "Emoji Eggplant",
//    "Guided Carrot",
//    "Fertilizer"
    
}

// ========================================================
//   Cannon
// ========================================================


Cannon::Cannon( Scene *scene, TeamInfo *_team, glm::vec3 anchorPos, glm::vec3 upDir )
{
    team = _team;
    
    // TODO: don't allow duplication of names
    name = _team->names[ rand() % _team->names.Size() ];
    
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

void Cannon::makeDead() {
    
    health = 0;
    
    glm::vec4 color = glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f );
    objBase->fsParams.TintColor = color;
    objBarrel->fsParams.TintColor = color;
    objBushing->fsParams.TintColor = color;
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
    glm::vec4 color = team->teamColor;
    objBase->fsParams.TintColor = color;
    objBarrel->fsParams.TintColor = color;
    objBushing->fsParams.TintColor = color;
    
}

void Cannon::pulseActive( float t )
{
    glm::vec4 highlightColor = glm::vec4(1.00,0.86,0.48,1.0);
    glm::vec4 color = glm::mix( team->teamColor, highlightColor, fabs(sinf( t * 10.0 )) );
    
    objBase->fsParams.TintColor = color;
    objBarrel->fsParams.TintColor = color;
    objBushing->fsParams.TintColor = color;
}

// ========================================================
//   Shot
// ========================================================

Shot::Shot( SceneObject *shotObj, AmmoInfo *_ammo, glm::vec3 pos, glm::vec3 startVel )
{
    ammo = _ammo;
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


