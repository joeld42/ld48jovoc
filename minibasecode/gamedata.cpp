//
//  ResourceFile.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 5/2/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "gamedata.h"

std::string getResourceDir()
{
#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);
    
    return std::string(path) +"/";
#else
    return std::string("./gamedata/");
#endif
}

std::string gameDataFile( std::string subdir, std::string filename )
{
    // TODO
#ifdef __APPLE__
    // resources don't use subdir.
    return getResourceDir() + "/" + filename;
#else
    return getResourceDir() + "/" + subdir + "/" + filename;
#endif
}
