//
//  TKGameScene.h
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//


#import <SpriteKit/SpriteKit.h>

#import "TKWorldIcon.h"

@class TKMapScene;

@interface TKGameScene : SKScene

- (void) setupWorld: (TKWorldIcon*)icon;

@property (nonatomic, strong) TKMapScene *parentScene;

@end
