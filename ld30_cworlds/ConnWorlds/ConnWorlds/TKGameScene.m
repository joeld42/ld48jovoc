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
#import "TKFigureNode.h"
#import "TKMapScene.h"

@interface TKPowerPoint : NSObject
@property (nonatomic, assign) CGPoint position;
@property (nonatomic, strong) TKFigureNode *gargoyle;
- (id) initAtPos: (CGPoint)pos;
@end

@implementation TKPowerPoint
- (id) initAtPos: (CGPoint)pos
{
    self = [super init];
    if (self)
    {
        _position = pos;
    }
    return self;
}
@end

@interface TKGameScene () <SKPhysicsContactDelegate>
{
    SKSpriteNode *_activeIsland;
    SKSpriteNode *_activeIslandFG;
    
    TKLaserNode *_laserBeam;
    
    NSMutableArray *_figures; // TKFigureNode
    
    TKFigureNode *_grabbedPlayer;
    
    CGVector _dragVelocity;
    
    CGPoint _playerLastPos;
    
    CGPoint _goalPos;
    NSInteger _goalUnlock;
    BOOL _goalReached;
    
    CFTimeInterval _lastUpdateTime;
    
    NSMutableArray *_powerPoints;
    
    BOOL _portalActive;
}
@end


@implementation TKGameScene

- (void) setupWorld:(TKWorldIcon*)icon
{
    _activeIsland = [SKSpriteNode spriteNodeWithImageNamed:[NSString stringWithFormat:@"island%ld", (long)icon.levelNum] ];
    _activeIsland.position = CGPointMake( 768/2, 1024/2 );
    
    [self addChild: _activeIsland];
    
    _laserBeam = [[TKLaserNode alloc] init];
    _laserBeam.innerRadius = CGPointMake( 88, 71 );
    [self addChild: _laserBeam];

    _figures = [NSMutableArray array];
    _powerPoints = [NSMutableArray array];
    
    _goalUnlock = icon.unlocksLevel;
    
    // Set up level specific stuff
    [self setupWorld1];
    
    // Add a generic blocker object for the world edge
    [self addEdgeBlocker];
    
    // use physics for collision
    self.physicsWorld.gravity = CGVectorMake(0,0);
    self.physicsWorld.contactDelegate = self;
    
    // Foreground stuff masks player
    _activeIslandFG = [SKSpriteNode spriteNodeWithImageNamed:[ NSString stringWithFormat: @"island%ld_fg", (long)icon.levelNum] ];
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

- (void)setupWorld1
{
    // Create a player node
    TKFigureNode *player = [[TKFigureNode alloc] initWithType: FigureType_PLAYER ];
    player.spawnPos = CGPointMake( 461, 1024-708 );
    [self addChild: player];
    [_figures addObject: player];
    
    for (int i=0; i < 4; i++)
    {
        TKFigureNode *garg = [[TKFigureNode alloc] initWithType: FigureType_GARGOYLE ];
        garg.spawnPos = CGPointMake( 275 + (i*80), 1024-300 );
        [self addChild: garg];
        [_figures addObject: garg];
    }
    
//    TKFigureNode *golem = [[TKFigureNode alloc] initWithType: FigureType_GOLEM ];
//    golem.position = CGPointMake( 502, 1024-584 );
//    [self addChild: golem];
//    [_figures addObject: golem];
    
    // Add obstacles
    NSMutableArray *blocker = [NSMutableArray array];
    
    // Shape 1
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 156, 544) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 218, 700) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 242, 694) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 244, 682) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 180, 526) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 156, 532) ]];
    [self addBlocker: blocker category: PHYSGROUP_Wall ];
    
    // Shape 2 (icosothingy)
    [blocker removeAllObjects];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 534, 638) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 574, 666) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 622, 648) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 626, 598) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 588, 568) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 538, 588) ]];
    [self addBlocker: blocker category: PHYSGROUP_Wall ];
    
    // Shape 3 (bottom thinngy)
    [blocker removeAllObjects];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 342, 422) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 554, 420) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 554, 388) ]];
    [blocker addObject: [NSValue valueWithPoint: CGPointMake( 342, 390) ]];
    [self addBlocker: blocker category: PHYSGROUP_Wall ];
    
    // Set up power dots
    [_powerPoints removeAllObjects];
    [_powerPoints addObject: [[TKPowerPoint alloc] initAtPos:CGPointMake(139,1024-637)] ];
    [_powerPoints addObject: [[TKPowerPoint alloc] initAtPos:CGPointMake(57,1024-378)] ];
    [_powerPoints addObject: [[TKPowerPoint alloc] initAtPos:CGPointMake(622,1024-283)] ];
    [_powerPoints addObject: [[TKPowerPoint alloc] initAtPos:CGPointMake(701,1024-528)] ];
   
    _goalPos = CGPointMake( 308, 1024-209);
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
    
    
#if 1
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
    if ((_grabbedPlayer) && (_grabbedPlayer.activeFigure))
    {
        _grabbedPlayer.physicsBody.velocity = CGVectorMake( _dragVelocity.dx / dt,
                                                           _dragVelocity.dy / dt );
    }
    
    // Update laser beam
    [self updateLaser: dt];
    
    // Check if any figures were hit
    for (TKFigureNode *fig in _figures)
    {
        if ((fig.zapped) && (!fig.respawning))
        {
            // Player is always vulnerable, other figures are if they are active
            if ( (fig.figureType == FigureType_PLAYER) || (fig.activeFigure))
            {
                // Uh oh, player was hit... game over
                if (_grabbedPlayer==fig)
                {
                    [self dropPlayer];
                }
                
                // Explode and respawn
                [fig explode];
            }
            fig.zapped = NO;
        }
    }
    
    // Update Power points
    [self updatePowerPoints];
    
    // Update Goals
    [self updateGoal];
}

- (void) updateLaser: (CFTimeInterval)dt
{
    [_laserBeam update: self.physicsWorld deltaTime: dt];
}

- (void) updatePowerPoints
{
    
    // Clear all gargoyles
    for (TKPowerPoint *pp in _powerPoints)
    {
        pp.gargoyle = nil;
    }
    
    for (TKFigureNode *fig in _figures)
    {
        if ( (fig.figureType == FigureType_GARGOYLE) &&
             (!fig.respawning) )
        {
            // Is this gargoyle standing on a power point?
            for (TKPowerPoint *pp in _powerPoints)
            {
                if (CGPointDist( pp.position, fig.position ) < 40.0 )
                {
                    pp.gargoyle = fig;
                }
            }
        }
    }
    
    // Now check if they all found gargoyles
    int gargCount = 0;
    for (TKPowerPoint *pp in _powerPoints)
    {
        if (pp.gargoyle) gargCount++;
    }
    
    _portalActive = (gargCount >= [_powerPoints count]);
    NSLog( @"%d gargoyles, active? %@", gargCount, _portalActive?@"YES":@"NO");
}

- (void) updateGoal
{
    if ((_portalActive) && (!_goalReached))
    {
        for (TKFigureNode *fig in _figures)
        {

            if ( (fig.figureType == FigureType_PLAYER) &&
                 (!fig.respawning))
            {
                // This is a non-zapped player, is it on the goal?
                if (CGPointDist( fig.position, _goalPos) < 30)
                {
                    // HERE unlock
                    _goalReached = YES;
                    [self goBackToParentScene];
                }
            }
        }

    }
}

- (void)didSimulatePhysics
{
//    NSLog( @"did simulate physics...");
    if (_grabbedPlayer)
    {
        CGVector zero = CGVectorMake(0.0,0.0);
        _grabbedPlayer.physicsBody.velocity = zero;
        _dragVelocity = zero;
    }
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


- (void) goBackToParentScene
{
    NSLog( @"In goBackToParentScene!!!");
    
    
    SKTransition *reveal = [SKTransition fadeWithColor: [SKColor whiteColor] duration: 2.0];
    [self.scene.view presentScene: self.parentScene transition: reveal];
}

- (void) willMoveFromView: (SKView *)view
{
    // Done with transition, tell parent scene if we unlocked anything
    NSLog( @"willMoveFromView...");
    if (_goalReached)
    {
        NSLog( @"Asking unlock...");
        self.parentScene.levelToUnlock = _goalUnlock;
    }
}

#pragma mark - Mouse Handling

- (void) dropPlayer
{
    
    [_grabbedPlayer deactivate];
    
    _grabbedPlayer = nil;

    _dragVelocity = CGVectorMake(0.0,0.0);
}

-(void)mouseDown:(NSEvent *)theEvent
{
    /* Called when a mouse click occurs */
    CGPoint location = [theEvent locationInNode:self];
    
    // Back button
    if ((location.x < 44) && (location.y > 1024-44))
    {
        [self goBackToParentScene];
    }
    
    if (_grabbedPlayer) return;
    
    for (TKFigureNode *figure in _figures)
    {
    
        CGPoint offs = CGPointMake( (location.x - figure.position.x),
                                    (location.y - figure.position.y));
        CGFloat distSqr = offs.x*offs.x + offs.y*offs.y;
        if (distSqr < 40*40)
        {
            // Grabbed player, make active
            _grabbedPlayer = figure;
            [_grabbedPlayer activate];
        }
    }
}

- (void)mouseDragged: (NSEvent*)theEvent
{
//    NSLog( @"Mouse Dragged" );
    CGPoint location = [theEvent locationInNode:self];
    
    if (_grabbedPlayer)
    {
        _playerLastPos = _grabbedPlayer.position;
        CGVector f = CGVectorMake( location.x - _grabbedPlayer.position.x,
                                  location.y - _grabbedPlayer.position.y );
        
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
