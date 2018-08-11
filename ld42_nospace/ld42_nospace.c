/*******************************************************************************************
*
*   nospace_gui - tool description
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 tapnik
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define ITEM_SQ (15)

typedef struct ItemGridStruct
{
    bool isSlot;
    Vector2 origin;
    int xsize;
    int ysize;

    struct ItemGridStruct *next;

} ItemGrid;

typedef struct InventoryContainerStruct
{
    char name[64];
    int width;
    Texture icon;

    ItemGrid *grids;

} InventoryContainer;

typedef struct GameStruct
{
    float health;
    float mana;

    InventoryContainer *invRoot;

    InventoryContainer *invStack[10];
    int invStackSize;

} Game;


//------------------------------------------------------------------------------------
// InventoryContainer
//------------------------------------------------------------------------------------
InventoryContainer *CreateInventoryCtr( char *name )
{
    InventoryContainer *ctr = (InventoryContainer*)malloc(sizeof(InventoryContainer));
    memset( ctr, 0, sizeof(InventoryContainer));
    if (name) {
        strcpy( ctr->name, name );
    }
    ctr->width = 130;

    return ctr;
}

void ReleaseInventoryCtr( InventoryContainer *ctr )
{
    free( ctr );
}

void DrawInventoryContainer( float xval, InventoryContainer *ctr )
{
    Rectangle rect = { 5, 50, 130, 140 };
    rect.x = xval;
    rect.width = ctr->width;
    GuiGroupBox(rect, ctr->name);

    // Draw grids
    ItemGrid *curr = ctr->grids;
    while (curr) {
        Vector2 og;
        og.x = rect.x + curr->origin.x;
        og.y = rect.y + curr->origin.y;        

        if (curr->isSlot)
        {
            // It's a slot
            //printf("Draw grid %f %f %d %d\n", og.x, og.y, curr->xsize, curr->ysize );
            DrawRectangle( og.x, og.y, 
                curr->xsize * ITEM_SQ +1, 
                curr->ysize * ITEM_SQ +1, BLUE );
            DrawRectangle( og.x+1, og.y+1, 
                curr->xsize * ITEM_SQ-1, 
                curr->ysize * ITEM_SQ-1, SKYBLUE );
        } else {

            for (int i=0; i < curr->xsize; i++) {
                for (int j=0; j < curr->ysize; j++) {

                    Vector2 sq;
                    sq.x = og.x + (i*ITEM_SQ);
                    sq.y = og.y + (j*ITEM_SQ);
                    DrawRectangle( sq.x, sq.y, ITEM_SQ+1, ITEM_SQ+1, BLUE );
                    DrawRectangle( sq.x+1, sq.y+1, ITEM_SQ-1, ITEM_SQ-1, SKYBLUE );
                }
            }
        }

        curr = curr->next;
    }
}

ItemGrid *AddGrid( InventoryContainer *ctr, Vector2 origin, int xs, int ys )
{
    ItemGrid *grid = (ItemGrid*)malloc(sizeof(ItemGrid));
    memset( grid, 0, sizeof(ItemGrid));
    grid->origin = origin;
    grid->xsize = xs;
    grid->ysize = ys;

    grid->next = ctr->grids;
    ctr->grids = grid;

    return grid;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "nospace_gui");
    SetMouseScale(0.5f);

    Game game = { 0 };
    game.health = 50.0f;
    game.mana = 50.0f;
    game.invRoot = CreateInventoryCtr( "Equipped" );
    game.invRoot->width = 130;

    game.invStack[0] = game.invRoot;
    game.invStackSize = 1;
    
    // Helmet
    ItemGrid *g = AddGrid( game.invRoot, (Vector2){ 65-ITEM_SQ, 5 }, 2, 2 );
    g->isSlot = true;

    // Body grid
    g = AddGrid( game.invRoot, (Vector2){ 35, 40}, 4, 4 );

    // Belt
    g = AddGrid( game.invRoot, (Vector2){ 28, 105 }, 5, 1 );

    // Left Weapon
    g = AddGrid( game.invRoot, (Vector2){ 3, 60}, 2, 2 );
    g->isSlot = true;

    // Left Hand
    g = AddGrid( game.invRoot, (Vector2){ 5, 93}, 1, 1 );

    // Right Weapon
    g = AddGrid( game.invRoot, (Vector2){ 97, 60}, 2, 2 );
    g->isSlot = true;

    // Right Hand
    g = AddGrid( game.invRoot, (Vector2){ 108, 93}, 1, 1 );

    InventoryContainer *backpack = CreateInventoryCtr( "Backpack");
    backpack->width = 110;
    game.invStack[game.invStackSize++] = backpack;

    g = AddGrid( backpack, (Vector2){ 10, 10}, 6, 6 );

    InventoryContainer *sachel = CreateInventoryCtr( "Sachel");
    sachel->width = (ITEM_SQ * 4) + 20;
    game.invStack[game.invStackSize++] = sachel;
    g = AddGrid( sachel, (Vector2){ 10, 10}, 4, 4 );


    SetTargetFPS(60);

    RenderTexture2D screenTarget = LoadRenderTexture(512, 512);
    SetTextureFilter(screenTarget.texture, FILTER_POINT);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            
            BeginTextureMode(screenTarget);

            ClearBackground(GetColor(style[DEFAULT_BACKGROUND_COLOR]));
            //ClearBackground((Color)BLUE);

            // Draw inventory stack
            int currX = 5;
            for (int i=0; i < game.invStackSize; i++)
            {
                InventoryContainer *ctr = game.invStack[i];
                DrawInventoryContainer( currX, ctr );        
                currX += ctr->width + 5;
            }

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            GuiGroupBox((Rectangle){ 6, 200, 380, 90 }, "Loot");
            game.health = GuiSliderBarEx((Rectangle){ 235, 8, 125, 15 }, game.health, 0, 100, "HP", true);
            game.mana = GuiSliderBarEx((Rectangle){ 235, 28, 125, 15 }, game.mana, 0, 100, "Mana", true);
            //----------------------------------------------------------------------------------
        EndTextureMode();

        DrawTexturePro(screenTarget.texture, 
            (Rectangle){ 0, 0, screenTarget.texture.width, -screenTarget.texture.height }, 
            (Rectangle){ 0, 0, screenTarget.texture.width*2, screenTarget.texture.height*2 }, 
            (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------
