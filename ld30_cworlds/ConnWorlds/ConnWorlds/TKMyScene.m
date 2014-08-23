//
//  TKMyScene.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/22/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import "TKMyScene.h"
#import "TKGameScene.h"

@interface TKMyScene ()
{
    SKLabelNode *_startLabel;
    SKSpriteNode *_startButton;
}
@end


@implementation TKMyScene

-(id)initWithSize:(CGSize)size {    
    if (self = [super initWithSize:size]) {
        /* Setup your scene here */
        
        self.backgroundColor = [SKColor colorWithRed:0.15 green:0.15 blue:0.3 alpha:1.0];
        
        _startLabel = [SKLabelNode labelNodeWithFontNamed:@"Helvetica Neue Light"];

        _startLabel.position = CGPointMake( CGRectGetMidX(self.frame),
                                            CGRectGetMidY(self.frame) - 293);
        
        _startLabel.text = @"Begin";
        _startLabel.fontSize = 65;
        
        [self addChild:_startLabel];
        
 
        _startButton = [SKSpriteNode spriteNodeWithImageNamed:@"button"];
        _startButton.centerRect = CGRectMake( 0.45, 0.45, 0.1, 0.1 );
        _startButton.position = CGPointMake( CGRectGetMidX(self.frame),
                                            CGRectGetMidY(self.frame) - 275);
        _startButton.xScale = 10.0;
        _startButton.yScale = 2.0;

        
        [self addChild:_startButton];
        

    }
    return self;
}

- (void)goToGameScene
{
    SKTransition *reveal = [SKTransition revealWithDirection:SKTransitionDirectionDown duration:1.0];
    TKGameScene *gameScene = [[TKGameScene alloc] initWithSize: CGSizeMake(1024,768)];
    
    //  Optionally, insert code to configure the new scene.
    [self.scene.view presentScene: gameScene transition: reveal];
}

#pragma mark - Events

-(void)mouseDown:(NSEvent *)theEvent
{
     /* Called when a mouse click occurs */
    CGPoint location = [theEvent locationInNode:self];

    if (CGRectContainsPoint( _startButton.frame, location))
    {
        NSLog( @"button hit");
        SKAction *buttonAction = [ SKAction sequence:
                                  @[ [SKAction scaleBy:  1.1 duration: 0.1 ],
                                     [SKAction scaleTo: 1.0 duration: 0.1 ] ] ];
                                  
        [_startLabel runAction:buttonAction completion: ^{
            NSLog( @"Button activate...");
            [self goToGameScene];
        }];
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
