//
//  load_obj.h
//  ld48jovoc
//
//  Created by Joel Davis on 8/25/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef ld48jovoc_load_obj_h
#define ld48jovoc_load_obj_h

#include <quadbuff.h>
#include <shapes.h>

QuadBuff<DrawVert> *load_obj( const char *filename );

#endif
