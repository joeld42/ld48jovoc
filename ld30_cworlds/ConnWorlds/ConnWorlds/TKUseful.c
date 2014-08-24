//
//  TKUseful.c
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#include <math.h>
#include "TKUseful.h"

CGPoint CGPointAdd(const CGPoint a, const CGPoint b)
{
    return CGPointMake(a.x + b.x, a.y + b.y);
}

CGPoint CGPointSub(const CGPoint a, const CGPoint b)
{
    return CGPointMake(a.x - b.x, a.y - b.y);
}

CGFloat CGPointDistSqr(const CGPoint a, const CGPoint b)
{
    return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y);
}

CGFloat CGPointDist(const CGPoint a, const CGPoint b)
{
    return sqrt( CGPointDistSqr(a,b) );
}

CGFloat CGVectorLenSqr(const CGVector  a )
{
    return a.dx*a.dx + a.dy*a.dy;
}

CGFloat CGVectorLen(const CGVector a )
{
    return sqrt( CGVectorLenSqr(a) );
}



