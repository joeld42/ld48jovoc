//
//  shader.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/23/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_shader_h
#define ld48jovoc_shader_h

#include "GLee.h"

#ifndef WIN32
#include <stdint.h>
#endif

#define SHADER_FAIL (0)

GLint loadShader( const char *shaderKey );

#endif
