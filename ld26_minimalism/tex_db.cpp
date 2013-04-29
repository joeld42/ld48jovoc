//
//  tex_db.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/28/13.
//
//

#include <map>
#include <string>
#include <png_loader.h>
#include <gamedata.h>

#include "tex_db.h"

typedef std::map<std::string,PNGImage> TexLookup;

GLint texDBLookup( const std::string &texname )
{
    static TexLookup texDB;
    
    // Check if we already have the object loaded?
    TexLookup::iterator findTexture = texDB.find( texname );
    
    if (findTexture == texDB.end())
    {
        printf("Didn't find texture: %s\n", texname.c_str() );
        PNGImage image = LoadImagePNG( gameDataFile("", texname ).c_str(), true, false );
        
        // store the image
        texDB[texname] = image;
        return image.textureId;
    }
    else
    {
        printf("Found texture: %s\n", texname.c_str());
        return (*findTexture).second.textureId;
    }

    return 0;
    
}
