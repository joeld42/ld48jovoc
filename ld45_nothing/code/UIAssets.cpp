#include "IO/IO.h"

#include "NKUI/NKUI.h"
#include "UIAssets.h"
#include "stb_image.h"

using namespace Oryol;
using namespace Tapnik;

void UIAssets::SetupUI()
{
    
    // setup custom fonts
    IO::Load("gamedata:uiassets/Roboto-Medium.ttf", [this](IO::LoadResult loadResult) {
        printf("Loaded font.\n");
        
        // need to make the data static
        this->ttfData = std::move(loadResult.Data);
        NKUI::BeginFontAtlas();
        this->font_14 = NKUI::AddFont(this->ttfData, 14.0f);
        this->font_30 = NKUI::AddFont(this->ttfData, 30.0f);
        NKUI::EndFontAtlas();
        
        LoadIcon( "gamedata:uiassets/btn_large_blue.png", &img_btn_large_blue );
        LoadIcon( "gamedata:uiassets/btn_large_yellow.png", &img_btn_large_yellow );
        LoadIcon( "gamedata:uiassets/btn_large_green.png", &img_btn_large_green );
        LoadIcon( "gamedata:uiassets/btn_large_red.png", &img_btn_large_red );
        LoadIcon( "gamedata:uiassets/btn_large_grey.png", &img_btn_large_grey );
        
        this->fontValid = true;
    });
}

//------------------------------------------------------------------------------
void UIAssets::buttonStyleNormal( nk_context* ctx ) {
    ctx->style.button.normal = nk_style_item_image( img_btn_large_blue );
    ctx->style.button.hover = nk_style_item_image( img_btn_large_yellow );
    ctx->style.button.text_normal = nk_rgb(255,255,255);
    ctx->style.button.text_hover = nk_rgb(64,28,2);
    ctx->style.button.padding = nk_vec2(0,0);
}

void UIAssets::buttonStyleCancel( nk_context* ctx ) {
    ctx->style.button.normal = nk_style_item_image( img_btn_large_red );
    ctx->style.button.hover = nk_style_item_image( img_btn_large_yellow );
    ctx->style.button.text_normal = nk_rgb(255,255,255);
    ctx->style.button.text_hover = nk_rgb(64,28,2);
    ctx->style.button.padding = nk_vec2(0,0);

}

void UIAssets::buttonStyleDisabled( nk_context* ctx ) {
    ctx->style.button.normal = nk_style_item_image( img_btn_large_grey );
    ctx->style.button.hover = nk_style_item_image( img_btn_large_grey );
    ctx->style.button.text_normal = nk_rgb(128,128,128);
    ctx->style.button.text_hover = nk_rgb(128,128,128);
    ctx->style.button.padding = nk_vec2(0,0);
}

//------------------------------------------------------------------------------
void UIAssets::LoadIcon(const char* url, struct nk_image* img) {
    
    // grab an image handle before data is loaded
    *img = NKUI::AllocImage();
    IO::Load(url, [img,this](IO::LoadResult loadResult) {
        // decode the PNG data via stb_image
        int w = 0, h = 0, n = 0;
        uint8_t* imgData = stbi_load_from_memory(loadResult.Data.Data(), loadResult.Data.Size(), &w, &h, &n, 0);
        
        // create an Oryol texture from the loaded data
        auto texSetup = TextureSetup::FromPixelData2D(w, h, 1, PixelFormat::RGBA8);
        // hmm... no mipmaps will not look good
        texSetup.Sampler.MinFilter = TextureFilterMode::Linear;
        texSetup.Sampler.MagFilter = TextureFilterMode::Linear;
        texSetup.Sampler.WrapU = TextureWrapMode::ClampToEdge;
        texSetup.Sampler.WrapV = TextureWrapMode::ClampToEdge;
        const int imgDataSize = w * h * PixelFormat::ByteSize(PixelFormat::RGBA8);
        texSetup.ImageData.Sizes[0][0] = imgDataSize;
        Id texId = Gfx::CreateResource(texSetup, imgData, imgDataSize);
        stbi_image_free(imgData);
        
        // ...and associate the Oryol texture with the Nuklear image handle
        NKUI::BindImage(*img, texId);
    });
}

