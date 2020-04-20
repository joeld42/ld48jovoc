
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
    struct nk_font *font_30;
    
    // large rectangulerish buttons
    struct nk_image img_btn_large_blue;
    struct nk_image img_btn_large_yellow;
    struct nk_image img_btn_large_green;
    struct nk_image img_btn_large_red;
    struct nk_image img_btn_large_grey;

	// keep alive images
	struct nk_image img_top_graphic;
	struct nk_image img_item_icons;
	struct nk_image img_shrine_icons;
    
    void SetupUI();
    
    bool fontValid = false;
    Oryol::Buffer ttfData;

    void LoadIcon(const char* url, struct nk_image* img);
    
    void buttonStyleNormal( nk_context* ctx );
    void buttonStyleCancel( nk_context* ctx );
    void buttonStyleDisabled( nk_context* ctx );

	void storeDefaultStyle(nk_context* ctx);
	void restoreDefaultStyle(nk_context* ctx);
	struct nk_style defaultStyle;
};
    
} // Namespace Tapnik

#endif
