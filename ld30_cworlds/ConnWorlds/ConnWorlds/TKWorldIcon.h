//
//  TKWorldIcon.h
//  ConnWorlds
//
//  Created by Joel Davis on 8/24/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import <SpriteKit/SpriteKit.h>

@interface TKWorldIcon : SKSpriteNode

@property (nonatomic, assign) NSInteger hexCoordX;
@property (nonatomic, assign) NSInteger hexCoordY;

@property (nonatomic, assign) BOOL activeWorld;

- (id) initWithImageNamed: (NSString*)imageName;

@end
