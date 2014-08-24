//
//  TKGameScene.h
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//


#import <SpriteKit/SpriteKit.h>

@interface TKGameScene : SKScene

- (void) setupWorld: (NSInteger)levelNum;

@property (nonatomic, strong) SKScene *parentScene;

@end
