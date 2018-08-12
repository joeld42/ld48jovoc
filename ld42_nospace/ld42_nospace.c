/*******************************************************************************************
*
*   nospace_gui - tool description
*
*
*   Copyright (c) 2018 tapnik
*
**********************************************************************************************/

// Notes for RayGui:
// -- Font support in GUI? Even just one global GUI font would be nice, doesn't have to
//    be super customizeable
// -- background color for groupboxes that supports alpha
// -- support double-click

#include <stdint.h>
#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define ITEM_SQ (15)
#define LOOTBOX_W (300)

#define MAX_PANELS (10)

#define TRASH_TIME (0.5)

#define CLEAR CLITERAL{ 0, 0, 0, 0 }

#define FRAME_DT (1.0/60.0)

enum {
    GridFlags_FULL = 1 << 0,
    GridFlags_HIGHLIGHTED = 1 << 1,
};

struct InventoryContainerStruct;

typedef struct ItemStruct
{
    char name[64];
    int xsize;
    int ysize;
    uint8_t grid[5][5];
    Texture icon;
    
    Vector2 lootPos;
    Rectangle screenRect;

    // An item can be a container (such as a backpack)
    struct InventoryContainerStruct *ctr;

    // for items placed into grid
    int placex;
    int placey;
    struct ItemStruct *nextInGrid; // for placed items

    // for trashed items
    float trashCountdown;

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

    struct ItemStruct *placedItems;

    bool highlightForDrop;

} ItemGrid;

typedef struct InventoryContainerStruct
{
    char name[64];
    int width;
    Texture icon;

    ItemGrid *grids;

} InventoryContainer;

typedef struct PanelStruct
{
    Item *inspectItem; // for inspect panels
    InventoryContainer *ctr; // for inventory containers
} Panel;

typedef struct GameStruct
{
    float health;
    float mana;

    InventoryContainer *invRoot;

    Panel panelStack[MAX_PANELS];
    int panelStackSize;

    Item *lootItems[100];
    int numLootItems;

    Item *dragItem;
    ItemGrid *dropTargetGrid;
    Vector2 dragItemCorner;

    Item *trashItems[10];
    int numTrashItems;

} Game;

// Textures are destroyed at the end of frame
Texture unloadTexQueue[50];
int unloadTexQueueSize = 0;

void PushUnloadTex( Texture tex )
{
    unloadTexQueue[unloadTexQueueSize++] = tex;
}

void FlushUnloadTexQueue()
{
    for (int i=0; i < unloadTexQueueSize; i++) {
        UnloadTexture( unloadTexQueue[i] );
    }

    unloadTexQueueSize = 0;
}

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

float IntMin( int a, int b) {
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

void DestroyItem( Item *item )
{
    if (item->icon.width > 0) {
        PushUnloadTex( item->icon );
    }    

    // If this item is a backpack, destroy all the items in it too
    if (item->ctr) {
        printf("TODO: destroy contained items.\n");
    }

    free( item );
}

void MakeItemImage( Item *item )
{
    int width = item->xsize * 8;
    int height = item->ysize * 8;
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    Color col1 = VIOLET;
    Color col2 = BEIGE;

    if (item->ctr) {
        col1 = BROWN;
        col2 = DARKBROWN;
    }

    int checksX = 8;
    int checksY = 8;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int xx = x/checksX;
            int yy = y/checksY;
            if (item->grid[xx][yy]) {
                if ((x/checksX + y/checksY)%2 == 0) pixels[y*width + x] = col1;
                else pixels[y*width + x] = col2;
            } else {
                pixels[y*width+x] = CLEAR;
            }

            // border
            if ((x==0)||(y==0)||(x==width-1)||(y==height-1)) {
                pixels[y*width+x] = MAGENTA;
            }

        }
    }

    Image image = LoadImageEx(pixels, width, height);
    free(pixels);

    item->icon = LoadTextureFromImage( image );
    UnloadImage( image );

}

//------------------------------------------------------------------------------------
// InventoryContainer
//------------------------------------------------------------------------------------

Panel MakeInventoryPanel( InventoryContainer *ctr )
{
    Panel result = {0};
    result.ctr = ctr;
    return result;
}

Panel MakeInspectPanel( Item *item )
{
    Panel result = {0};
    result.inspectItem = item;
    return result;    
}

void PushPanel( Game *game, Panel panel )
{
    if (game->panelStackSize < MAX_PANELS) {
        game->panelStack[game->panelStackSize++] = panel;
    }
}

void PopPanel( Game *game ) {
    if (game->panelStackSize > 1) {
        game->panelStackSize--;
    }
}



InventoryContainer *CreateInventoryCtr( char *name )
{
    InventoryContainer *ctr = Alloc(InventoryContainer);

    if (name) {
        strcpy( ctr->name, name );
    }
    ctr->width = 130;

    return ctr;
}

void InspectItem( Game *game, InventoryContainer *currCtr, Item *item ) {

    // Pop until we're back at the current container

    while (game->panelStack[game->panelStackSize-1].ctr != currCtr) {
        PopPanel( game );
    }

    // If this is an item, make an inspect panel
    if (item->ctr) {
        PushPanel( game, MakeInventoryPanel( item->ctr ) );
    } else {
        PushPanel( game, MakeInspectPanel( item ));    
    }
}

void ReleaseInventoryCtr( InventoryContainer *ctr )
{
    free( ctr );
}

void GridRemovePlacedItem( ItemGrid *grid, Item *item ) 
{
    Item *prev = NULL;
    Item *curr = grid->placedItems;
    while (curr) {

        if (curr == item) {

            // remove the full tags            
            for (int i=0; i < item->xsize; i++) {
                for (int j=0; j < item->ysize; j++) {
                    if (item->grid[i][j]) {                        
                        grid->gridflags[item->placex + i][item->placey + j] &= ~GridFlags_FULL;
                    }
                }
            }            
            // clean up the list
            if (prev) {
                prev->nextInGrid = curr->nextInGrid;
            } else {
                grid->placedItems = curr->nextInGrid;
            }
            return;
        }

        prev = curr;
        curr = curr->nextInGrid;
    }
}

void ContainerRemoveItem( InventoryContainer *ctr, Item *item )
{
    // Try to remove it from all grids
    ItemGrid *currGrid = ctr->grids;
    while (currGrid) {

        if (currGrid->isSlot) {
            if (currGrid->slotItem == item) {
                currGrid->slotItem = NULL;
            }
        } else {
            GridRemovePlacedItem( currGrid, item );
        }

        currGrid = currGrid->next;
    }
}

void PickupDragItem( Game *game, Item *item )
{
    game->dragItem = item;

    // If this is a container and it's open on the panel stack, close it
    // to prevent nesting cycles of containers
    if ((item) && (item->ctr)) {
        for (int i=1; i < game->panelStackSize;i++) {
            if (game->panelStack[i].ctr == item->ctr) {
                game->panelStackSize = i;
                break;
            }
        }
    }
}


void DrawInventoryContainer( Game *game, float xval, InventoryContainer *ctr )
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

        Vector2 mousePos = GetMousePosition();

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
                Vector2 center = Vector2Make( og.x + (curr->xsize * ITEM_SQ +1)/ 2,
                                           og.y + (curr->ysize * ITEM_SQ +1)/ 2 );

                float sz = FloatMax( fitSize.x, fitSize.y );
                float itemSz = FloatMax( curr->slotItem->icon.width, curr->slotItem->icon.height );

                float scale = FloatMin( 1.0f, sz/itemSz );

                //printf("Scale %f, sz is %f itemSz %f\n", scale, sz, itemSz );
                Color color = WHITE;
                bool canPickup = false;
                if (CheckCollisionPointRec(mousePos, curr->screenRect)) {
                    if (!game->dragItem) {
                        color = GREEN;
                        canPickup = true;
                    }
                }


                float w2 = curr->slotItem->icon.width * scale * 0.5;
                float h2 = curr->slotItem->icon.height * scale * 0.5;
                DrawTextureEx( curr->slotItem->icon,
                    Vector2Make( center.x - w2, center.y - h2), 0.0f, scale, color );

                if ((canPickup) && (IsMouseButtonPressed( MOUSE_LEFT_BUTTON )) ) {                    
                    PickupDragItem( game, curr->slotItem );
                    curr->slotItem = NULL;
                } else if ((canPickup) && (IsMouseButtonPressed( MOUSE_RIGHT_BUTTON )) ) {
                    InspectItem( game, ctr, curr->slotItem );
                }
            }

        } else {
            
            int cursorX = -1;
            int cursorY = -1;
            if (mousePos.x >= curr->screenRect.x) {
                cursorX = (mousePos.x - curr->screenRect.x) / ITEM_SQ;
            }
            if (mousePos.y >= curr->screenRect.y) {
                cursorY = (mousePos.y - curr->screenRect.y) / ITEM_SQ;
            }

            for (int i=0; i < curr->xsize; i++) {
                for (int j=0; j < curr->ysize; j++) {

                    Vector2 sq;
                    sq.x = og.x + (i*ITEM_SQ);
                    sq.y = og.y + (j*ITEM_SQ);

                    Color cellFillColor = fillColor;

                    if ((cursorX==i) && (cursorY==j)) {
                        cellFillColor = GREEN;
                    }

                    if (curr->gridflags[i][j] & GridFlags_HIGHLIGHTED) {
                        if (curr->highlightForDrop) {
                            cellFillColor = YELLOW;                        
                        } else {
                            cellFillColor = RED;
                        }
                    }

                    DrawRectangle( sq.x, sq.y, ITEM_SQ+1, ITEM_SQ+1, borderColor );
                    DrawRectangle( sq.x+1, sq.y+1, ITEM_SQ-1, ITEM_SQ-1, cellFillColor );
                }
            }

            // Draw placed items
            Item *currItem = curr->placedItems;
            while (currItem) {

                Vector2 sq;
                sq.x = og.x + (currItem->placex*ITEM_SQ);
                sq.y = og.y + (currItem->placey*ITEM_SQ);

                // see if this overlaps cursorXY
                Color color = WHITE;
                bool canPickup = false;
                if (!game->dragItem) {
                    for (int i=0; i < currItem->xsize; i++) {
                        for (int j=0; j < currItem->ysize; j++) {
                            if ((cursorX==currItem->placex + i) && 
                                (cursorY==currItem->placey + j) &&
                                (currItem->grid[i][j]) ) {
                                // Can pickup this item
                                color = GREEN;
                                canPickup = true;
                            }
                        }                        
                    }
                }

                DrawTextureEx( currItem->icon, sq, 0.0f, 1.875f, color );

                if ((canPickup) && (IsMouseButtonPressed( MOUSE_LEFT_BUTTON )) ) {
                    PickupDragItem( game, currItem );
                    GridRemovePlacedItem( curr, currItem );
                } else if ((canPickup) && (IsMouseButtonPressed( MOUSE_RIGHT_BUTTON )) ) {
                    InspectItem( game, ctr, currItem );                
                }

                currItem = currItem->nextInGrid;
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

void ClearGridHighlight( ItemGrid *grid ) 
{
    if (!grid->isSlot) {
        // Clear all highlight flags
        for (int i=0; i < grid->xsize; i++ ) {
            for (int j=0; j < grid->ysize; j++ ) {
                grid->gridflags[i][j] &= ~GridFlags_HIGHLIGHTED;
            }
        }
    }
}


bool TryDropItemOnGrid( Game *game, ItemGrid *grid, Item *item, Vector2 hoverPos, bool performDrop )
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

        // Highlight the cells where it would land
        bool isOverlap = false;
        int x0 = (int)((hoverPos.x - grid->screenRect.x) / ITEM_SQ);
        int y0 = (int)((hoverPos.y - grid->screenRect.y) / ITEM_SQ);

        if (x0 + item->xsize > grid->xsize) {
            isOverlap = true;
        }

        if (y0 + item->ysize > grid->ysize) {
            isOverlap = true;
        }

        if ((x0 < 0) || (y0 < 0)) {
            isOverlap = true;
        }

        for (int i=x0; i < IntMin( grid->xsize, x0 + item->xsize); i++ ) {
            for (int j=y0; j < IntMin( grid->xsize, y0 + item->ysize); j++ ) {

                if ((i < 0) || (j < 0)) {
                    continue;
                }

                if (item->grid[i-x0][j-y0]) {
                   grid->gridflags[i][j] |= GridFlags_HIGHLIGHTED;

                   if (grid->gridflags[i][j] & GridFlags_FULL) {
                        isOverlap = true;
                   }
                }
            }
        }

        if ((!isOverlap) && (performDrop)) {
            item->nextInGrid = grid->placedItems;
            item->placex = x0;
            item->placey = y0;
            grid->placedItems = item;

            for (int i=0; i < item->xsize; i++) {
                for (int j=0; j < item->ysize; j++) {
                    if (item->grid[i][j]) {
                        grid->gridflags[x0+i][y0+j] |= GridFlags_FULL;
                    }
                }
            }

        }

        return !isOverlap;
    }
}

void ConsumeItem( Game *game, Item *item )
{
    // remove from any container
    for (int i=0; i < game->panelStackSize; i++) {
        InventoryContainer *ctr = game->panelStack[i].ctr;
        if (ctr) {
            ContainerRemoveItem( ctr, item );
        }        
    }
    
    // TODO: Add health or stuff here

    DestroyItem( item );

}

void DrawItemPanel( Game *game, int xval, Item *item )
{
    Rectangle rect = { 5, 50, 130, 140 };
    rect.x = xval;
    rect.width = item->xsize * ITEM_SQ + 20;
    GuiGroupBox(rect, item->name);

    float centerx = xval + (rect.width/2);

    float scale = 2.0f;
    float w2 = item->icon.width * scale * 0.5f;
    float h2 = item->icon.height * scale * 0.5f;
    Vector2 itemCorner = Vector2Make( centerx - w2, rect.y + 30 );
    DrawTextureEx( item->icon, itemCorner, 0.0f, scale, WHITE );

    // For potions
    if (GuiButton( RectMake( centerx - 20, 170, 40, 14 ), "Drink")) {
        PopPanel( game );
            
        ConsumeItem( game, item );
    }
}

//------------------------------------------------------------------------------------
// Random item generation
//------------------------------------------------------------------------------------

Item *GenRandomLoot( Game *game )
{
    Item *item = CreateItem();

    strcpy(item->name,"Item");
    item->xsize = GetRandomValue( 1, 5 );
    item->ysize = GetRandomValue( 1, 5 );
    item->lootPos = Vector2Make( GetRandomValue( item->xsize, LOOTBOX_W-item->xsize),
                                 GetRandomValue( 200 + (item->ysize/2), 300-(item->ysize/2) ) );

    // dbg make item shape
    for (int i=0; i < item->xsize; i++) {
        for (int j=0; j < item->ysize; j++) {
            if (GetRandomValue(0,10) < 3) {
                item->grid[i][j] = 0;
            } else {
                item->grid[i][j] = 1;
            }
        }
    }

    // Some items are containers
    if (GetRandomValue(0,4) < 1) {

        for (int i=0; i < item->xsize; i++) {
            for (int j=0; j < item->ysize; j++) {
                item->grid[i][j] = 1;
            }
        }

        InventoryContainer *ctr = CreateInventoryCtr( "Container");
        ctr->width = 110;
        //game.invStack[game.invStackSize++] = backpack;
        ItemGrid *g = AddGrid( ctr, (Vector2){ 10, 10}, 6, 6 );

        item->ctr = ctr;
    }

    MakeItemImage( item );

    return item;
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

    game.panelStack[0] = MakeInventoryPanel( game.invRoot );
    game.panelStackSize = 1;
    
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

    // InventoryContainer *backpack = CreateInventoryCtr( "Backpack");
    // backpack->width = 110;
    // game.invStack[game.invStackSize++] = backpack;

    // g = AddGrid( backpack, (Vector2){ 10, 10}, 6, 6 );

    // InventoryContainer *sachel = CreateInventoryCtr( "Sachel");
    // sachel->width = (ITEM_SQ * 4) + 20;
    // game.invStack[game.invStackSize++] = sachel;
    // g = AddGrid( sachel, (Vector2){ 10, 10}, 4, 4 );


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
            } else if (item->screenRect.x + item->icon.width > LOOTBOX_W-8 ) {
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

            Item *item = GenRandomLoot( &game );

            game.lootItems[game.numLootItems++] = item;
        }

        Vector2 mousePos = GetMousePosition();

        game.dropTargetGrid = NULL;
        for (int i=0; i < game.panelStackSize; i++) {
            Panel *panel = game.panelStack + i;
            InventoryContainer *ctr = panel->ctr;
            if (ctr) {
                ItemGrid *curr = ctr->grids;
                while (curr) {
                    bool canDrop = false;

                    ClearGridHighlight( curr );

                    if ((game.dragItem) && (CheckCollisionPointRec(mousePos, curr->screenRect ))) {
                        // check if it's a valid drop
                        if (TryDropItemOnGrid( &game, curr, game.dragItem, game.dragItemCorner, false )) {
                            canDrop = true;  
                            game.dropTargetGrid = curr;
                        }
                    }
                    curr->highlightForDrop = canDrop;

                    curr = curr->next;
                }
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
            for (int i=0; i < game.panelStackSize; i++) {
                Panel *panel = game.panelStack + i;
                if (panel->ctr) {                    
                    DrawInventoryContainer( &game, currX, panel->ctr );        
                    currX += panel->ctr->width + 5;
                } else if (panel->inspectItem) {
                    DrawItemPanel( &game, currX, panel->inspectItem );
                    currX += panel->inspectItem->xsize * ITEM_SQ + 20;
                }
            }

            // raygui: controls drawing
            //----------------------------------------------------------------------------------            
            game.health = GuiSliderBarEx((Rectangle){ 235, 8, 125, 15 }, game.health, 0, 100, "HP", true);
            game.mana = GuiSliderBarEx((Rectangle){ 235, 28, 125, 15 }, game.mana, 0, 100, "Mana", true);

            GuiGroupBox((Rectangle){ 6, 200, LOOTBOX_W, 90 }, "Loot");

            GuiGroupBox((Rectangle){ LOOTBOX_W + 8, 200, 400 - (LOOTBOX_W+12), 90 }, "Trash");

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
                        
                        PickupDragItem( &game, item );
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
                        if (TryDropItemOnGrid( &game, dropGrid, game.dragItem, game.dragItemCorner, true )) {
                            PickupDragItem( &game, NULL );
                        }
                    } 

                    // If we didn't drop it on the dropTarget, trash?
                    if (game.dragItem) {
                        if ((mousePos.x > LOOTBOX_W) && (mousePos.y > 200)) {
                            
                            // Close all containers so we don't have to deal with tem
                            game.panelStackSize = 1;


                            // Trash item
                            game.dragItem->lootPos = mousePos;
                            game.dragItem->trashCountdown = TRASH_TIME;
                            game.trashItems[game.numTrashItems++] = game.dragItem;
                            PickupDragItem( &game, NULL );
                        }
                    }

                    // Drop it back in the loot pile
                    if (game.dragItem) {
                        // Drop item back in the lootins
                        game.dragItem->lootPos = Vector2Make( mousePos.x, FloatMax( mousePos.y, 200) );
                        game.lootItems[game.numLootItems++] = game.dragItem;
                    }
                    PickupDragItem( &game, NULL );
                }
            }

            // Draw drag item
            if (game.dragItem) {
                
                Color itemColor = Fade( WHITE, 0.5 );
                if ((mousePos.x > LOOTBOX_W) && (mousePos.y > 200)) {
                    itemColor = RED;
                }

                float w2 = game.dragItem->icon.width * 0.9375f;
                float h2 = game.dragItem->icon.height * 0.9375f;
                game.dragItemCorner = Vector2Make( mousePos.x - w2, mousePos.y - h2);
                DrawTextureEx( game.dragItem->icon, game.dragItemCorner, 0.0f, 1.875f, itemColor );
            }

            // Draw trash items
            for (int i=game.numTrashItems-1; i >=0; i--) {

                Item *trashItem = game.trashItems[i];

                float t = trashItem->trashCountdown / TRASH_TIME;

                float w2 = trashItem->icon.width * 0.9375f * t;
                float h2 = trashItem->icon.height * 0.9375f * t;
                Color itemTrashColor = Fade( PINK, t );
                
                DrawTextureEx( trashItem->icon, 
                    Vector2Make( trashItem->lootPos.x - w2, trashItem->lootPos.y - h2), 
                    t * 360.0f, 2.0f * t, itemTrashColor ); 

                trashItem->trashCountdown -= FRAME_DT;
                if (trashItem->trashCountdown < 0.0) {
                    game.trashItems[i] = game.trashItems[--game.numTrashItems];
                    DestroyItem( trashItem );
                }
            }


            //----------------------------------------------------------------------------------
        EndTextureMode();

        DrawTexturePro(screenTarget.texture, 
            (Rectangle){ 0, 0, screenTarget.texture.width, -screenTarget.texture.height }, 
            (Rectangle){ 0, 0, screenTarget.texture.width*2, screenTarget.texture.height*2 }, 
            (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();

        FlushUnloadTexQueue();

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
