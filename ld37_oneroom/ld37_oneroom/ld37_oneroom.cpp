
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "raylib.h"

//#define RAYMATH_IMPLEMENTATION
#include "raymath.h"
#include "math.h"

#include <OpenGL/gl3.h>

Color colorBackground = (Color){ 90, 102, 140, 255 };
Color colorSun = (Color){ 242, 232, 144, 255 };
Color colorAmbient = (Color){ 49, 56, 80 };

#define SCENE_MAX_INST (100)

//#define NUM_AGES

inline Vector3 Vector3Make( float x, float y, float z )
{
    Vector3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

inline float RandUniform() {
    return (float)rand() / (float)RAND_MAX;
}

inline float RandUniformRange( float rangeMin, float rangeMax ) {
    return rangeMin + RandUniform()*(rangeMax - rangeMin);
}

inline Color ColorLerp( Color a, Color b, float t) {
    Color result;
    float tInv = 1.0 - t;
    result.r = t*a.r + tInv*b.r;
    result.g = t*a.g + tInv*b.g;
    result.b = t*a.b + tInv*b.b;
    result.a = t*a.a + tInv*b.a;
    return result;
}


struct CreepObject;

// ===================================================================
struct SceneObject {
    const char *name;
    Model model;
    
    // Special for spawners
    CreepObject *spawner;
    
    // Construction Params
    int lifetimeMin;
    int lifetimeMax;
    float growthRate;
    Vector3 startSize;
    SceneObject *decayInto;
    
    float spawnTime;
};

struct SceneInstance {
    SceneObject *object;
    uint32_t objId;
    Vector3 pos;
    Vector3 scale;
    float rotation;
    BoundingBox bbox;
    bool drawWires;
    Color wireColor;
    
    // Spawner settings
    float spawnCooldown;
    
    SceneInstance *nextInst;
};

SceneInstance *SceneInstanceCreate()
{
    SceneInstance *inst = (SceneInstance*)malloc(sizeof(SceneInstance));
    memset( inst, 0, sizeof(SceneInstance));
    inst->objId = rand();
    inst->scale = Vector3Make( 1.0, 1.0, 1.0 );
    
    return inst;
}

SceneInstance *SceneInstanceClone( SceneInstance *orig )
{
    SceneInstance *inst = (SceneInstance*)malloc(sizeof(SceneInstance));
    memcpy( inst, orig, sizeof(SceneInstance));
    
    inst->nextInst = NULL;
    
    return inst;
}

struct Scene {
    char *name;
    SceneInstance *firstInst;
};

#define NUM_YEARS (101)
Scene sceneYear[NUM_YEARS];

int currentYear;
Scene *scene = NULL;

#define MAX_OBJECTS (100)
SceneObject objects[MAX_OBJECTS];
int numObjects = 0;

SceneObject *LoadSceneObject( const char *name )
{
    assert( numObjects < MAX_OBJECTS );
    char buff[1024];
    
    SceneObject *obj = objects + numObjects++;
    obj->name = strdup(name);
    
    sprintf(buff, "gamedata/%s.obj", name );
    obj->model = LoadModel( buff );
    obj->model.material = LoadStandardMaterial();
    sprintf(buff, "gamedata/%s.png", name );
    Texture2D texture = LoadTexture(buff);
    obj->model.material.texDiffuse = texture;
    obj->startSize = Vector3Make(1.0, 1.0, 1.0);
    
    return obj;
}


void SceneStampObject( Scene *targScene, Vector3 pos, SceneObject *obj )
{
    SceneInstance *inst = SceneInstanceCreate();
    
    // Add to the current scene
    inst->nextInst = targScene->firstInst;
    targScene->firstInst = inst;

    inst->object = obj;
    inst->pos = pos;
    inst->scale = obj->startSize;
    
    inst->rotation = RandUniformRange( 0.0, 360 );
    
    // Add "future" copies
    int lifetime = GetRandomValue(obj->lifetimeMin, obj->lifetimeMax );
    
    Scene *currScene = targScene;
    SceneInstance *lastInst = inst;
    while (lifetime > 0) {
        currScene = currScene+1;
        
        // Reached the end of time?
        if (currScene >= sceneYear + NUM_YEARS) {
            break;
        }
        
        SceneInstance *childInst = SceneInstanceClone( lastInst );

        // TODO: grow
        childInst->scale = VectorAdd( childInst->scale, Vector3Make( obj->growthRate, obj->growthRate, obj->growthRate));
        
        childInst->nextInst = currScene->firstInst;
        currScene->firstInst = childInst;
        
        lastInst = childInst;
        
        lifetime--;
    }
    
    if (obj->decayInto) {
        currScene = currScene+1;
        if (currScene < sceneYear + NUM_YEARS) {
            SceneStampObject( currScene, pos, obj->decayInto);
        }
    }
}

SceneObject *FindObject( char *objName )
{
    for (int i=0; i < numObjects; i++) {
        SceneObject *obj = objects + i;
        if (!strcmp(obj->name, objName )) {
            return obj;
        }
    }
    return NULL;
}

void SaveWorld( const char *filename )
{
    FILE *fp = fopen( filename, "wt");
    
    for (int year=0; year < NUM_YEARS; year++) {
        fprintf( fp, "#-------------------------------\n");
        fprintf( fp, "year: %d\n", year );
        fprintf( fp, "name: todo\n");
        
        Scene *currScene = sceneYear + year;
        
        SceneInstance *inst = currScene->firstInst;
        while (inst) {
            fprintf( fp, "\n" );
            fprintf( fp, "obj: %s\n", inst->object->name );
            fprintf( fp, "pos: %f %f %f\n", inst->pos.x, inst->pos.y, inst->pos.z );
            fprintf( fp, "scl: %f %f %f\n", inst->scale.x, inst->scale.y, inst->scale.z );
            fprintf( fp, "rot: %f\n", inst->rotation );
            
            fprintf( fp, "wire: %s\n", inst->drawWires?"YES":"NO");
            fprintf( fp, "wireColor: 0x%08X\n", GetHexValue(inst->wireColor) );
//            bool drawWires;
//            Color wireColor;

            inst = inst->nextInst;
        }
    }
    
    fclose(fp);
}

void LoadWorld( const char *filename )
{
    FILE *fp = fopen(filename, "rt");
    char line[1024];
    
    if (!fp) {
        printf("ERR: Could not load %s\n", filename );
        return;
    }
    
    Scene *currScene = sceneYear;
    SceneInstance *inst = NULL;
    
    while (fgets(line, 1024, fp)) {
        
        // Comments
        char *ch = strchr( line, '#');
        if (ch) *ch = 0;
        
        char tok[256];
        sscanf( line, "%s", tok );
        if (strlen(tok)==0) {
            continue;
        }
        
        char *data = strchr(line, ':' );
        if (data) {
            data ++;
        } else {
            continue;
        }
        
        if (!strcmp(tok, "year:")) {
            int yearNum;
            sscanf( data, "%d", &yearNum );
            currScene = sceneYear + yearNum;
        } else if (!strcmp(tok, "name:")) {
            char *yearName = strchr( line, ':');
            if (yearName) {
                if (yearName[strlen(yearName)-1]=='\0') {
                    yearName[strlen(yearName)-1]=0;
                }
                currScene->name = strdup(yearName+1);
            }
        
        } else if (!strcmp(tok, "obj:")) {
            char objName[256];
            sscanf( data, "%s", objName );
            SceneObject *obj = FindObject( objName );
            if (!obj) {
                printf("WARN: Could Not find Object: %s\n" , objName);
                inst = NULL;
            } else {
                inst = SceneInstanceCreate();
                inst->object = obj;
                
                inst->nextInst = currScene->firstInst;
                currScene->firstInst = inst;

            }
        } else if (!strcmp(tok, "pos:")) {
            float x, y, z;
            sscanf( data, "%f %f %f", &x, &y, &z );
            if (inst) {
                inst->pos = Vector3Make( x, y, z );
            }
        } else if (!strcmp(tok, "scl:")) {
            float x, y, z;
            sscanf( data, "%f %f %f", &x, &y, &z );
            if (inst) {
                inst->scale = Vector3Make( x, y, z );
            }
        } else if (!strcmp(tok, "rot:")) {
            float rot;
            sscanf( data, "%f", &rot );
            if (inst) {
                inst->rotation = rot;
            }
        } else {
            printf("Ignoring Token: %s\n", tok );
        }
        
    }
}

// ===================================================================
//      CREEPS
// ===================================================================

struct CreepObject {
    char *name;
    Model model;
    
    float startingHealth;
    float spawnRateMin;
    float spawnRateMax;
    float collideRadius;
};

struct CreepInst {
    CreepObject *creepObj;
    Vector3 position;
    float hp;
    float angle;
};

#define MAX_CREEP_OBJS (50)
CreepObject creepObjects[MAX_CREEP_OBJS];
int numCreepObjects = 0;

#define MAX_CREEPS (1000)
CreepInst creeps[MAX_CREEPS];
int numCreeps = 0;

SceneObject *MakeSpawner( CreepObject *creepObj )
{
    assert( numObjects < MAX_OBJECTS );
    
    char buff[1024];
    sprintf(buff, "SPAWN_%s", creepObj->name );

    SceneObject *obj = objects + numObjects++;
    obj->name = strdup(buff);

    obj->spawner = creepObj;
    
    obj->lifetimeMin = 10;
    obj->lifetimeMax = 12;
    
    return obj;
}


CreepObject *LoadCreep( const char *name)
{
    assert( numCreepObjects < MAX_CREEP_OBJS );
    CreepObject *creepObj = creepObjects + numCreepObjects++;
    
    char buff[1024];
    sprintf(buff, "gamedata/%s.obj", name );
    creepObj->name = strdup( name );
    creepObj->model = LoadModel( buff );
    creepObj->model.material = LoadStandardMaterial();
    sprintf(buff, "gamedata/%s.png", name );
    Texture2D texture = LoadTexture(buff);
    creepObj->model.material.texDiffuse = texture;
    
    // Defaults
    creepObj->startingHealth = 10.0;
    creepObj->spawnRateMin = 2.0;
    creepObj->spawnRateMin = 3.0;
    creepObj->collideRadius = 1.5;
    
    return creepObj;
}

CreepInst *SpawnCreep( CreepObject *creepObj, Vector3 pos )
{
    assert( numCreeps < MAX_CREEPS );
    
    CreepInst *creep = creeps + numCreeps++;
    memset( creep, 0, sizeof(CreepInst));
    
    creep->creepObj = creepObj;
    creep->hp = creepObj->startingHealth;
    creep->position = pos;
    
    return creep;
}

CreepInst *CheckCollideCreeps( Vector3 pos, float radius, CreepInst *self )
{
    for (int i=0; i < numCreeps; i++) {
        CreepInst *creep = creeps + i;
        
        if (creep == self) continue;
        
        Vector3 v = VectorSubtract( pos, creep->position );
        float dd = VectorLength( v );
        
        if (dd < radius + creep->creepObj->collideRadius ) {
            return creep;
        }
    }
    return NULL;
}

void VanishAllCreeps( bool effects ) {
    // TODO: effects
    
    numCreeps = 0;
}


// ===================================================================
//      WEAPONS
// ===================================================================

struct Weapon {
    char *name;
    float cooldown;
    float cooldownRate;
    float muzzleVel;
};

struct Bullet
{
    Vector3 pos;
    Vector3 vel;
    // type..
};

#define MAX_WEAPONS (10)
Weapon weapons[MAX_WEAPONS];
int numWeapons = 0;

#define MAX_BULLETS (5000)
Bullet bullets[MAX_BULLETS];
int numBullets = 0;

Weapon *MakeWeapon( char *name )
{
    assert( numWeapons < MAX_WEAPONS );
    Weapon *w = weapons + numWeapons++;
    w->name = strdup( name );
    w->cooldown = 0.6;
    w->muzzleVel = 80.0;
    return w;
}

Bullet *Shoot( Weapon *weapon, Vector3 pos, Vector3 dir )
{
    assert( numBullets < MAX_BULLETS);
    Bullet *b = bullets + numBullets++;
    
    b->pos = pos;
    b->vel = dir;
    VectorNormalize( &(b->vel) );
    VectorScale( &(b->vel), weapon->muzzleVel );
    
    weapon->cooldown = weapon->cooldownRate;
    
    return b;
}

// ===================================================================
//      NAV MESH
// ===================================================================

#define MAX_NAV (100)

enum {
    NAV_CONNECTED = 0x01, // Connected in the nav mesh
    NAV_ACTIVE = 0x02,    // Active in this year
};

struct NavPoint {
    Vector3 pos;
};

int numNavPoints;
NavPoint navPoints[MAX_NAV];
uint8_t navAdj[MAX_NAV][MAX_NAV];

void AddNavPoint( Vector3 pos )
{
    // don't assert since this is a low ceiling
    if (numNavPoints == MAX_NAV) return;
    
    NavPoint *np = navPoints + numNavPoints;
    np->pos = pos;
    
    for (int i=0; i < numNavPoints; i++) {
        navAdj[i][numNavPoints] = 0;
        navAdj[numNavPoints][i] = 0;
    }
    
    navAdj[numNavPoints][numNavPoints] = NAV_CONNECTED;
    numNavPoints++;
}

void DrawNavPoints()
{
    for (int i=0; i < numNavPoints; i++) {
        NavPoint *np = navPoints + i;
        DrawCube( np->pos, 1.0, 1.0, 1.0, PURPLE );
    }
}

// ===================================================================
//       EDITOR
// ===================================================================
SceneInstance *edSelectedInst = NULL;
SceneObject *edCurrentObject = NULL;

// somebody needs to move the editor code up heres...

// ===================================================================
//      EL MAIN
// ===================================================================

int main()
{
	int screenWidth = 1280;
	int screenHeight = 800;
    
    int pixelWidth = 320;
    int pixelHeight = 200;
    
    float animTime = 0.0;
    
    // Constants
    Vector3 ScaleOne = { 1.0, 1.0, 1.0 };

    // DBG/test options
    bool retinaHack = false; // TODO: set this automatically with glfwSetFramebufferSizeCallback
    bool doPostProcess = false;
    bool editorMode = false;
    bool showNavMesh = true;
    
    int frameCounter = 0;
    
 	//SetConfigFlags(FLAG_MSAA_4X_HINT);
    
	InitWindow( screenWidth, screenHeight, "ld37_oneroom");
    Camera camera;
    camera.position = (Vector3){ 0.0f, 21.0f, -21.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, -3.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    
    Vector3 cubePosition = { 0.0f, -0.2f, 0.0f };
    
    camera.fovy = 45.0;
    
	SetTargetFPS(60);
    
    Shader shader = LoadShader("gamedata/base.vs",
                               "gamedata/bloom.fs");       // Load postpro shader
    
    // Create a RenderTexture2D to be used for render to texture
    //RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture2D pixelTarget = LoadRenderTexture(pixelWidth, pixelHeight);
    SetTextureFilter( pixelTarget.texture, FILTER_POINT );
    
    RenderTexture2D postProcTarget = LoadRenderTexture(pixelWidth, pixelHeight);
    SetTextureFilter( postProcTarget.texture, FILTER_POINT );
    
#if 0
	// TEST build mesh
	Mesh mesh = {0};
	mesh.triangleCount = 100;
    mesh.vertexCount = mesh.triangleCount * 3;
	mesh.vertices = (float*)malloc( mesh.vertexCount * sizeof(float) * 3 );
    mesh.normals = (float*)malloc( mesh.vertexCount * sizeof(float) * 3 );
    mesh.indices = (unsigned short *)malloc( mesh.vertexCount * sizeof(int) * 3 );
	for (int i=0; i < mesh.vertexCount; i++) {
		*(mesh.vertices + i) = ((float)GetRandomValue( 0, 2000 ) / 1000.0) - 1.0;
        *(mesh.normals + i) = ((float)GetRandomValue( 0, 2000 ) / 1000.0) - 1.0;
        *(mesh.indices +i) = i;
	}

	Model model = LoadModelEx( mesh, false );
#endif

    Vector3 lightDir = { -4.0, 5.0, 0.5 };
    VectorNormalize( &lightDir );
    Light lgtSun = CreateLight( LIGHT_DIRECTIONAL, lightDir, colorSun );
    lgtSun->target = Vector3{ 0.0f, 0.0f, 0.0f };
    lgtSun->intensity = 1.0f;

    // Load Objects
    SceneObject *objTree = LoadSceneObject( "blerghTree" );
    objTree->growthRate = 0.05;
    objTree->startSize = Vector3Make( 0.6, 0.6, 0.6);
    objTree->lifetimeMin = 10;
    objTree->lifetimeMax = 20;
    
    SceneObject *objStump = LoadSceneObject( "stump" );
    objStump->lifetimeMin = 3;
    objStump->lifetimeMax = 5;
    objTree->decayInto = objStump;
    
    // Load Creeps
    CreepObject *creepTest = LoadCreep( "creep" );
    MakeSpawner( creepTest );
    
    // Load the world
    LoadWorld( "gamedata/world.txt");
    
    edCurrentObject = objects;
    
    Model player = LoadModel( "gamedata/puck.obj");
    player.material = LoadStandardMaterial();
    player.material.texDiffuse = LoadTexture( "gamedata/puck.png");
    SetTextureFilter( player.material.texDiffuse, FILTER_POINT );
    Vector3 playerStartPos = (Vector3){ 0.0, 2.0, -10.0 };
    Vector3 playerPos = playerStartPos;
    
    float playerAngle = 180.0; // angle used for gameplay
    float playerDisplayAngle = 180.0; // angle used for display
    
    float playerSz = 2.0;
    float playerSz2 =  playerSz;
    float playerHealth = 100.0;
    float playerInvulerable = 0.0;
    
    Vector3 playerUp = (Vector3){ 0.0, 1.0, 0.0};
    Vector3 playerScale = (Vector3){ playerSz,  playerSz, playerSz };
    Vector3 playerMoveDir = (Vector3){ 0.0, 0.0, 1.0 };

    Weapon *currWeapon = MakeWeapon("shooty");
    
//    Material mtlTerrain = LoadStandardMaterial();
//    mtlTerrain.colDiffuse = LIGHTGRAY;
//    mtlTerrain.colAmbient = colorAmbient;
//    model.material = mtlTerrain;

    float angle = (M_PI/180.0)*120.0;

    // Build World
    Model model = LoadModel( "gamedata/ground1.obj");
    model.material = LoadStandardMaterial();
    model.material.texDiffuse = LoadTexture( "gamedata/ground1.png");
    model.material.colAmbient = LIGHTGRAY;
    SetTextureFilter( model.material.texDiffuse, FILTER_POINT );
    
    // playArea is shrunk by the player size
    BoundingBox bboxPlayArea = (BoundingBox) { -10.0f + playerSz2, -0.1f, -10.0f + playerSz2,
                                                10.0f - playerSz2, 1000.0f,  10.0f - playerSz2 };

    BoundingBox bboxWorld = (BoundingBox) { -30.0, -1.0,  -30.0,
                                            30.0, 1000.0f,  30.0 };

    
    // Start at year 0
    currentYear = 0;
    scene = sceneYear + currentYear;
    
//    int numTree = 30;
//    for (int i=0; i < numTree; i++) {
//        
//        SceneInstance *inst = SceneInstanceCreate();
//        
//        inst->nextInst = scene->firstInst;
//        scene->firstInst = inst;
//        
//        inst->object = &treeObj;
//        inst->pos.x  = RandUniformRange( -10.0, 10.0 );
//        inst->pos.z  = RandUniformRange( -10.0, 10.0 );
//        inst->rotation = RandUniformRange( 0.0, 360 );
//        
//        float xpush = 10.0;
//        float zpush = 10.0;
//        if (inst->pos.x < 0) xpush = -xpush;
//        if (inst->pos.z < 0) zpush = -zpush;
//        inst->pos.x += xpush;
//        inst->pos.z += zpush;
//        
//        inst->drawWires = true;
//    }
    
    Vector3 lastShootyDir;
    
	while (!WindowShouldClose())
	{
        float dt = GetFrameTime();
        animTime += dt;
        
        frameCounter += 1;
        if (frameCounter > 100000) frameCounter = 0;
        
        UpdateCamera( &camera );
         if (IsKeyDown('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
        
        // Settings and Options
        if (IsKeyPressed('P')) {
            doPostProcess = !doPostProcess;
        }
        
        if (IsKeyPressed('R')) {
            retinaHack = !retinaHack;
        }

        
        if (IsKeyPressed( '`' )) {
            editorMode = !editorMode;
        }
        
        lgtSun->position.x = -cos( angle + 2 ) * 10.0;
        lgtSun->position.y = 8.0;
        lgtSun->position.z = -sin( angle + 2) * 10.0;
        
        if (editorMode) {
            
            if (IsKeyDown('.')) {
                if (currentYear < NUM_YEARS-1 ) {
                    currentYear++;
                    scene = sceneYear + currentYear;
                }
            }
            
            if (IsKeyDown(',')) {
                if (currentYear > 0 ) {
                    currentYear--;
                    scene = sceneYear + currentYear;
                }
            }
            
            if (IsKeyPressed(']')) {
                edCurrentObject += 1;
                if ((edCurrentObject - objects)==numObjects) {
                    edCurrentObject = objects;
                }
            }
            
            if (IsKeyPressed('[')) {
                
                if (edCurrentObject > objects) {
                    edCurrentObject -= 1;
                } else {
                    edCurrentObject = objects + (numObjects-1);
                }
            }
            
            if (IsKeyPressed('S')) {
                SaveWorld( "gamedata/editorworld.txt");
            }
            
            if (IsKeyPressed('M')) {
                showNavMesh = !showNavMesh;
            }
            
            if (IsKeyPressed('C')) {
                // Toggle Camera
                if (camera.position.y > 25.0) {
                    camera.position = (Vector3){ 0.0f, 21.0f, -21.0f };
                } else {
                    camera.position = (Vector3){ 0.0f, 50.0f, -21.0f };
                }
            }
            
            
        } else {
            // ===================================
            // Update player movement
            // ===================================
            playerMoveDir = (Vector3){ 0.0, 0.0, 0.0 };
            if (IsKeyDown('W')) {
                playerMoveDir.z += 1.0;
            }
            
            if (IsKeyDown('S')) {
                playerMoveDir.z -= 1.0;
            }
            
            if (IsKeyDown('A')) {
                playerMoveDir.x += 1.0;
            }
            
            if (IsKeyDown('D')) {
                playerMoveDir.x -= 1.0;
            }
            
            Vector3 shootyDir = { 0.0, 0.0, 0.0 };
            bool wantShoot = false;
            if (IsKeyDown( KEY_UP)) {
                shootyDir.z += 1.0;
                wantShoot = true;
            }
            
            if (IsKeyDown( KEY_DOWN)) {
                shootyDir.z -= 1.0;
                wantShoot = true;
            }

            if (IsKeyDown( KEY_LEFT)) {
                shootyDir.x += 1.0;
                wantShoot = true;
            }

            if (IsKeyDown( KEY_RIGHT)) {
                shootyDir.x -= 1.0;
                wantShoot = true;
            }
            
            if (wantShoot) {
                // if player is holding opposing shoot directions, keep firing
                // in last valid direction
                if (VectorLength(shootyDir) < 0.1) {
                    shootyDir = lastShootyDir;
                } else {
                    lastShootyDir = shootyDir;
                }
            }
            
            currWeapon->cooldown -= dt;
            
            if ((wantShoot) && (currWeapon->cooldown <= 0.0)) {
                Vector3 shootPos = playerPos;
                shootPos.y = 2.0;
                Shoot( currWeapon, shootPos, shootyDir );
            }
            
            // Special Keys
            if (IsKeyPressed('O')) {
                // Reset Level
                VanishAllCreeps( true );
                playerHealth = 100.0;
                playerPos = playerStartPos;
                numBullets = 0;
                playerInvulerable = 0.0;
            }
            
            float moveSpeed = 20.0;
            if (VectorLength( playerMoveDir) > 0.01)
            {
                VectorScale( &playerMoveDir, dt * moveSpeed );
                Vector3 newPlayerPos = VectorAdd( playerPos, playerMoveDir );
                
                BoundingBox bboxPlayer;
                bboxPlayer.min = Vector3Make( newPlayerPos.x - playerSz, 0.0, newPlayerPos.z - playerSz);
                bboxPlayer.max = Vector3Make( newPlayerPos.x + playerSz, 5.0, newPlayerPos.z + playerSz);
                
                if (CheckCollisionBoxes( bboxPlayArea, bboxPlayer)) {
                    playerPos = newPlayerPos;
                }
                
                float targetAngle =  atan2f( playerMoveDir.x, -playerMoveDir.z ) * (180.0/M_PI);
                
                float slew = 0.01;
                playerDisplayAngle = (targetAngle * slew) + (playerAngle * (1.0-slew));
                
                playerAngle = targetAngle;
            }
            
            // Check hits
            if (playerInvulerable <= 0.0) {
                CreepInst *ouchCreep = CheckCollideCreeps( playerPos, 1.5, NULL );
                if (ouchCreep) {
                    playerHealth -= RandUniformRange( 5.0, 8.0 );
                    playerInvulerable = 1.0;
                    
                    // Knockback
                    Vector3 knockBackDir = VectorSubtract( playerPos,ouchCreep->position );
                    knockBackDir.y = 0.0;
                    VectorNormalize( &knockBackDir );
                    VectorScale( &knockBackDir, 2.0 );
                    
                    // fixme - cleanup
                    Vector3 newPlayerPos;
                    BoundingBox bboxPlayer;
                    newPlayerPos = VectorAdd( playerPos, knockBackDir );
                    bboxPlayer.min = Vector3Make( newPlayerPos.x - playerSz, 0.0, newPlayerPos.z - playerSz);
                    bboxPlayer.max = Vector3Make( newPlayerPos.x + playerSz, 5.0, newPlayerPos.z + playerSz);
                    if (CheckCollisionBoxes( bboxPlayArea, bboxPlayer)) {
                        playerPos = newPlayerPos;
                    }
                    
                    if (playerHealth < 0.0 ) {
                        // TMP -- Game Over
                        VanishAllCreeps( true );
                        playerHealth = 100.0;
                    }
                }
            }

            
            
            
            // Update Creep movement
            for (int cndx = 0; cndx < numCreeps; cndx++) {
                CreepInst *creep = creeps + cndx;
                
                Vector3 creepDir = VectorSubtract( playerPos, creep->position );
                creep->angle =  atan2f( creepDir.x, -creepDir.z ) * (180.0/M_PI);
                
                VectorNormalize( &creepDir );
                VectorScale( &creepDir, dt * 6.0);
                
                Vector3 creepNewPos = VectorAdd( creep->position, creepDir );
                
                if (!CheckCollideCreeps( creepNewPos, creep->creepObj->collideRadius, creep )) {
                    creep->position = creepNewPos;
                }
            }
            
            // Spawn Creeps
            SceneInstance *inst = scene->firstInst;
            while (inst) {
                
                // Is this a spawner?
                if (inst->object->spawner) {
                    
                    inst->spawnCooldown -= dt;
                    
                    if (inst->spawnCooldown < 0.0) {
                        
                        // See if there's room to spawn
                        if (!CheckCollideCreeps( inst->pos, inst->object->spawner->collideRadius, NULL )) {
                            
                            // Reset cooldown
                            inst->spawnCooldown = RandUniformRange( inst->object->spawner->spawnRateMin,
                                                                    inst->object->spawner->spawnRateMax );
                            SpawnCreep( inst->object->spawner, inst->pos );
                        }
                    }
                }
                
                inst = inst->nextInst;
            }
            
            // Update Bullets
            for (int i=0; i < numBullets; i++) {
                Bullet *b = bullets + i;
                Vector3 bulletMove = b->vel;
                VectorScale( &bulletMove, dt );
                b->pos = VectorAdd( b->pos, bulletMove );
            }
            
            // Expire bullets and check collides
            
            for (int i=numBullets-1; i >= 0; i--) {
                Bullet *b = bullets + i;
                BoundingBox bulletBbox;
                bulletBbox.min = VectorAdd( b->pos, (Vector3){ -0.5, -0.5, -0.5 });
                bulletBbox.max = VectorAdd( b->pos, (Vector3){ 0.5, 0.5, 0.5 });
                
                CreepInst *hitCreep = CheckCollideCreeps( b->pos, 0.6, NULL );
                if (hitCreep) {
                    printf("Hit Creep\n");
                    numCreeps--;
                    if (numCreeps) {
                        *hitCreep = *(creeps + numCreeps);
                    }
                }
                
                // Out of bounds?
                if ((hitCreep) || (!CheckCollisionBoxes( bulletBbox, bboxWorld ))) {
                    numBullets -= 1;
                    if (i < numBullets) {
                        *b = *(bullets + numBullets);
                        i++; // Re-eval this slot
                    }
                }
            }

            
        }
		BeginDrawing();
        {
            
			ClearBackground( colorBackground );
            
            // ===================================
            // -- Low-res main game drawing pass
            // ===================================

            BeginTextureMode(pixelTarget);   // Enable drawing to texture
        
            Begin3dMode(camera);
            {
//                DrawCube( cubePosition, 2.0f, 2.0f, 2.0f, RED );
//                DrawCubeWires( cubePosition, 2.0f, 2.0f, 2.0f, WHITE );
                
                DrawModel( model, cubePosition, 1.0, WHITE );
//                DrawModelWires( model, cubePosition, 1.0, WHITE );

                glPolygonOffset( -1.0, 1.0 );
                glEnable( GL_POLYGON_OFFSET_LINE );
                
                SceneInstance *inst = scene->firstInst;
                while (inst) {

                    // Is this a spawner?
                    if (inst->object->spawner) {
                        
                        DrawCylinderWires( inst->pos, 1.5, 1.5, 3.0, 8, BLUE );
                        
                    } else {
                    
                        DrawModelEx(inst->object->model, inst->pos,
                                    playerUp, inst->rotation, inst->scale,
                                    WHITE );
                        if (inst->drawWires) {
                            //DrawModelWires(inst->object->model, inst->pos, 1.0, BLACK );
                            DrawModelWiresEx(inst->object->model, inst->pos,
                                        playerUp, inst->rotation, inst->scale,
                                        BLACK );
                        }
                    }
                    
                    inst = inst->nextInst;
                }
                
                for (int i=0; i < numCreeps; i++) {
                    CreepInst *creepInst = creeps + i;
                    DrawModelEx( creepInst->creepObj->model,
                                creepInst->position,
                                playerUp, creepInst->angle, ScaleOne, WHITE );
                }
                
                // Draw bullets
                for (int i=0; i < numBullets; i++) {
                    Bullet *b = bullets + i;
                    DrawCube( b->pos, 0.5, 0.5, 0.5, RED );
                }
                
                if (!editorMode) {
                    
                    Color tintColor = YELLOW;
                    
                    if (playerInvulerable > 0.0) {
                        playerInvulerable -= dt;
                        
                        if (frameCounter % 2) {
                            tintColor = RED;
                        }
                    }
                    
                    DrawModelEx( player, playerPos, playerUp, playerDisplayAngle,
                                playerScale, tintColor );
                    
                } else {
                    // Edit Mode
                    DrawLight( lgtSun );
                    DrawGrid( 10, 2.0f );
                    
                    if (showNavMesh) {
                        DrawNavPoints();
                    }
                    
                    // draw cursor
                    Vector2 mousePos = GetMousePosition();
                    Ray ray = GetMouseRay( mousePos, camera);
                    
                    float t = ray.position.y / -ray.direction.y;
                    Vector3 camDir = ray.direction;
                    VectorScale( &camDir, t );
                    Vector3 groundPos = VectorAdd( ray.position, camDir );

//                    printf("Ground Pos: %3.2f %3.2f %3.2f tval %3.2f\n",
//                           groundPos.x, groundPos.y, groundPos.z, t );

                    Color cursorBlink = ColorLerp( SKYBLUE, WHITE, fabs(sin(animTime * 10.0)) );
                    if (edCurrentObject) {
                        
                        if (edCurrentObject->spawner) {
                            DrawModelWires( edCurrentObject->spawner->model, groundPos, 1.0, cursorBlink );
                        } else {
                            DrawModelWires( edCurrentObject->model, groundPos, 1.0, cursorBlink );
                        }
                        
                        
                        if (IsKeyPressed( KEY_SPACE )) {
                            SceneStampObject( scene, groundPos, edCurrentObject );
                        }
                        else if (IsKeyPressed( 'K')) {
                            SpawnCreep( creepTest, groundPos );
                        }
                        
                        if (IsKeyPressed('N')) {
                            AddNavPoint( groundPos );
                        }


                        
                    } else {
                        DrawCubeWires( groundPos, 1.0, 0.2, 1.0, cursorBlink );
                    }
                }
                
            }
            End3dMode();
            
            EndTextureMode();
            
            // ===================================
            // -- Low-res post-process
            // ===================================
            BeginTextureMode(postProcTarget);
            BeginShaderMode(shader);
            // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
//            DrawTextureRec(target.texture, (Rectangle){ 0, 0,
//                        target.texture.width, -target.texture.height },
//                           (Vector2){ 0, 0 }, WHITE);
            DrawTexturePro(pixelTarget.texture,
                           (Rectangle){ 0, 0, pixelTarget.texture.width, -pixelTarget.texture.height },
                           (Rectangle){ 0, 0, pixelWidth, pixelHeight },
                           (Vector2){ 0, 0 }, 0, WHITE);

            
            EndShaderMode();
            EndTextureMode();
            
            // HACK work around displayScale
            if (retinaHack) {
                glViewport(0, 0, GetScreenWidth() * 2.0, GetScreenHeight() * 2.0 );
            }
            
            // ===================================
            // Blit to final screen res
            // ===================================

            Rectangle textureRect = (Rectangle){ 0, 0, postProcTarget.texture.width, -postProcTarget.texture.height };
            
            Rectangle screenRect = (Rectangle){ 0, 0, screenWidth, screenHeight };
            
//            float displayScale = 2.0;
//            textureRect.width *= displayScale;
//            textureRect.height *= displayScale;
//            screenRect.width *= displayScale;
//            screenRect.height *= displayScale;
            
            DrawTexturePro( doPostProcess?postProcTarget.texture:pixelTarget.texture,
                           textureRect, screenRect, (Vector2){ 0, 0 }, 0, WHITE);

            
            // ===================================
            // Draw Timeline and HUD
            // ===================================
            
            if (editorMode) {
                DrawText( "Edit Mode", 572, 22, 40, BLACK );
                DrawText( "Edit Mode", 570, 20, 40, GOLD );
                
                DrawRectangle( screenWidth-250, 0, 250, 200, Fade( BLACK, 0.5f));
                char buff[256];
                sprintf(buff, "BRUSH: %s", edCurrentObject->name );
                
                DrawText( buff, screenWidth-244, 4, 10, SKYBLUE );
            } else {
                DrawText( "Health:", 10, 12, 20, PINK );
                DrawRectangle( 80, 10, 204, 24, BLACK );
                DrawRectangle( 82, 12, 2*playerHealth, 20, PINK );
            }
            
            int timelineY = 750;
            
            DrawRectangle( 130, timelineY - 10, 1020, 42, Fade( WHITE, 0.6) );
            
            DrawRectangle( 140, timelineY, 1000, 2, SKYBLUE );
            
            int step = 1000 / 100;
            for (int i=0; i <= 100; i++) {
                DrawRectangle( 140 + i*step , timelineY - 5, 1, 13, SKYBLUE );
            }
            
            // Highlight current year
            DrawRectangle( 140 + currentYear*step - 2, timelineY - 6, 5, 15, GOLD );

        }
        
        char buff[256];
        sprintf(buff, "Bullets: %d", numBullets );
        DrawText( buff, 10, 50, 20, WHITE );
        DrawFPS(10, screenHeight - 30);
        
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
