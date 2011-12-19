//
//  Platform.h
//  LastDino
//
//  Created by Joel Davis on 12/16/11.
//  Copyright (c) 2011 Joel Davis. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "cocos2d.h"

@interface Platform : NSObject

@property (nonatomic, retain) CCSprite *sprite;
@property (nonatomic, assign) BOOL movable;

- (id) initWithSprite: (CCSprite*)spr;

//- (void) dbgDraw;

@end
