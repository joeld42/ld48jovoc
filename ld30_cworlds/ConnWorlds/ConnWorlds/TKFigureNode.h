//
//  TKPlayerFigureNode.h
//  ConnWorlds
//
//  Created by Joel Davis on 8/24/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import <SpriteKit/SpriteKit.h>

typedef enum
{
    FigureType_PLAYER, // "The Wanderer"
    FigureType_GARGOYLE,
    FigureType_GOLEM
    
} FigureType;

@interface TKFigureNode : SKSpriteNode

@property (nonatomic, assign) FigureType figureType;

@property (nonatomic, readonly, assign) BOOL activeFigure;
@property (nonatomic, assign) BOOL zapped;

- (id) initWithType: (FigureType)figureType;

- (void) activate;
- (void) deactivate;

@end
