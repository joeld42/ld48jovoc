/*******************************************************************************************
*
*   nospace_gui - tool description
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 tapnik
*
**********************************************************************************************/

#include <stdint.h>
#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define ITEM_SQ (15)

enum {
    CellFlags_FULL = 1 << 0,
    CellFlags_HIGHLIGHTED = 1 << 1,
};

typedef struct ItemStruct
{
    char name[64];
    int xsize;
    int ysize;
    uint8_t grid[5][5];
    Texture icon;
    
    Vector2 lootPos;
    Rectangle screenRect;
} Item;

typedef struct ItemGridStruct
{
    bool isSlot;
    Vector2 origin;
    int xsize;
    int ysize;
    Item *slotItem; // for isSlot = true

    uint8_t gridflags[7][7];

    Rectangle screenRect;
    struct ItemGridStruct *next;

    bool highlightForDrop;

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

    Item *lootItems[100];
    int numLootItems;

    Item *dragItem;
    ItemGrid *dropTargetGrid;

} Game;

void *BaseAllocFunc( size_t size, char *file, int line )
{
    void *result = malloc( size );
    memset( result, 0, size );
    return result;
}

#define Alloc( TYPE ) \
    (TYPE*)BaseAllocFunc(sizeof(TYPE),__FILE__,__LINE__)

Rectangle RectMake( float x, float y, float width, float height )
{
    Rectangle result;
    result.x = x;
    result.y = y;
    result.width = width;
    result.height = height;
    return result;
}

Vector2 Vector2Make( float x, float y )
{
    Vector2 result;
    result.x = x;
    result.y = y;
    return result;
}

float FloatMin( float a, float b) {
    if (a < b) return a; else return b;
}

float FloatMax( float a, float b) {
    if (a > b) return a; else return b;
}


Vector2 Vector2MultAdd( Vector2 a, Vector2 b, float cf )
{
    return Vector2Add( a, Vector2Scale( b, cf ) );
}

Rectangle ExpandRect( Rectangle rect, float amount ) {
    return RectMake( rect.x - amount, rect.y - amount, 
            rect.width + amount, rect.height + amount );
}

//------------------------------------------------------------------------------------
// Items
//------------------------------------------------------------------------------------

Item *CreateItem()
{
    Item *item = Alloc(Item);
    return item;
}

void MakeItemImage( Item *item )
{
    int width = item->xsize * 8;
    int height = item->ysize * 8;
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    Color col1 = BLUE;
    Color col2 = GREEN;
    int checksX = 8;
    int checksY = 8;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x/checksX + y/checksY)%2 == 0) pixels[y*width + x] = col1;
            else pixels[y*width + x] = col2;
        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    item->icon = LoadTextureFromImage( image );
    // TODO free image?

}

//------------------------------------------------------------------------------------
// InventoryContainer
//------------------------------------------------------------------------------------
InventoryContainer *CreateInventoryCtr( char *name )
{
    InventoryContainer *ctr = Alloc(InventoryContainer);

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

        curr->screenRect = RectMake( og.x, og.y, curr->xsize * ITEM_SQ +1, curr->ysize * ITEM_SQ +1);

        Color borderColor = BLUE;
        Color fillColor = SKYBLUE;
        if (curr->highlightForDrop) {
            borderColor = DARKGREEN;
            fillColor = GREEN;
        }

        if (curr->isSlot)
        {
            // It's a slot
            //printf("Draw grid %f %f %d %d\n", og.x, og.y, curr->xsize, curr->ysize );            
            DrawRectangle( og.x, og.y, 
                curr->xsize * ITEM_SQ +1, 
                curr->ysize * ITEM_SQ +1, borderColor );
            DrawRectangle( og.x+1, og.y+1, 
                curr->xsize * ITEM_SQ-1, 
                curr->ysize * ITEM_SQ-1, fillColor );
            
            if (curr->slotItem) {

                Vector2 fitSize = Vector2Make( curr->xsize * ITEM_SQ-1, curr->ysize * ITEM_SQ-1 );
                Vector2 ctr = Vector2Make( og.x + (curr->xsize * ITEM_SQ +1)/ 2,
                                           og.y + (curr->ysize * ITEM_SQ +1)/ 2 );

                float sz = FloatMax( fitSize.x, fitSize.y );
                float itemSz = FloatMax( curr->slotItem->icon.width, curr->slotItem->icon.height );

                float scale = FloatMin( 1.0f, sz/itemSz );

                //printf("Scale %f, sz is %f itemSz %f\n", scale, sz, itemSz );

                float w2 = curr->slotItem->icon.width * scale * 0.5;
                float h2 = curr->slotItem->icon.height * scale * 0.5;
                DrawTextureEx( curr->slotItem->icon,
                    Vector2Make( ctr.x - w2, ctr.y - h2), 0.0f, scale, WHITE );

                //DrawTextureEx( curr->slotItem->icon, og, 0.0f, 1.875f, WHITE );
            }

        } else {

            for (int i=0; i < curr->xsize; i++) {
                for (int j=0; j < curr->ysize; j++) {

                    Vector2 sq;
                    sq.x = og.x + (i*ITEM_SQ);
                    sq.y = og.y + (j*ITEM_SQ);
                    DrawRectangle( sq.x, sq.y, ITEM_SQ+1, ITEM_SQ+1, borderColor );
                    DrawRectangle( sq.x+1, sq.y+1, ITEM_SQ-1, ITEM_SQ-1, fillColor );
                }
            }
        }

        curr = curr->next;
    }
}

ItemGrid *AddGrid( InventoryContainer *ctr, Vector2 origin, int xs, int ys )
{
    ItemGrid *grid = Alloc(ItemGrid);

    grid->origin = origin;
    grid->xsize = xs;
    grid->ysize = ys;

    grid->next = ctr->grids;
    ctr->grids = grid;

    return grid;
}

bool TryDropItemOnGrid( ItemGrid *grid, Item *item, Vector2 hoverPos, bool performDrop )
{
    if (grid->isSlot) {
        // If it's a slot, doesn't matter can hold one item
        if (grid->slotItem) {
            return false; // already full
        } else {
            if (performDrop) {
                grid->slotItem = item;
            }
            return true;
        }
    } else {
        return false;
    }
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



        // Update loot items so they try not to overlap
        bool isOverlap = false;
        for (int i=0; i < game.numLootItems; i++) {
            Item *item = game.lootItems[i];
            if (item->screenRect.width ==0 ) continue;

            if (item->screenRect.x < 8) {
                isOverlap= true;
                item->lootPos.x += 1;
            } else if (item->screenRect.x + item->icon.width > 400-8 ) {
                isOverlap= true;
                item->lootPos.x -= 1;
            } else if (item->screenRect.y < 208) {
                isOverlap= true;
                item->lootPos.y += 1;
            } else if (item->screenRect.y + item->icon.height > 300-8 ) {
                isOverlap= true;
                item->lootPos.y -= 1;
            }

            for (int j=0; j < game.numLootItems; j++) {
                if (i==j) continue;

                Item *other = game.lootItems[j];
                if (other->screenRect.width == 0) continue;

                // Overlap                
                if (CheckCollisionRecs( ExpandRect( item->screenRect, 2.0), 
                                        ExpandRect( other->screenRect, 2.0) )) {
                    isOverlap= true;

                    Vector2 ab = Vector2Normalize( Vector2Subtract( item->lootPos, other->lootPos ) );

                    float strength = 0.2f; // Could be a pow of distance if we want this smoother
                    item->lootPos = Vector2MultAdd( item->lootPos, ab, strength );
                    other->lootPos = Vector2MultAdd( other->lootPos, ab, -strength );
                }
            }
        }


        // Update Loot
        // TODO add timer
        if ((!isOverlap) && (game.numLootItems < 100)) {
            Item *item = Alloc(Item);
            strcpy(item->name,"Item");
            item->xsize = GetRandomValue( 1, 5 );
            item->ysize = GetRandomValue( 1, 5 );
            item->lootPos = Vector2Make( GetRandomValue( item->xsize, 400-item->xsize),
                                         GetRandomValue( 200 + (item->ysize/2), 300-(item->ysize/2) ) );

            MakeItemImage( item );
            game.lootItems[game.numLootItems++] = item;
        }

        Vector2 mousePos = GetMousePosition();

        game.dropTargetGrid = NULL;
        for (int i=0; i < game.invStackSize; i++) {
            ItemGrid *curr = game.invStack[i]->grids;
            while (curr) {
                bool canDrop = false;
                if ((game.dragItem) && (CheckCollisionPointRec(mousePos, curr->screenRect ))) {
                    // check if it's a valid drop
                    if (TryDropItemOnGrid( curr, game.dragItem, mousePos, false )) {
                        canDrop = true;  
                        game.dropTargetGrid = curr;
                    }                    
                }
                curr->highlightForDrop = canDrop;

                curr = curr->next;
            }
        }

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
            game.health = GuiSliderBarEx((Rectangle){ 235, 8, 125, 15 }, game.health, 0, 100, "HP", true);
            game.mana = GuiSliderBarEx((Rectangle){ 235, 28, 125, 15 }, game.mana, 0, 100, "Mana", true);

            GuiGroupBox((Rectangle){ 6, 200, 380, 90 }, "Loot");
            bool didPickup = false;
            int pickupItemIndex = -1;
            for (int i = 0; i < game.numLootItems; i++) {

                Item *item = game.lootItems[i];
                Vector2 drawPos = Vector2Make(item->lootPos.x - item->icon.width/2, 
                                              item->lootPos.y - item->icon.height/2 );

                
                item->screenRect = RectMake( drawPos.x, drawPos.y, 
                                             item->icon.width, item->icon.height );

                Color highlightColor = WHITE;
                if ((!game.dragItem) && (CheckCollisionPointRec( mousePos, item->screenRect)) ) {
                    highlightColor = GREEN;

                    if (IsMouseButtonPressed( MOUSE_LEFT_BUTTON )) {
                        pickupItemIndex = i;
                        
                        game.dragItem = item;
                        didPickup = true;
                    }
                }

                DrawTexture( item->icon, drawPos.x, drawPos.y, highlightColor );

                currX += item->icon.width +5;                
            }

            if (pickupItemIndex >= 0) {
                game.numLootItems -= 1;
                game.lootItems[pickupItemIndex] = game.lootItems[game.numLootItems];
            }


            if ((!didPickup) && (IsMouseButtonReleased( MOUSE_LEFT_BUTTON )) ) 
            {
                if (game.dragItem) {

                    // If there's a grid we can drop into, do that                    
                    if (game.dropTargetGrid) {
                        ItemGrid *dropGrid = game.dropTargetGrid;
                        if (TryDropItemOnGrid( dropGrid, game.dragItem, mousePos, true )) {
                            game.dragItem = NULL;
                        }
                    } 

                    // If we didn't drop it on the dropTarget, drop it back in the loot
                    if (game.dragItem) {
                        // Drop item back in the lootins
                        game.dragItem->lootPos = Vector2Make( mousePos.x, FloatMax( mousePos.y, 200) );
                        game.lootItems[game.numLootItems++] = game.dragItem;
                    }
                    game.dragItem = NULL;
                }
            }

            if (game.dragItem) {
                // DrawTexture( game.dragItem->icon, 
                //                 mousePos.x - game.dragItem->icon.width/2, 
                //                 mousePos.y - game.dragItem->icon.height/2, 
                //                 WHITE );
                float w2 = game.dragItem->icon.width * 0.9375f;
                float h2 = game.dragItem->icon.height * 0.9375f;
                DrawTextureEx( game.dragItem->icon,
                    Vector2Make( mousePos.x - w2, mousePos.y - h2), 0.0f, 1.875f, WHITE );
            }

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
