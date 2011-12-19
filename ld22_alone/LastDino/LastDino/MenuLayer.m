//
//  MenuLayer.m
//  LastDino
//
//  Created by Joel Davis on 12/17/11.
//  Copyright (c) 2011 Joel Davis. All rights reserved.
//

#import "MenuLayer.h"
#import "GameLayer.h"

@implementation MenuLayer

+(CCScene *) scene
{
	// 'scene' is an autorelease object.
	CCScene *scene = [CCScene node];
	
	// 'layer' is an autorelease object.
	MenuLayer *layer = [MenuLayer node];
	
	// add layer as a child to scene
	[scene addChild: layer];
	
	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
-(id) init
{
	// always call "super" init
	// Apple recommends to re-assign "self" with the "super" return value
	if( (self=[super initWithColor:ccc4(4,128,255,255)])) {
		
        // ask director the the window size
		CGSize size = [[CCDirector sharedDirector] winSize];

        
//        // create and initialize a Labele
//		CCLabelTTF *label = [CCLabelTTF labelWithString:@"The Loneliest Dinosaur"
////                                               fontName:@"STHeitiSC-Light" 
//                                               fontName: @"PartyLetPlain"
//                                               fontSize:48];        
//		// position the label on the center of the screen
//		label.position =  ccp( size.width /2 , 250 );
//		// add the label as a child to this Layer
//		[self addChild: label];
        
        // Title
        CCSprite *title = [CCSprite spriteWithFile: @"Title.png" ];
        title.position = ccp( size.width/2, 150 );
        [self addChild: title];

        
        // Menu buttons
		[CCMenuItemFont setFontSize:20];
        [CCMenuItemFont setFontName:@"Helvetica"];
        CCMenuItem *start = [CCMenuItemFont itemFromString:@"Start Game"
                                                target:self
                                              selector:@selector(startGame:)];
        CCMenuItem *help = [CCMenuItemFont itemFromString:@"Help"
                                               target:self
                                             selector:@selector(help:)];
        CCMenu *menu = [CCMenu menuWithItems:start, help, nil];
        [menu alignItemsHorizontallyWithPadding: 100 ];
        menu.position = ccp( menu.position.x, 50 );
        [self addChild:menu];
	}
	return self;
}

// on "dealloc" you need to release all your retained objects
- (void) dealloc
{
	// in case you have something to dealloc, do it in this method
	// in this particular example nothing needs to be released.
	// cocos2d will automatically release all the children (Label)
	
	// don't forget to call "super dealloc"
	[super dealloc];
}

-(void)startGame: (id)sender 
{
    NSLog(@"start game");
    
    [[CCDirector sharedDirector] replaceScene: [GameLayer scene] ];
}

-(void)help: (id)sender 
{
    NSLog(@"help");
}

@end
