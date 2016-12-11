
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

// ===================================================================
struct SceneObject {
    const char *name;
    Model model;
};

struct SceneInstance {
    SceneObject *object;
    uint32_t objId;
    Vector3 pos;
    float rotation;
    BoundingBox bbox;
    bool drawWires;
    Color wireColor;
    
    SceneInstance *nextInst;
};

SceneInstance *SceneInstanceCreate()
{
    SceneInstance *inst = (SceneInstance*)malloc(sizeof(SceneInstance));
    memset( inst, 0, sizeof(SceneInstance));
    inst->objId = rand();
    
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
    SceneInstance *firstInst;
};

#define NUM_YEARS (101)
Scene sceneYear[NUM_YEARS];

int currentYear;
Scene *scene = NULL;

#define MAX_OBJECTS (100)
SceneObject objects[MAX_OBJECTS];
int numObjects = 0;

void LoadSceneObject( const char *name )
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
}

// ===================================================================
//       EDITOR
// ===================================================================
SceneInstance *edSelectedInst = NULL;
SceneObject *edCurrentObject = NULL;


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

//    SceneObject treeObj = {0};
//    treeObj.name = "Tree";
//    treeObj.model = LoadModel( "gamedata/blerghTree.obj" );
//    treeObj.model.material = LoadStandardMaterial();
//    Texture2D texture = LoadTexture("gamedata/blerghTree.png");
//    treeObj.model.material.texDiffuse = texture;

    LoadSceneObject( "blerghTree" );
    LoadSceneObject( "stump" );
    
    edCurrentObject = objects;
    
    Model player = LoadModel( "gamedata/puck.obj");
    player.material = LoadStandardMaterial();
    player.material.texDiffuse = LoadTexture( "gamedata/puck.png");
    SetTextureFilter( player.material.texDiffuse, FILTER_POINT );
    Vector3 playerPos = (Vector3){ 0.0, 0.0, -10.0 };
    
    float playerAngle = 180.0; // angle used for gameplay
    float playerDisplayAngle = 180.0; // angle used for display
    
    float playerSz = 2.0;
    float playerSz2 =  playerSz;
    Vector3 playerUp = (Vector3){ 0.0, 1.0, 0.0};
    Vector3 playerScale = (Vector3){ playerSz,  playerSz, playerSz };
    Vector3 playerMoveDir = (Vector3){ 0.0, 0.0, 1.0 };

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
    
	while (!WindowShouldClose())
	{
        animTime += GetFrameTime();
        
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
            
            float moveSpeed = 20.0;
            if (VectorLength( playerMoveDir) > 0.01)
            {
                VectorScale( &playerMoveDir, GetFrameTime() * moveSpeed );
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

                    DrawModelEx(inst->object->model, inst->pos,
                                playerUp, inst->rotation, ScaleOne,
                                WHITE );
                    if (inst->drawWires) {
                        //DrawModelWires(inst->object->model, inst->pos, 1.0, BLACK );
                        DrawModelWiresEx(inst->object->model, inst->pos,
                                    playerUp, inst->rotation, ScaleOne,
                                    BLACK );
                    }
                    
                    inst = inst->nextInst;
                }
                
                if (!editorMode) {
                    DrawModelEx( player, playerPos, playerUp, playerDisplayAngle,
                                playerScale, YELLOW );
                    
                } else {
                    // Edit Mode
                    DrawLight( lgtSun );
                    DrawGrid( 10, 2.0f );
                    
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
                        DrawModelWires( edCurrentObject->model, groundPos, 1.0, cursorBlink );
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
            }
            
            int timelineY = 600;
            DrawRectangle( 140, timelineY, 1000, 2, SKYBLUE );
            
            int step = 1000 / 100;
            for (int i=0; i <= 100; i++) {
                DrawRectangle( 140 + i*step , timelineY - 5, 1, 13, SKYBLUE );
            }
            
            // Highlight current year
            DrawRectangle( 140 + currentYear*step - 2, timelineY - 6, 5, 15, GOLD );

        }
        
        DrawFPS(10, 10);
        
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
