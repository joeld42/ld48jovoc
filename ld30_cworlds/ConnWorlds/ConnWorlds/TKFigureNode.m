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
    
    CGFloat _activationTime;
    SKEmitterNode *_particles;
}
@end

@implementation TKFigureNode

- (id) initWithType: (FigureType)figureType
{
    NSString *texActiveName, *texInactiveName;
    CGFloat physicsRadius;
    switch (figureType) {
        case FigureType_PLAYER:
            texActiveName = @"testplayer_active";
            texInactiveName = @"testplayer_inactive";
            physicsRadius = 10.0;
            break;
    
        case FigureType_GARGOYLE:
            texActiveName = @"testgarg_active";
            texInactiveName = @"testgarg_inactive";
            physicsRadius = 8.0;
            break;

        case FigureType_GOLEM:
            texActiveName = @"testgolem_active";
            texInactiveName = @"testgolem_inactive";
            physicsRadius = 20.0;
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
        
        self.physicsBody = [SKPhysicsBody bodyWithCircleOfRadius: physicsRadius ];
        self.physicsBody.dynamic = YES;
        self.physicsBody.categoryBitMask = PHYSGROUP_Player;
        self.physicsBody.collisionBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
        self.physicsBody.contactTestBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
        self.physicsBody.allowsRotation = NO;
        self.anchorPoint = CGPointMake( 0.5, 0.05 );
        
        switch (figureType)
        {
            case FigureType_PLAYER:
                self.physicsBody.mass = 1000;
                self.physicsBody.friction = 0.51;
                
                _activationTime = 0.5;
                break;
                
            case FigureType_GARGOYLE:
                self.physicsBody.mass = 500;
                self.physicsBody.friction = 0.2;
                
                _activationTime = 0.25;
                break;
                
            case FigureType_GOLEM:
                self.physicsBody.mass = 5000;
                self.physicsBody.friction = 0.8;
                
                _activationTime = 0.8;
                break;
        }
        
        
        // Add our emitter
        NSString *myParticlePath = [[NSBundle mainBundle] pathForResource:@"figure_die" ofType:@"sks"];
        _particles = [NSKeyedUnarchiver unarchiveObjectWithFile:myParticlePath];
        
        [self addChild: _particles];

    }
    return self;
}

- (void) activate
{
    // Delay, animate
    SKAction *activateAction = [SKAction colorizeWithColor: [SKColor orangeColor]
                                          colorBlendFactor: 0.6
                                                  duration: _activationTime ];
    [self runAction: activateAction completion: ^{
            self.colorBlendFactor = 0.0;
            self.texture = _texActive;
            _activeFigure = YES;
        }];
}

- (void) deactivate
{
    [self removeAllActions];
    
    self.texture = _texInactive;
    self.colorBlendFactor = 0.0;
    _activeFigure = NO;
}

- (void) explode
{
    NSLog( @"Explode....");
    self.respawning = YES;
    
    [_particles resetSimulation];
    _particles.particleBirthRate = 50000.0;
    _particles.numParticlesToEmit = 100.0;

    self.color = [SKColor blackColor];
    self.colorBlendFactor = 1.0;
    self.physicsBody.velocity = CGVectorMake(0.0,0.0);
    
    SKAction *actionStopParty = [SKAction runBlock: ^{
        _particles.particleBirthRate = 0.0;
        
        // Move player to respawn pos
        NSLog( @"Move to respawn pos");
        self.position = self.spawnPos;
        self.physicsBody.velocity = CGVectorMake(0.0,0.0);
        
        self.colorBlendFactor = 0.0;
        self.alpha = 0.0;
    }];
    SKAction *actionRespawn = [SKAction fadeInWithDuration: 2.0];
    
    [self runAction: [SKAction sequence: @[
                      [SKAction waitForDuration: 1.0 ],
                      actionStopParty,
                      [SKAction waitForDuration: 2.0 ],
                      actionRespawn ]]
         completion: ^{
             NSLog( @"Clear respawn flag");
                 self.respawning = NO;
         }];
}

- (void) setSpawnPos: (CGPoint) spawnPos
{
    _spawnPos = spawnPos;
    self.position = spawnPos;
}

@end
