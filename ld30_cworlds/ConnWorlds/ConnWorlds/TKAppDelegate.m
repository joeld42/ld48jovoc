//
//  TKAppDelegate.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/22/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import "TKAppDelegate.h"
#import "TKMyScene.h"

@implementation TKAppDelegate

@synthesize window = _window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    /* Pick a size for the scene */
    SKScene *scene = [TKMyScene sceneWithSize:CGSizeMake(1024, 768)];

    /* Set the scale mode to scale to fit the window */
    scene.scaleMode = SKSceneScaleModeAspectFit;

    [self.skView presentScene:scene];

    self.skView.showsFPS = YES;
    self.skView.showsNodeCount = YES;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
