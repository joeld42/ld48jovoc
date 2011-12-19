//
//  AppDelegate.h
//  LastDino
//
//  Created by Joel Davis on 12/16/11.
//  Copyright Joel Davis 2011. All rights reserved.
//

#import <UIKit/UIKit.h>

@class RootViewController;

@interface AppDelegate : NSObject <UIApplicationDelegate> {
	UIWindow			*window;
	RootViewController	*viewController;
    
}

@property (nonatomic, retain) UIWindow *window;

@end
