//
//  Platform.m
//  LastDino
//
//  Created by Joel Davis on 12/16/11.
//  Copyright (c) 2011 Joel Davis. All rights reserved.
//

#import "Platform.h"

@implementation Platform

@synthesize sprite=_sprite;
@synthesize movable=_movable;

- (id) initWithSprite:(CCSprite *)spr
{
    self = [super init];
    if (self)
    {
        self.sprite = spr;
        self.movable = TRUE;
    }
    
    return self;
}

//- (void) dbgDraw
//{
//    
//    glEnable(GL_LINE_SMOOTH);
//    glColor4ub(255, 0, 255, 255);
//    glLineWidth(2);
//    CGPoint vertices2[] = { 
//        _rect.origin,
//        ccp( _rect.origin.x, _rect.origin.y + _rect.size.height),
//        ccpAdd( _rect.origin, ccp( _rect.size.width, _rect.size.height) ),
//        ccp( _rect.origin.x + _rect.size.width, _rect.origin.y )
//    };
//    
////    CGPoint vertices2[] = { ccp(79,299), ccp(134,299), ccp(134,229), ccp(79,229) };
//    
//    ccDrawPoly(vertices2, 4, YES);
//
//}

@end
