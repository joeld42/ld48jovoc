//
//  TKLaserNode.h
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import <SpriteKit/SpriteKit.h>

@interface TKLaserNode : SKNode

- (void) update: (CFTimeInterval) dt;
- (id) init;

@property (nonatomic, assign) CGPoint innerRadius;

@property (nonatomic, readonly) CGPoint segmentA;
@property (nonatomic, readonly) CGPoint segmentB;


@end
