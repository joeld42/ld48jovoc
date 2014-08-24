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


@interface TKGameScene () <SKPhysicsContactDelegate>
{
    SKSpriteNode *_activeIsland;
    SKSpriteNode *_activeIslandFG;
    
    TKLaserNode *_laserBeam;
    
    SKTexture *_playerTextureActive, *_playerTextureInactive;
    SKSpriteNode *_player;
    
    SKSpriteNode *_grabbedPlayer;
    
    CGVector _dragVelocity;
    
    CGPoint _playerLastPos;
    
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
    
    _player.physicsBody = [SKPhysicsBody bodyWithCircleOfRadius: 10 ];
    _player.physicsBody.dynamic = YES;
    _player.physicsBody.categoryBitMask = PHYSGROUP_Player;
    _player.physicsBody.collisionBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
    _player.physicsBody.contactTestBitMask = PHYSGROUP_Player | PHYSGROUP_PWall | PHYSGROUP_Wall;
    
    _player.physicsBody.allowsRotation = NO;
    _player.physicsBody.mass = 1000;
    _player.physicsBody.friction = 0.51;
    
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
    [self addBlocker: blocker1 category: PHYSGROUP_Wall ];
    
    NSMutableArray *blocker2 = [NSMutableArray array];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(63, 567)]];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(276, 575)]];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(278, 544)]];
    [blocker2 addObject: [NSValue valueWithPoint: CGPointMake(65, 534)]];
    [self addBlocker: blocker2 category: PHYSGROUP_Wall ];

    // Add a generic blocker object for the edge
    [self addEdgeBlocker];
    
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

- (void) addEdgeBlocker
{
    NSMutableArray *blockerEdge = [NSMutableArray array];
    
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 40, 657) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 74, 661) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 109, 707) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 147, 741) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 204, 774) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 271, 804) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 282, 835) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 317, 841) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 342, 822) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 385, 818) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 492, 806) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 576, 776) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 601, 761) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 637, 769) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 661, 749) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 652, 721) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 673, 693) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 703, 644) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 716, 607) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 721, 556) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 713, 521) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 740, 502) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 731, 478) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 692, 468) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 651, 407) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 589, 361) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 498, 317) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 484, 295) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 451, 287) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 426, 306) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 375, 302) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 308, 307) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 245, 323) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 186, 345) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 159, 367) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 130, 362) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 103, 380) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 114, 402) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 66, 465) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 47, 537) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 51, 610) ]];
    [blockerEdge addObject: [NSValue valueWithPoint: CGPointMake( 28, 630) ]];

    [self addBlocker: blockerEdge category: PHYSGROUP_PWall ];
}

- (void)addBlocker: (NSArray*)points category: (uint32_t)cat
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
    blockerNode.strokeColor = cat==PHYSGROUP_Wall?[SKColor orangeColor]:[SKColor yellowColor];
    blockerNode.lineWidth = 1.0;
    blockerNode.zPosition = 2.0;
    
#endif

    
//    blockerNode.physicsBody = [SKPhysicsBody bodyWithPolygonFromPath:path ];
    blockerNode.physicsBody = [SKPhysicsBody bodyWithEdgeLoopFromPath:path ];
    blockerNode.physicsBody.dynamic = YES;
    blockerNode.physicsBody.categoryBitMask = cat;
    blockerNode.physicsBody.contactTestBitMask = cat;
    blockerNode.physicsBody.collisionBitMask = cat;
    
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
    
    // drag velocity?
    _player.physicsBody.velocity = CGVectorMake( _dragVelocity.dx / dt,
                                                 _dragVelocity.dy / dt );
    
    [self updateLaser: dt];
}

- (void) updateLaser: (CFTimeInterval)dt
{
    [_laserBeam update: self.physicsWorld deltaTime: dt];
}

- (void)didSimulatePhysics
{
//    NSLog( @"did simulate physics...");
    CGVector zero = CGVectorMake(0.0,0.0);
    _player.physicsBody.velocity = zero;
    _dragVelocity = zero;
}

#pragma mark - Collisions

- (void)didBeginContact:(SKPhysicsContact *)contact
{
    SKPhysicsBody *firstBody, *secondBody;
    
//    NSLog( @"didBeginContact!");
    
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

- (void) dropPlayer
{
    _grabbedPlayer = nil;
    _player.texture = _playerTextureInactive;
    _dragVelocity = CGVectorMake(0.0,0.0);
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
//    NSLog( @"Mouse Dragged" );
    CGPoint location = [theEvent locationInNode:self];
    
    if (_grabbedPlayer)
    {
        _playerLastPos = _player.position;
        CGVector f = CGVectorMake( location.x - _player.position.x,
                                  location.y - _player.position.y );
        
        _dragVelocity = f;
        
        // If we're farther than this, player is probably stuck
        // behind a wall, drop the piece
        CGFloat len = CGVectorLen(f);
        if (len > 60.0)
        {
            printf("Len is %3.2f, dropping\n", len);
            [self dropPlayer];
        }
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
        [self dropPlayer];
    }
    
}

@end
