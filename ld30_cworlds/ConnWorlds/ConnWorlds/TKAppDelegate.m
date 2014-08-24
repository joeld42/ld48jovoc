//
//  TKAppDelegate.m
//  ConnWorlds
//
//  Created by Joel Davis on 8/22/14.
//  Copyright (c) 2014 Tapnik. All rights reserved.
//

#import "TKAppDelegate.h"
#import "TKMapScene.h"

@implementation TKAppDelegate

@synthesize window = _window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    /* This makes it send mouse move events */
    _window.acceptsMouseMovedEvents = YES;
    [_window makeFirstResponder:self.skView.scene];
    
    /* Pick a size for the scene */
    SKScene *scene = [TKMapScene sceneWithSize:CGSizeMake(768, 1024)];

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
