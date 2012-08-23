#ifndef RESOURCE_FILE_H
#define RESOURCE_FILE_H

//
//  ResourceFile.h
//  ld48jovoc
//
//  Created by Joel Davis on 5/2/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//

#include <string>

std::string getResourceDir();

std::string gameDataFile( std::string subdir, std::string filename );

#endif