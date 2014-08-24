//
//  TKUseful.h
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#ifndef ConnWorlds_TKUseful_h
#define ConnWorlds_TKUseful_h

#include <CoreGraphics/CoreGraphics.h>

enum
{
    PHYSGROUP_Player     =  0x1 << 0,
    PHYSGROUP_Wall       =  0x1 << 1, // blocks player and laser
    PHYSGROUP_PWall      =  0x1 << 2, // only blocks player, not laser
};


CGPoint CGPointAdd(const CGPoint a, const CGPoint b);
CGPoint CGPointSub(const CGPoint a, const CGPoint b);

CGFloat CGPointDistSqr(const CGPoint a, const CGPoint b);
CGFloat CGPointDist(const CGPoint a, const CGPoint b);

CGFloat CGVectorLenSqr(const CGVector a );
CGFloat CGVectorLen(const CGVector a );


#endif
