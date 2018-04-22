
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
    struct nk_font *font_20;
    struct nk_font *font_30;
    
    // large rectangulerish buttons
    struct nk_image img_btn_large_blue;
    struct nk_image img_btn_large_yellow;
    struct nk_image img_btn_large_green;
    struct nk_image img_btn_large_red;
    struct nk_image img_btn_large_grey;
    
    struct nk_image img_line_tension;
    struct nk_image img_fish_icon;
    struct nk_image img_tension_high;
    struct nk_image img_tension_low;
    struct nk_image img_fishing_line;
    
    void SetupUI();
    
    bool fontValid = false;
    Oryol::Buffer ttfData;

    void LoadIcon(const char* url, struct nk_image* img);
    
    void buttonStyleNormal( nk_context* ctx );
    void buttonStyleCancel( nk_context* ctx );
    void buttonStyleDisabled( nk_context* ctx );
};
    
} // Namespace Tapnik

#endif
