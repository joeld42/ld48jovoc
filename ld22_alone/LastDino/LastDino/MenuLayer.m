//
//  MenuLayer.m
//  LastDino
//
//  Created by Joel Davis on 12/17/11.
//  Copyright (c) 2011 Joel Davis. All rights reserved.
//

#import "MenuLayer.h"
#import "GameLayer.h"

@interface MenuLayer ()
@property (nonatomic, readwrite, retain) CCSprite *titlebg;
@property (nonatomic, readwrite, retain) CCSprite *title;
@property (nonatomic, readwrite, retain) CCSprite *whiteCard;
@property (nonatomic, readwrite, retain) CCSprite *meteor;
@property (nonatomic, readwrite, retain) NSMutableArray *dinos;

@property (nonatomic, readwrite, retain) CCMenu *menu;

@end;

@implementation MenuLayer

@synthesize title=_title;
@synthesize titlebg=_titlebg;
@synthesize dinos=_dinos;
@synthesize meteor=_meteor;
@synthesize whiteCard=_whiteCard;

@synthesize menu=_menu;

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

        // High Score Label
        int bestDist = [[NSUserDefaults standardUserDefaults] integerForKey: @"bestDistance"];
        NSLog( @"BestDist %d\n", bestDist );
		CCLabelTTF *label = [CCLabelTTF labelWithString: 
                             [NSString stringWithFormat: @"Best Distance: %d", bestDist]
//                                               fontName:@"STHeitiSC-Light" 
                                               fontName: @"Helvetica"
                                               fontSize:18];        
        label.anchorPoint = ccp( 1.0, 1.0 );
        label.position = ccp( 470, 310 );
        [self addChild: label z:4];
        
        
        // Background
        self.titlebg = [CCSprite spriteWithFile: @"title_bg.png"];
        _titlebg.position = ccp( size.width/2, size.height/2 );
        [self addChild: _titlebg];
        
        // Title
        self.title = [CCSprite spriteWithFile: @"Title.png" ];
        _title.position = ccp( size.width/2, 220 );
        [self addChild: _title];

        // Dino friends
        self.dinos = [NSMutableArray arrayWithCapacity: 4];
        
        NSArray *dinos = [NSArray arrayWithObjects: 
                          @"Stegasaur.png", @"Brontosaur.png",
                          @"Triceratop.png", @"Dino.png", nil ];
        for (NSString *dinoName in dinos)
        {
            CCSprite *dino = [CCSprite spriteWithFile: dinoName];
            dino.anchorPoint = ccp( 0.5, 0.0 );


            
            if ([dinoName compare: @"Dino.png"] != NSOrderedSame)
            {
                dino.position = ccp( CCRANDOM_0_1() * 480, 30 );
                [self walkTheDinosaur: dino];
            }
            else
            {
                dino.tag = 42;
                dino.position = ccp( 200, 30 );                
            }
            
            [self addChild: dino];
            
            [_dinos addObject: dino ];
        }
        
        // Meteor
        self.meteor = [CCSprite spriteWithFile: @"meteor1.png"];         
        
        // Card
        self.whiteCard = [CCSprite spriteWithFile: @"allwhite.png"];
        
        // Menu buttons
        CCSprite *startGameSpr = [CCSprite spriteWithFile: @"StartGame.png"];
        CCSprite *startGameSelSpr = [CCSprite spriteWithFile: @"StartGame.png"];
        
        CCMenuItem *start = [CCMenuItemSprite itemFromNormalSprite: startGameSpr
                                                    selectedSprite: startGameSelSpr
                                                            target:self
                                                          selector: @selector(startGame:)];
//		[CCMenuItemFont setFontSize:20];
//        [CCMenuItemFont setFontName:@"Helvetica"];                
//        CCMenuItem *start = [CCMenuItemSprite itemFromString:@"Start Game"
//                                                target:self
//                                              selector:@selector(startGame:)];
//        CCMenuItem *help = [CCMenuItemFont itemFromString:@"Help"
//                                               target:self
//                                             selector:@selector(help:)];
        self.menu = [CCMenu menuWithItems:start,  nil];
        [_menu alignItemsHorizontallyWithPadding: 100 ];
        _menu.position = ccp( _menu.position.x, 130 );
        [self addChild: _menu];
	}
	return self;
}

- (void) walkTheDinosaur: (CCSprite *)dino
{

    float destX = CCRANDOM_0_1() * 480;
    float moveTime = fabs(dino.position.x - destX) * 0.02;
    
    
    id moveAction = [CCMoveTo actionWithDuration: moveTime
                                        position: ccp(destX,30)];
    
    if (dino.position.x < destX)
    {
        dino.flipX = NO; 
    }
    else
    {
        dino.flipX = YES;
    }
    
    id walkMore = [CCCallFuncN actionWithTarget:self selector:@selector(walkTheDinosaur:)];
    
    [dino runAction: [CCSequence actions: moveAction, walkMore, nil] ];

}

- (void) extinctionEvent
{
    [self removeChild: _meteor cleanup: YES ];
    
    for (CCSprite *dino in _dinos)
    {
        if (dino.tag!=42)
        {
            [self removeChild: dino cleanup:YES ];
        }
    }
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
    
    // Fade title
    CCAction *fadeOut = [CCFadeTo actionWithDuration:1.5 opacity:0.0];    
    [_title runAction: fadeOut ];
    
    // Show meteor
    [self addChild: _meteor];
    _meteor.position = ccp( -100.0, 420.0 );
    _meteor.rotation = 230.0;
    id moveAction = [CCMoveTo actionWithDuration: 0.5
                                        position: ccp(300,30)];
    id extinctionEvent = [CCCallFunc actionWithTarget:self selector:@selector(extinctionEvent) ];
    [_meteor runAction: [CCSequence actions: moveAction, extinctionEvent, nil] ];

    // White flash of comet death
    [self addChild:_whiteCard];
    _whiteCard.scale = 20;
    _whiteCard.position=ccp( 240, 160 );
    _whiteCard.opacity = 0.0;
    id delay = [CCDelayTime actionWithDuration:  0.5 ];
    id delay2 = [CCDelayTime actionWithDuration: 1.0 ];
    id startPlaying = [CCCallFunc actionWithTarget:self selector:@selector(doStartGame) ];
    [_whiteCard runAction: [CCSequence actions: delay, 
                            [CCFadeOut actionWithDuration: 1.0],
                            delay2, startPlaying,
                            nil ] ];
    
    // hide menu
    _menu.visible = NO;
    

}

-(void)doStartGame
{    
    NSLog( @"In doStartGame..." );
    [[CCDirector sharedDirector] replaceScene: [GameLayer scene] ];    
}

-(void)help: (id)sender 
{
    NSLog(@"help");
}

@end
