//
//  TKMyScene.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/22/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import "TKMapScene.h"
#import "TKGameScene.h"
#import "TKWorldIcon.h"

@interface TKMapScene ()
{
    SKLabelNode *_startLabel;
    SKSpriteNode *_startButton;
    
    NSMutableArray *_worldIcons;
}
@end


@implementation TKMapScene

-(id)initWithSize:(CGSize)size {    
    if (self = [super initWithSize:size]) {
        
        _worldIcons = [NSMutableArray array];
        
        /* Setup your scene here */
        
        self.backgroundColor = [SKColor colorWithRed:0.15 green:0.15 blue:0.3 alpha:1.0];
        
        
        [self setupWorldIcons];
        
        _startLabel = [SKLabelNode labelNodeWithFontNamed:@"Helvetica Neue Light"];

        _startLabel.position = CGPointMake( CGRectGetMidX(self.frame),
                                            CGRectGetMidY(self.frame) + 293);
        
        _startLabel.text = @"World Game";
        _startLabel.fontSize = 65;
        
        [self addChild:_startLabel];
        
 
        _startButton = [SKSpriteNode spriteNodeWithImageNamed:@"button"];
        _startButton.centerRect = CGRectMake( 0.45, 0.45, 0.1, 0.1 );
        _startButton.position = CGPointMake( CGRectGetMidX(self.frame),
                                            CGRectGetMidY(self.frame) + 293+18);
        _startButton.xScale = 10.0;
        _startButton.yScale = 2.0;

        
        [self addChild:_startButton];
        

    }
    return self;
}

- (void)setupWorldIcons
{
    TKWorldIcon *startingWorld = nil;
    
    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 3; j++)
        {
            if ((j==2) && (i!=1)) continue;
            
            TKWorldIcon *icon = [[TKWorldIcon alloc] initWithImageNamed:@"icon_world1"];
            icon.hexCoordX = i;
            icon.hexCoordY = j;
            
            if ((i==1)&&(j==0))
            {
                startingWorld = icon;
            }
            
            icon.position = [self screenPosFromHexCoordX: icon.hexCoordX
                                                       Y:icon.hexCoordY];
            
            icon.zPosition = 3-j;
            
            [self addChild: icon];
            
            // Add a bobbing action
            CGFloat dx = ((rand()/(float)RAND_MAX) * 8.0) - 4.0;
            CGFloat dy = ((rand()/(float)RAND_MAX) * 30.0) - 15.0;
            
            SKAction *bobbleUp = [SKAction moveByX:-dx y: dy duration:2.0 + (rand()/(float)RAND_MAX)*1.5];
            bobbleUp.timingMode = SKActionTimingEaseInEaseOut;

            SKAction *bobbleDown = [SKAction moveByX:dx y:-dy duration:2.0 + (rand()/(float)RAND_MAX)*1.5];
            bobbleDown.timingMode = SKActionTimingEaseInEaseOut;
            
            SKAction *repeatAnimation = [SKAction repeatActionForever:
                                         [SKAction sequence: @[ bobbleUp, bobbleDown ]] ];
            
            [icon runAction: repeatAnimation];
            
            [_worldIcons addObject: icon];
        }
    }
    
    startingWorld.activeWorld = YES;
    
}

- (CGPoint) screenPosFromHexCoordX: (NSInteger)hexCoordX Y:(NSInteger)hexCoordY
{
    const CGFloat SPACING = 236;
    const CGFloat YSPACING = 250;
    CGFloat yOffs = (hexCoordX & 0x01) ? 0:124;
    return CGPointMake( 150 + hexCoordX*SPACING,
                        190 + yOffs + hexCoordY*YSPACING );
}

- (void)goToGameScene: (TKWorldIcon*)icon
{
//    SKTransition *reveal = [SKTransition revealWithDirection:SKTransitionDirectionDown duration:1.0];
//    SKTransition *reveal = [SKTransition crossFadeWithDuration: 1.5 ];
    SKTransition *reveal = [SKTransition fadeWithColor: [SKColor whiteColor] duration: 2.0];
    TKGameScene *gameScene = [[TKGameScene alloc] initWithSize: CGSizeMake(768,1024)];
    gameScene.scaleMode = SKSceneScaleModeAspectFit;
    
    [gameScene  setupWorld: 1];
    
    gameScene.parentScene = self;
    
    //  Optionally, insert code to configure the new scene.
    [self.scene.view presentScene: gameScene transition: reveal];
}

#pragma mark - Events

-(void)mouseDown:(NSEvent *)theEvent
{
     /* Called when a mouse click occurs */
    CGPoint location = [theEvent locationInNode:self];

    // DBG: go back to menu
    
    
    for (TKWorldIcon *icon in _worldIcons)
    {
        if (CGRectContainsPoint( icon.frame, location))
        {
            NSLog( @"world hit");
            if (icon.activeWorld)
            {
                SKAction *worldAction = [ SKAction sequence:
                                          @[ [SKAction scaleBy:  1.1 duration: 0.1 ],
                                             [SKAction scaleTo: 1.0 duration: 0.1 ] ] ];
                                          
                [icon runAction:worldAction completion: ^{
                    NSLog( @"Button activate...");
                    [self goToGameScene: icon ];
                }];
            }
        }
    }

//    SKSpriteNode *sprite = [SKSpriteNode spriteNodeWithImageNamed:@"Spaceship"];
//    
//    sprite.position = location;
//    sprite.scale = 0.5;
//    
//    SKAction *action = [SKAction rotateByAngle:M_PI duration:1];
//    
//    [sprite runAction:[SKAction repeatActionForever:action]];
//    
//    [self addChild:sprite];
}

-(void)update:(CFTimeInterval)currentTime {
    /* Called before each frame is rendered */
}

@end
