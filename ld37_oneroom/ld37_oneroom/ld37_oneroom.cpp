
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#undef DEBUG
#include "raylib.h"

//#define RAYMATH_IMPLEMENTATION
#include "raymath.h"
#include "math.h"

#include <OpenGL/gl3.h>


// raylib defines "DEBUG" for loggins :(
#ifdef DEBUG
#undef DEBUG
#endif
#include "rlgl.h"

//Color colorBackground = (Color){ 0, 0, 60, 255 };
Color colorBackground = (Color){ 68, 137, 26 };
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

inline Vector3 VectorMultScalar( Vector3 v, float x )
{
    return Vector3Make( v.x * x, v.y*x, v.z*x );
}

inline Vector3 VectorHadamard( Vector3 a, Vector3 b )
{
    return (Vector3){ a.x*b.x, a.y * b.y, a.z*b.z };
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

inline BoundingBox BoundingBoxMake( Vector3 point, float halfSize )
{
    BoundingBox result;
    result.min = Vector3Make( point.x-halfSize,  point.y-halfSize,  point.z-halfSize );
    result.max = Vector3Make( point.x+halfSize,  point.y+halfSize,  point.z+halfSize );
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
    BoundingBox bboxLocal;
    
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

SceneObject *MakeSceneObject( const char *name, const Model &model, const Texture2D &texture )
{
    assert( numObjects < MAX_OBJECTS );

    SceneObject *obj = objects + numObjects++;
    obj->name = strdup(name);
    obj->model = model;
    if (name !="lite_test") {
        obj->model.material = LoadStandardMaterial();
    }
    obj->model.material.texDiffuse = texture;
    //obj->model.material.colAmbient = BLACK;
    //    mtlTerrain.colDiffuse = LIGHTGRAY;
    //    mtlTerrain.colAmbient = colorAmbient;

    obj->startSize = Vector3Make(1.0, 1.0, 1.0);
    obj->bboxLocal = CalculateBoundingBox( obj->model.mesh);
    
    return obj;
}

SceneObject *LoadSceneObject( const char *name )
{
    char buff[1024];
    sprintf(buff, "gamedata/%s.obj", name );
    Model model = LoadModel( buff );
    
    sprintf(buff, "gamedata/%s.png", name );
    Texture2D texture = LoadTexture(buff);
    
    return MakeSceneObject( name, model, texture );
}

SceneObject *LoadSceneObjectReuseTexture( const char *name, const Texture2D &texture )
{
    char buff[1024];
    sprintf(buff, "gamedata/%s.obj", name );
    Model model = LoadModel( buff );
    
    return MakeSceneObject( name, model, texture );
}


void SceneInstUpdateBBox( SceneInstance *inst )
{
    
    inst->bbox.min = VectorAdd( VectorHadamard( inst->object->bboxLocal.min, inst->scale), inst->pos );
    inst->bbox.max = VectorAdd( VectorHadamard( inst->object->bboxLocal.max, inst->scale), inst->pos );
    
}

void SceneStampObject( Scene *targScene, Vector3 pos, float rotation, Vector3 scale, SceneObject *obj )
{
    SceneInstance *inst = SceneInstanceCreate();
    
    // Add to the current scene
    inst->nextInst = targScene->firstInst;
    targScene->firstInst = inst;

    inst->object = obj;
    inst->pos = pos;
    
    inst->scale =  VectorHadamard( obj->startSize, scale );
    
    inst->rotation = rotation;
    
    SceneInstUpdateBBox( inst );
    
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
            SceneStampObject( currScene, pos, rotation, scale, obj->decayInto);
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
            fprintf( fp, "id: 0x%08X\n", inst->objId );
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
        } else if (!strcmp(tok, "id:")) {
            int objId;
            sscanf( data, "%X", &objId );
            inst->objId = objId;
        } else if (!strcmp(tok, "pos:")) {
            float x, y, z;
            sscanf( data, "%f %f %f", &x, &y, &z );
            if (inst) {
                inst->pos = Vector3Make( x, y, z );
                SceneInstUpdateBBox( inst );
            }
        } else if (!strcmp(tok, "scl:")) {
            float x, y, z;
            sscanf( data, "%f %f %f", &x, &y, &z );
            if (inst) {
                inst->scale = Vector3Make( x, y, z );
                SceneInstUpdateBBox( inst );
            }
        } else if (!strcmp(tok, "rot:")) {
            float rot;
            sscanf( data, "%f", &rot );
            if (inst) {
                inst->rotation = rot;
                SceneInstUpdateBBox( inst );
            }
        } else {
//            printf("Ignoring Token: %s\n", tok );
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

struct NavPoint;

struct CreepInst {
    CreepObject *creepObj;
    Vector3 position;
    float hp;
    float angle;
    
    NavPoint *currTarget;
    
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
//     DITHER EFFECT
// ===================================================================

GLuint texPalette;
GLuint texBayerDither;

// ARNE retro 16-color palette from
// https://androidarts.com/palette/16pal.htm
Color arnePalette[16] = {
    { 0, 0, 0, 255 },
    { 157, 157, 157, 255 },
    { 255, 255, 255, 255 },
    { 190, 38, 51, 255 },
    { 224, 111, 139, 255 }, // MEAT
    //{ 220, 220, 230, 255 }, // LIGHTER GRAY
    { 73, 60, 43, 255 },
    { 164, 100, 34, 255 },
    { 235, 137, 49, 255 },
    { 247, 226, 107, 255 },
    { 47, 72, 78, 255 },
    { 68, 137, 26, 255 },
    { 163, 206, 39, 255 },
    { 27, 38, 50, 255 },
    { 0, 87, 132, 255 },
    { 49, 162, 242, 255 },
    { 178, 220, 239, 255 },
};

// Bayer dither pattern
// https://en.wikipedia.org/wiki/Ordered_dithering
unsigned char bayerDither[16] = {
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 6
};

int checkForGLErrors( const char *s, const char * file, int line )
{
    int errors = 0 ;
    int counter = 0 ;
    
    while ( counter < 1000 )
    {
        GLenum x = glGetError() ;
        
        if ( x == GL_NO_ERROR )
            return errors ;
        
        //printf( "%s:%d [%s] OpenGL error: %s [%08x]\n",  file, line, s ? s : "", gluErrorString ( x ), x ) ;
        printf( "%s:%d [%s] OpenGL error: [%08x]\n",  file, line, s ? s : "",  x ) ;
        errors++ ;
        counter++ ;
    }
    return 0;
}

#ifdef NDEBUG
#define CHECKGL( msg )
#else
#define CHECKGL( msg ) checkForGLErrors( msg, __FILE__, __LINE__ );
#endif


// For now just use RGB distance, should use a better distance like L*a*b* deltaE
float ColorDistance( Color a, Color b) {
    return sqrt( (a.r-b.r)*(a.r-b.r) + (a.g-b.g)*(a.g-b.g) + (a.b-b.b)*(a.b-b.b) );
}

// from
// http://www.compuphase.com/cmetric.htm
double ColorDistance2(Color e1, Color e2)
{
    long rmean = ( (long)e1.r + (long)e2.r ) / 2;
    long r = (long)e1.r - (long)e2.r;
    long g = (long)e1.g - (long)e2.g;
    long b = (long)e1.b - (long)e2.b;
    return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}

void MakePaletteTexture( int size ) {
    uint8_t *paldata = (uint8_t*)malloc( size*size*size*3 );
    
    int shiftAmt =0;
    int t = size;
    while (t < 255) {
        shiftAmt += 1;
        t <<= 1;
    }
    printf("shiftamt %d %d\n", shiftAmt, size << shiftAmt );
    
    for (int i=0; i < size; i++) {
        for (int j=0; j < size; j++) {
            for (int k=0; k < size; k++) {
                int ndx = ((k*size*size) + (j*size) + i) * 3;
                Color cellc;
                cellc.r = (i << shiftAmt);
                cellc.g = (j << shiftAmt);
                cellc.b = (k << shiftAmt);
                
                int bestIndex = 0;
                float bestDistance = 0.0;
                for (int pal=0; pal < 16; pal++) {
                    float d = ColorDistance2( arnePalette[pal], cellc );
                    if ((pal==0)||(d < bestDistance)) {
                        bestIndex = pal;
                        bestDistance = d;
                    }
                }
                
                Color palColor = arnePalette[bestIndex];
                paldata[ndx + 0] = palColor.r;
                paldata[ndx + 1] = palColor.g;
                paldata[ndx + 2] = palColor.b;
                
//                paldata[ndx + 0] = cellc.r;
//                paldata[ndx + 1] = cellc.g;
//                paldata[ndx + 2] = cellc.b;
//                
//                paldata[ndx + 0] = rand() & 0xFF;
//                paldata[ndx + 1] = rand() & 0xFF;
//                paldata[ndx + 2] = rand() & 0xFF;
            }
        }
    }
    
    // Make a 3D pallete lookup texture
    glGenTextures( 1, &texPalette );
    glBindTexture( GL_TEXTURE_3D, texPalette );
    CHECKGL("makepal");
    
    glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB8, size, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, paldata );
    CHECKGL("makepal teximage");
    
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    printf("Palette texture is texid %d\n", texPalette );

    // Make a 2d Bayer dither texture
    
    // scale up to 0..255
    for (int i=0; i < 16; i++) {
        bayerDither[i] = bayerDither[i] * 17;
    }
    
    glGenTextures( 1, &texBayerDither );
    glBindTexture( GL_TEXTURE_2D, texBayerDither );
    CHECKGL("makepal");
    
    glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, bayerDither );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    
    printf("texBayer is %d\n", texBayerDither );
    CHECKGL("makepal texbayer");

    
    
    free (paldata);
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

#define MAX_NAV (256)

enum {
    NAV_CONNECTED = 0x01, // Connected in the nav mesh
    NAV_ACTIVE = 0x02,    // Active in this year
};

struct NavPoint {
    Vector3 pos;
    bool active;
    
    // pathed distance to target
    float pdist;
};

int numNavPoints;
NavPoint navPoints[MAX_NAV];
uint8_t navAdj[MAX_NAV][MAX_NAV];
float navMaxDist;

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
        
        
        Color color = ColorLerp( ORANGE, DARKPURPLE, np->pdist / navMaxDist );
        
        if (np->pdist > 9000) {
            color = RED;
        }
        
        if (!np->active) {
            color = DARKGRAY;
            continue;
        }
        DrawCube( np->pos, 1.0, 1.0, 1.0, color );
    }
    
    for (int i=0; i < numNavPoints; i++) {
        for (int j=0; j < numNavPoints; j++) {
            if (navAdj[i][j] & NAV_CONNECTED) {
                Color color = VIOLET;
                if (!(navAdj[i][j] & NAV_ACTIVE)) {
                    color = DARKGRAY;
                    continue;
                }
                DrawLine3D( navPoints[i].pos, navPoints[j].pos, color );
            }
        }
    }
}

void UpdateNavActives();

// This is old code from LudumDare 5 ! whoa..

struct Edge {
    float x1, y1, x2, y2;
    int count;
};

#define MAX_EDGE (1000)
Edge edgeList[MAX_EDGE];
int numEdges = 0;

// used for road making
struct Triangle {
    float ax, ay, bx, by, cx, cy;
};

void circumcenter( Triangle &tri, float &ccx, float &ccy, float &rad ) {
    
    float d;
    float ax = tri.ax, ay = tri.ay;
    float bx = tri.bx, by = tri.by;
    float cx = tri.cx, cy = tri.cy;
    float ax2 = ax*ax, ay2 = ay*ay;
    float bx2 = bx*bx, by2 = by*by;
    float cx2 = cx*cx, cy2 = cy*cy;
    
    d = 2.0 * ( ay*cx + by*ax - by*cx - ay*bx - cy*ax + cy*bx );
    if (fabs(d) < 0.000001) {
        ccx = ax; ccy = ay;
        rad = 0.0;
    } else {
        ccx = ( by*ax2 - cy*ax2 - by2*ay + cy2*ay +
               bx2*cy + ay2*by + cx2*ay - cy2*by -
               cx2*by - bx2*ay + by2*cy - ay2*cy ) / d;
        
        
        ccy = ( ax2*cx + ay2*cx + bx2*ax - bx2*cx +
               by2*ax - by2*cx - ax2*bx - ay2*bx -
               cx2*ax + cx2*bx - cy2*ax + cy2*bx) / d;
        
        rad = sqrt( (ccx-ax)*(ccx-ax) + (ccy-ay)*(ccy-ay) );
    }
    
}

void AddEdge( float x1, float y1, float x2, float y2 )
{
    // is edge already there
    for (int i=0; i < numEdges; i++) {
        
        int match = 0;
        if ( (fabs( edgeList[i].x1 - x1 ) < 0.1) &&
            (fabs( edgeList[i].y1 - y1 ) < 0.1) &&
            (fabs( edgeList[i].x2 - x2 ) < 0.1) &&
            (fabs( edgeList[i].y2 - y2 ) < 0.1) ) {
            
            match = 1;
        } else if (
                   (fabs( edgeList[i].x1 - x2 ) < 0.1) &&
                   (fabs( edgeList[i].y1 - y2 ) < 0.1) &&
                   (fabs( edgeList[i].x2 - x1 ) < 0.1) &&
                   (fabs( edgeList[i].y2 - y1 ) < 0.1) ) {
            match = 1;
        }
        
        if (match) {
            //printf("Edge matches...count %d\n", edge[i].count );
            edgeList[i].count++;
            return;
        }
    }
    
    // didn't find it, add
    Edge e;
    e.count = 1;
    e.x1 = x1; e.y1 = y1;
    e.x2 = x2; e.y2 = y2;
    edgeList[numEdges++] = e;
}

bool EdgeMatch( int ax1, int ay1, int ax2, int ay2,
                int bx1, int by1, int bx2, int by2 ) {
    if (  ( ((ax1==bx1) && (ay1==by1) &&
             (ax2==bx2) && (ay2==by2)) )  ||
        
        ( ((ax1==bx2) && (ay1==by2) &&
           (ax2==bx1) && (ay2==by1)) ) ) {
        
        return true;
    }
    return false;
}

#define MAX_TRI (1000)
void MakeEdgesDelauney( )
{
    for (int i=0; i < numNavPoints; i++) {
        for (int j=0; j < numNavPoints; j++) {
            navAdj[i][j] = 0;
        }
    }
    
    // DBG: fill in edges at random
#if 0
    for (i=0; i <10; i++) {
        int l1,l2;
        l1 = random( nloc );
        l2 = random( nloc );
        adj[l1][l2].pass = 1;
        adj[l2][l1].pass = 1;
    }
#endif
    
    Triangle *tris  = (Triangle*)malloc( sizeof(Triangle) * MAX_TRI );
    int numTris = 0;
    
    Triangle *tris2 = (Triangle*)malloc( sizeof(Triangle) * MAX_TRI );
    int numTris2 = 0;
    
    tris[0].ax = -200; tris[0].ay = -200;
    tris[0].bx = 200; tris[0].by = -200;
    tris[0].cx = 200; tris[0].cy = 200;
    
    tris[1].ax = -200;   tris[1].ay = -200;
    tris[1].bx = -200;   tris[1].by = 200;
    tris[1].cx = 200; tris[1].cy = 200;
    
    numTris = 2;
    
    // edgelist
    numEdges = 0;
    
    for (int ndx=0; ndx < numNavPoints; ndx++) {
        
        // init lists
        numTris2 = 0;
        numEdges = 0;
        
        
        // find all triangle whose circumcenter contains loc ndx
        for (int i=0; i < numTris; i++) {
            float cx, cy, rad;
            
            circumcenter( tris[i], cx, cy, rad );
            float d = sqrt( (navPoints[ndx].pos.x - cx) * (navPoints[ndx].pos.x - cx) +
                            (navPoints[ndx].pos.z - cy) * (navPoints[ndx].pos.z - cy) );
            if (d <= rad) {
                // in triangle circumcenter, add to edgelist
                AddEdge( tris[i].ax, tris[i].ay, tris[i].bx, tris[i].by );
                AddEdge( tris[i].bx, tris[i].by, tris[i].cx, tris[i].cy );
                AddEdge( tris[i].cx, tris[i].cy, tris[i].ax, tris[i].ay );
                
            } else {
                // just keep the tri
                tris2[ numTris2++ ] = tris[i];
                //printf("Keeping tri %i\n", i );
            }
        }
        
        // add a triangle for every edge appearing once in the list
        for (int i=0; i < numEdges; i++) {
            if ( edgeList[i].count == 1 ) {
                Triangle t;
                t.ax = navPoints[ndx].pos.x;
                t.ay = navPoints[ndx].pos.z;
                t.bx = edgeList[i].x1; t.by = edgeList[i].y1;
                t.cx = edgeList[i].x2; t.cy = edgeList[i].y2;
                tris2[ numTris2++ ] = t;
                
                //printf("constructing tri\n" );
            }
        }
        
        // update the list
        memcpy( tris, tris2, sizeof(Triangle) * numTris2 );
        numTris = numTris2;
    }
    
    
    // convert the tris to adjacency
    printf("built tris done, numTris is %d\n", numTris );
    for (int i=0; i < numTris; i++) {
        int andx, bndx, cndx;
        
        andx = -1; bndx = -1; cndx = -1;
        for (int j=0; j < numNavPoints; j++) {
            if ( (fabs(tris[i].ax - navPoints[j].pos.x) < 0.01) &&
               ( (fabs(tris[i].ay - navPoints[j].pos.z) < 0.01))) andx = j;

            if ( (fabs(tris[i].bx - navPoints[j].pos.x) < 0.01) &&
                ( (fabs(tris[i].by - navPoints[j].pos.z) < 0.01))) bndx = j;

            if ( (fabs(tris[i].cx - navPoints[j].pos.x) < 0.01) &&
                ( (fabs(tris[i].cy - navPoints[j].pos.z) < 0.01))) cndx = j;
        }
        
        if ( (andx >= 0) && (bndx >=0 )) {
            printf("navpoints AB %d and %d connected\n", andx, bndx );
            navAdj[andx][bndx] |= NAV_CONNECTED;
            navAdj[bndx][andx] |= NAV_CONNECTED;
        }
        
        if ( (bndx > 0) && (cndx >=0 )) {
            printf("navpoints BC %d and %d connected\n", bndx, cndx );
            navAdj[bndx][cndx] |= NAV_CONNECTED;
            navAdj[cndx][bndx] |= NAV_CONNECTED;
        }
        
        if ( (cndx > 0) && (andx >=0 )) {
            printf("navpoints CA %d and %d connected\n", cndx, andx );
            navAdj[cndx][andx] |= NAV_CONNECTED;
            navAdj[andx][cndx] |= NAV_CONNECTED;
        }
    }
    
    free( tris );
    free( tris2 );
    
}


void SaveNavMesh( const char *filename )
{
    FILE *fp = fopen( filename, "wt");
    
    for (int i=0; i < numNavPoints; i++) {
        fprintf( fp, "p %f %f\n", navPoints[i].pos.x, navPoints[i].pos.z );
    }
    
    for (int i=0; i< numNavPoints; i++) {
        for (int j=0; j < i; j++) {
            if (navAdj[i][j] & NAV_CONNECTED) {
                fprintf( fp, "c %d %d\n", i, j );
            }
        }
    }
    
    fclose(fp);
}

void LoadNavMesh( const char *filename )
{
    FILE *fp = fopen( filename, "rt" );
    if (!fp) return;
    
    numNavPoints = 0;
    memset( navAdj, 0, MAX_NAV*MAX_NAV );
    
    char line[1024];
    char tok[100];
    while (!feof(fp)) {
        fgets( line, 1024, fp);
        sscanf( line, "%s", tok );
        if (tok[0]=='p') {
            float x, y;
            sscanf( line, "%*s %f %f", &x, &y );
            AddNavPoint( Vector3Make( x, 0, y ));
        } else if (tok[0]=='c') {
            int i, j;
            sscanf( line, "%*s %d %d", &i, &j );
            navAdj[i][j] |= NAV_CONNECTED;
        }
    }
}

NavPoint *GetClosestNav( Vector3 target )
{
    NavPoint *result = NULL;
    float bestDist = 0;
    
    for (int i=0; i < numNavPoints; i++) {
        NavPoint *p = navPoints + i;
        if (!p->active) continue;
        
        float pdist = VectorDistance( p->pos, target );
        if ((i==0)||(pdist < bestDist)) {
            bestDist = pdist;
            result = p;
        }
    }
    
    return result;
}

void UpdateNavDists( Vector3 target )
{
    NavPoint *startPoint = GetClosestNav( target );
    if (!startPoint) return;
    
    for (int i=0; i < numNavPoints; i++) {
        navPoints[i].pdist = 99999.0;
    }
    
//    startPoint->pdist = VectorDistance( startPoint->pos, target );
        startPoint->pdist = 1.0;
    
    bool changed = true;
    int count = 0;
    while (changed) {
        count++;
//        printf("iter %d ---\n" );
        changed = false;
        for (int i=0; i < numNavPoints; i++) {
            if (!navPoints[i].active) continue;
            
            NavPoint *np = navPoints + i;
            for (int j=0; j < numNavPoints; j++) {
                
                if (j==i) continue;
                if (!navPoints[j].active) continue;
                
                if ((navAdj[i][j] & NAV_CONNECTED) || (navAdj[j][i] & NAV_CONNECTED)) {
                    
//                    printf("Conn: %d %d\n", i, j );
                    
                    NavPoint *np2 = navPoints + j;
                    float d = np->pdist + VectorDistance( np->pos, np2->pos );
//                    float d = np->pdist + 1.0;
                    if (d < np2->pdist) {
//                        printf("update dist %d %d, old %f new %f\n", i, j,  np2->pdist, d );
                        np2->pdist = d;
                        changed = true;
                    }
                }
            }
        }
    }
    
    navMaxDist = 0.0;
    for (int i=0; i < numNavPoints; i++) {
        if (navPoints[i].pdist > 9000) continue;
        if (navPoints[i].pdist > navMaxDist) {
            navMaxDist = navPoints[i].pdist;
        }
    }
    
    
    //printf("Update NavDists: count %d maxDist %f\n", count , navMaxDist );
    
    
}



// ===================================================================
//       EDITOR
// ===================================================================

SceneInstance *edSelectedInst = NULL;
SceneObject *edCurrBrush = NULL;

// somebody needs to move the editor code up heres...

enum {
    EditXformMode_ROTATE,
    EditXformMode_SCALE,
    EditXformMode_MOVE,
    EditXformMode_DITHER,
    
    EditXformMode_COUNT
};
int editXformMode = EditXformMode_ROTATE;

void UpdateSelection()
{
    SceneInstance *oldSelectedInst = edSelectedInst;
    
    if (!edSelectedInst) {
        return;
    }
    
    edSelectedInst = NULL;
    
    // See if there's a matching object in the current year
    for (SceneInstance *inst = scene->firstInst; inst; inst = inst->nextInst) {
        if (inst->objId == oldSelectedInst->objId) {
            edSelectedInst = inst;
            break;
        }
    }
    return;
}

void UpdateNavActives()
{
    
    for (int i=0; i < numNavPoints; i++) {
        for (int j=0; j < numNavPoints; j++) {
            if (navAdj[i][j] & NAV_CONNECTED) {
                navAdj[i][j] |= NAV_ACTIVE;
            } else {
                navAdj[i][j] = 0;
            }
        }
    }
    
    for (int i=0; i < numNavPoints; i++) {
        NavPoint *p = navPoints + i;
        
        p->active = true;
        for (SceneInstance *inst=scene->firstInst;
             inst; inst = inst->nextInst ) {
            if (CheckCollisionBoxSphere( inst->bbox, p->pos, 0.5)) {
                p->active = false;
                for (int j=0; j < numNavPoints; j++) {
                    navAdj[i][j] &= ~NAV_ACTIVE;
                    navAdj[j][i] &= ~NAV_ACTIVE;
                }
                
                break;
            }
        }
    }
    
    
}

void UpdateAfterYearChanged()
{
    UpdateSelection();
    UpdateNavActives();
}

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
    bool doPostProcess = true;
    bool editorMode = false;
    bool showNavMesh = true;
    
    // dither controls
    bool showLookup = false;
    float ditherStrength = 0.1;
    
    // editor stamp controls
    bool randomRotate = false;
    float currRotation = 0.0;
    float currScale = 1.0;
    float currZoffs = 0.0;
    
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
    lgtSun->target = (Vector3){ 0.0f, 0.0f, 0.0f };
    lgtSun->position = (Vector3){ -5.0f, 0.0f, 0.0f };
    lgtSun->intensity = 2.0f;

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
    
    SceneObject *objWall = LoadSceneObject( "wall" );
    objWall->lifetimeMin = 10;
    objWall->lifetimeMax = 10;
    //objTree->decayInto = objStump;

    SceneObject *objLiteTest = LoadSceneObject( "lite_test" );
    objLiteTest->lifetimeMin = 10;
    objLiteTest->lifetimeMax = 10;

    SceneObject *objTile1 = LoadSceneObject( "tile_basic" );
    objTile1->lifetimeMin = 4;
    objTile1->lifetimeMax = 6;
    
    SceneObject *objTileBroke = LoadSceneObjectReuseTexture( "tile_broke1", objTile1->model.material.texDiffuse );
    objTileBroke->lifetimeMin = 2;
    objTileBroke->lifetimeMax = 4;

    SceneObject *objTileBroke2 = LoadSceneObjectReuseTexture( "tile_broke2", objTile1->model.material.texDiffuse );
    objTileBroke2->lifetimeMin = 2;
    objTileBroke2->lifetimeMax = 4;

    objTile1->decayInto = objTileBroke;
    objTileBroke->decayInto = objTileBroke2;
    
    SceneObject *objTileTall = LoadSceneObjectReuseTexture( "tile_tall", objTile1->model.material.texDiffuse );
    objTileTall->lifetimeMin = 2;
    objTileTall->lifetimeMax = 4;

    
    SceneObject *objWall2 = LoadSceneObjectReuseTexture( "wall2", objWall->model.material.texDiffuse );
    objWall2->lifetimeMin = 10;
    objWall2->lifetimeMax = 10;

    SceneObject *objBigWall = LoadSceneObjectReuseTexture( "bigwall", objWall->model.material.texDiffuse );
    objBigWall->lifetimeMin = 10;
    objBigWall->lifetimeMax = 10;

    SceneObject *objSph1 = LoadSceneObjectReuseTexture( "sph_smooth", objWall->model.material.texDiffuse );
    objSph1->lifetimeMin = 10;
    objSph1->lifetimeMax = 10;

    SceneObject *objSph2 = LoadSceneObjectReuseTexture( "sph_facet", objWall->model.material.texDiffuse );
    objSph2->lifetimeMin = 10;
    objSph2->lifetimeMax = 10;

    
    // Load Creeps
    CreepObject *creepTest = LoadCreep( "creep" );
    MakeSpawner( creepTest );
    
    // Load the world
    LoadWorld( "gamedata/world.txt");
    LoadNavMesh( "gamedata/navmesh.txt");
    
    edCurrBrush = objects;
    
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

    float angle = (M_PI/180.0)*90.0;

    // Build World
#if 1
    Model model = LoadModel( "gamedata/ground1.obj");
    //model.material = LoadStandardMaterial();
    model.material.texDiffuse = LoadTexture( "gamedata/ground2.png");
    //model.material.texDiffuse = LoadTexture( "gamedata/colortest.png");
    //pmodel.material.colAmbient = LIGHTGRAY;
    SetTextureFilter( model.material.texDiffuse, FILTER_POINT );
#endif
    
    // playArea is shrunk by the player size
    BoundingBox bboxPlayArea = (BoundingBox) { -10.0f + playerSz2, -0.1f, -10.0f + playerSz2,
                                                10.0f - playerSz2, 1000.0f,  10.0f - playerSz2 };

    BoundingBox bboxWorld = (BoundingBox) { -30.0, -1.0,  -30.0,
                                            30.0, 1000.0f,  30.0 };

    
    // Start at year 0
    currentYear = 0;
    scene = sceneYear + currentYear;
    
    UpdateNavActives();
    
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
    
    
    // for testing
    Texture2D dbgLookup = LoadTexture("gamedata/lookup.png");
//    Texture2D dbgLookup = LoadTexture("gamedata/lite_test.png");
    
    // Set up pixelate filter
    MakePaletteTexture(64);

    
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
        
        if (IsKeyPressed('I')) {
            showLookup = !showLookup;
        }

        
        if (IsKeyPressed('R')) {
            retinaHack = !retinaHack;
        }
        
        if (IsKeyPressed( '`' )) {
            editorMode = !editorMode;
        }
        
//        lgtSun->position.x = -cos( angle + 2 ) * 10.0;
//        lgtSun->position.y = 0.0;
//        lgtSun->position.z = -sin( angle + 2) * 10.0;
        
        if (editorMode) {
            
            if (IsKeyDown('.')) {
                if (currentYear < NUM_YEARS-1 ) {
                    currentYear++;
                    scene = sceneYear + currentYear;
                }
                UpdateAfterYearChanged();
            }
            
            if (IsKeyDown(',')) {
                if (currentYear > 0 ) {
                    currentYear--;
                    scene = sceneYear + currentYear;
                }
                UpdateAfterYearChanged();
            }
            
            if (IsKeyPressed(']')) {
                edCurrBrush += 1;
                if ((edCurrBrush - objects) >= numObjects) {
                    edCurrBrush = objects;
                }
            }
            
            if (IsKeyPressed('[')) {
                
                if (edCurrBrush > objects) {
                    edCurrBrush -= 1;
                } else {
                    edCurrBrush = objects + (numObjects-1);
                }
            }
            
            if (IsKeyPressed('S')) {
                SaveWorld( "gamedata/editorworld.txt");
                SaveNavMesh( "gamedata/editornavmesh.txt");
            }
            
            if (IsKeyPressed('M')) {
                showNavMesh = !showNavMesh;
            }
            
            if (IsKeyPressed('T')) {
                MakeEdgesDelauney();
            }
            
            if (IsKeyPressed('C')) {
                // Toggle Camera
                if (camera.position.y > 25.0) {
                    camera.position = (Vector3){ 0.0f, 21.0f, -21.0f };
                } else {
                    camera.position = (Vector3){ 0.0f, 50.0f, -21.0f };
                }
            }
            
            if (IsKeyPressed('B')) {
                if (edCurrBrush) {
                    edCurrBrush = NULL;
                } else {
                    edCurrBrush = objects;
                }
            }
            
            if (IsKeyPressed('1')) {
                randomRotate = !randomRotate;
            }
            
            if (IsKeyPressed('2')) {
                editXformMode ++;
                if (editXformMode == EditXformMode_COUNT) {
                    editXformMode = EditXformMode_ROTATE;
                }
            }
            
            if (GetMouseWheelMove() != 0) {
                
                if (editXformMode == EditXformMode_SCALE) {
                    
                    currScale += -(float)GetMouseWheelMove() * 0.1;
                    if (currScale < 0.1) currScale = 0.1;
                    
                } if (editXformMode == EditXformMode_MOVE) {
                    currZoffs +=  -(float)GetMouseWheelMove() * 0.1;
                } if (editXformMode == EditXformMode_DITHER) {
                    ditherStrength +=  -(float)GetMouseWheelMove() * 0.01;
                    if (ditherStrength < 0.0) ditherStrength = 0.0;
                    if (ditherStrength > 1.0) ditherStrength = 1.0;
                } if (editXformMode == EditXformMode_ROTATE) {
                    currRotation += (GetMouseWheelMove() * 1.0);
                    while (currRotation < 0.0) {
                        currRotation += 360.0;
                    }
                    
                    while (currRotation > 360.0) {
                        currRotation -= 360.0;
                    }
                }
            }
            
            float sz = 2.0;
            if (IsKeyPressed('J')) {
                for (int i=0; i < 10; i++) {
                    float offs = 0;
                    int rowNum = 10;
                    if (i%2) {
                        offs = (sz/2.0);
                        rowNum--;
                    }
                    for (int j=0; j < rowNum; j++) {
                        Vector3 pp = Vector3Make( ((float)i * sz) - (5.0 * sz) + (sz/2.0),
                                                 0.0, ((float)j *sz )- (5.0*sz) + offs + (sz/2.0) );
                        AddNavPoint( pp );
                    }
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

            // Update nav
            UpdateNavDists( playerPos );
            
            
            // Update Creep movement
            for (int cndx = 0; cndx < numCreeps; cndx++) {
                CreepInst *creep = creeps + cndx;
                
                Vector3 creepDir;
                if (!creep->currTarget) {
                    creep->currTarget = GetClosestNav( creep->position );
                }
                
                if (creep->currTarget) {
                    
                    Vector3 creepNav = VectorSubtract( creep->currTarget->pos, creep->position );
                    float dist = VectorLength( creepNav );
                    if (dist < 0.5) {
                        NavPoint *bestTarg = NULL;
                        
                        // Go to nearest connected navPoint
                        int navNdx = (int)(creep->currTarget - navPoints);
                        for (int i=0; i < numNavPoints; i++) {
                            if (i == navNdx) continue;
                            if (!navPoints[i].active) continue;
                            
                            if ((navAdj[navNdx][i] & NAV_CONNECTED) ||
                                (navAdj[i][navNdx] & NAV_CONNECTED) ) {
                                NavPoint *pp = navPoints + i;
                                if ((!bestTarg)||(pp->pdist < bestTarg->pdist)) {
                                    bestTarg = pp;
                                }
                            }
                        }
                        
                        creep->currTarget = bestTarg;
                    }
                    
                }
                
                if (creep->currTarget) {
                     creepDir = VectorSubtract( creep->currTarget->pos, creep->position );
                } else {
                     creepDir = VectorSubtract( playerPos, creep->position );
                }
                
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
                
#if 1
                DrawModel( model, cubePosition, 1.0, WHITE );
#endif
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
                        if ((inst->drawWires) || (inst->object == objTree)) {
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
                    
                    UpdateNavDists( groundPos );
                    
                    // Snap pos
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        groundPos = Vector3Make( floorf(groundPos.x), floorf(groundPos.y), floorf(groundPos.z));
                    }
                    
                    Vector3 stampPos = groundPos;
                    stampPos.y += currZoffs;

                    Color cursorBlink = ColorLerp( SKYBLUE, WHITE, fabs(sin(animTime * 10.0)) );
                    if (edCurrBrush) {
                        
                        if (edCurrBrush->spawner) {
                            DrawModelWires( edCurrBrush->spawner->model, groundPos, 1.0, cursorBlink );
                        } else {
                            //DrawModelWires( edCurrBrush->model, groundPos, 1.0, cursorBlink );
                            DrawModelWiresEx( edCurrBrush->model, stampPos, playerUp,
                                             currRotation,
                                             VectorMultScalar( edCurrBrush->startSize, currScale), cursorBlink );
                        }
                        
                        
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            float angle = currRotation;
                            if (randomRotate) {
                                angle = RandUniformRange( 0.0, 360.0);
                            }
                            
                            SceneStampObject( scene, stampPos, angle, Vector3Make( currScale, currScale, currScale), edCurrBrush );
                            UpdateNavActives();
                        }
                        else if (IsKeyPressed( 'K')) {
                            SpawnCreep( creepTest, groundPos );
                        }
                        
                        if (IsKeyPressed('N')) {
                            groundPos.y = 0.5;
                            AddNavPoint( groundPos );
                        }
                        
                    } else {
                        
                        // See if the current brush overlaps any instances
                        SceneInstance *hoverInst = scene->firstInst;
//                        BoundingBox cursorBox;
//                        cursorBox.min = VectorAdd( groundPos, (Vector3){ -0.5, -0.1, -0.5  } );
//                        cursorBox.max = VectorAdd( groundPos, (Vector3){  0.5,  1.0,  0.5  } );
                        
                        SceneInstance *selectInst = NULL;
                        while (hoverInst)
                        {
                            if (CheckCollisionRayBox( ray, hoverInst->bbox )) {
                                DrawModelWiresEx( hoverInst->object->model, hoverInst->pos, playerUp, hoverInst->rotation,
                                                 hoverInst->scale, cursorBlink );
                                
                                if (!selectInst) {
                                  selectInst = hoverInst;
                                }
                            }
//                            else {
//                                DrawBoundingBox( hoverInst->bbox, RED );
//                            }
                            hoverInst = hoverInst->nextInst;
                        }
                        
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            edSelectedInst = selectInst;
                        }
                        

                        
                        DrawCubeWires( groundPos, 1.0, 0.2, 1.0, cursorBlink );
                        
                        if (edSelectedInst) {
                                DrawBoundingBox( edSelectedInst->bbox, WHITE );
                        }
                    }
                }
                
            }
            End3dMode();

            if (showLookup) {
                DrawTexturePro( dbgLookup,
                               (Rectangle){ 0, 0, 512, 512 },
                               //(Rectangle){ 0, 0, 128, 128 },
                               (Rectangle){ 0, 0, 200, 200 },
                               (Vector2){ 0, 0 }, 0, WHITE);
            }
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

            CHECKGL("drawtex");
            EndShaderMode();
            CHECKGL("start1");
            
            
            // directly bind palette shader
            static GLint samplerPally = -2;
            static GLint samplerDither = -2;
            static GLint ditherStrengthParam;
            CHECKGL("start");
            if (samplerPally < -1) {
                
                glUseProgram(shader.id);
                
                samplerPally = glGetUniformLocation( shader.id, "pally" );
                CHECKGL("get uniform");
                printf("pixelize shader prog %d samplerId %d\n", shader.id, samplerPally );
                
                samplerDither = glGetUniformLocation( shader.id, "dither" );
                
                ditherStrengthParam = glGetUniformLocation( shader.id, "ditherStrength" );
            }
            
            if (samplerPally >= 0) {
                glUseProgram(shader.id);
                CHECKGL("use shader");
                
                glUniform1i( samplerPally, 3 );
                CHECKGL("set pally sampler");
                
                glUniform1i( samplerDither, 4 );
                CHECKGL("set dither sampler");
                
                glUniform1f( ditherStrengthParam, ditherStrength );

                
                glActiveTexture( GL_TEXTURE3 );
                glBindTexture( GL_TEXTURE_3D, texPalette );
                CHECKGL("bind pally texture");
                
                glActiveTexture( GL_TEXTURE4 );
                glBindTexture( GL_TEXTURE_2D, texBayerDither );
                CHECKGL("bind dither texture");

                
                glActiveTexture( GL_TEXTURE0 );
            }
            
            EndTextureMode();
            
            // HACK work around displayScale
            //if (retinaHack) {
            //    glViewport(0, 0, GetScreenWidth() * 2.0, GetScreenHeight() * 2.0 );
           // }
            
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
                if (edCurrBrush) {
                    sprintf(buff, "BRUSH: %s", edCurrBrush->name );
                } else {
                    sprintf( buff, "Select Mode");
                }
                
                DrawText( buff, screenWidth-244, 4, 10, SKYBLUE );
                
                if (!edCurrBrush) {
                    if (edSelectedInst){
                        // Selected object info
                        float curry = 15;
                        float leading = 12;
                        sprintf( buff, "%s #%04X", edSelectedInst->object->name,
                                (edSelectedInst->objId & 0xFFFF) );
                        DrawText( buff, screenWidth-244, curry, 10, SKYBLUE );
                        curry += leading;
                        
                        sprintf( buff, "POS: %3.2f %3.2f %3.2f",
                                edSelectedInst->pos.x, edSelectedInst->pos.y, edSelectedInst->pos.z );
                        DrawText( buff, screenWidth-244, curry, 10, SKYBLUE );
                        curry += leading;
                        
                        sprintf( buff, "SCL: %3.2f %3.2f %3.2f",
                                edSelectedInst->scale.x, edSelectedInst->scale.y, edSelectedInst->scale.z );
                        DrawText( buff, screenWidth-244, curry, 10, SKYBLUE );
                        curry += leading;
                        
                        sprintf( buff, "ANG: %3.2f",
                                edSelectedInst->rotation );
                        DrawText( buff, screenWidth-244, curry, 10, SKYBLUE );
                        curry += leading;
                    }
                } else {
                    float curry = 15;
                    float leading = 12;
                    sprintf( buff, "RND ROT: %s", randomRotate?"ON":"OFF" );
                    DrawText( buff, screenWidth-244, curry, 10, SKYBLUE );
                    curry += leading;
                    
                    sprintf( buff, "Angle: %f", currRotation );
                    Color lineColor = SKYBLUE;
                    if (editXformMode==EditXformMode_ROTATE) lineColor = WHITE;
                    DrawText( buff, screenWidth-244, curry, 10, lineColor );
                    curry += leading;
                    
                    sprintf( buff, "Scale: %3.2f", currScale );
                    lineColor = SKYBLUE;
                    if (editXformMode==EditXformMode_SCALE) lineColor = WHITE;
                    DrawText( buff, screenWidth-244, curry, 10, lineColor );
                    curry += leading;
                    
                    sprintf( buff, "Move: %3.2f", currZoffs );
                    lineColor = SKYBLUE;
                    if (editXformMode==EditXformMode_MOVE) lineColor = WHITE;
                    DrawText( buff, screenWidth-244, curry, 10, lineColor );
                    curry += leading;
                    
                    
                    sprintf( buff, "Dither: %3.2f", ditherStrength );
                    lineColor = SKYBLUE;
                    if (editXformMode==EditXformMode_DITHER) lineColor = WHITE;
                    DrawText( buff, screenWidth-244, curry, 10, lineColor );
                    curry += leading;


                    
                }
                
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
