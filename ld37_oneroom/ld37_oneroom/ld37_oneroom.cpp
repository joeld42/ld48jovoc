
#include <stdlib.h>
#include "raylib.h"

int main()
{
	int screenWidth = 1280;
	int screenHeight = 720;

 	SetConfigFlags(FLAG_MSAA_4X_HINT);
    
	InitWindow( screenWidth, screenHeight, "ld37_oneroom");
    Camera camera;
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    camera.fovy = 45.0;
    
	SetTargetFPS(60);

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
    
    Material mtlTerrain = LoadStandardMaterial();
    mtlTerrain.colDiffuse = ORANGE;
    mtlTerrain.colAmbient = (Color){ 10, 10, 10, 255 };
    model.material = mtlTerrain;
    
    Light lgtSun = CreateLight( LIGHT_DIRECTIONAL, (Vector3){ 1.0, -3.0, -0.5 }, WHITE );
    lgtSun->target = Vector3{ 0.0f, 0.0f, 0.0f };
    lgtSun->intensity = 2.0f;
#endif

	while (!WindowShouldClose())
	{
		BeginDrawing();
        {
			ClearBackground( RAYWHITE );
        
        
            Begin3dMode(camera);
            {
                DrawCube( cubePosition, 2.0f, 2.0f, 2.0f, RED );
                DrawCubeWires( cubePosition, 2.0f, 2.0f, 2.0f, MAROON );
                
                //DrawModel( model, cubePosition, 1.0, ORANGE );
                //DrawModelWires( model, cubePosition, 1.0, BLACK );
                //DrawLight( lgtSun );

                DrawGrid( 10, 1.0f );
                
            }
            End3dMode();
            
            DrawText("LD37 OneRoom!", 190, 200, 20, LIGHTGRAY );

        }
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
