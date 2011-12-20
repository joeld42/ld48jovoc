//
//  GameScene.m
//  LastDino
//
//  Created by Joel Davis on 12/16/11.
//  Copyright (c) 2011 Joel Davis. All rights reserved.
//

#import "GameLayer.h"
#import "MenuLayer.h"
#import "Platform.h"
#import "GameSoundManager.h"

#define PLAYER_HPOS (240)

#define INITIAL_SPEED (200)

#define TAG_COIN (100)

#define TARG_OFFS (150)

@implementation GameLayer

@synthesize bgImage=_bgImage;
@synthesize player=_player;
@synthesize marker=_marker;
@synthesize target=_target;
@synthesize midgroundImage=_midgroundImage;

@synthesize coinsLabel=_coinsLabel;
@synthesize distLabel=_distLabel;

@synthesize animRun=_animRun;
@synthesize animCoins=_animCoins;

+(CCScene *) scene
{
	// 'scene' is an autorelease object.
	CCScene *scene = [CCScene node];
	
	// 'layer' is an autorelease object.
	GameLayer *layer = [GameLayer node];
	
	// add layer as a child to scene
	[scene addChild: layer];
    
    // repeating background
    CCSprite *bg = [CCSprite spriteWithFile:@"background.png" 
                                rect:CGRectMake(0, 0, 512*20, 320)];
    ccTexParams params = {GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT};
    [bg.texture setTexParameters:&params];
    
    [bg setPosition: ccp(0, 160)];
    [scene addChild: bg z:-2];	
    layer.bgImage = bg;

    // repeating midground
    CCSprite *midg = [CCSprite spriteWithFile:@"midground.png" 
                                       rect:CGRectMake(0, 0, 512*20, 320)];
    [midg.texture setTexParameters:&params];
    
    [midg setPosition: ccp(0, 160)];
    [scene addChild: midg z:-1];	
    layer.midgroundImage = midg;
    
    // Create another layer for the HUD
    CCLayer *hudLayer = [CCLayer node];

    // Coins label
    CCLabelBMFont *coinsLbl = [CCLabelBMFont
                                 labelWithString:@"0" 
                                 fntFile:@"font.fnt"];         
    
    coinsLbl.position = ccp( 30, 280 );
    coinsLbl.anchorPoint = ccp( 0.0, 0.0 );
    [hudLayer addChild: coinsLbl z:3 ];
    
    layer.coinsLabel = coinsLbl;
    
    CCSprite *coin = [CCSprite spriteWithFile: @"coin.png"];
    coin.position = ccp( 10, 300);
//    coin.scale = 0.5;
    [hudLayer addChild: coin ];
    
    // Distance label
    CCLabelBMFont *distLbl = [CCLabelBMFont
                               labelWithString:@"0" 
                               fntFile:@"font.fnt"];         
    
    distLbl.position = ccp( 440, 280 );
    distLbl.anchorPoint = ccp( 1.0, 0.0 );
    [hudLayer addChild: distLbl z:3 ];

    layer.distLabel = distLbl;
    
    CCSprite *distIcon = [CCSprite spriteWithFile: @"distance.png"];
    distIcon.position = ccp( 460, 300);
    [hudLayer addChild: distIcon ];

    
    
    // add HUD
    [scene addChild: hudLayer ];
    
    
	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
-(id) init
{
	// always call "super" init
	// Apple recommends to re-assign "self" with the "super" return value
	if( (self=[super init])) {
		
        // enable touches
		self.isTouchEnabled = YES;
        
        _gameState = GameState_PLAYING;
        
        _dragPlatform = nil;

        _coins = 0;
        
		// create and initialize a Label
//		CCLabelTTF *label = [CCLabelTTF labelWithString:@"GameLayer" fontName:@"Marker Felt" fontSize:64];
        
		// ask director the the window size
		CGSize screenSize = [[CCDirector sharedDirector] winSize];
        
		// position the label on the center of the screen
//		label.position =  ccp( size.width /2 , size.height/2 );
		
		// add the label as a child to this Layer
//		[self addChild: label];
        
        // Add the player
//        self.player = [[CCSprite alloc] init];

//        self.player = [CCSprite spriteWithFile: @"Dino.png" ];
//        [_player setPosition: ccp( PLAYER_HPOS, 160 ) ];
//        [_player setAnchorPoint: ccp( 0.5, 0.0 ) ];
//        [self addChild: _player z:1 ];        
//        
        
        _blink = 0.0;
        
        // ================================================
        // Player marker
        self.marker = [CCSprite spriteWithFile: @"dinomarker.png" ];
        [_marker setAnchorPoint: ccp( 0.5, 1.0 ) ];        
        _marker.visible = NO;
        [self addChild: _marker z:3 ];
        
        // ================================================
        // Player animation
        [[CCSpriteFrameCache sharedSpriteFrameCache] addSpriteFramesWithFile:
                    @"dino_sheet.plist"];
        
        CCSpriteBatchNode *spriteSheet = [CCSpriteBatchNode 
                                          batchNodeWithFile:@"dino_sheet.png"];
        [self addChild:spriteSheet];
        
        // Load walk anim
        NSMutableArray *runAnimFrames = [NSMutableArray array];
        for(int i = 1; i <= 17; ++i) {
            [runAnimFrames addObject:
             [[CCSpriteFrameCache sharedSpriteFrameCache] spriteFrameByName:
              [NSString stringWithFormat:@"DinoRun%04d.png", i]]];
        }
        
        CCAnimation *runAnim = [CCAnimation 
                                 animationWithFrames:runAnimFrames delay:0.04f];
        
        self.player = [CCSprite spriteWithSpriteFrameName:@"DinoRun0001.png"];        
        [_player setPosition: ccp( PLAYER_HPOS, 160 ) ];
        [_player setAnchorPoint: ccp( 0.5, 0.0 ) ];
        
        self.animRun = [CCRepeatForever actionWithAction:
                           [CCAnimate actionWithAnimation:runAnim restoreOriginalFrame:NO]];
        [_player runAction:_animRun];
        [spriteSheet addChild: _player z:1 ];
        
        // ================================================
        // Coin animation
        [[CCSpriteFrameCache sharedSpriteFrameCache] addSpriteFramesWithFile:
         @"coin_loop.plist"];
        
        // Load coin anim
        _coinAnimFrames = [NSMutableArray array];
        for(int i = 1; i <= 24; ++i) {
            [_coinAnimFrames addObject:
             [[CCSpriteFrameCache sharedSpriteFrameCache] spriteFrameByName:
              [NSString stringWithFormat:@"Coin%04d.png", i]]];
        }
        
        _coinAnim = [[CCAnimation animationWithFrames:_coinAnimFrames delay:0.04f] retain];
            
        
        // ----------------------------------------------
        // Add the target
        self.target = [CCSprite spriteWithFile: @"target.png" ];
        _target.position = ccp( _player.position.x + TARG_OFFS, _player.position.y );
        [self addChild: _target z:2 ];

        _target.visible = NO;
        
        // follow target
        [self runAction:[CCFollow actionWithTarget:_target 
                        worldBoundary:CGRectMake(0,0,
                                                 1000*screenSize.width,
                                                 screenSize.height)]];

        
        // init scroll speed
        _scrollSpeed = INITIAL_SPEED;
        
        // build the platforms
        _platforms = [[NSMutableArray arrayWithCapacity: 100] retain];        

        // initial build
        _levelExtentX = 0.0;
        _lastLandIndex = Land_LONG;
        _landFirst = TRUE;
        [self buildPlatforms];

        // Start playing background music
        _soundEngine = [GameSoundManager sharedManager].soundEngine;
        
		_soundEngine.backgroundMusicVolume = 0.0f;
		[_soundEngine rewindBackgroundMusic];
		[_soundEngine playBackgroundMusic:@"aloneasaurus.m4a"];
        
        [[GameSoundManager sharedManager] fadeUpMusic];

        
        // Kick off the update loop
        [self schedule: @selector(tick:)];
	}
	return self;
}


-(void) tick: (ccTime) dt
{
    // update background
    _bgImage.position = ccp( self.position.x * 0.1, 160 );
    _midgroundImage.position = ccp( self.position.x * 0.25, 160 );
    
    // Check if collision with the ground
    BOOL isCollide = NO;
    
    CGPoint oldPos = _player.position;
    
//    NSLog( @"tick.. %d plat", [_platforms count] );
    
    // move player in x first
    _player.position = ccp(_player.position.x + (_scrollSpeed * dt), 
                           _player.position.y );
    
    // hit platforms?
    for (Platform *p in _platforms)
    {
        CGRect platBound = [p.sprite boundingBox];
        if (CGRectIntersectsRect( [_player boundingBox],
                                 platBound ))
        {
//            NSLog( @"Collide X...\n" );
            
            // hit something .. back to old pos
            _player.position = oldPos;
            
            // slow down
            _scrollSpeed = INITIAL_SPEED;
            
            break;
        }
    }

    // gravity
    playerYVel = playerYVel + (-1000 * dt );

    // move player in y
    _player.position = ccp(_player.position.x, 
                           _player.position.y + (playerYVel * dt) );

    //dbg -- fly over everything to test scrolling/building
//    _player.position = ccp( _player.position.x, 300 );
    
    for (Platform *p in _platforms)
    {
        CGRect platBound = [p.sprite boundingBox];
        if (CGRectIntersectsRect( [_player boundingBox],
                                  platBound ))
        {
            
            // Are we on the platform or on the edge??
            bool onPlatform = false;
            if ( (_player.position.x >= platBound.origin.x) &&
                 (_player.position.y <= platBound.origin.y + platBound.size.width))
            {
                onPlatform = true;
            }
            
            // how far are we?
            float platY =  platBound.origin.y + platBound.size.height;
            
            // on platform -- adjust to platform height
//            if ((platY - _player.position.y) < 32)
            if ((onPlatform) || ((platY - _player.position.y) < 10) )
            {
                // Move the player up to the contact
//                NSLog( @"Diff %f, snapping up\n", platY - _player.position.y );
                _player.position = ccp( _player.position.x, platY );
                                
                // yes, we hit something
                playerYVel = 0.0;
                isCollide = YES;
            }
            break;
        }            
    }
    

    
    // Hit stuff?
    CGRect playerBound = [_player boundingBox];
    
    
    // Hit obstacles??
    bool dead = FALSE;
    for (Platform *p in _platforms)
    {
        for (CCNode *node in p.sprite.children)
        {
            int tag = node.tag;
            if ((tag >= Obstacle_BUSH) && (tag <= Obstacle_SPIKES))
            {
                CGRect nodeBox = [node boundingBox];            
                CGRect nodeBound = nodeBox;
                
                // add parent (platform) position
                nodeBound.origin = ccpAdd( node.parent.position, nodeBound.origin );
                
                // subtract half a platform worth of height
                nodeBound.origin = ccpSub( nodeBound.origin, ccp( 0, 160) );
                
                if (CGRectIntersectsRect( playerBound, nodeBound ))
                {
                    NSLog( @"Hit obstacle..." );
                    
                    // Ouch! Slow down...
                    _blink = 1.5;
                    _scrollSpeed = INITIAL_SPEED;
                    
                    if (tag == Obstacle_SPIKES)
                    {
                        dead = TRUE;
                    }
                }
            }
        }
    }
    

    // coin get?    
    for (Platform *p in _platforms)
    {
        NSMutableArray *platCoins = p.coins;
        for (CCSprite *coin in platCoins)
        {
            CGRect coinBox = [coin boundingBox];
            
            CGRect coinBound = coinBox;
            
            // direct parent is spritesheet, next parent is platform
            coinBound.origin = ccpAdd( coin.parent.parent.position, coinBound.origin );
            
            // subtract half a platform worth of height
            coinBound.origin = ccpSub( coinBound.origin, ccp( 0, 160) );
            
            if (CGRectIntersectsRect( playerBound, coinBound ))
            {
                [self getCoin: coin];
                
                printf("COIN GET #%d %f %f (coinBox %f %f) (coin %f %f) (parent %f %f)\n", 
                       _coins, 
                       
                       coinBound.origin.x, coinBound.origin.y,
                       coinBox.origin.x, coinBox.origin.y,
                       
                       coin.position.x, coin.position.y, 
                       coin.parent.position.x, coin.parent.position.y                   
                       );
                
                // remove from parent
                [coin removeFromParentAndCleanup:YES ];
            }    
        }
    }
    
//    NSLog( @"Is Collide: %s\n", isCollide?"TRUE":"FALSE" );
    
    
    // move target

    // bring target back to player
    float targDrag = 1.0;
    float targDiff = (_target.position.x - _player.position.x);
    
    if (targDiff > TARG_OFFS)
    {
        targDrag = 1.0f - MIN( targDiff / 260.0, 1.0 );
    }

    _target.position = ccp( _target.position.x + (_scrollSpeed * dt * targDrag),
                           _player.position.y );
    
        
    // update vel
//    float yvel = (_player.position.y - oldPos.y) / dt;
//    yvel = MIN(yvel, 2500.0 );
//    playerYVel = yvel;
    
    // Don't let player fly away
    if ((_player.position.y > 500) && (playerYVel > 0))
    {
        playerYVel = -100.0;
    }
    
//    // jump sound
//    if ((oldVel <= 0.0) && (playerYVel > 10.0))
//    {
//        [_soundEngine playEffect:@"Jump.wav" 
//                           pitch:1.0f
//                             pan:0.0f gain: 1.0];
//        
//    }

    
    // Did the player fall off the screen or do something otherwise
    // fatal???
    float screenX = _player.position.x + self.position.x;
//    printf("Player x %f screenEdge %f\n",
//           _player.position.x,
//           screenX );
    
    // GAME OVERS
    if ((dead) || (_player.position.y < -64) || (screenX < 0) )
    {
        // Stop updates..
        [self unschedule: @selector(tick:)];
        
        // turn down the music
        [[GameSoundManager sharedManager] fadeOutMusic];
        
        [_soundEngine playEffect:@"Fall.wav" 
                           pitch:1.0f
                             pan:0.0f gain: 1.0];

        
        // High score?
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        int bestDist = [userDefaults integerForKey: @"bestDistance"];
        int gameDist = (int)_gameDist;
        if (gameDist > bestDist)
        {
            [userDefaults setInteger: gameDist forKey:@"bestDistance"];            
            [userDefaults synchronize];
        }
        
        // Go back to menu screen 
        [[CCDirector sharedDirector] replaceScene: [MenuLayer scene] ];
    }
    
    // Aaaand... speed up a little
    _scrollSpeed += 30.0 * dt;
    
    
    // Update the player marker
    _marker.position = ccp( _player.position.x, 320 );
    _marker.visible = (_player.position.y > 320);
    
    // Update the distance 
    _gameDist = _player.position.x / 32.0;
    
    _distLabel.string = [NSString stringWithFormat: @"%d", (int)(_gameDist) ];
    
    // update blink
    if (_blink > 0.0)
    {
        _player.visible = !_player.visible;
        _blink -= dt;
        
        if (_blink <= 0.0)
        {
            _blink = 0.0;
            _player.visible = TRUE;
        }
    }    
    
    // Build more stuff
    [self buildPlatforms];
    
    // clean up stuff
    [self cleanupPassedStuff];
}

- (void)ccTouchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSLog( @"Touch began....\n" );
    UITouch *touch = [touches anyObject];
    
    CGPoint locationScreen = [touch locationInView: [touch view]];
    locationScreen = [[CCDirector sharedDirector] convertToGL: locationScreen];
    
    CGPoint location = [self convertToNodeSpace: locationScreen];
 
    NSLog( @"TouchPos is %f %f\n", location.x, location.y );
    
    _dragPlatform = nil;
    for (Platform *p in _platforms)
    {
        if (!p.movable) continue;
        
        if (CGRectContainsPoint( [p.sprite boundingBox], location) )
        {
            _dragPlatform = p;
            _dragStart = location;
            _platformGrabStart = p.sprite.position;
            break;
        }
    }
}

- (void)ccTouchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    
    CGPoint locationScreen = [touch locationInView: [touch view]];
    locationScreen = [[CCDirector sharedDirector] convertToGL: locationScreen];

    CGPoint location = [self convertToNodeSpace: locationScreen];
    
    if (_dragPlatform)
    {
        float dragAmt = location.y - _dragStart.y;
        
        // Get the pos the platform is dragged to
        CGPoint dragPos = ccp( _platformGrabStart.x, 
                              _platformGrabStart.y + dragAmt ); 

        // max drag pos
        float maxPlatformY = 160 - 64; // Player size
        if (dragPos.y > maxPlatformY) 
        {
            dragPos.y = maxPlatformY;
        }
            
        // Apply to platform
        _dragPlatform.sprite.position = dragPos;
        
        // Did we hit the player?
        CGRect platBound = [_dragPlatform.sprite boundingBox];
        if (CGRectIntersectsRect( [_player boundingBox],
                                 platBound ))            
        {
            float dragDist = (platBound.origin.y + platBound.size.height) -
                    _player.position.y;
            
            // Yes.. first just scoot them up to where we are
            _player.position = ccp( _player.position.x, 
                                   platBound.origin.y + platBound.size.height );
            
            // Depending on push speed, trigger a jump
            printf("dragDistance... %f\n", dragDist );
            
            // if we're not already jumping
            if (playerYVel <= 0.001)
            {
                float jumpGain = 0.0;
                if (dragDist < 5.0)
                {
                    playerYVel = 0.0; // no jump, just stick to the ground
                }
                else if (dragDist < 10.0)
                {
                    // small jump
                    playerYVel = 200.0;
                    jumpGain = 0.25;
                }
                else 
                {
                    // big jump
                    playerYVel = 700.0;
                    jumpGain = 1.0;
                }
                
                // play sound
                if (jumpGain > 0.0)
                {
                    printf("Jump %f\n", jumpGain );
                    
                    [_soundEngine playEffect:@"Jump.wav" 
                                       pitch:1.0f
                                         pan:0.0f gain: jumpGain];

                }
            }
            
        }

    }
}

- (void)ccTouchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    
    CGPoint locationScreen = [touch locationInView: [touch view]];
    locationScreen = [[CCDirector sharedDirector] convertToGL: locationScreen];
    
//    CGPoint location = [self convertToNodeSpace: locationScreen];

    _dragPlatform = nil;
    
    // DBG: tap top of screen to redrop dino
//    if (locationScreen.y > 300)
//    {
//        _player.position = ccp( _player.position.x, 300 );
//    }
    
}

//-(void) draw
//{
//    NSLog( @"Platform count %d\n", [_platforms count] );
//    
//    for (Platform *p in _platforms)
//    {
//        [p dbgDraw];
//    }
//}


// on "dealloc" you need to release all your retained objects
- (void) dealloc
{
	// in case you have something to dealloc, do it in this method
	// in this particular example nothing needs to be released.
	// cocos2d will automatically release all the children (Label)
	
	// don't forget to call "super dealloc"
	[super dealloc];
}

#pragma mark - Coins

- (void) getCoin: (CCSprite*)coin
{
    _coins++;
    _coinsLabel.string = [NSString stringWithFormat: @"%d", _coins ];
    
    [_soundEngine playEffect:@"Pickup_Coin.wav" 
                      pitch:1.0f + ( CCRANDOM_MINUS1_1() * 0.01)
                        pan: CCRANDOM_MINUS1_1() 
                        gain: 0.75];

}

#pragma mark - Level Building

- (void) buildPlatforms
{
    
    while (_levelExtentX < _player.position.x + 800)
    {
        int landSpriteIndex = Land_LONG;
        float platHeight;
        
        // preset first platform
        if (_landFirst)
        {
            landSpriteIndex=Land_LONG;
            platHeight = -100;
        }
        else
        {
            
            bool good = false;
            while (!good)
            {
                good = true;
                landSpriteIndex = (int)(CCRANDOM_0_1() * 3);
                
                // don't put to rocks in a row
                if ((landSpriteIndex == Land_ROCK) && (_lastLandIndex==Land_ROCK))
                {
                    good = false;
                }
            }
            
            // MAX: 160 
            platHeight = (CCRANDOM_0_1() * 260) - 150;

        }
        CCSprite *landSprite = [CCSprite spriteWithFile: 
                                [NSString stringWithFormat: @"land%d.png", landSpriteIndex+1]];  
        
        landSprite.anchorPoint = ccp( 0.0, 0.5 );
        
        Platform *p = [[[Platform alloc] initWithSprite: landSprite ] retain];
        _lastLandIndex = landSpriteIndex;
        if (landSpriteIndex==1)
        {
            p.movable = FALSE;
        }
        
        [_platforms addObject: p ];
        
        
        p.sprite.position = ccp( _levelExtentX, platHeight );
        
        // Platform with no gap
        _levelExtentX += [landSprite boundingBox].size.width;
        
        // Add a gap??
        if ((_levelExtentX > 1000) && (CCRANDOM_0_1() < 0.75))
        {
            _levelExtentX += 70 + (CCRANDOM_0_1() * 100);
        }
        
        CGRect platBounds = p.sprite.boundingBox;
        
        // Add some coins??
        if (CCRANDOM_0_1() < 0.25)
        {
            //                CCSprite *coin = [CCSprite spriteWithFile: @"coin.png" ];
            CCSpriteBatchNode *spriteSheet = [CCSpriteBatchNode 
                                              batchNodeWithFile:@"coin_loop.png"];
            spriteSheet.tag = TAG_COIN;
            [p.sprite addChild:spriteSheet];
            

            for (float cx = 15.0; cx < platBounds.size.width; cx += 30)
            {
                CCSprite *coin = [CCSprite spriteWithSpriteFrameName: @"Coin0001.png"];     
                
                CCAction *animCoin = [CCRepeatForever actionWithAction:
                                      [CCAnimate actionWithAnimation:_coinAnim restoreOriginalFrame:NO]];
                
                [coin runAction:animCoin];
                [spriteSheet addChild: coin z:2 ];                
                coin.position = ccp( cx, platBounds.size.height + 20 );            
                coin.tag = TAG_COIN;
                
                [p.coins addObject: coin];
            }
        }
        
        // Add some obstacles?
        else if ( (CCRANDOM_0_1() < 0.3) && (!_landFirst))
        {
            CCSprite *obstacle = nil;
            NSString *obstacleName;
            int obstacleType = (int)(CCRANDOM_0_1() * 4) + Obstacle_BUSH;
            int offs = 10;
            
            // Don't but spikes on the rock platform -- too mean
            if ((landSpriteIndex == Land_ROCK) && 
                (obstacleType == Obstacle_SPIKES) )
            {
                obstacleType = Obstacle_BUSH;
            }
            
            switch (obstacleType)
            {
                case Obstacle_BUSH: obstacleName = @"bush.png"; break;
                case Obstacle_BONES: obstacleName = @"PileOfBones.png"; break;
                case Obstacle_BONES2: obstacleName = @"PileOfBones2.png"; break;
                case Obstacle_SPIKES: obstacleName = @"SpikedMeteor.png"; offs = 20; break;
                default: obstacleName = @"bush.png"; break;
            }
            
            obstacle = [CCSprite spriteWithFile: obstacleName];
            obstacle.anchorPoint = ccp( 0.5, 0.0 );
            obstacle.position = ccp ( (CCRANDOM_0_1() * (platBounds.size.width * 0.8)) 
                                 + (platBounds.size.width * 0.2),
                                     platBounds.size.height - offs );
         
            obstacle.tag = obstacleType;
            
            [p.sprite addChild: obstacle];
        }

        
        [self addChild: p.sprite];
        
        // Not first platform
        _landFirst = FALSE;
    }
    
}

- (void) cleanupPassedStuff
{    
    NSMutableArray *platformsToClean = [NSMutableArray arrayWithCapacity:10 ];
    for (Platform *p in _platforms)
    {
        CGRect platBound = [p.sprite boundingBox];
        if (platBound.origin.x + platBound.size.width < _player.position.x - 480)
        {
            // remove from parent (with childs)
            [p.sprite removeFromParentAndCleanup:YES ];
            p.sprite = nil;
            
            // mark to remove from platforms
            [platformsToClean addObject: p];
        }    
    }
    
    // remove the skipped coins
    [_platforms removeObjectsInArray: platformsToClean];
}

@end
