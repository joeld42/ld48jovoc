//
//  TKGameScene.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/23/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#include "TKUseful.h"

#import "TKGameScene.h"
#import "TKLaserNode.h"

static const uint32_t PHYSGROUP_Player     =  0x1 << 0;
static const uint32_t PHYSGROUP_Wall       =  0x1 << 0; // blocks player and laser
static const uint32_t PHYSGROUP_PWall      =  0x1 << 0; // only blocks player, not laser

@interface TKGameScene () <SKPhysicsContactDelegate>
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
    
    _player.physicsBody = [SKPhysicsBody bodyWithCircleOfRadius:25 ];
    _player.physicsBody.dynamic = YES;
    _player.physicsBody.categoryBitMask = PHYSGROUP_Player;
    _player.physicsBody.collisionBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
    _player.physicsBody.contactTestBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
    
    _player.anchorPoint = CGPointMake( 0.5, 0.05 );
    _player.position = CGPointMake( 130, 1024-635 );
    
    [self addChild: _player];

    // Add obstacles
    NSMutableArray *blocker1 = [NSMutableArray array];
    [blocker1 addObject: [NSValue valueWithPoint: CGPointMake(607, 632)]];
    [blocker1 addObject: [NSValue valueWithPoint: CGPointMake(628, 625)]];
    [blocker1 addObject: [NSValue valueWithPoint: CGPointMake(629, 610)]];
    [blocker1 addObject: [NSValue valueWithPoint: CGPointMake(526, 469)]];
    [blocker1 addObject: [NSValue valueWithPoint: CGPointMake(505, 475)]];
    [blocker1 addObject: [NSValue valueWithPoint: CGPointMake(503, 493)]];
    [self addBlocker: blocker1];
    
    NSMutableArray *blocker2 = [NSMutableArray array];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(63, 567)]];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(276, 575)]];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(278, 544)]];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(65, 534)]];
    [self addBlocker: blocker2];
//    CGPathMoveToPoint(path, NULL, 63 - offsetX, 567 - offsetY);
//    CGPathAddLineToPoint(path, NULL, 276 - offsetX, 575 - offsetY);
//    CGPathAddLineToPoint(path, NULL, 278 - offsetX, 544 - offsetY);
//    CGPathAddLineToPoint(path, NULL, 65 - offsetX, 534 - offsetY);

    
    // use physics for collision
    self.physicsWorld.gravity = CGVectorMake(0,0);
    self.physicsWorld.contactDelegate = self;
    
    // Foreground stuff masks player
    _activeIslandFG = [SKSpriteNode spriteNodeWithImageNamed:@"island1_fg"];
    _activeIslandFG.position = CGPointMake( 768/2, 1024/2 );
    [self addChild: _activeIslandFG ];
}

- (void)addBlocker: (NSArray*)points
{
    if ([points count]==0) return;
    
    CGPoint centeroid = CGPointZero;
    
    for (NSValue *val in points)
    {
        CGPoint p = [val pointValue];
        centeroid.x += p.x;
        centeroid.y += p.y;
    }
    
    centeroid.x /= [points count];
    centeroid.y /= [points count];
    
    CGFloat offsetX = centeroid.x;
    CGFloat offsetY = centeroid.y;

    NSLog( @"Add Blocker: center is %f %f", centeroid.x, centeroid.y );
    
    CGMutablePathRef path = CGPathCreateMutable();
    
    BOOL first = YES;
    for (NSValue *val in points)
    {
        CGPoint p = [val pointValue];
        if (first)
        {
            first = NO;
            CGPathMoveToPoint( path, NULL, p.x - offsetX, p.y - offsetY );
        }
        else
        {
            CGPathAddLineToPoint(path, NULL, p.x - offsetX, p.y - offsetY );
        }
    }
    CGPathCloseSubpath(path);
    
    
#if 0
    SKNode *blockerNode = [[SKNode alloc] init];
    blockerNode.position = centeroid;
#else
    // Also add a shape node for debugging
    SKShapeNode *blockerNode = [[SKShapeNode alloc] init];
    
    blockerNode.position = centeroid;
    blockerNode.path = path;
    blockerNode.fillColor = [SKColor orangeColor];
    blockerNode.lineWidth = 1.0;
    blockerNode.zPosition = 2.0;
    
#endif

    
//    blockerNode.physicsBody = [SKPhysicsBody bodyWithPolygonFromPath:path ];
    blockerNode.physicsBody = [SKPhysicsBody bodyWithEdgeLoopFromPath:path ];
    blockerNode.physicsBody.dynamic = YES;
    blockerNode.physicsBody.categoryBitMask = PHYSGROUP_Wall;
    blockerNode.physicsBody.contactTestBitMask = PHYSGROUP_Wall;
    blockerNode.physicsBody.collisionBitMask = PHYSGROUP_Wall;
    
    [self addChild: blockerNode];
    
   CGPathRelease( path);
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
    
    [self updateLaser: dt];
}

- (void) updateLaser: (CFTimeInterval)dt
{
    [_laserBeam update: self.physicsWorld deltaTime: dt];
}

#pragma mark - Collisions

- (void)didBeginContact:(SKPhysicsContact *)contact
{
    SKPhysicsBody *firstBody, *secondBody;
    
    NSLog( @"didBeginContact!");
    
    if (contact.bodyA.categoryBitMask < contact.bodyB.categoryBitMask)
    {
        firstBody = contact.bodyA;
        secondBody = contact.bodyB;
    }
    else
    {
        firstBody = contact.bodyB;
        secondBody = contact.bodyA;
    }

    //    if ((firstBody.categoryBitMask & PHYSGROUP_Player) != 0)
//    {
//        [self attack: secondBody.node withMissile:firstBody.node];
//    }
//    ...
}

#pragma mark - Mouse Handling

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
//    NSLog( @"Mouse Dragged" );
    CGPoint location = [theEvent locationInNode:self];
    
    if (_grabbedPlayer)
    {
        _player.position = location;
    }
    

}

- (void)mouseMoved: (NSEvent*)theEvent
{
//    NSLog( @"Mouse Moved" );
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
