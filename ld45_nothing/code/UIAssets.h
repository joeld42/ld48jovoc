
#ifndef UI_ASSETS_H
#define UI_ASSETS_H
//------------------------------------------------------------------------------
//  UI Assets
//------------------------------------------------------------------------------

// NKUI and stb-image for UI stuff
#include "NKUI/NKUI.h"
#include "Core/Types.h"

namespace Tapnik
{
    
struct UIAssets {
    
    // UI metrics
    Oryol::float32 fbWidth;
    Oryol::float32 fbHeight;
    
    // Common UI assets
    struct nk_font *font_14;
	struct nk_font* font_20;
    struct nk_font *font_30;
    
    // large rectangulerish buttons
    struct nk_image img_btn_large_blue;
    struct nk_image img_btn_large_yellow;
    struct nk_image img_btn_large_green;
    struct nk_image img_btn_large_red;
    struct nk_image img_btn_large_grey;

	struct nk_image img_panel_bldg_food;
	struct nk_image img_panel_enemies;

	struct nk_image icon_food;
	struct nk_image icon_food_big;

	struct nk_image icon_explore;
	struct nk_image icon_explore_big;

	struct nk_image icon_startfood;
	struct nk_image icon_startfood_big;

	struct nk_image icon_enemy;
	struct nk_image icon_enemy_big;
    
    void SetupUI();
    
    bool fontValid = false;
    Oryol::Buffer ttfDataTitleFont;
	Oryol::Buffer ttfDataBodyFont;

    void LoadIcon(const char* url, struct nk_image* img);

	void storeDefaultStyle(nk_context* ctx);
	void restoreDefaultStyle(nk_context* ctx);
    
    void buttonStyleNormal( nk_context* ctx );
    void buttonStyleCancel( nk_context* ctx );
    void buttonStyleDisabled( nk_context* ctx );

	struct nk_style defaultStyle;
};
    
} // Namespace Tapnik

#endif
