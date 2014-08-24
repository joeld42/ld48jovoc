//
//  TKWorldIcon.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/24/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import "TKWorldIcon.h"

@implementation TKWorldIcon

- (id) initWithImageNamed: (NSString*)imageName
{
    self = [super initWithImageNamed:imageName];
    if (self)
    {
        // Init...
//        self.color = [SKColor colorWithRed:0.45 green:0.45 blue:0.6 alpha:1.0];
        self.color = [SKColor blackColor];
        self.colorBlendFactor = 1.0;
        
        _activeWorld = NO;
    }
    return self;
}

- (void) setActiveWorld:(BOOL)active
{
    if (active == _activeWorld) return;
    
    if (active)
    {
        SKAction *activateAction = [SKAction colorizeWithColor: [SKColor colorWithRed:0.45 green:0.45 blue:0.6 alpha:1.0]
                                              colorBlendFactor: 0.0
                                                      duration: 2.0 ];
        [self runAction: activateAction completion: ^{
            _activeWorld = YES;
        }];
    }
    else
    {
        SKAction *activateAction = [SKAction colorizeWithColor: [SKColor colorWithRed:0.45 green:0.45 blue:0.6 alpha:1.0]
                                              colorBlendFactor: 1.0
                                                      duration: 2.0 ];
        [self runAction: activateAction completion: ^{
            _activeWorld = NO;
        }];
    }
}

@end
