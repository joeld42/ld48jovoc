//
//  TKPlayerFigureNode.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/24/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#include "TKUseful.h"

#import <SpriteKit/SpriteKit.h>

#import "TKFigureNode.h"

@interface TKFigureNode ()
{
    SKTexture *_texInactive;
    SKTexture *_texActive;
}
@end

@implementation TKFigureNode

- (id) initWithType: (FigureType)figureType
{
    NSString *texActiveName, *texInactiveName;
    switch (figureType) {
        case FigureType_PLAYER:
            texActiveName = @"testplayer_active";
            texInactiveName = @"testplayer_inactive";
            break;
    
        case FigureType_GARGOYLE:
            texActiveName = @"testgarg_active";
            texInactiveName = @"testgarg_inactive";
            break;

        case FigureType_GOLEM:
            texActiveName = @"testgolem_active";
            texInactiveName = @"testgolem_inactive";
            break;
            
    }
    
    SKTexture *texActive = [SKTexture textureWithImageNamed: texActiveName];
    SKTexture *texInactive = [SKTexture textureWithImageNamed: texInactiveName ];
    self = [super initWithTexture: texInactive];
    if (self)
    {
        _figureType = figureType;
        
        _texActive = texActive;
        _texInactive = texInactive;
        
        self.physicsBody = [SKPhysicsBody bodyWithCircleOfRadius: 10 ];
        self.physicsBody.dynamic = YES;
        self.physicsBody.categoryBitMask = PHYSGROUP_Player;
        self.physicsBody.collisionBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
        self.physicsBody.contactTestBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
        
        self.physicsBody.allowsRotation = NO;
        self.physicsBody.mass = 1000;
        self.physicsBody.friction = 0.51;
        
        self.anchorPoint = CGPointMake( 0.5, 0.05 );
    }
    return self;
}

- (void) activate
{
    // TODO: delay, animate
    self.texture = _texActive;
    _activeFigure = YES;
}

- (void) deactivate
{
    self.texture = _texInactive;
    _activeFigure = NO;
    
}


@end
