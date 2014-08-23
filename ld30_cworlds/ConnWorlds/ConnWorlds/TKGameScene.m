//
//  TKGameScene.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import "TKGameScene.h"
#import "TKLaserNode.h"

@interface TKGameScene ()
{
    SKSpriteNode *_activeIsland;
    SKSpriteNode *_activeIslandFG;
    
    TKLaserNode *_laserBeam;
    
    SKTexture *_playerTextureActive, *_playerTextureInactive;
    SKSpriteNode *_player;
    
    SKSpriteNode *_grabbedPlayer;
    
    CFTimeInterval _lastUpdateTime;
}
@end


@implementation TKGameScene

- (void) setupWorld
{
    _activeIsland = [SKSpriteNode spriteNodeWithImageNamed:@"island1"];
    _activeIsland.position = CGPointMake( 768/2, 1024/2 );
    
    [self addChild: _activeIsland];
    
    _laserBeam = [[TKLaserNode alloc] init];
    _laserBeam.innerRadius = CGPointMake( 88, 71 );
    [self addChild: _laserBeam];
    
    _playerTextureActive = [SKTexture textureWithImageNamed: @"testplayer_active"];
    _playerTextureInactive = [SKTexture textureWithImageNamed: @"testplayer_inactive"];
    _player = [SKSpriteNode spriteNodeWithTexture: _playerTextureInactive ];
    
    _player.anchorPoint = CGPointMake( 0.5, 0.05 );
    _player.position = CGPointMake( 130, 1024-635 );
    [self addChild: _player];
    
    // Foreground stuff masks player
    _activeIslandFG = [SKSpriteNode spriteNodeWithImageNamed:@"island1_fg"];
    _activeIslandFG.position = CGPointMake( 768/2, 1024/2 );
    [self addChild: _activeIslandFG ];
}

-(void)update:(CFTimeInterval)currentTime
{
    if (_lastUpdateTime==0)
    {
        _lastUpdateTime = currentTime;
        return;
    }
    
    CFTimeInterval dt = currentTime - _lastUpdateTime;
    _lastUpdateTime = currentTime;
    
    [self updateLaser: dt ];
}

- (void) updateLaser: (CFTimeInterval)dt
{
    [_laserBeam update: dt];
}

-(void)mouseDown:(NSEvent *)theEvent
{
    /* Called when a mouse click occurs */
    CGPoint location = [theEvent locationInNode:self];
    CGPoint offs = CGPointMake( (location.x - _player.position.x),
                               (location.y - _player.position.y));
    CGFloat distSqr = offs.x*offs.x + offs.y*offs.y;
    if ((!_grabbedPlayer) && (distSqr < 40*40))
    {
        // Grabbed player, make active
        _grabbedPlayer = _player;
        _player.texture = _playerTextureActive;
    }
}

- (void)mouseDragged: (NSEvent*)theEvent
{
    NSLog( @"Mouse Dragged" );
    CGPoint location = [theEvent locationInNode:self];
    
    if (_grabbedPlayer)
    {
        _player.position = location;
    }
    

}

- (void)mouseMoved: (NSEvent*)theEvent
{
    NSLog( @"Mouse Moved" );
}

- (void)mouseUp: (NSEvent*)theEvent
{
    if (_grabbedPlayer)
    {
        _grabbedPlayer = nil;
        _player.texture = _playerTextureInactive;
    }
    
}

@end
